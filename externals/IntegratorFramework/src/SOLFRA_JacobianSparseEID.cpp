#include "SOLFRA_JacobianSparseEID.h"

#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_Constants.h"

#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include <cvode/cvode.h>
#include <sundials/sundials_timer.h>

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_SparseMatrix.h>
#include <IBK_messages.h>
#include <IBK_FormatString.h>

namespace SOLFRA {


// k - column in index vector (_not_ in original matrix)
// index - Pointer to index vector
// offset - offset to first element in current row in index vector
#define INVALID_INDEX(k,index,offset)  ((k) > 0 && (index)[(offset) + (k)] == (index)[(offset) + (k) - 1])

JacobianSparseEID::JacobianSparseEID(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices, bool symmetric) :
	IBKMK::SparseMatrixEID(n, elementsPerRow, indices),
	m_relToleranceDQ(1e-7),
	m_absToleranceDQ(1e-8),
	m_model(nullptr),
	m_coloringType(Dense),
	m_symmetric(symmetric),
	m_bandWidth(0),
	m_nRhsEvals(0)
{
}


void JacobianSparseEID::init(ModelInterface * model) {
	// initialize all variables needed for Jacobian
	const char * const FUNC_ID = "[JacobianSparseEID::init]";

	m_model = model;

	// check size of model quantities
	if (m_n != m_model->n() )
		throw IBK::Exception(IBK::FormatString("Mismatching dimensions in JacobianSparseEID and model."), FUNC_ID);

	// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
	m_yMod.resize(m_n);
	m_ydotMod.resize(m_n);
	m_ydiff.resize(m_n);
}


void JacobianSparseEID::setColoringType(ColoringType coloringType) {
	const char * const FUNC_ID = "[JacobianSparseEID::setColoringType]";
	m_coloringType = coloringType;
	m_bandWidth = 0;
	m_colors.clear();

	switch (m_coloringType) {
		case Automatic : {
			IBK_ASSERT(m_symmetric);
			IBK::IBK_Message("SparseMatrix: generating color arrays\n",  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

			// generate coloring information

			// vector to hold colors associated with individual columns
			std::vector<unsigned int> colarray(m_n, 0);

			// array to flag used colors
			std::vector<unsigned int> scols(m_n+1); // must have size = m_n+1 since valid color numbers start with 1

			// loop over all columns
			for (unsigned int i=0; i<m_n; ++i) {

//#define DEBUG_OUTPUT_COLORING
#ifdef DEBUG_OUTPUT_COLORING
				std::cout << "column i=" << i << std::endl;
#endif // DEBUG_OUTPUT_COLORING
				// clear vector with neighboring colors
				std::fill(scols.begin(), scols.end(), 0);

				// loop over all rows, that have have entries in this column
				// Note: this currently only works for symmetric matricies
				unsigned int j;
				for (unsigned int jind = 0; jind < m_elementsPerRow; ++jind) {
					j = m_index[i*m_elementsPerRow + jind];
					// stop when invalid element was found
					if (INVALID_INDEX(jind, m_index, i*m_elementsPerRow))
						break;

					// j now holds a valid row number

					// search all columns in this row < column i and add their colors to our "used color set" scol
					unsigned int k;
					for (unsigned int kind = 0; kind < m_elementsPerRow; ++kind) {
						k = m_index[j*m_elementsPerRow + kind];
						// stop when invalid element was found
						if (INVALID_INDEX(kind, m_index, j*m_elementsPerRow))
							break;

						// k now holds column number in row j
						if (k >= i && kind != 0) break; // stop if this column is > our current column i
						// retrieve color of column and mark color as used
						scols[ colarray[k] ] = 1;
#ifdef DEBUG_OUTPUT_COLORING
						if (colarray[k] != 0)
							std::cout << "  marked color = " << colarray[k] << " while processing cell (row,col) " << j << "," << k << std::endl;
#endif // DEBUG_OUTPUT_COLORING
					}
				}
				// search lowest unused color
				unsigned int colIdx = 1;
				for (; colIdx < m_n; ++colIdx)
					if (scols[colIdx] == 0)
						break;
				IBK_ASSERT(colIdx != m_n); /// \todo check this, might fail when dense matrix is being used!!!
				// set this color number in our colarray
				colarray[i] = colIdx;
#ifdef DEBUG_OUTPUT_COLORING
				std::cout << "  column gets color = " << colIdx << std::endl;
#endif // DEBUG_OUTPUT_COLORING
				// store color index in colors array
				if (m_colors.size() < colIdx)
					m_colors.resize(colIdx);
				m_colors[colIdx-1].push_back(i); // associate column number with color
			}

			IBK::IBK_Message(IBK::FormatString("  %1 colors\n").arg((unsigned int) m_colors.size()),
				IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		} break;

		case CurtisPowellReid : {
			unsigned int muMax = 0;
			unsigned int mlMax = 0;
			// compute bandwidth in each row
			for (unsigned int i=0; i<m_n; ++i) {
				unsigned int minIndex = m_index[i*m_elementsPerRow];
				muMax = std::max(i-minIndex, muMax);
				for (unsigned int j=1; j<m_elementsPerRow; ++j) {
					unsigned int index = m_index[i*m_elementsPerRow + j];
					if (index < i) // skip column indices less than row number
						continue;
					// check for invalid index
					if (INVALID_INDEX(j, m_index, i*m_elementsPerRow))
						break; // next row
					mlMax = std::max(index-i, mlMax);
				}
			}
			m_bandWidth = muMax + mlMax + 1;
			IBK::IBK_Message(IBK::FormatString("SparseMatrix: %1 bandwidth\n").arg(m_bandWidth),
				IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		} break;

		case Dense :
			m_coloringType = Dense;
	}
}


int JacobianSparseEID::setup(double t, const double * y, const double * ydot, const double * residuals, double gamma) {
	(void)t;
	(void)residuals;
	(void)gamma;
	IBK_ASSERT(m_model != nullptr);

	// do not update Jacobian matrix if already done
	if (m_linearProblem && m_linearProblemSetupDone)
		return 0;

	// store current solution guess
	std::memcpy(&m_yMod[0], y, m_n*sizeof(double));

	const unsigned int * indexArray = index();
	double * dataArray = data();
	switch (m_coloringType) {
		case Dense : {
			// Use Dense algorithm, column by column
			for (unsigned int j=0; j<m_n; ++j) {

				// modify m_yMod[]
				// modify all y value in row j
				m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
				m_yMod[j] += m_ydiff[j];

				SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION,
					// calculate modified right hand side
					m_model->setY(&m_yMod[0]);
					// calculate modified right hand side of the model, and store f(t,y) in m_FMod
					m_model->ydot(&m_ydotMod[0]);
				);
				// update statistics
				++m_nRhsEvals;

				// restrict jacobian assembly to the filled positions of the band
				if (m_symmetric) {
					// we need to compute at max m_elementsPerRow DQ approximations
					// loop over all column indices in current row -> since matrix
					// is symmetrical (by structure), these column indices are also
					// the row indices i of the elements i,j to compute
					for (unsigned int k = 0; k < m_elementsPerRow; ++k) {
						unsigned int rowStorageIndex = j*m_elementsPerRow + k;
						unsigned int rowIdx = indexArray[rowStorageIndex];
						if (INVALID_INDEX(k, indexArray, j*m_elementsPerRow))
							break; // done with this column
						// compute finite-differences column j in row i
						double val = ( m_ydotMod[rowIdx] - ydot[rowIdx] )/m_ydiff[j];

						// now set the computed derivative in the data storage
						unsigned int colStorageIndex = storageIndex(rowIdx, j);
						dataArray[colStorageIndex] = val;
					}
				}
				else {

					const unsigned int invalid_index = m_n * m_elementsPerRow;
					for (unsigned int k = 0; k < m_n; ++k) {
						// skip invalid matrix index, mind that storageIndex() implements linear search
						unsigned int storageIdx = storageIndex(k,j);
						if (storageIdx == invalid_index)
							continue;
						// compute finite-differences column j in row i
						double val = ( m_ydotMod[k] - ydot[k] )/m_ydiff[j];
						dataArray[storageIdx] = val;
					}

				}
				// restore original y vector at modified location
				m_yMod[j] = y[j];
			} // for j
		} break;

		case CurtisPowellReid : {

			// Use Curtis-Powell-Reid algorithm, modify y in groups
			for (unsigned int i=0; i<m_bandWidth; ++i) {

				// modify m_yMod[] with stride m_bandWidth
				for (unsigned int j=i; j<m_n; j += m_bandWidth) {
					// modify all y value in row j
					m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
					m_yMod[j] += m_ydiff[j];
				}

				SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION,
					// calculate modified right hand side
					m_model->setY(&m_yMod[0]);
					// calculate modified right hand side of the model, and store f(t,y) in m_FMod
					m_model->ydot(&m_ydotMod[0]);
				);
				// update statistics
				++m_nRhsEvals;

				// compute Jacobian elements in groups
				// df/dy = (f(y+eps) - f(y) )/eps
				for (unsigned int j=i; j<m_n; j += m_bandWidth) {
					// we compute now all Jacobian elements in the column j

					// restrict jacobian assembly to the filled positions of the band
					if (m_symmetric) {
						// we need to compute at max m_elementsPerRow DQ approximations
						// loop over all column indices in current row -> since matrix
						// is symmetrical (by structure), these column indices are also
						// the row indices i of the elements i,j to compute
						for (unsigned int k = 0; k < m_elementsPerRow; ++k) {
							unsigned int rowStorageIndex = j*m_elementsPerRow + k;
							unsigned int rowIdx = indexArray[rowStorageIndex];
							if (INVALID_INDEX(k, indexArray, j*m_elementsPerRow))
								break; // done with this column
							// compute finite-differences column j in row i
							double val = ( m_ydotMod[rowIdx] - ydot[rowIdx] )/m_ydiff[j];
							// now set the computed derivative in the data storage
							unsigned int colStorageIndex = storageIndex(rowIdx, j);
							dataArray[colStorageIndex] = val;
						}
					}
					else {
						const unsigned int invalid_index = m_n * m_elementsPerRow;
						for (unsigned int k = 0; k < m_n; ++k) {
							// skip invalid matrix index, mind that storageIndex() implements linear search
							unsigned int storageIdx = storageIndex(k,j);
							if (storageIdx == invalid_index)
								continue;
							// compute finite-differences column j in row i
							double val = ( m_ydotMod[k] - ydot[k] )/m_ydiff[j];
							dataArray[storageIdx] = val;
						}
					}

				} // for j

				// restore original y vector at modified locations
				for (unsigned int j=i; j<m_n; j += m_bandWidth) {
					m_yMod[j] = y[j];
				} // for j

			} // for i
		} break;

		case Automatic : {
			// process all colors individually and modify y in groups
			for (unsigned int i=0; i<m_colors.size(); ++i) {  // i == color index

				// modify m_yMod[] in all columns marked by color i
				for (unsigned int jind=0; jind<m_colors[i].size(); ++jind) {
					unsigned int j = m_colors[i][jind];
					// modify all y value in row j
					m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
					m_yMod[j] += m_ydiff[j];
				}

				SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION,
					// calculate modified right hand side
					m_model->setY(&m_yMod[0]);
					// calculate modified right hand side of the model, and store f(t,y) in m_FMod
					m_model->ydot(&m_ydotMod[0]);
				);
				// update statistics
				++m_nRhsEvals;

				// compute Jacobian elements in groups
				// df/dy = (f(y+eps) - f(y) )/eps
				for (unsigned int jind=0; jind<m_colors[i].size(); ++jind) {
					unsigned int j = m_colors[i][jind];
					// we compute now all Jacobian elements in the column j

					// restrict jacobian assembly to the filled positions of the band
					IBK_ASSERT(m_symmetric);
					// we need to compute at max m_elementsPerRow DQ approximations
					// loop over all column indices in current row -> since matrix
					// is symmetrical (by structure), these column indices are also
					// the row indices i of the elements i,j to compute
					for (unsigned int k = 0; k < m_elementsPerRow; ++k) {
						unsigned int rowStorageIndex = j*m_elementsPerRow + k;
						unsigned int rowIdx = indexArray[rowStorageIndex];
						if (INVALID_INDEX(k, indexArray, j*m_elementsPerRow))
							break; // done with this column
						// compute finite-differences column j in row i
						double val = ( m_ydotMod[rowIdx] - ydot[rowIdx] )/m_ydiff[j];
						// now set the computed derivative in the data storage
						unsigned int colStorageIndex = storageIndex(rowIdx, j);
						dataArray[colStorageIndex] = val;
					} // for k

				} // for jind

				// restore original y vector at modified locations
				for (unsigned int jind=0; jind<m_colors[i].size(); ++jind) {
					unsigned int j = m_colors[i][jind];
					m_yMod[j] = y[j];
				} // for jind

			} // for i
		} break;

	} // switch

	// m_jacobian now holds df/dy

// dump defines are defined in SOLFRA_JacobianInterface.h
#ifdef DUMP_JACOBIAN_TEXT
	std::ofstream jacdump("jacobian_sparse_EID.txt");
	write(jacdump, nullptr, false, 15);
	jacdump.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[JacobianSparseEID::setup]");
#endif
#ifdef DUMP_JACOBIAN_BINARY
	IBK::write_matrix_binary(*this, "jacobian_sparse.bin");
	throw IBK::Exception("Done with test-dump of Jacobian", "[JacobianSparseEID::setup]");
#endif
#ifdef DUMP_JACOBIAN_POSTSCRIPT
	printPostScript( "jacobian_sparse.eps", "Jacobian Sparse", 10.0, 0, true, false);
	throw IBK::Exception("Done with test-post script dump of Jacobian", "[JacobianSparseEID::setup]");
#endif

	if (m_linearProblem)
		m_linearProblemSetupDone = true;

	return 0;
}


IBKMK::SparseMatrix * JacobianSparseEID::createAndReleaseJacobianCopy() const {
	IBKMK::SparseMatrixEID * jacCopy = new IBKMK::SparseMatrixEID(m_n, m_elementsPerRow, constIndex());
	return jacCopy;
}


std::size_t JacobianSparseEID::serializationSize() const {
	// we only need to serialize the actual data and statistics
	return IBKMK::SparseMatrixEID::serializationSize() + sizeof(unsigned int);
}


void JacobianSparseEID::serialize(void* & dataPtr) const {
	IBKMK::SparseMatrixEID::serialize(dataPtr);
	*(unsigned int*)dataPtr = m_nRhsEvals;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}


void JacobianSparseEID::deserialize(void* & dataPtr) {
	const char * const FUNC_ID = "[JacobianSparseEID::deserialize]";
	try {
		IBKMK::SparseMatrixEID::deserialize(dataPtr);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error in Sparse Jacobian EID deserialization.", FUNC_ID);
	}
	m_nRhsEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}


void JacobianSparseEID::recreate(void* & dataPtr) {
	const char * const FUNC_ID = "[JacobianSparseEID::recreate]";
	try {
		IBKMK::SparseMatrixEID::recreate(dataPtr);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error in Sparse Jacobian EID recreation.", FUNC_ID);
	}
	m_nRhsEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}

} // namespace SOLFRA

