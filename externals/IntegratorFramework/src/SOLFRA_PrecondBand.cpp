#include "SOLFRA_PrecondBand.h"

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorImplicitEuler.h"
#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_JacobianDense.h"
#include "SOLFRA_JacobianSparse.h"
#include "SOLFRA_JacobianSparseCSR.h"

#include <fstream>
#include <iomanip>
#include <algorithm>

#include <IBKMK_BandMatrix.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_assert.h>

#include <cvode/cvode_bandpre.h>
#include <cvode/cvode_spils.h>


namespace SOLFRA {

PrecondBand::PrecondBand(unsigned int ml, unsigned int mu, PreconditionerType precondType) :
	PrecondInterface(precondType),
	m_ml(ml),
	m_mu(mu),
	m_jacobianInterface(nullptr),
	m_jacobian(nullptr),
	m_partialJacobianCopy(nullptr)
{
}


PrecondBand::~PrecondBand() {
	delete m_jacobian;
	delete m_partialJacobianCopy;
}


void PrecondBand::init(ModelInterface * model, IntegratorInterface * integrator,
					   const JacobianInterface *jacobianInterface)
{
	const char * const FUNC_ID = "[PrecondBand::init]";

	m_integrator = integrator;
	m_jacobianInterface = jacobianInterface;

	// retreive pointer to the current model
	m_model = model;

	m_n = m_model->n();

	// determine which integrator we are using:
	// CVODE integrator uses Sundials implementation, but only if assymmetric band matrix is being used
	if (m_ml != m_mu || m_jacobianInterface == nullptr) {
		if (IntegratorSundialsCVODE * intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(integrator)) {
			int res = CVBandPrecInit(intCVODE->cvodeMem(), model->n(), m_mu, m_ml);
			if (res != CVSPILS_SUCCESS) {
				throw IBK::Exception("Error initializing band pre-conditioner.", FUNC_ID);
			}
			IBK::IBK_Message("Using Sundials::BandPrec implementation which generates Jacobian data itself.\n", IBK::MSG_PROGRESS, FUNC_ID, 2);
			return;
		}
		else {
			throw IBK::Exception("No band preconditioner implementation available for ml != mu or missig Jacobian interface.", FUNC_ID);
		}
	}

	IBK::IBK_Message("PrecondBand uses provided JacobianInterface implementation to retrieve Jacobian data.\n", IBK::MSG_PROGRESS, FUNC_ID, 2);

	// resize all vectors
	m_jacobian = new IBKMK::BandMatrix(m_n, m_mu, m_ml, std::min(m_n, m_ml + m_mu + 1));

	// only use jacobian copy if we have an ODE system
	m_partialJacobianCopy = new IBKMK::BandMatrix(m_n, m_mu, m_ml, std::min(m_n, m_ml + m_mu + 1));
}


int PrecondBand::setup(double t, const double * y, const double * ydot, const double * residuals,
	bool jacOk, bool & jacUpdated, double gamma)
{
	const char * const FUNC_ID = "[PrecondBand::setupODE]";
	(void)t;
	(void)y;
	(void)ydot;
	(void)residuals;

	if (jacOk) {
		// restore partial Jacobian from backup-copy
		std::memcpy(&m_jacobian->data()[0],
					&m_partialJacobianCopy->data()[0],
					sizeof(double)*m_partialJacobianCopy->data().size());
		jacUpdated = false;
	}
	else {
		// recompute Jacobian

		m_jacobian->setZero();

		IBK_ASSERT(m_jacobianInterface != nullptr);

		// transfer data from Jacobian
		const JacobianDense * jacDense;
		const JacobianSparse * jacSparse;

		// Transfer from dense Jacobian
		if ((jacDense = dynamic_cast<const JacobianDense *>(m_jacobianInterface)) != nullptr) {
			m_jacobian->setZero();
			// transfer values column by column
			for (unsigned int j=0; j<m_n; ++j) {
				// restrict jacobian assembly to the filled positions of the band
				unsigned int kl = std::max( 0, (int)(j - m_mu));
				unsigned int ku = std::min(j + m_ml, m_n - 1 );
				for (unsigned int k=kl; k<=ku; ++k) {
					/// \todo improve this through direct mem copies from jacobian internal storage
					(*m_jacobian)(k,j) = jacDense->value(k,j);
				}
			}
		}

		// Transfer from sparse Jacobian
		else if ((jacSparse = dynamic_cast<const JacobianSparse *>(m_jacobianInterface)) != nullptr) {
			m_jacobian->setZero();
			// transfer values column by column
			for (unsigned int j=0; j<m_n; ++j) {
				// restrict jacobian assembly to the filled positions of the band
				unsigned int kl = std::max( 0, (int)(j - m_mu));
				unsigned int ku = std::min(j + m_ml, m_n - 1 );
				for (unsigned int k=kl; k<=ku; ++k) {
					/// \todo improve this through direct mem copies from jacobian internal storage
					(*m_jacobian)(k,j) = jacSparse->jacobian()->value(k,j);
				}
			}
		}

		else
			throw IBK::Exception("No transfer algorithm from selected Jacobian matrix implementation to"
									"Band preconditioner implemented, currently supported are JacobianDense "
									"and JacobianSparse!", FUNC_ID);
		// m_jacobian now holds df/dy, actually the partial Jacobian

		// save backup copy of newly computed partial Jacobian
		std::memcpy(&m_partialJacobianCopy->data()[0],
					&m_jacobian->data()[0],
					sizeof(double)*m_partialJacobianCopy->data().size());
		jacUpdated = true;

	}

	// scale with -gamma
	double * data_ptr = &m_jacobian->data()[0];
	for (unsigned int i=0; i<m_jacobian->data().size(); ++i)
		data_ptr[i] *= -gamma;

	// add identity matrix
	for (unsigned int j=0; j<m_n; ++j)
		(*m_jacobian)(j,j) += 1.0;

#if 0
	std::ofstream jacdump("jacobian_band_dfdy.txt");
	m_partialJacobianCopy->write(jacdump, nullptr, false, 15);
	jacdump.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondILU::setupODE]");
#endif

#if 0
		std::ofstream jacdump("jacobian_band.txt");
		m_jacobian->write(jacdump, nullptr, false, 15);
		jacdump.close();
		throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondILU::setupODE]");
#endif

	// perform LU-factorization of the full jacobian
	int res = m_jacobian->lu();

	return res;
}


int PrecondBand::solve(double t, const double * y, const double * ydot, const double * residuals,
	const double * r, double * z, double gamma, double delta, int lr)
{
	(void)t;
	(void)y;
	// copy right hand side to z-vector
	std::memcpy(z,r,m_n*sizeof(double));
//#define SOLVE_DEBUG_OUTPUT
#ifdef SOLVE_DEBUG_OUTPUT
	std::vector<double> vec(r, r + m_n);
	{
		std::ofstream dump("band_precond_r.dump", std::ios_base::app);
		dump << std::setw(10) << std::right << t;
		dump << std::setw(10) << std::right << gamma;
		dump << std::setw(10) << std::right << delta;
		dump << std::setw(10) << std::right << lr << std::endl;
		for (unsigned int i=0; i<vec.size(); ++i) {
			dump << std::setw(10) << std::right << vec[i] << std::endl;
		}
		dump << std::endl;
	}
#else
	(void)ydot;
	(void)residuals;
	(void)gamma;
	(void)delta;
	(void)lr;
#endif
	// backsolve with given lu factorisation
	m_jacobian->backsolve(z);

#ifdef SOLVE_DEBUG_OUTPUT
	{
		copy(z, z + m_n, vec.begin());
		std::ofstream dump("band_precond_z.dump", std::ios_base::app);
		for (unsigned int i=0; i<vec.size(); ++i) {
			dump << std::setw(10) << std::right << vec[i] << std::endl;
		}
		dump << std::endl;
	}
#endif

	return 0;
}


std::size_t PrecondBand::serializationSize() const {
	// one jacobian consists of data storage and pivot array
	size_t jacSize = m_jacobian->serializationSize();
	// we need to copy both Jacobians
	return 2*jacSize;
}


void PrecondBand::serialize(void* & dataPtr) const {
	m_jacobian->serialize(dataPtr);
	m_partialJacobianCopy->serialize(dataPtr);
}


void PrecondBand::deserialize(void* & dataPtr) {
	m_jacobian->deserialize(dataPtr);
	m_partialJacobianCopy->deserialize(dataPtr);
}


} // namespace SOLFRA

