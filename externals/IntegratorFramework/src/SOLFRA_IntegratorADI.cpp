/*	Solver Control Framework
	Copyright (C) 2010  Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SOLFRA_IntegratorADI.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_FileUtils.h>

#include <IBKMK_BlockTridiagMatrix.h>
#include <IBKMK_TridiagMatrix.h>
#include <IBKMK.h>

#include "SOLFRA_LESInterfaceDirect.h"
#include "SOLFRA_PrecondInterface.h"
#include "SOLFRA_JacobianInterface.h"

#include <sundials/sundials_timer.h>


using namespace std;

//#define DUMP_ADI_JACOBIANX

namespace SOLFRA {

inline double * DOUBLE_PTR(std::vector<double> & vec) { return &vec[0]; }
inline const double * DOUBLE_PTR(const std::vector<double> & vec) { return &vec[0]; }

/*! Error norm-based scale factor used in time step adjustment. */
const double ERROR_NORM_SAFETY = 6;

// LES based on ADI
class LESADIDirect : public SOLFRA::LESInterfaceDirect {
public:

	enum PrecondSolveType {
		SolveLeft = 1,
		SolveRight = 2
	};

	/*! Initializes LESADIDirect. */
	LESADIDirect();

	~LESADIDirect();

	/*! Re-implemented from LESInterface::init(). 	*/
	void init(ModelInterface * model, IntegratorInterface * integrator,
			  PrecondInterface * precond, JacobianInterface * jacobian);

	/*! Re-implemented from LESInterface::setup(). 	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma);

	/*! Re-implemented from LESInterface::solve(). */
	virtual void solve(double * rhs);

private:
	void setupODE(const double * y, const double * ydot, const double * residuals, double gamma);

	void setupDAE(const double * y, const double * ydot, const double * residuals, double gamma);

	/*! Pointer to the underlying model. */
	SOLFRA::ModelInterfaceADI				*m_modelADI;

	/*! Number of matrix bocks = number of discretisation elements. */
	unsigned int							m_n;
	/*! Block size = number of equations. */
	unsigned int							m_m;

	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotX;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotY;
	/*! Global solution vector of two splitting steps. */
	std::vector<double>						m_residualsX;
	/*! Global rhs vector. */
	std::vector<double>						m_residualsY;

	/*! ID numbers for ODE equations (size n). */
	std::vector<double>						m_odeIDs;

	/*! Used to store individual ydot elements. */
	std::vector<double>						m_y;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydot;
	/*! Used to store individual y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store individual right hand side of ODE models. */
	std::vector<double>						m_FMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydotDiff;
	/*! Stores the modified residuals/ydot values during the FD algorithm. */
	std::vector<double>						m_residualsMod;

	/*! Global solution vector of two splitting steps. */
	std::vector<double>						m_z;
	/*! Used to store solution vector for splitting step 1. */
	std::vector<double>						m_zX;
	/*! Used to store solution vector for splitting step 2. */
	std::vector<double>						m_zY;
	/*! Band matrix implementation for x-splitting (owned). */
	IBKMK::BlockTridiagMatrix				*m_jacobianX;
	/*! Band matrix implementation for y-splitting (owned). */
	IBKMK::BlockTridiagMatrix				*m_jacobianY;
};


LESADIDirect::LESADIDirect() :
	m_modelADI(nullptr),
	m_jacobianX(nullptr),
	m_jacobianY(nullptr)
{
}

LESADIDirect::~LESADIDirect() {
	delete m_jacobianX;
	delete m_jacobianY;
}

void LESADIDirect::init(ModelInterface * model, IntegratorInterface * integrator,
						PrecondInterface * precond, JacobianInterface * /*jacobian*/)
{

	const char * const FUNC_ID = "[LESADIDirect::init]";
	// initialize all variables needed for Jacobian
	//const char * const FUNC_ID = "[LESADIDirect::init]";

	// retreive pointer to the current integrator and current model
	m_integrator = integrator;
	// retreive pointer to the current model
	m_modelADI = dynamic_cast<ModelInterfaceADI *> (model);

	if(m_modelADI == nullptr)
		throw IBK::Exception("Error initializing ADI direct  linear solver: solver is only "
			"defined for model types derivated from ModelInterfaceADI",FUNC_ID);

	// determine which preconditioner we are using
	if (precond != nullptr)
		IBK::IBK_Message( "LESADIDirect linear solver is chosen: Preconditioner will be ignored.", IBK::MSG_WARNING, FUNC_ID);

	// retreive problem size
	m_n = m_modelADI->nElements();
	m_m = m_modelADI->nEquations();

	/// \todo check me !
	// intADI is never used actually
	if ( /*SOLFRA::IntegratorADI* intADI =*/ dynamic_cast<SOLFRA::IntegratorADI*>(m_integrator)) {
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
		m_FMod.resize(m_n * m_m);
		m_ydiff.resize(m_n * m_m);

	}
	else {
		throw IBK::Exception("Error initializing ADI direct linear solver: solver is only "
			"defined for ADI integrator",FUNC_ID);
	}
}


void LESADIDirect::setup(const double * y, const double * /*ydot*/, const double * /*residuals*/, double gamma) {
	// retrieve model indices
	const unsigned int *indicesX = m_modelADI->indicesX();
	const unsigned int *indicesY = m_modelADI->indicesY();

	// loop over all variables
	for(unsigned int j = 0; j < m_n * m_m; ++j) {
		m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
	}

	// copy y values
	std::memcpy(&m_yMod[0], y, m_n * m_m * sizeof(double));
	// update y values inside model
	m_modelADI->setY(y);

	if (m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
		// first splitting step
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
	}
	else {
		// reset y values inside model
		m_modelADI->setY(y);
		// second splitting step
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
	}
	// update solver statistics
	++m_statNumJacEvals;

	// scale with -gamma
	if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
		double* dataJacX = m_jacobianX->submatrix(0,IBKMK::BlockTridiagMatrix::Lower);
		for (unsigned int i = 0; i < m_m * m_m * m_n * 3; ++i) {
			dataJacX[i] *= -gamma;
		}
	}
	else {
		double* dataJacY = m_jacobianY->submatrix(0,IBKMK::BlockTridiagMatrix::Lower);
		for (unsigned int i = 0; i < m_m * m_m * m_n * 3; ++i) {
			dataJacY[i] *= -gamma;
		}
	}

	// add identity matrix
	if (m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
		for (unsigned int row = 0; row < m_n; ++row) {
			double *blockJacobianMainX = m_jacobianX->submatrix(row,IBKMK::BlockTridiagMatrix::Main);
			// add identity matrix to the main submatrix
			for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
				blockJacobianMainX[eqn*m_m +eqn] += 1.0;
			}
		}
	}
	else {
		for (unsigned int row = 0; row < m_n; ++row) {
			double *blockJacobianMainY = m_jacobianY->submatrix(row,IBKMK::BlockTridiagMatrix::Main);
			// add identity matrix to the main submatrix
			for (unsigned int eqn = 0; eqn < m_m; ++eqn) {
				blockJacobianMainY[eqn*m_m +eqn] += 1.0;
			}
		}
	}

#ifdef DUMP_ADI_JACOBIANX
	std::ofstream jacdumpX("jacobian_adiX.txt");
	m_jacobianX->write(jacdumpX, nullptr, false, 15);
	jacdumpX.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESADIDirect::setup]");
#endif
#ifdef DUMP_ADI_JACOBIANY
	std::ofstream jacdumpY("jacobian_adiY.txt");
	m_jacobianY->write(jacdumpY, nullptr, false, 15);
	jacdumpY.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESADIDirect::setup]");
#endif

	// perform lU-factorization
	if (m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X)
		m_jacobianX->lu();
	else
		m_jacobianY->lu();


}


void LESADIDirect::solve(double * rhs) {
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

	if (m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
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
	}
	else {
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
	}

	// copy m_z into rhs
	std::memcpy(rhs, &m_z[0], m_n * m_m * sizeof(double) );
}



// *** IntegratorADI ***

IntegratorADI::IntegratorADI() :
	m_maximumNonlinearIterations(10),
	m_NLResidualTolerance(1e-12),
	m_nonLinConvCoeff(0.1),
	m_modifiedNewtonStrategy(MN_ONCE_AT_STEP_START),
	m_dtMax(3600),
	m_lesSolver(new LESADIDirect()),
	m_dt(0),
	m_dtn(0),
	m_statsFileStream(nullptr),
	m_truncationError(0),
	m_splittingError(0),
	m_residualError(0),
	m_statNumSteps(0),
	m_statNumRHSEvals(0),
	m_statNumItersX(0),
	m_statNumItersY(0),
	m_statNumConvFailsX(0),
	m_statNumConvFailsY(0),
	m_statNumConvDivergingX(0),
	m_statNumConvDivergingY(0),
	m_statNumJacEvalsX(0),
	m_statNumJacEvalsY(0),
	m_statNumErrFails(0)
{
	m_relTol = 1e-5;
	m_absTol = 1e-6;
}

IntegratorADI::~IntegratorADI() {
	delete m_lesSolver;
	delete m_statsFileStream;
}

void IntegratorADI::init(ModelInterface * model, double t0, const double *y0,
	LESInterface * /*lesSolver*/, PrecondInterface *precond, JacobianInterface *jacobian)
{
	const char * const FUNC_ID = "[IntegratorADI::init]";
	m_modelADI = dynamic_cast<ModelInterfaceADI*>(model);
	if (m_modelADI == nullptr)
		throw IBK::Exception("Invalid model implementation, expected model "
			"implementing the interface of ModelInterfaceADI.", FUNC_ID);

	// ignore les solver settings and use own les solver
	m_preconditioner = precond;

	m_t = t0;
	// if user didn't specify time step, take the value from the model
	if (m_dt == 0) {
		m_dtNext = m_dt = model->dt0();
		m_dtn = m_dt;
	}
	else {
		m_dtNext = m_dt;
		m_dtn = m_dt;
	}

	// resize vectors
	m_n=model->n();
	m_y.resize(m_n);
	m_ynHalf.resize(m_n);
	m_yn.resize(m_n);
	m_ydot.resize(m_n);
	m_ydotX.resize(m_n);
	m_ydotY.resize(m_n);
	m_ydotn.resize(m_n);
	m_ydotXnHalf.resize(m_n);
	m_ydotYn.resize(m_n);
	m_yout.resize(m_n);
	m_residuals.resize(m_n);
	m_splittingErrors.resize(m_n);
	m_residualErrors.resize(m_n);
	m_residualErrorsn.resize(m_n);
	m_errors.resize(m_n);
	m_weights.resize(m_n);
	m_deltaY.resize(m_n);
	m_weights.resize(m_n);

	// transfer initial conditions
	std::copy(y0, y0+m_n, DOUBLE_PTR(m_y));

	// implementation refering to model type (either ODE or DAE)
	// initialize state of model to initial conditions
	ModelInterface::CalculationResult res = m_modelADI->setTime(m_t);
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model: failed setting the start time.", FUNC_ID);

	res = m_modelADI->setY(DOUBLE_PTR(m_y));
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model: failed setting the initial states.", FUNC_ID);

	// for now, we have to start with the assumption, that the ydot0 values are zero
	res = m_modelADI->ydot(DOUBLE_PTR(m_ydot)); ++m_statNumRHSEvals;

	// check for size of absTolVec
	if (!m_absTolVec.empty() && m_absTolVec.size() != m_n)
		throw IBK::Exception("Invalid size of absTolVec.", FUNC_ID);

	IBK::IBK_Message( IBK::FormatString("Setting NonlinConvCoef to %1.\n").arg(m_nonLinConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Setting MaxNonLinIters to %1.\n").arg(m_maximumNonlinearIterations), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// *** Initialize Linear Equation Solver***
	m_lesSolver->init(model, this, precond, jacobian);

	// *** Initialize Jacobian ***
	if (jacobian != nullptr)
		jacobian->init(model);

	// *** Initialize Preconditioner ***
	if (precond != nullptr)
		precond->init(model, this, jacobian);

	// reset counters
	m_nItersSinceJacUpdateX = 0;
	m_nItersSinceJacUpdateY = 0;
	m_nStepsSinceJacUpdateX = 0;
	m_nStepsSinceJacUpdateY = 0;
	m_dtJacUpdateX = m_dt;
	m_dtJacUpdateY = m_dt;
	m_newtonConvergenceErrorX = false;
	m_newtonConvergenceErrorY = false;

	// on first step we need to update the Jacobian
	m_jacUpdateNeededX = true;
	m_jacUpdateNeededY = true;
}


IntegratorInterface::StepResultType IntegratorADI::step() {
	const char * const FUNC_ID = "[IntegratorADI::step]";
	// input state:		m_t contains tn
	//					m_dtNext contains proposed time step for next integration step
	//					m_y contains yn
	//					m_ydot contains ydotn

	//					model state is undefined
	//					(can be m_tn, m_yn or any other value when outputs were written previously)

	// backup current solution in xxxn variables
	m_dtn = m_dt;
	m_yn.swap(m_y);			// m_yn contains yn
	m_ydotn.swap(m_ydot);	// m_ydotn contains dydtn
	m_ydotYn.swap(m_ydotY);
	m_residualErrorsn.swap(m_residualErrors);	// m_ydotn contains dydtn

	// set maximum scale factor for time step enlargement
	m_dtScaleMax = 10;

	// update the model's state to the previous solution, time state is set below
	ModelInterface::CalculationResult res;

	// *** time step reduction loop ***
	for (;;) {
		// move new time step suggestion to m_dt
		m_dt = m_dtNext;

		// compute new time point that we solve our equation system for
		double tnew = m_t + m_dt;

		// restrict time value to stop time
		// safeguard against bad m_stopTime value
		IBK_ASSERT(m_stopTime == 0 || m_stopTime > m_t);
		if (m_stopTime != 0 && tnew > m_stopTime)
			tnew = m_stopTime;

		// update the model's time-dependent components to this new time point
		res = m_modelADI->setTime(tnew);
		handleErrors(res);

		// *** Prediction ***

		// compute estimate y_{m=0} of new solution using slopes at last time level
		// m_ydotn holds the derivativ of y_n, i.e. the functional evaluation of f(t_n, y_n)
		// m_y[i] = m_yn[i] + m_ydotn[i]*m_dt;
		IBKMK::vector_linear_sum(m_n, IBKMK_ONE, DOUBLE_PTR(m_yn), m_dt, DOUBLE_PTR(m_ydotn), DOUBLE_PTR(m_y));

//#define USE_CONSTANT_EXTRAPOLATION
#ifdef USE_CONSTANT_EXTRAPOLATION

		// we use the last yn values as starting values for the Newton iteration instead of
		// the predicted (explicit Euler) solution
		IBK::vector_copy(m_n, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_y));

#else //  USE_CONSTANT_EXTRAPOLATION

		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);

#endif //  USE_CONSTANT_EXTRAPOLATION

		// check if residual norm is too large which indicates that extrapolation may not be suitable
		const double RES_NORM_LIMIT = 300000; // this is a GUESS!!!!
		if (m_residualNorm > RES_NORM_LIMIT) {
			IBK::IBK_Message( IBK::FormatString("Reducing time step, residual of extrapolation was  %1.").arg(m_residualNorm), IBK::MSG_WARNING,
				FUNC_ID, IBK::VL_DETAILED);
			decreaseTimeStepNonLinearFailure();
			continue;
		}

		// reset iteration counter
		m_nIterations = 0;

		// start with x-direction
		m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_X);

		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);

		// try to take a non-linear step, from m_t using step size m_dt
		StepResult result = tryStep();

		if (result == TryAgain) {
			// retry step with same time step size but énforce Jacobian update
			m_newtonConvergenceErrorX = true;
			continue;
		}
		else if(result != Success) {
			decreaseTimeStepNonLinearFailure();
			continue;
		}

		// we accept the step as yn1/2
		IBKMK::vector_copy(m_n, DOUBLE_PTR(m_y), DOUBLE_PTR(m_ynHalf));
		m_ydotXnHalf.swap(m_ydotX);

		// reset iteration counter
		m_nIterations = 0;

		// try y-direction
		m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_Y);

		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);

		// try to take a non-linear step, from m_t using step size m_dt
		result = tryStep();

		if (result == TryAgain) {
			// retry step with same time step size but énforce Jacobian update
			m_newtonConvergenceErrorY = true;
			continue;
		}
		else if(result != Success) {
			decreaseTimeStepNonLinearFailure();
			continue;
		}

		// activate all
		m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_ALL);

		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);

		// check if residual norm is too large which indicates that extrapolation may not be suitable
		if (m_residualNorm > RES_NORM_LIMIT) {
			IBK::IBK_Message( IBK::FormatString("Reducing time step, residual of extrapolation was  %1.").arg(m_residualNorm), IBK::MSG_WARNING,
				FUNC_ID, IBK::VL_DETAILED);
			decreaseTimeStepNonLinearFailure();
			continue;
		}

		bool errTestSuccess = errorTest();
		if (errTestSuccess) {
			adjustTimeStep(); // increase time step based on error test
			break; // break loop because we passed the error test
		}

	} // *** time step reduction loop ***


#ifdef SOLVER_STEP_STATS2
	std::cout
		<< std::setw(12) << std::right << m_t/3600 << " "
		<< std::setw(10) << std::right << m_dt << " "
		<< std::setw(6) << std::right << m_statNumSteps
		<< std::setw(6) << std::right << m_statNumIters
		<< std::setw(6) << std::right << m_statNumRHSEvals
		<< std::setw(6) << std::right << m_statNumJacEvals
		<< std::setw(6) << std::right << m_statNumConvFails
		<< std::setw(6) << std::right << m_statNumConvDiverging
		<< std::endl;
#endif // SOLVER_STEP_STATS

	// advance solution in time
	m_t += m_dt;
	// restrict time value to stop time
	if(m_stopTime != 0 && m_t > m_stopTime)
		m_t = m_stopTime;

	// m_y and m_ydot hold solution and time derivatives at time level m_t
	// model state is set to m_t, and m_y

	++m_statNumSteps;
	return IntegratorInterface::StepSuccess;

}


const double * IntegratorADI::yOut(double t_out) const {
	if (t_out == m_t)
		return &m_y[0];
	// interpolate between last step and current step
	double t = std::min(t_out, m_t); // we don't allow extrapolations
	double alpha = (m_t-t)/m_dt;
	double beta = 1-alpha;
	// m_yout[i] = m_yn[i] * alpha + m_y[i] * beta;
	IBKMK::vector_linear_sum(m_n, alpha, DOUBLE_PTR(m_yn), beta, DOUBLE_PTR(m_y), DOUBLE_PTR(m_yout));
	return &m_yout[0];
}


void IntegratorADI::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	if (doRestart) {
		m_statsFileStream = IBK::create_ofstream(logfilePath / "integrator_ADI_stats.tsv", std::ios_base::app);
	}
	else {
		m_statsFileStream = IBK::create_ofstream(logfilePath / "integrator_ADI_stats.tsv");
		std::ostream & out = *(m_statsFileStream);
		out << setw(25) << right << "Time\t";
		out << setw(10) << right << "Steps\t";
		out << setw(10) << right << "RhsEvals\t";
		out << setw(11) << right << "NJacEvals\t";
		out << setw(8) << right << "NIters\t";
		out << setw(11) << right << "NDiverged" << "\t";
		out << setw(11) << right << "NConvFails" << "\t";
		out << setw(11) << right << "NErrFails" << "\t";
		out << setw(14) << right << "TruncError\t";
		out << setw(11) << right << "ResError\t";
		out << setw(14) << right << "SplitError\t";
		out << setw(14) << right << "StepSize [s]";
		out << endl;
	}

#ifdef SOLVER_DEBUG_OUTPUT
	m_logFilePath = logfilePath;
#endif

}


void IntegratorADI::writeStatistics() {
	std::ostream & out = *(m_statsFileStream);
	// Time after last step was completed
	out << fixed << setprecision(4) << setw(25) << right << m_t;
	// Steps
	out << fixed << setprecision(0) << setw(10) << right << m_statNumSteps << "\t";
	// RhsEvals
	out << fixed << setprecision(0) << setw(10) << right << m_statNumRHSEvals << "\t";
	// NJacEvals
	out << fixed << setprecision(0) << setw(11) << right << m_statNumJacEvalsX + m_statNumJacEvalsY << "\t";
	// NIters
	out << fixed << setprecision(0) << setw(8) << right << m_statNumItersX + m_statNumItersY << "\t";
	// NDiverged
	out << fixed << setprecision(0) << setw(11) << right << m_statNumConvDivergingX + m_statNumConvDivergingY << "\t";
	// NConvFails
	out << fixed << setprecision(0) << setw(11) << right << m_statNumConvFailsX + m_statNumConvFailsY << "\t";
	// NErrFails
	out << fixed << setprecision(0) << setw(11) << right << m_statNumErrFails << "\t";
	// Truncation error
	out << fixed << setprecision(4) << setw(14) << right << m_truncationError << "\t";
	// CNError
	out << fixed << setprecision(4) << setw(11) << right << m_residualError << "\t";
	// SplittingError
	out << fixed << setprecision(4) << setw(14) << right << m_splittingError << "\t";
	// StepSize
	if (m_dt < 1e-5)
		out << setprecision(8) << scientific;
	else
		out << setprecision(6) << fixed;
	out << setw(14) << right << m_dt;
	out << endl;
}


// *** PRIVATE FUNCTIONS ***
void IntegratorADI::evaluateModel() {
	// set model state to predicted y values
	ModelInterface::CalculationResult res = m_modelADI->setY( DOUBLE_PTR(m_y) );
	handleErrors(res);

	if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X)  {
		// and retreive the derivatives estimated by the model for the new time point, store in m_ydot
		res = m_modelADI->ydot( DOUBLE_PTR(m_ydotX) ); ++m_statNumRHSEvals;
		handleErrors(res);
		// compute residuals for predicted values
		//	 m_residuals[i] = m_y[i] - m_yn[i] - 0.5 * m_dt*(m_ydot[i] + ydotYn[i]);
		IBKMK::vector_copy(m_n, DOUBLE_PTR(m_y), DOUBLE_PTR(m_residuals));
		IBKMK::vector_sub(m_n, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_residuals));
		IBKMK::vector_add(m_n, -0.5 * m_dt, DOUBLE_PTR(m_ydotYn), DOUBLE_PTR(m_residuals));
		IBKMK::vector_add(m_n, -0.5 * m_dt, DOUBLE_PTR(m_ydotX), DOUBLE_PTR(m_residuals));
	}
	else if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_Y ) {
		// and retreive the derivatives estimated by the model for the new time point, store in m_ydot
		res = m_modelADI->ydot( DOUBLE_PTR(m_ydotY) ); ++m_statNumRHSEvals;
		handleErrors(res);
		// compute residuals for predicted values
		//	 m_residuals[i] = m_y[i] - m_yn1/2[i] - 0.5 * m_dt*(m_ydot[i] + m_ydotnX1/2[i]);
		IBKMK::vector_copy(m_n, DOUBLE_PTR(m_y), DOUBLE_PTR(m_residuals));
		IBKMK::vector_sub(m_n, DOUBLE_PTR(m_ynHalf), DOUBLE_PTR(m_residuals));
		IBKMK::vector_add(m_n, -0.5 * m_dt, DOUBLE_PTR(m_ydotXnHalf), DOUBLE_PTR(m_residuals));
		IBKMK::vector_add(m_n, -0.5 * m_dt, DOUBLE_PTR(m_ydotY), DOUBLE_PTR(m_residuals));
	}
	// all directions mode
	else {
		// and retreive the derivatives estimated by the model for the new time point, store in m_ydot
		res = m_modelADI->ydot( DOUBLE_PTR(m_ydot) ); ++m_statNumRHSEvals;
		handleErrors(res);
		// compute residuals for predicted values
		//	 m_residuals[i] = m_y[i] - m_yn[i] - 0.5 * m_dt*(m_ydot[i] + ydotn[i]);
		//IBKMK::vector_copy(m_n, DOUBLE_PTR(m_y), DOUBLE_PTR(m_residuals));
		//IBKMK::vector_sub(m_n, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_residuals));
		//IBKMK::vector_add(m_n, -0.5 * m_dt, DOUBLE_PTR(m_ydotn), DOUBLE_PTR(m_residuals));
		//IBKMK::vector_add(m_n, -0.5 * m_dt, DOUBLE_PTR(m_ydot), DOUBLE_PTR(m_residuals));
	}
	// use either the models error weights function
	if (m_modelADI->hasErrorWeightsFunction()) {
		SOLFRA::ModelInterface::CalculationResult res = m_modelADI->calculateErrorWeights(DOUBLE_PTR(m_y), DOUBLE_PTR(m_weights));
		(void)res; // to make compiler happy
		// ensure validity of the implementation
		IBK_ASSERT(res == SOLFRA::ModelInterface::CalculationSuccess);
	}
	// or equation-dependent absolute tolerances
	else if (m_absTolVec.empty()) {
		for (unsigned int i=0; i<m_n; ++i) {
			m_weights[i] = 1.0/(std::fabs(m_y[i])*m_relTol + m_absTol);
		}
	}
	// or vector of tolerances
	else {
		for (unsigned int i=0; i<m_n; ++i) {
			m_weights[i] = 1.0/(std::fabs(m_y[i])*m_relTol + m_absTolVec[i]);
		}
	}

	// compute current residual norm, scaling the residuals by magnitude of m_y
	m_residualNorm = WRMSNorm(m_residuals);
}


bool IntegratorADI::errorTest() {
	const char * const FUNC_ID = "[IntegratorADI::errorTest()]";
	// error test is based on a direct evaluation of the first truncated Taylor series term
	// of the implicit euler formulation: err = \frac{\delta_t^2}{2}\ddot{y}^{n+1}
	// where the second derivative is obtained using a Taylor-series expansion on
	// the first derivatives backwards: \ddot{y}^{n+1} = \delta_t \left( \ddot{y}^{n+1} - \dot{y}^{n}\right)
	for (unsigned int i=0; i<m_n; ++i) {
		// Note: both variants yield approximately the same error estimate.
		//       However, the second requires the evaluation of the model to compute m_ydot with
		//       the computed solution whereas the first version only requires the states m_y.
		// First version:
		m_splittingErrors[i] = m_y[i] - m_yn[i] - 0.5 * m_dt* (m_ydot[i] + m_ydotn[i]);
		m_residualErrors[i]  =- 0.5 * m_dt* (m_ydot[i] - m_ydotn[i]);
		// extrapolate residual error of the last time step, (f^n-1 - f^n-2)/delta t^n-1 = konst
		m_errors[i] = m_residualErrors[i] - m_dt / m_dtn * m_residualErrorsn[i];
		// Second version:
	}

	m_splittingError = WRMSNorm(m_splittingErrors);
	m_residualError = 1.0/6.0 * WRMSNorm(m_residualErrors);
	m_truncationError = 1.0/6.0 * WRMSNorm(m_errors) + m_splittingError;

	if (m_truncationError >= 1) {
		++m_statNumErrFails;
		IBK::IBK_Message( IBK::FormatString("Error test failed with |err| = %1.")
			.arg(m_truncationError), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);

		// reduce time step
		//double scale = 1/( std::sqrt(ERROR_NORM_SAFETY* m_truncationError)+0.000001);
		double scale = 1/( std::pow(ERROR_NORM_SAFETY* m_truncationError, 1.0/3.0)+0.000001);
		// ensure that scale is reduced at least by SCALE_MIN value
		const double SCALE_MIN = 0.2;
		scale = std::min(scale, SCALE_MIN);
		m_dtNext = m_dt * scale;
		if (m_t + m_dtNext == m_t)
			throw IBK::Exception("Time step too small (rounding errors occur).", FUNC_ID);

		// do not allow increase of time step at end of this step
		m_dtScaleMax = 1;

		// reevaluate model for m_yn at t_n and store now correctly evaluated
		// derivative in m_ydotn for use in next extrapolation
		//IBK::vector_copy(m_n, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_y));
		//evaluateModel();
		//IBK::vector_copy(m_n, DOUBLE_PTR(m_ydot), DOUBLE_PTR(m_ydotn));
	}
	return m_truncationError < 1;
	//return m_splittingError < 1 && m_residualError < 0.5 * (1.0 - m_splittingError);
}


void IntegratorADI::adjustTimeStep() {
	// we increase the time step based on m_errorNorm, which should be < 1
	IBK_ASSERT(m_truncationError < 1);
	//IBK_ASSERT(m_splittingError < 1 && m_residualError < 0.5 * (1.0 - m_splittingError));

	// we enlarge our current error estimate by a certain factor that we assume that
	// the error norm may enlarge due to non-linearities in the next step
	//double scale = 1/( std::sqrt(ERROR_NORM_SAFETY* m_truncationError)+0.000001);
	double scale = 1/( std::pow(ERROR_NORM_SAFETY* m_truncationError, 1.0/3.0)+0.000001);
	scale = std::min(scale, m_dtScaleMax);
	scale = std::max(1.0, scale);
	const double DT_SCALE_THRESHOLD = 1.5;
	// only allow step size change for significant increase of step sizes
	// as a safeguard against keeping the time step constant forever, only keep it constant for
	// a limited number of steps
	// decrease of time step is not allowed at all
	if (scale < DT_SCALE_THRESHOLD && m_nStepsSinceJacUpdateX < 20
		&& m_nStepsSinceJacUpdateY < 20) {
		scale = 1;
	}

	double dtNew = m_dtNext * scale;
	m_dtNext = std::min(m_dtMax, dtNew);
}


void IntegratorADI::determineJacobianUpdate() {

	if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
		m_jacUpdateNeededX = false;

		// always update on first call
		if (m_statNumItersX == 0) {
			m_jacUpdateNeededX = true;
			return;
		}

		// For all but the ONCE_AT_STEP_START option we always update the Jacobian
		if (m_modifiedNewtonStrategy > MN_ONCE_AT_STEP_START) {
			m_jacUpdateNeededX = true;
			return;
		}

		// if two convergence errors occurred after another
		if (m_newtonConvergenceErrorX) {
			m_jacUpdateNeededX = true;
			return;
		}

		// when time step has changed too much
		double dtRatio = m_dt/m_dtJacUpdateX;
		const double MAX_DT_RATIO = 1.3;
		if (dtRatio > MAX_DT_RATIO || 1.0/dtRatio > MAX_DT_RATIO) {
			m_jacUpdateNeededX = true;
			return;
		}

		// when number of steps since Jacobian update exceeds a certain limit and the
		// time step had changed
		if (m_nStepsSinceJacUpdateX > 200 && dtRatio != 1) {
			m_jacUpdateNeededX = true;
			return;
		}
	}
	else {
		m_jacUpdateNeededY = false;

		// always update on first call
		if (m_statNumItersY == 0) {
			m_jacUpdateNeededY = true;
			return;
		}

		// For all but the ONCE_AT_STEP_START option we always update the Jacobian
		if (m_modifiedNewtonStrategy > MN_ONCE_AT_STEP_START) {
			m_jacUpdateNeededY = true;
			return;
		}

		// if two convergence errors occurred after another
		if (m_newtonConvergenceErrorY) {
			m_jacUpdateNeededY = true;
			return;
		}

		// when time step has changed too much
		double dtRatio = m_dt/m_dtJacUpdateY;
		const double MAX_DT_RATIO = 1.3;
		if (dtRatio > MAX_DT_RATIO || 1.0/dtRatio > MAX_DT_RATIO) {
			m_jacUpdateNeededY = true;
			return;
		}

		// when number of steps since Jacobian update exceeds a certain limit and the
		// time step had changed
		if (m_nStepsSinceJacUpdateY > 200 && dtRatio != 1) {
			m_jacUpdateNeededY = true;
			return;
		}
	}
}


IntegratorADI::StepResult IntegratorADI::tryStep()
{
	// this flag is true when we have updated the Jacobian matrix at least once in this tryStep() run with the current time step size
	if (m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X)
		m_jacCurrentX = false;
	else
		m_jacCurrentY = false;

	// *** Jacobian Update ***

	// set m_jacUpdateNeeded to true if we need to recompute the Jacobian
	determineJacobianUpdate();

	// update Jacobian if needed, typically, this is only done once for each Jacobian step when using modified Newton
	if (m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X && m_jacUpdateNeededX) {
		// clear Newton convergence error flag
		m_newtonConvergenceErrorX = false;

#ifdef SOLVER_STEP_STATS2
		std::cout << "Jac-update(" << m_dt << ") after " << m_nStepsSinceJacUpdate << " Steps, " << m_nIterations << " Iters" << std::endl;
#endif // SOLVER_STEP_STATS
		// update Jacobian matrix, using first-order finite-difference approximations,
		// also for the estimation of ydot = dy/dt
		// m_ydot contains f(t,y)

		// we are updating the Jacobian with predicted/iterative y values
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_LS_SETUP,
			m_lesSolver->setup(DOUBLE_PTR(m_y), DOUBLE_PTR(m_ydotX), 0, 0.5 * m_dt);
		);
		++m_statNumJacEvalsX;

		// reset counters
		m_nStepsSinceJacUpdateX = 0;
		m_jacCurrentX = true;

		// update Jacobian time step
		m_dtJacUpdateX = m_dt;

		// reset convergence rate estimate to 1
		m_convRateDeltaNormX = 1;

	}
	else if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_Y && m_jacUpdateNeededY) {
		// clear Newton convergence error flag
		m_newtonConvergenceErrorY = false;

#ifdef SOLVER_STEP_STATS2
		std::cout << "Jac-update(" << m_dt << ") after " << m_nStepsSinceJacUpdate << " Steps, " << m_nIterations << " Iters" << std::endl;
#endif // SOLVER_STEP_STATS
		// update Jacobian matrix, using first-order finite-difference approximations,
		// also for the estimation of ydot = dy/dt
		// m_ydot contains f(t,y)

		// we are updating the Jacobian with predicted/iterative y values
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_LS_SETUP,
			m_lesSolver->setup(DOUBLE_PTR(m_y), DOUBLE_PTR(m_ydotY), 0, 0.5 * m_dt);
		);
		++m_statNumJacEvalsY;

		// reset counters
		m_nStepsSinceJacUpdateY = 0;
		m_jacCurrentY = true;

		// update Jacobian time step
		m_dtJacUpdateY = m_dt;

		// reset convergence rate estimate to 1
		m_convRateDeltaNormY = 1;

	}

	// do Newton iteration and return result
	IntegratorADI::StepResult res = newtonIteration();
	if (res == IntegratorADI::Success) {
		// we now have m_y set to the new solution, but m_ydot still
		// holds derivative of m_y of last iteration
//#define ALWAYS_REEVALUATE_MODEL_FOR_PREDICTION
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);
	}
	return res;

}

IntegratorADI::StepResult IntegratorADI::newtonIteration() {
	const char * const FUNC_ID = "[IntegratorADI::newtonIteration]";

	// we monitor the change of vector norms over the whole newton step, and the
	// change between the last update of the Jacobian matrix.
	double residualNormlast = m_residualNorm;
	double deltaNormLast = 1;
	double deltaNorm = 1;
//	double deltaNormPred = 1;

	double convRateResiduals = 1;
	double convRateDeltaNorm = 1;

	// *** Newton-Iteration ***

	// update m_y until vector norm of changes is small enough
	for (;;) {

// enable/disable SOLVER_DEBUG_OUTPUT in header file
#ifdef SOLVER_DEBUG_OUTPUT
		{
			char fname[200];
			FILE * dumpfile;
			sprintf(fname, (m_logFilePath + "/v6_impleuler_y_yn_fy_fyn_res_%d.txt").c_str(), m_statNumRHSEvals);
			dumpfile = fopen(fname, "w");
			for (unsigned int i=0; i<m_n; ++i) {
				fprintf(dumpfile, "%15.2f %10.15g %10.15g %10.15g %10.15g\n", m_y[i], m_yn[i], m_ydot[i], m_ydotn[i], -m_residuals[i]);
			}
			fclose(dumpfile);
		}
#endif // SOLVER_DEBUG_OUTPUT

		// compose right-hand-side of equation system in vector m_deltaY
		//    m_deltaY[i] = -m_residuals[i];
		IBKMK::vector_scale(m_n, -IBKMK_ONE, DOUBLE_PTR(m_residuals), DOUBLE_PTR(m_deltaY));

		// solve equation system, results will be stored in m_deltaY
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_LS_SOLVE,
			m_lesSolver->solve(DOUBLE_PTR(m_deltaY));
		);

		++m_nIterations;  // local Newton iterations
		// enable/disable SOLVER_DEBUG_OUTPUT in header file
		#ifdef SOLVER_DEBUG_OUTPUT
				{
					char fname[200];
					FILE * dumpfile;
					sprintf(fname, (m_logFilePath + "/v6_impleuler_b_%d.txt").c_str(), m_statNumRHSEvals);
					dumpfile = fopen(fname, "w");
					for (unsigned int i=0; i<m_n; ++i) {
						fprintf(dumpfile, "%10.15g\n", m_deltaY[i]);
					}
					fclose(dumpfile);
				}
		#endif // SOLVER_DEBUG_OUTPUT

		// Since Jacobian matrix is of form: J = I - m_dtJacUpdate*df/dy
		// the solution in m_deltaY will be too large by a factor about m_dt/m_dtJacUpdate
		// and we correct this by multiplying with the inverse ratio m_dtRatioInv
		double dtRatioInv;
		if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
			// increase iteration counters
			++m_statNumItersX; // total Newton iterations

			dtRatioInv = m_dtJacUpdateX/m_dt;
			if (m_dtJacUpdateX != m_dt) {
				//   m_deltaY[i] *= dtRatioInv;
				IBKMK::vector_scale_by(m_n, dtRatioInv, DOUBLE_PTR(m_deltaY));
			}
		}
		else {
			// increase iteration counters
			++m_statNumItersY; // total Newton iterations

			dtRatioInv = m_dtJacUpdateY/m_dt;
			if (m_dtJacUpdateY != m_dt) {
				//   m_deltaY[i] *= dtRatioInv;
				IBKMK::vector_scale_by(m_n, dtRatioInv, DOUBLE_PTR(m_deltaY));
			}
		}

		// compute current norm of changes (delta norm)
		deltaNorm = WRMSNorm(m_deltaY);
//		if (m_nIterations == 1)
//			deltaNormPred = deltaNorm;


		// *** Divergence Check #1 ***
		//
		// out-of-bounds divergence check
		if (deltaNorm > 10000) {

			if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
				// we are diverging
				++m_statNumConvDivergingX;

				IBK::IBK_Message( IBK::FormatString("Diverging (delta t ratio = %1), "
					"residual/delta norms have become too large!").arg(m_dt/m_dtJacUpdateX), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				if (m_jacCurrentX)
					return FailureDiverged;
				else
					return TryAgain;
			}
			else {
				// we are diverging
				++m_statNumConvDivergingY;

				IBK::IBK_Message( IBK::FormatString("Diverging (delta t ratio = %1), "
					"residual/delta norms have become too large!").arg(m_dt/m_dtJacUpdateY), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				if (m_jacCurrentY)
					return FailureDiverged;
				else
					return TryAgain;
			}
		}

		// compute new solution, y_{m+1} = y_m + deltaY
//#define IMPLICIT_EULER_USE_CLIPPING
#ifdef IMPLICIT_EULER_USE_CLIPPING
		for (unsigned int i=0; i<m_n; ++i) {
			double yNew = m_y[i] + m_deltaY[i];
			if (yNew > m_yMax[i]) {
				yNew = m_yMax[i];
			}
			else if (yNew < m_yMin[i]) {
				yNew = m_yMin[i];
			}
			m_y[i] = yNew;
		}
#else // IMPLICIT_EULER_USE_CLIPPING
		//  m_y[i] += m_deltaY[i];
		IBKMK::vector_add(m_n, IBKMK_ONE, DOUBLE_PTR(m_deltaY), DOUBLE_PTR(m_y));
#endif // IMPLICIT_EULER_USE_CLIPPING

		// *** Convergence Check #1 ***
		//
		convRateDeltaNorm = deltaNorm/deltaNormLast;
		// update Jacobian-based convergence rate estimate
		const double CONVERGENCE_RATE_REDUCTION_FACTOR = 0.3;

		if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
			if (m_nIterations > 1)
				m_convRateDeltaNormX = std::max(CONVERGENCE_RATE_REDUCTION_FACTOR*m_convRateDeltaNormX, deltaNorm/deltaNormLast);
			// deltaNorm must be less than m_nonLinConvCoeff and the factor 0.5 (from the error test)
			double deltaNormScaled = std::min(1.0, m_convRateDeltaNormX)*deltaNorm/m_nonLinConvCoeff*0.5;

			// TODO : add protection against convergence stall
			//        accept solution as converged if sufficient progress towards the solution has been made
			if (deltaNormScaled <= 1.0) {
				break; // converged
			}
			// *** Iteration limit check ***
			if (m_nIterations >= m_maximumNonlinearIterations) {
				// doesn't work, only creates more error failes
				//if (m_residualNorm/residualNormPred < 0.1 || deltaNorm/deltaNormPred < 0.1) {
				//	break; // converged?
				//}
				++m_statNumConvFailsX;
				// reset iteration counter
				m_nIterations = 0;
				if (m_jacCurrentX) {
					IBK::IBK_Message( "Too many Newton iterations, reducing time step!", IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
					return FailureIterationLimitExceeded;
				}

				// signal to update Jacobian
				m_newtonConvergenceErrorX = true;
				IBK::IBK_Message( IBK::FormatString("Too many Newton iterations (delta t ratio = %1), "
					"but we retry with Jacobian computed for predicted solution!").arg(m_dt/m_dtJacUpdateX), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				return TryAgain;
			}

		}
		else {
			if (m_nIterations > 1)
				m_convRateDeltaNormY = std::max(CONVERGENCE_RATE_REDUCTION_FACTOR*m_convRateDeltaNormY, deltaNorm/deltaNormLast);
			// deltaNorm must be less than m_nonLinConvCoeff and the factor 0.5 (from the error test)
			double deltaNormScaled = std::min(1.0, m_convRateDeltaNormY)*deltaNorm/m_nonLinConvCoeff*0.5;

			// TODO : add protection against convergence stall
			//        accept solution as converged if sufficient progress towards the solution has been made
			if (deltaNormScaled <= 1.0) {
				break; // converged
			}

			// *** Iteration limit check ***
			if (m_nIterations >= m_maximumNonlinearIterations) {
				// doesn't work, only creates more error failes
				//if (m_residualNorm/residualNormPred < 0.1 || deltaNorm/deltaNormPred < 0.1) {
				//	break; // converged?
				//}
				++m_statNumConvFailsY;
				// reset iteration counter
				m_nIterations = 0;
				if (m_jacCurrentY) {
					IBK::IBK_Message( "Too many Newton iterations, reducing time step!", IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
					return FailureIterationLimitExceeded;
				}

				// signal to update Jacobian
				m_newtonConvergenceErrorY = true;
				IBK::IBK_Message( IBK::FormatString("Too many Newton iterations (delta t ratio = %1), "
					"but we retry with Jacobian computed for predicted solution!").arg(m_dt/m_dtJacUpdateY), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				return TryAgain;
			}
		}

		// Newton-Step done, evaluate model for new solution
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);

		// compute convergence rates
		convRateResiduals = m_residualNorm/residualNormlast;

		// store norms for next iteration
		residualNormlast = m_residualNorm;
		deltaNormLast = deltaNorm;

		if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
			// we have had at least DIVERGENCE_CHECK_ITER_LIMIT iterations since start of Newton Step
			// so we can compute convergence rates
			const unsigned int DIVERGENCE_CHECK_ITER_LIMIT = 3;
			if (m_nIterations > DIVERGENCE_CHECK_ITER_LIMIT) {
				// if either the residual or the delta norms are diverging, try again
				if (convRateResiduals > 1 && convRateDeltaNorm > 1) {
					// we are diverging

					IBK::IBK_Message( IBK::FormatString("Diverging (delta t ratio = %1)!").arg(m_dt/m_dtJacUpdateX), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
					++m_statNumConvDivergingX;
					if (m_jacCurrentX) {
						return FailureDiverged;
					}
					else {
						return TryAgain;
					}
				}
			} // if (m_nIterations > 1)

			// for strict Newton variant, update Jacobian matrix, but DO NOT reset convergence rate
			if ((m_modifiedNewtonStrategy == MN_EVERY_ITERATION) ||
				(m_modifiedNewtonStrategy == MN_EVERY_ITERATION_UNTIL_THRESHOLD && m_residualNorm > 0.01))
			{
				m_lesSolver->setup(DOUBLE_PTR(m_y), DOUBLE_PTR(m_ydot), 0, 0.5 * m_dt);
				++m_statNumJacEvalsX;

				// reset counters
				m_nStepsSinceJacUpdateX = 0;
				m_jacCurrentX = true;

				// update Jacobian time step
				m_dtJacUpdateX = m_dt;
			}
		}
		else {
			// we have had at least DIVERGENCE_CHECK_ITER_LIMIT iterations since start of Newton Step
			// so we can compute convergence rates
			const unsigned int DIVERGENCE_CHECK_ITER_LIMIT = 3;
			if (m_nIterations > DIVERGENCE_CHECK_ITER_LIMIT) {
				// if either the residual or the delta norms are diverging, try again
				if (convRateResiduals > 1 && convRateDeltaNorm > 1) {
					// we are diverging

					IBK::IBK_Message( IBK::FormatString("Diverging (delta t ratio = %1)!").arg(m_dt/m_dtJacUpdateY), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
					++m_statNumConvDivergingY;
					if (m_jacCurrentY) {
						return FailureDiverged;
					}
					else {
						return TryAgain;
					}
				}
			} // if (m_nIterations > 1)

			// for strict Newton variant, update Jacobian matrix, but DO NOT reset convergence rate
			if ((m_modifiedNewtonStrategy == MN_EVERY_ITERATION) ||
				(m_modifiedNewtonStrategy == MN_EVERY_ITERATION_UNTIL_THRESHOLD && m_residualNorm > 0.01))
			{
				m_lesSolver->setup(DOUBLE_PTR(m_y), DOUBLE_PTR(m_ydot), 0, 0.5 * m_dt);
				++m_statNumJacEvalsY;

				// reset counters
				m_nStepsSinceJacUpdateY = 0;
				m_jacCurrentY = true;

				// update Jacobian time step
				m_dtJacUpdateY = m_dt;
			}
		}
	} // while iteration

	if(m_modelADI->activeFluxDirection() == ModelInterfaceADI::FD_X) {
		++m_nStepsSinceJacUpdateX;
	}
	else {
		++m_nStepsSinceJacUpdateY;
	}
	return Success;
}

void IntegratorADI::handleErrors(ModelInterface::CalculationResult res) const {
	if (res == ModelInterface::CalculationAbort)
		throw IBK::Exception("Aborting simulation (signaled from within model).", "[IntegratorADI::handleErrors]");
}


double IntegratorADI::WRMSNorm(std::vector<double> & vec) const {
	double r = 0;
	// three variants for computing Euclidian Norm:
	// 1. do not divide by m_n at all
	// 2. divide r_normalized by m_n
	// 3. divide r by m_n
	// assume m_n = 100
	// 1 - gives error of order 1e-5
	// 2 - gives error of order 1e-7
	// 3 - gives error of order 1e-6
	for (unsigned int i=0; i<m_n; ++i) {
		double r_normalized = vec[i]*m_weights[i];
//		double r_normalized = vec[i]*m_weights[i]/m_n; // Option #2
		r += r_normalized*r_normalized;
	}
	r = std::sqrt(r/m_n); // Option #3
//	r = std::sqrt(r); // Option #1
	return r;
}


void IntegratorADI::decreaseTimeStepNonLinearFailure() {
	// half the time step
	m_dtNext = m_dt/2;
	if (m_t + m_dt == m_t)
		throw IBK::Exception("Time step too small (rounding errors occur).", "[IntegratorADI::decreaseTimeStepNonLinearFailure]");
}


} // namespace SOLFRA
