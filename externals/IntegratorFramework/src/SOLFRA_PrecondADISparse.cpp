#include "SOLFRA_PrecondADISparse.h"

#include "SOLFRA_IntegratorInterface.h"
#include "SOLFRA_ModelInterfaceADI.h"

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBK_FileUtils.h>
#include <IBKMK_SparseMatrixEID.h>

#include <fstream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace SOLFRA {

PrecondADISparse::PrecondADISparse(unsigned int n, PreconditionerType precondType) :
	PrecondInterface(precondType),
	m_modelADI(nullptr),
	m_n(n),
	m_jacobianX(nullptr),
	m_jacobianY(nullptr),
	m_partialJacobianCopyX(nullptr),
	m_partialJacobianCopyY(nullptr),
	m_bandWidthX(0),
	m_bandWidthY(0),
	m_relToleranceDQ(1e-7),
	m_absToleranceDQ(1e-8),
	m_nJacEvals(0),
	m_nRhsEvals(0)
{
}

PrecondADISparse::~PrecondADISparse() {
	delete m_jacobianX;
	delete m_jacobianY;
	delete m_partialJacobianCopyX;
	delete m_partialJacobianCopyY;
}

void PrecondADISparse::init(SOLFRA::ModelInterface * model, SOLFRA::IntegratorInterface * integrator,
							const JacobianInterface * /*jacobianInterface */ )
{

	const char * const FUNC_ID = "[PrecondADISparse::init]";

	// initialize all variables needed for Jacobian
	//const char * const FUNC_ID = "[PrecondADISparse::init]";

	// retreive pointer to the current integrator and current model
	m_integrator = integrator;
	// retreive pointer to the current model
	m_modelADI = dynamic_cast<ModelInterfaceADI *> (model);

	if(m_modelADI == nullptr)
		throw IBK::Exception("Error initializing ADI linear solver: solver is only "
			"defined for model types derivated from ModelInterfaceADI",FUNC_ID);

	// retreive model size
	m_n = m_modelADI->n();
	// retreive ADI inidices
	unsigned int *indicesADIX = m_modelADI->indicesX();
	unsigned int *indicesADIY = m_modelADI->indicesY();

	// generate sparse indices
	m_indicesX.resize(3 * m_n);
	m_indicesY.resize(3 * m_n);

	// fill indices with -1
	for(unsigned int i = 0; i < 3 * m_n; ++i) {
		m_indicesX[i] = (unsigned int)-1;
		m_indicesY[i] = (unsigned int)-1;
	}

	// fill sparse indices
	for (unsigned int i = 0; i < m_n; ++i) {

		// first diagonal element
		if(i == 0) {
			unsigned int sparseIdxX = 3*indicesADIX[i];
			unsigned int sparseIdxY = 3*indicesADIY[i];
			// fill neighbor elements into sparse matrix
			m_indicesX[sparseIdxX]	   = indicesADIX[i];
			if(indicesADIX[i + 1] > indicesADIX[i])
				m_indicesX[++sparseIdxX] = indicesADIX[i + 1];

			m_indicesY[sparseIdxY]	   = indicesADIY[i];
			if(indicesADIY[i + 1] > indicesADIY[i])
				m_indicesY[++sparseIdxY] = indicesADIY[i + 1];
		}

		// last diagonal element
		else if(i == m_modelADI->n() - 1)
		{
			unsigned int sparseIdxX = 3*indicesADIX[i];
			unsigned int sparseIdxY = 3*indicesADIY[i];
			// fill neighbor elements into sparse matrix
			m_indicesX[sparseIdxX] 	   = indicesADIX[i];
			if(indicesADIX[i - 1] < indicesADIX[i])
				m_indicesX[++sparseIdxX] = indicesADIX[i - 1];

			m_indicesY[sparseIdxY]     = indicesADIY[i];
			if(indicesADIY[i - 1] < indicesADIY[i])
				m_indicesY[++sparseIdxY] = indicesADIY[i - 1];
		}

		else {
			unsigned int sparseIdxX = 3*indicesADIX[i];
			unsigned int sparseIdxY = 3*indicesADIY[i];
			// fill neighbor elements into sparse matrix
			m_indicesX[sparseIdxX]	   = indicesADIX[i];
			if(indicesADIX[i - 1] < indicesADIX[i])
				m_indicesX[++sparseIdxX] = indicesADIX[i - 1];
			if(indicesADIX[i + 1] > indicesADIX[i])
				m_indicesX[++sparseIdxX] = indicesADIX[i + 1];

			m_indicesY[sparseIdxY]	   = indicesADIY[i];
			if(indicesADIY[i - 1] < indicesADIY[i])
				m_indicesY[++sparseIdxY] = indicesADIY[i - 1];
			if(indicesADIY[i + 1] > indicesADIY[i])
				m_indicesY[++sparseIdxY] = indicesADIY[i + 1];
		}
	}
	// resize jacobian
	m_jacobianX = new IBKMK::SparseMatrixEID(m_modelADI->n(),3,&m_indicesX[0]);
	m_jacobianY = new IBKMK::SparseMatrixEID(m_modelADI->n(),3,&m_indicesY[0]);
	m_partialJacobianCopyX = new IBKMK::SparseMatrixEID(m_modelADI->n(),3,&m_indicesX[0]);
	m_partialJacobianCopyY = new IBKMK::SparseMatrixEID(m_modelADI->n(),3,&m_indicesY[0]);

	int maxHalfBandWidth = 0;
	// compute bandwidth in each row: first x-inidces
	for (unsigned int i=0; i<m_n; ++i) {
		for (unsigned int j=1; j<3; ++j) {
			int index = m_indicesX[i*3 + j];
			if (index == -1)
				break; // next row
			int halfbw = std::abs((int)i - index);
			maxHalfBandWidth = std::max(maxHalfBandWidth, halfbw);
		}
	}
	m_bandWidthX = std::min<unsigned int>(m_n-1, 2*maxHalfBandWidth + 1);
	maxHalfBandWidth = 0;
	// than y-indices
	for (unsigned int i=0; i<m_n; ++i) {
		for (unsigned int j=1; j<3; ++j) {
			int index = m_indicesY[i*3 + j];
			if (index == -1)
				break; // next row
			int halfbw = std::abs((int)i - index);
			maxHalfBandWidth = std::max(maxHalfBandWidth, halfbw);
		}
	}
	m_bandWidthY = std::min<unsigned int>(m_n-1, 2*maxHalfBandWidth + 1);

	m_JvX.resize(m_n);
	m_JvY.resize(m_n);

	// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
	m_z.resize(m_n);
	m_zX.resize(m_n);
	m_zY.resize(m_n);
	m_vX.resize(m_n);
	m_vY.resize(m_n);
	m_ydotX.resize(m_n);
	m_ydotY.resize(m_n);
	m_yMod.resize(m_n);
	m_ydotMod.resize(m_n);
	m_ydiff.resize(m_n);
}


int PrecondADISparse::setup(double /* t */, const double * y, const double * /* ydot */, const double * /* residuals */,
	bool jacOk, bool & jacUpdated, double gamma)
{
	if (jacOk) {
		// restore partial Jacobian from backup-copy
		std::memcpy(m_jacobianX->data(),
					m_partialJacobianCopyX->data(),
					sizeof(double)*m_n*3);
		std::memcpy(m_jacobianY->data(),
					m_partialJacobianCopyY->data(),
					sizeof(double)*m_n*3);
		jacUpdated = false;
	}
	else {
		// loop over all variables
		for(unsigned int j = 0; j < m_n; ++j) {
			m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
		}
		// copy y values
		std::memcpy(&m_yMod[0], y, m_n * sizeof(double));
		// update model states and fluxes
		m_modelADI->setY(y);

		// first splitting step
		m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_X);
		m_modelADI->ydot(&m_ydotX[0]);
		// Use Curtis-Powel-Reid algorithm, modify y in groups
		for (unsigned int i=0; i<m_bandWidthX; ++i) {

			// modify y[] with stride m
			for (unsigned int j=i; j< m_n; j += m_bandWidthX) {
				// modify all y value in row j
				m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
				m_yMod[j] += m_ydiff[j];
			}

			// calculate modified right hand side
			m_modelADI->setY(&m_yMod[0]);
			// calculate modified right hand side of the model, and store f(t,y) in m_FMod
			m_modelADI->ydot(&m_ydotMod[0]);
			// update statistics
			++m_nRhsEvals;

			// compute Jacobian elements in groups
			// df/dy = (f(y+eps) - f(y) )/eps
			for (unsigned int j=i; j< m_n; j += m_bandWidthX) {
				// we compute now all Jacobian elements in the column j

				// restrict jacobian assembly to the filled positions of the band
				const unsigned int invalid_index = m_n * 3;
				for (unsigned int k = 0; k < 3; ++k) {
					int rowIdx = m_jacobianX->index()[j*3 + k];
					if (rowIdx == -1) break; // done with this column
					// skip invalid matrix index
					unsigned int storageIndex =  m_jacobianX->storageIndex(rowIdx, j);
					if (storageIndex == invalid_index)
						continue;
					// compute finite-differences column j in row i
					double val = ( m_ydotMod[rowIdx] - m_ydotX[rowIdx] )/m_ydiff[j];
					m_jacobianX->data()[storageIndex] = val;
				}
			}

			// restore original y vector at modified locations
			for (unsigned int j=i; j<m_n; j += m_bandWidthX) {
				m_yMod[j] = y[j];
			} // for j
		} // for i

		// save backup copy of newly computed partial Jacobian
		std::memcpy(m_partialJacobianCopyX->data(),
					m_jacobianX->data(),
					sizeof(double)*m_n*3);
		// update solver statistics
		++m_nJacEvals;

		// reset model states and fluxes
		m_modelADI->setY(y);

		// second splitting step
		m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_Y);
		m_modelADI->ydot(&m_ydotY[0]);
		// Use Curtis-Powel-Reid algorithm, modify y in groups
		for (unsigned int i=0; i<m_bandWidthY; ++i) {

			// modify y[] with stride m
			for (unsigned int j=i; j< m_n; j += m_bandWidthY) {
				// modify all y value in row j
				m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
				m_yMod[j] += m_ydiff[j];
			}

			// calculate modified right hand side
			m_modelADI->setY(&m_yMod[0]);
			// calculate modified right hand side of the model, and store f(t,y) in m_FMod
			m_modelADI->ydot(&m_ydotMod[0]);
			// update statistics
			++m_nRhsEvals;

			// compute Jacobian elements in groups
			// df/dy = (f(y+eps) - f(y) )/eps
			for (unsigned int j=i; j< m_n; j += m_bandWidthY) {
				// we compute now all Jacobian elements in the column j

				// restrict jacobian assembly to the filled positions of the band
				const unsigned int invalid_index = m_n * 3;
				for (unsigned int k = 0; k < 3; ++k) {

					int rowIdx = m_jacobianY->index()[j*3 + k];
					if (rowIdx == -1) break; // done with this column
					// skip invalid matrix index
					unsigned int storageIndex =  m_jacobianY->storageIndex(rowIdx, j);
					if (storageIndex == invalid_index)
						throw IBK::Exception("Invalid storage index in ILU", "[PrecondILU::setupODE]");
					// compute finite-differences column j in row i
					double val = ( m_ydotMod[rowIdx] - m_ydotY[rowIdx] )/m_ydiff[j];
					m_jacobianY->data()[storageIndex] = val;
				}
			}

			// restore original y vector at modified locations
			for (unsigned int j=i; j< m_n; j += m_bandWidthY) {
				m_yMod[j] = y[j];
			} // for j
		} // for i

		// m_jacobian now holds df/dy

		// save backup copy of newly computed partial Jacobian
		std::memcpy(m_partialJacobianCopyY->data(),
					m_jacobianY->data(),
					sizeof(double)*m_n*3);
		jacUpdated = true;
		// update solver statistics
		++m_nJacEvals;
	}

	// reset active flux directions inside the model
	m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_ALL);

	// scale with -gamma
	double * data_ptr = m_jacobianX->data();
	for (unsigned int i=0; i<m_n*3; ++i)
		data_ptr[i] *= -gamma;
	// add identity matrix
	for (unsigned int j=0; j<m_n; ++j)
		data_ptr[j*3] += 1.0;

	// scale with -gamma
	data_ptr = m_jacobianY->data();
	for (unsigned int i=0; i<m_n*3; ++i)
		data_ptr[i] *= -gamma;
	// add identity matrix
	for (unsigned int j=0; j<m_n; ++j)
		data_ptr[j*3] += 1.0;


#if 0
	std::ofstream jacdump("jacobian_ilu.txt");
	m_jacobian->write(jacdump, nullptr, false, 15);
	jacdump.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondILU::setupODE]");
#endif

	// perform incomplete LU-factorization
	m_jacobianX->ilu();
	m_jacobianY->ilu();
	return 0;
}


int PrecondADISparse::solve(double t, const double * y, const double * ydot, const double * residuals,
	const double * r, double * z, double gamma, double delta, int lr)
{
	(void)(t);
	(void)(y);
	(void)(ydot);
	(void)(residuals);
	(void)(gamma);
	(void)(delta);
//	std::memcpy(z, r,  m_n * m_m *sizeof(double));
//	return 0;
	// perform the following splitting:
	// Instead of (I -  gamma_inv ( A1 + A2) ) z =  r
	// solve	(I - gamma_inv * A1) (I - gamma_inv * A2) z = r
	// error estimate:
	// from		(I - gamma_inv * A1) (I - gamma_inv * A2) z =  (I - gamma_inv * (A1 + A2) ) z  + gamma_inv^2 * A1 * A2 * z
	// with      error = gamma_inv^2 * A1 * A2 * z

	bool solveXDirection = false;
	bool solveYDirection = false;

	switch (m_precondType) {
		case Left:	{
			solveXDirection = true;
			solveYDirection = true;
		}
		break;

		case Right:	{
			solveXDirection = true;
			solveYDirection = true;
		}
		break;

		case Both: {
			switch(lr) {
				case (int) SolveLeft: solveXDirection = true;
				break;
				case (int) SolveRight: solveYDirection = true;
				break;
			}
		}
		break;

		case None : ;
		default: ;
	}

	// copy r -> m_z
	std::memcpy(&m_z[0], r,  m_n * sizeof(double));

	if (solveXDirection) {
		// fill z-vector with solution of the linear system
		m_jacobianX->backsolveILU(&m_z[0]);
	}
	if (solveYDirection) {
		// fill z-vector with solution of the linear system
		m_jacobianY->backsolveILU(&m_z[0]);
	}

	// copy m_z into z
	std::memcpy(z, &m_z[0], m_n * sizeof(double) );

#if 0
	std::vector<double> vec(z, z + m_model->n());
	std::ofstream dump("adi_precond.dump", std::ios_base::app);
	dump << std::setw(10) << std::right << t;
	dump << std::setw(10) << std::right << gamma;
	dump << std::setw(10) << std::right << delta;
	dump << std::setw(10) << std::right << lr;
	for (unsigned int i=0; i<vec.size(); ++i) {
		dump << std::setw(10) << std::right << vec[i];
	}
	dump << std::endl;
//	throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondADISparse::solve]");
#endif
	return 0;
}


void PrecondADISparse::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	std::stringstream logfname;
	logfname << "precond_adi";
	switch (m_precondType) {
		case Left  : logfname << "left"; break;
		case Right : logfname << "right"; break;
		case Both  : logfname << "both"; break;
		case None : ;
		default : ;
	}
	logfname << "_stats.tsv";
	if (doRestart) {
		m_statsFileStream = IBK::create_ofstream(logfilePath / logfname.str(), std::ios_base::app);
	}
	else {
		m_statsFileStream = IBK::create_ofstream(logfilePath / logfname.str());
		std::ostream & out = *m_statsFileStream;
		out << std::setw(25) << std::right << "Time\t";
		out << std::setw(17) << std::right << "BTriDiagJacEvals\t";
		out << std::setw(17) << std::right << "BTriDiagRhsEvals";
		out << std::endl;
	}
}


void PrecondADISparse::writeStatistics(double t) {
	// append to output file
	std::ostream & out = *m_statsFileStream;
	// Time after last step was completed
	out << std::fixed << std::setprecision(10) << std::setw(25) << std::right << t << "\t";
	// DlsMatJacEvals
	out << std::fixed << std::setprecision(0) << std::setw(17) << std::right << m_nJacEvals << "\t";
	// NRhsEvals
	out << std::fixed << std::setprecision(0) << std::setw(17) << std::right << m_nRhsEvals;
	out << std::endl;
}



} // namespace SOLFRA

