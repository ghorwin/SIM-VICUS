#include "SOLFRA_LESADI.h"

#include "SOLFRA_IntegratorInterface.h"
#include "SOLFRA_ModelInterfaceADI.h"
#include "SOLFRA_IntegratorImplicitEuler.h"

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBK_messages.h>

#include <IBKMK_BlockTridiagMatrix.h>
#include <IBKMK_TridiagMatrix.h>


#include <fstream>
#include <cmath>

namespace SOLFRA {

LESADI::LESADI(unsigned int n, unsigned int m) :
	m_modelADI(nullptr),
	m_n(n),
	m_m(m),
	m_jacobianX(nullptr),
	m_jacobianY(nullptr)
{
}

LESADI::~LESADI() {
	delete m_jacobianX;
	delete m_jacobianY;
}

void LESADI::init(SOLFRA::ModelInterface * model, SOLFRA::IntegratorInterface * integrator, SOLFRA::PrecondInterface * precond, JacobianInterface * /*jacobian*/) {

	const char * const FUNC_ID = "[LESADI::init]";
	// initialize all variables needed for Jacobian
	//const char * const FUNC_ID = "[LESADI::init]";

	// retreive pointer to the current integrator and current model
	m_integrator = integrator;

	// retreive pointer to the current model
	m_modelADI = dynamic_cast<ModelInterfaceADI *> (model);

	if(m_modelADI == nullptr)
		throw IBK::Exception("Error initializing ADI linear solver: solver is only "
			"defined for model types derivated from ModelInterfaceADI",FUNC_ID);

	// determine which preconditioner we are using
	if (precond != nullptr)
		IBK::IBK_Message( "LESADI linear solver is chosen: Preconditioner will be ignored.", IBK::MSG_WARNING, FUNC_ID);

	if (SOLFRA::IntegratorImplicitEuler* intEuler = dynamic_cast<SOLFRA::IntegratorImplicitEuler*>(m_integrator)) {
		(void)intEuler;
		// resize jacobian
		m_jacobianX = new IBKMK::BlockTridiagMatrix(m_n, m_m);
		m_jacobianY = new IBKMK::BlockTridiagMatrix(m_n, m_m);
		double * dataJacX = m_jacobianX->submatrix(0, IBKMK::BlockTridiagMatrix::Lower);
		double * dataJacY = m_jacobianX->submatrix(0, IBKMK::BlockTridiagMatrix::Lower);
		for (unsigned int i=0; i<m_n*m_m*m_m*3; ++i) {
			dataJacX[i] = 0;
			dataJacY[i] = 0;
		}

		// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
		m_z.resize(m_n * m_m);
		m_zX.resize(m_n * m_m);
		m_zY.resize(m_n * m_m);
		m_ydotX.resize(m_n * m_m);
		m_ydotY.resize(m_n * m_m);
		m_yMod.resize(m_n * m_m);
		m_ydotMod.resize(m_n * m_m);
		m_ydiff.resize(m_n * m_m);

	}

	else {
		throw IBK::Exception("Error initializing ADI linear solver: solver is only "
			"defined for implicit Euler integrator",FUNC_ID);
	}
}


void LESADI::setup(const double * y, const double * /* ydot */, const double * /* residuals */, double gamma) {

	// loop over all variables
	for(unsigned int j = 0; j < m_n * m_m; ++j) {
		m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
	}

	// copy y values
	std::memcpy(&m_yMod[0], y, m_n * m_m * sizeof(double));
	// update y values inside model
	m_modelADI->setY(y);

	// retrieve model indices
	const unsigned int *indicesX = m_modelADI->indicesX();
	const unsigned int *indicesY = m_modelADI->indicesY();
	// first splitting step
	m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_X);
	m_modelADI->ydot(&m_ydotX[0]);
	// process all balance equations
	for (unsigned int var = 0; var < m_m; ++var) {
		// Use Curtis-Powel-Reid algorithm, modify y in groups
		for (unsigned int i=0; i < 3; ++i) {

			// modify y[] with stride 3
			// loop over all discretisation elements
			for (unsigned int j=i; j<m_n; j += 3) {
				// modify all y value in row j
				unsigned int jIdx	 = indicesX[j] * m_m + var;
				m_yMod[jIdx]		+= m_ydiff[jIdx];
			}
			// calculate modified right hand side
			m_modelADI->setY(&m_yMod[0]);
			// calculate modified right hand side of the model, and store f(t,y) in m_FMod
			m_modelADI->ydot(&m_ydotMod[0]);

			// compute Jacobian elements in groups
			for (unsigned int j=i; j<m_n; j += 3) {
				unsigned int jIdx	 = indicesX[j] * m_m + var;
				// F = y - yn - dt*ydot,
				// derivative: 1 - dt * dydot/dy
				// block position j-1,j
				if ((int)(j) - 1 >= 0) {
					unsigned int k = j - 1;
					double* blockJacobianX = m_jacobianX->submatrix(k,m_jacobianX->bandType(k,j));
					for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
						unsigned int rowIdx	 = eqn;
						unsigned int colIdx	 = var;
						unsigned int kIdx	 = indicesX[k] * m_m + eqn;
						blockJacobianX[rowIdx + colIdx*m_m] = ( m_ydotMod[kIdx] - m_ydotX[kIdx] )/m_ydiff[jIdx];
					}
				}
				// block position j,j
				unsigned int k = j;
				for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
					double* blockJacobianX = m_jacobianX->submatrix(k,m_jacobianX->bandType(k,j));
					unsigned int rowIdx	 = eqn;
					unsigned int colIdx	 = var;
					unsigned int kIdx	 = indicesX[k] * m_m + eqn;
					blockJacobianX[rowIdx + colIdx*m_m] = ( m_ydotMod[kIdx] - m_ydotX[kIdx] )/m_ydiff[jIdx];
				}
				// block position j+1,j
				if (j + 1 < m_n) {
					unsigned int k = j + 1;
					double* blockJacobianX = m_jacobianX->submatrix(k,m_jacobianX->bandType(k,j));
					for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
						unsigned int rowIdx	 = eqn;
						unsigned int colIdx	 = var;
						unsigned int kIdx	 = indicesX[k] * m_m + eqn;
						blockJacobianX[rowIdx + colIdx*m_m] = ( m_ydotMod[kIdx] - m_ydotX[kIdx] )/m_ydiff[jIdx];
					}
				}
			}
			// Jacobian matrix now holds df/dy
			// update solver statistics
			++m_statNumRhsEvals;

			// restore original vector
			for (unsigned int j=i; j<m_n; j += 3) {
				// restore y vector
				unsigned int jIdx	 = indicesX[j] * m_m + var;
				m_yMod[jIdx] = y[jIdx];
			} // for j
		} // for i
	} // for var
	// update solver statistics
	++m_statNumJacEvals;

	// reset y values inside model
	m_modelADI->setY(y);
	// second splitting step
	m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_Y);
	m_modelADI->ydot(&m_ydotY[0]);
	for(unsigned int var = 0; var < m_m; ++var) {
		// Use Curtis-Powel-Reid algorithm, modify y in groups
		for (unsigned int i=0; i < 3; ++i) {

			// modify y[] with stride m
			for (unsigned int j=i; j<m_n; j += 3) {
				unsigned int jIdx  = indicesY[j] * m_m + var;
				// modify all y value in row j
				m_yMod[jIdx]	  += m_ydiff[jIdx];
			}

			// calculate modified right hand side
			m_modelADI->setY(&m_yMod[0]);
			// calculate modified right hand side of the model, and store f(t,y) in m_FMod
			m_modelADI->ydot(&m_ydotMod[0]);

			// compute Jacobian elements in groups
			for (unsigned int j=i; j<m_n; j += 3) {
				unsigned int jIdx	 = indicesY[j] * m_m + var;
				// restrict jacobian assembly to the filled positions of the band
				// F = y - yn - dt*ydot,
				// derivative: 1 - dt * dydot/dy
				// block position j-1,j
				if ((int)(j) - 1 >= 0) {
					unsigned int k = j - 1;
					double* blockJacobianY = m_jacobianY->submatrix(k,m_jacobianY->bandType(k,j));
					for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
						unsigned int rowIdx	 = eqn;
						unsigned int colIdx	 = var;
						unsigned int kIdx	 = indicesY[k] * m_m + eqn;
						blockJacobianY[rowIdx + colIdx*m_m] = ( m_ydotMod[kIdx] - m_ydotY[kIdx] )/m_ydiff[jIdx];
					}
				}
				// block position j,j
				unsigned int k = j;
				for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
					double* blockJacobianY = m_jacobianY->submatrix(k,m_jacobianY->bandType(k,j));
					unsigned int rowIdx	 = eqn;
					unsigned int colIdx	 = var;
					unsigned int kIdx	 = indicesY[k] * m_m + eqn;
					blockJacobianY[rowIdx + colIdx*m_m] = ( m_ydotMod[kIdx] - m_ydotY[kIdx] )/m_ydiff[jIdx];
				}
				// block position j+1,j
				if (j + 1 < m_n) {
					unsigned int k = j + 1;
					double* blockJacobianY = m_jacobianY->submatrix(k,m_jacobianY->bandType(k,j));
					for(unsigned int eqn = 0; eqn < m_m; ++eqn)
					{
						unsigned int rowIdx	 = eqn;
						unsigned int colIdx	 = var;
						unsigned int kIdx	 = indicesY[k] * m_m + eqn;
						blockJacobianY[rowIdx + colIdx*m_m] = ( m_ydotMod[kIdx] - m_ydotY[kIdx] )/m_ydiff[jIdx];
					}
				}
			}
			// Jacobian matrix now holds df/dy
			// update solver statistics
			++m_statNumRhsEvals;

			// restore original vector
			for (unsigned int j=i; j<m_n; j += 3) {
				unsigned int jIdx  = indicesY[j] * m_m + var;
				// restore y vector
				m_yMod[jIdx] = y[jIdx];
			} // for j
		} // for i
	} // for var
	// update solver statistics
	++m_statNumJacEvals;

	// reset active flux directions inside the model
	m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_ALL);

#if 0
	std::ofstream jacdumpX("jacobian_adiX.txt");
	std::ofstream jacdumpY("jacobian_adiY.txt");
	m_jacobianY->write(jacdumpX, nullptr, false, 15);
	m_jacobianY->write(jacdumpY, nullptr, false, 15);
#endif

	// scale with -gamma
	double* dataJacX = m_jacobianX->submatrix(0,IBKMK::BlockTridiagMatrix::Lower);
	double* dataJacY = m_jacobianY->submatrix(0,IBKMK::BlockTridiagMatrix::Lower);
	for (unsigned int i = 0; i < m_m * m_m * m_n * 3; ++i) {
		dataJacX[i] *= -gamma;
		dataJacY[i] *= -gamma;
	}

#if 0
	m_jacobianY->write(jacdumpX, nullptr, false, 15);
	m_jacobianY->write(jacdumpY, nullptr, false, 15);
#endif

	// add identity matrix
	for (unsigned int row = 0; row < m_n; ++row) {
		double *blockJacobianMainX = m_jacobianX->submatrix(row,IBKMK::BlockTridiagMatrix::Main);
		double *blockJacobianMainY = m_jacobianY->submatrix(row,IBKMK::BlockTridiagMatrix::Main);
		// add identity matrix to the main submatrix
		for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
			blockJacobianMainX[eqn*m_m +eqn] += 1.0;
			blockJacobianMainY[eqn*m_m +eqn] += 1.0;
		}
	}

#if 0
	std::ofstream jacdumpX("jacobian_adiX.txt", std::ios_base::out);
	std::ofstream jacdumpY("jacobian_adiY.txt", std::ios_base::out);
	m_jacobianY->write(jacdumpX, nullptr, false, 15);
	m_jacobianY->write(jacdumpY, nullptr, false, 15);
	jacdumpX.close();
	jacdumpY.close();
	//if(t > 7)
	//	throw IBK::Exception("Done with test-dump of Jacobian", "[LESADI::setupODE]");
#endif

	// perform lU-factorization
	m_jacobianX->lu();
	m_jacobianY->lu();
}


void LESADI::solve(double * rhs) {
//	std::memcpy(z, r,  m_n * m_m *sizeof(double));
//	return 0;
	// perform the following splitting:
	// Instead of (I -  gamma_inv ( A1 + A2) ) z =  r
	// solve	(I - gamma_inv * A1) (I - gamma_inv * A2) z = r
	// error estimate:
	// from		(I - gamma_inv * A1) (I - gamma_inv * A2) z =  (I - gamma_inv * (A1 + A2) ) z  + gamma_inv^2 * A1 * A2 * z
	// with      error = gamma_inv^2 * A1 * A2 * z

	// retrieve model indices
	const unsigned int *indicesX = m_modelADI->indicesX();
	const unsigned int *indicesY = m_modelADI->indicesY();

	// copy r -> m_z
	std::memcpy(&m_z[0], rhs,  m_n * m_m * sizeof(double));

	// provide all vectors for preconditioner 2
	for (unsigned int i = 0; i < m_n; ++i) {
		// sort and only store vector elements of indices X
		for(unsigned int eqn = 0; eqn < m_m; ++eqn) {
			m_zX[i * m_m + eqn] = m_z[indicesX[i] * m_m + eqn];
		}
	}
	// solve: (I - gamma_inv A1 ) z = r
	m_jacobianX->backsolve(&m_zX[0]);
	// copy zX into m_z
	for (unsigned int i = 0; i < m_n; ++i){
		for(unsigned int eqn = 0; eqn < m_m; ++eqn) {
			m_z[indicesX[i] * m_m + eqn] = m_zX[i * m_m + eqn];
		}
	}

	for (unsigned int i = 0; i < m_n; ++i) {
		// sort and only store vector elements of indices X
		for(unsigned int eqn = 0; eqn < m_m; ++eqn) {
			m_zY[i * m_m + eqn] = m_z[indicesY[i] * m_m + eqn];
		}
	}
	// solve: (I - gamma_inv A2 ) z = r
	m_jacobianY->backsolve(&m_zY[0]);
	// copy zY into m_z
	for (unsigned int i = 0; i < m_n; ++i) {
		// sort and only store vector elements of indices X
		for(unsigned int eqn = 0; eqn < m_m; ++eqn) {
			m_z[indicesY[i] * m_m + eqn] = m_zY[i * m_m + eqn];
		}
	}

	// copy m_z into rhs
	std::memcpy(rhs, &m_z[0], m_n * m_m * sizeof(double) );
}


} // namespace DELPHIN_MODEL

