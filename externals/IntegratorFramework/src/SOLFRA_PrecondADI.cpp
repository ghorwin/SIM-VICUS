#include "SOLFRA_PrecondADI.h"

#include "SOLFRA_IntegratorInterface.h"
#include "SOLFRA_ModelInterfaceADI.h"

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_BlockTridiagMatrix.h>
#include <IBKMK_TridiagMatrix.h>
#include <IBK_FileUtils.h>

#include <fstream>
#include <cmath>
#include <functional>

namespace SOLFRA {

PrecondADI::PrecondADI(	unsigned int n,
						unsigned int m,
						PreconditionerType precondType) :
	PrecondInterface(precondType),
	m_modelADI(nullptr),
	m_n(n),
	m_m(m),
	m_jacobianX(nullptr),
	m_jacobianY(nullptr),
	m_partialJacobianCopyX(nullptr),
	m_partialJacobianCopyY(nullptr),
	m_psi(0.5),
	m_relToleranceDQ(1e-7),
	m_absToleranceDQ(1e-8),
	m_nJacEvals(0),
	m_nRhsEvals(0)
{
}

PrecondADI::~PrecondADI() {
	delete m_jacobianX;
	delete m_jacobianY;
	delete m_partialJacobianCopyX;
	delete m_partialJacobianCopyY;
}

void PrecondADI::init(SOLFRA::ModelInterface * model)
{
	const char * const FUNC_ID = "[PrecondADI::init]";

	// initialize all variables needed for Jacobian
	//const char * const FUNC_ID = "[PrecondADI::init]";

	// retreive pointer to the current model
	m_modelADI = dynamic_cast<ModelInterfaceADI *> (model);

	if(m_modelADI == nullptr)
		throw IBK::Exception("Error initializing ADI linear solver: solver is only "
			"defined for model types derivated from ModelInterfaceADI",FUNC_ID);

	// resize jacobian
	m_jacobianX = new IBKMK::BlockTridiagMatrix(m_n, m_m);
	m_jacobianY = new IBKMK::BlockTridiagMatrix(m_n, m_m);
	double * dataJacX = m_jacobianX->submatrix(0, IBKMK::BlockTridiagMatrix::Lower);
	double * dataJacY = m_jacobianX->submatrix(0, IBKMK::BlockTridiagMatrix::Lower);
	for (unsigned int i=0; i<m_n*m_m*m_m*3; ++i) {
		dataJacX[i] = 0;
		dataJacY[i] = 0;
	}
	m_partialJacobianCopyX = new IBKMK::BlockTridiagMatrix(m_n,m_m);
	m_partialJacobianCopyY = new IBKMK::BlockTridiagMatrix(m_n,m_m);

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

void PrecondADI::init(SOLFRA::ModelInterface * model, SOLFRA::IntegratorInterface * integrator,
					  const JacobianInterface * /*jacobianInterface */)
{

	const char * const FUNC_ID = "[PrecondADI::init]";

	// initialize all variables needed for Jacobian
	//const char * const FUNC_ID = "[PrecondADI::init]";

	// retreive pointer to the current integrator and current model
	m_integrator = integrator;
	// retreive pointer to the current model
	m_modelADI = dynamic_cast<ModelInterfaceADI *> (model);

	if(m_modelADI == nullptr)
		throw IBK::Exception("Error initializing ADI linear solver: solver is only "
			"defined for model types derivated from ModelInterfaceADI",FUNC_ID);
}


int PrecondADI::setup(double t, const double * y, const double * ydot, const double * residuals,
		double gamma)
{
	(void)t;
	(void)ydot;
	(void)residuals;

	// loop over all variables
	for(unsigned int j = 0; j < m_n * m_m; ++j) {
		m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
	}

	// retrieve model indices
	const unsigned int *indicesX = m_modelADI->indicesX();
	const unsigned int *indicesY = m_modelADI->indicesY();

	// copy y values
	std::memcpy(&m_yMod[0], y, m_n * m_m * sizeof(double));
	// update model states and fluxes
	m_modelADI->setY(y);

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
			++m_nRhsEvals;

			// restore original vector
			for (unsigned int j=i; j<m_n; j += 3) {
				// restore y vector
				unsigned int jIdx	 = indicesX[j] * m_m + var;
				m_yMod[jIdx] = y[jIdx];
			} // for j
		} // for i
	} // for var
	// update solver statistics
	++m_nJacEvals;

	// reset model states and fluxes
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
			++m_nRhsEvals;

			// restore original vector
			for (unsigned int j=i; j<m_n; j += 3) {
				unsigned int jIdx  = indicesY[j] * m_m + var;
				// restore y vector
				m_yMod[jIdx] = y[jIdx];
			} // for j
		} // for i
	} // for var
	// update solver statistics
	++m_nJacEvals;

	// reset active flux directions inside the model
	m_modelADI->setActiveFluxDirection(ModelInterfaceADI::FD_ALL);

	// m_jacobian now holds df/dy

	// save backup copy of newly computed partial Jacobian
	*m_partialJacobianCopyX = *m_jacobianX;
	*m_partialJacobianCopyY = *m_jacobianY;

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
		dataJacX[i] *= -m_psi * gamma;
		dataJacY[i] *= -m_psi * gamma;
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
	//	throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondADI::setupODE]");
#endif

	// perform lU-factorization
	m_jacobianX->lu();
	m_jacobianY->lu();

	return 0;
}

int PrecondADI::setup(double t, const double * y, const double * ydot, const double * residuals,
	bool jacOk, bool & jacUpdated, double gamma)
{
	(void)t;
	(void)y;
	(void)ydot;
	(void)residuals;
	(void)jacOk;
	(void)jacUpdated;
	(void)gamma;

	// \todo: add calculation of psi
	return 0;
}


int PrecondADI::solve(double t, const double * y, const double * ydot, const double * residuals,
	const double * r, double * z, double gamma, double delta, int lr)
{
	(void)t;
	(void)y;
	(void)ydot;
	(void)residuals;
	(void)gamma;
	(void)delta;

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

		case None: ;
		default: ; /// \todo exception here? Error handling?
	}

	// retrieve model indices
	const unsigned int *indicesX = m_modelADI->indicesX();
	const unsigned int *indicesY = m_modelADI->indicesY();

	// copy r -> m_z
	std::memcpy(&m_z[0], r,  m_n * m_m * sizeof(double));

	if (solveXDirection) {
		// provide all vectors for preconditioner 2
		for (unsigned int i = 0; i < m_n; ++i) {
			// sort and only store vector elements of index X
			for(unsigned int var = 0; var < m_m; ++var) {
				m_zX[i * m_m + var] = m_z[indicesX[i] * m_m + var];
			}
		}
		// solve: (I - gamma_inv A1 ) z = r
		m_jacobianX->backsolve(&m_zX[0]);
		// copy zX into m_z
		for (unsigned int i = 0; i < m_n; ++i){
			for(unsigned int var = 0; var < m_m; ++var) {
				m_z[indicesX[i] * m_m + var] = m_zX[i * m_m + var];
			}
		}
	}
	if (solveYDirection) {
		for (unsigned int i = 0; i < m_n; ++i) {
			// sort and only store vector elements of index X
			for(unsigned int var = 0; var < m_m; ++var) {
				m_zY[i * m_m + var] = m_z[indicesY[i] * m_m + var];
			}
		}
		// solve: (I - gamma_inv A2 ) z = r
		m_jacobianY->backsolve(&m_zY[0]);
		// copy zY into m_z
		for (unsigned int i = 0; i < m_n; ++i) {
			// sort and only store vector elements of index X
			for(unsigned int var = 0; var < m_m; ++var) {
				m_z[indicesY[i] * m_m + var] = m_zY[i * m_m + var];
			}
		}
		// correct z-value multiplying 2*psi
		std::transform(m_z.begin(), m_z.end(), m_z.begin(),
			   std::bind1st(std::multiplies<double>(), m_psi));
	}

	// copy m_z into z
	std::memcpy(z, &m_z[0], m_n * m_m * sizeof(double) );

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
//	throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondADI::solve]");
#endif
	return 0;
}

int PrecondADI::jacTimesVec(const double * v, double * Jv) const
{
	std::vector<double> JvX(m_n * m_m);
	std::vector<double> JvY(m_n * m_m);
	std::vector<double> vX(m_n * m_m);
	std::vector<double> vY(m_n * m_m);

	// retrieve model indices
	const unsigned int *indicesX = m_modelADI->indicesX();
	const unsigned int *indicesY = m_modelADI->indicesY();

	for (unsigned int i = 0; i < m_n; ++i) {
		for(unsigned int var = 0; var < m_m; ++var) {
			vX[i * m_m + var] = v[indicesX[i] * m_m +var];
			vY[i * m_m + var] = v[indicesY[i] * m_m +var];
		}
	}
	m_partialJacobianCopyX->multiply(&vX[0], &JvX[0]);
	m_partialJacobianCopyY->multiply(&vY[0], &JvY[0]);

	// set Jv to zero
	std::memset(Jv, 0, m_n * m_m *sizeof(double) );
	// add both results and store in vector Jv
	for (unsigned int i = 0; i < m_n; ++i) {
		for(unsigned int eqn = 0; eqn < m_m; ++eqn) {
			Jv[indicesX[i] * m_m + eqn] += JvX[i * m_m + eqn];
			Jv[indicesY[i] * m_m + eqn] += JvY[i * m_m + eqn];
		}
	}

#if 0
	std::vector<double> vec(Jv, Jv + m_model->n());
	std::ofstream dump("adi_precond_Jv.dump", std::ios_base::app);
	for (unsigned int i=0; i<vec.size(); ++i) {
		dump << std::setw(10) << std::right << vec[i];
	}
	dump << std::endl;
	throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondADI::jacTimesVec]");
#endif
	return 0;
}

void PrecondADI::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	std::stringstream logfname;
	logfname << "precond_adi";
	switch (m_precondType) {
		case Left  : logfname << "left"; break;
		case Right : logfname << "right"; break;
		case Both  : logfname << "both"; break;
		case None :
		default : ; // nothing to add
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
		out << std::setw(17) << std::right << "ADIParameter";
		out << std::endl;
	}
}


void PrecondADI::writeStatistics(double t) {
	// append to output file
	std::ostream & out = *m_statsFileStream;
	// Time after last step was completed
	out << std::fixed << std::setprecision(10) << std::setw(25) << std::right << t << "\t";
	// DlsMatJacEvals
	out << std::fixed << std::setprecision(0) << std::setw(17) << std::right << m_nJacEvals << "\t";
	// NRhsEvals
	out << std::fixed << std::setprecision(0) << std::setw(17) << std::right << m_nRhsEvals;
	// phi
	out << std::fixed << std::setprecision(0) << std::setw(17) << std::right << m_psi;
	out << std::endl;
}



} // namespace SOLFRA

