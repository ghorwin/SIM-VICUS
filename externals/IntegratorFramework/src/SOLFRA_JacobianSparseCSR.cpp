#include "SOLFRA_JacobianSparseCSR.h"

#include "SOLFRA_ModelInterface.h"

#include <fstream>
#include <iomanip>
#include <cmath>

#include <cvode/cvode.h>
#include <sundials/sundials_timer.h>

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_SparseMatrix.h>
#include <IBK_messages.h>
#include <IBK_FormatString.h>

namespace SOLFRA {

JacobianSparseCSR::JacobianSparseCSR(unsigned int n, unsigned int nnz, const unsigned int *ia, const unsigned int * ja,
	const unsigned int *iaT, const unsigned int *jaT) :
	IBKMK::SparseMatrixCSR(n, nnz, ia, ja, iaT, jaT),
	m_relToleranceDQ(1e-7),
	m_absToleranceDQ(1e-8),
	m_model(nullptr),
	m_nRhsEvals(0)
{
}

void JacobianSparseCSR::init(ModelInterface * model) {
	// initialize all variables needed for Jacobian
    FUNCID(JacobianSparseCSR::init);

	m_model = model;

	// check size of model quantities
	if (m_n != m_model->n() )
		throw IBK::Exception(IBK::FormatString("Mismatching dimensions in JacobianSparseCSR and model."), FUNC_ID);

	// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
	m_yMod.resize(m_n);
	m_ydotMod.resize(m_n);
	m_ydiff.resize(m_n);
	m_colors.clear();

	IBK::IBK_Message("SparseMatrix: generating color arrays\n",  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// generate coloring information

	// vector to hold colors associated with individual columns
	std::vector<unsigned int> colarray(m_n, 0);

	// array to flag used colors
	std::vector<unsigned int> scols(m_n+1); // must have size = m_n+1 since valid color numbers start with 1

	const unsigned int * iaIdx  = ia();
	const unsigned int * jaIdx  = ja();
	const unsigned int * iaIdxT = iaT();
	const unsigned int * jaIdxT = jaT();

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
		for (unsigned int jind = iaIdxT[i]; jind < iaIdxT[i + 1]; ++jind) {
			// j always holds a valid row number
			j = jaIdxT[jind];

			// search all columns in this row < column i and add their colors to our "used color set" scol
			unsigned int k;
			for (unsigned int kind = iaIdx[j]; kind < iaIdx[j + 1]; ++kind) {
				k = jaIdx[kind];

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
		//IBK_ASSERT(colIdx != m_n); /// \todo check this, might fail when dense matrix is being used!!!
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
}


int JacobianSparseCSR::setup(double t, const double * y, const double * ydot, const double * residuals, double gamma) {
	(void)t;
	(void)residuals;
	(void)gamma;
	IBK_ASSERT(m_model != nullptr);

	// store current solution guess
	std::memcpy(&m_yMod[0], y, m_n*sizeof(double));

	const unsigned int * iaIdxT = iaT();
	const unsigned int * jaIdxT = jaT();
	double * dataArray = data();

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

			// we need to compute at max m_elementsPerRow DQ approximations
			// loop over all column indices in current row -> since matrix
			// is symmetrical (by structure), these column indices are also
			// the row indices i of the elements i,j to compute
			for (unsigned int k = iaIdxT[j]; k < iaIdxT[j + 1]; ++k) {
				unsigned int rowIdx = jaIdxT[k];
				// compute finite-differences column j in row i
				double val = ( m_ydotMod[rowIdx] - ydot[rowIdx] )/m_ydiff[j];
				// now set the computed derivative in the data storage
				unsigned int colStorageIndex = storageIndex(rowIdx,j);
				dataArray[colStorageIndex] = val;
			} // for k

		} // for jind

		// restore original y vector at modified locations
		for (unsigned int jind=0; jind<m_colors[i].size(); ++jind) {
			unsigned int j = m_colors[i][jind];
			m_yMod[j] = y[j];
			// special case: dense pattern
			m_ydiff[j] = 0;
		} // for jind

	} // for i

	// m_jacobian now holds df/dy

// dump defines are defined in SOLFRA_JacobianInterface.h
#ifdef DUMP_JACOBIAN_TEXT
	std::ofstream jacdump("jacobian_sparse_CSR.txt");
	write(jacdump, nullptr, false, 15);
	jacdump.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[JacobianSparseCSR::setup]");
#endif
#ifdef DUMP_JACOBIAN_BINARY
	std::ofstream jacdump("jacobian_sparse.bin", std::ios_base::binary);
	char* jacData = new char[serializationSize() + sizeof(size_t) + 4 * sizeof(unsigned int)];
	void * jacDataPtr = jacData + sizeof(size_t) + 4 * sizeof(unsigned int);
	// serialize all matrix data
	serialize(jacDataPtr);
	// store data size and matrix dimensions
	unsigned int offset = 0;
	*(size_t*)(jacData) = serializationSize();
	offset += sizeof(size_t);
	*(unsigned int*)(jacData + offset) = m_n;
	offset += sizeof(unsigned int);
	*(unsigned int*)(jacData + offset) = m_nnz;
	offset += sizeof(unsigned int);
	*(unsigned int*)(jacData + offset) = m_iaT.size();
	offset += sizeof(unsigned int);
	*(unsigned int*)(jacData + offset) = m_jaT.size();
	// write data
	jacdump.write(jacData, serializationSize() + sizeof(size_t) + 4 * sizeof(unsigned int) );
	jacdump.close();
	delete[] jacData;
	throw IBK::Exception("Done with test-dump of Jacobian", "[JacobianSparseCSR::setup]");
#endif
#ifdef DUMP_JACOBIAN_POSTSCRIPT
	printPostScript( "jacobian_sparse.eps", "Jacobian Sparse", 10.0, 0, true, false);
	throw IBK::Exception("Done with test-post script dump of Jacobian", "[JacobianSparseCSR::setup]");
#endif

	return 0;
}


IBKMK::SparseMatrix * JacobianSparseCSR::createAndReleaseJacobianCopy() const {
	IBKMK::SparseMatrixCSR * jacCopy = new IBKMK::SparseMatrixCSR(m_n, m_nnz, constIa(), constJa());
	return jacCopy;
}


std::size_t JacobianSparseCSR::serializationSize() const {
	// we only need to serialize the actual data and statistics
	return IBKMK::SparseMatrixCSR::serializationSize() + sizeof(unsigned int);
}


void JacobianSparseCSR::serialize(void* & dataPtr) const {
	IBKMK::SparseMatrixCSR::serialize(dataPtr);
	*(unsigned int*)dataPtr = m_nRhsEvals;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}


void JacobianSparseCSR::deserialize(void* & dataPtr) {
    FUNCID(JacobianSparseCSR::deserialize);
	try {
		IBKMK::SparseMatrixCSR::deserialize(dataPtr);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error in Sparse Jacobian CSR deserialization.", FUNC_ID);
	}
	m_nRhsEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}


void JacobianSparseCSR::recreate(void* & dataPtr) {
    FUNCID(JacobianSparseCSR::recreate);
	try {
		IBKMK::SparseMatrixCSR::recreate(dataPtr);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error in Sparse Jacobian CSR recreation.", FUNC_ID);
	}
	m_nRhsEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}

} // namespace SOLFRA

