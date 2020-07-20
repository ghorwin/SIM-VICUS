#include "SOLFRA_PrecondILU.h"

#include <fstream>
#include <iomanip>
#include <cmath>

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_SparseMatrix.h>
#include <IBKMKC_vector_operations.h>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_JacobianSparse.h"
#include "SOLFRA_ModelInterface.h"

namespace SOLFRA {

PrecondILU::PrecondILU(PreconditionerType precondType) :
	PrecondInterface(precondType),
	m_jacobianSparse(nullptr),
	m_precondMatrix(nullptr)
{
}


PrecondILU::~PrecondILU() {
	if (m_precondMatrix != nullptr)
		delete m_precondMatrix;
}


void PrecondILU::init(ModelInterface * model, IntegratorInterface * integrator,
					  const JacobianInterface *jacobianInterface)
{
	//const char * const FUNC_ID = "[PrecondILU::init]";

	// initialize all variables needed for Jacobian
	m_integrator = integrator;
	m_model = model;

	// transfer jacobian matrix and ensure it is a sparse Jacobian implementation
	m_jacobianSparse = dynamic_cast<const JacobianSparse*>(jacobianInterface);
	IBK_ASSERT(m_jacobianSparse != nullptr);

	// let Jacobian implementation construct a copy for us and transfer ownership
	m_precondMatrix = m_jacobianSparse->createAndReleaseJacobianCopy();
}


int PrecondILU::setup(double t, const double * y, const double * ydot, const double * residuals,
	bool jacOk, bool & jacUpdated, double gamma)
{
	(void)t;
	(void)y;
	(void)ydot;
	(void)residuals;
	(void)jacUpdated;
	(void)jacOk;

	// restore partial Jacobian data in our local copy
	double * jacData = m_precondMatrix->data();
	IBKMK::vector_copy(m_precondMatrix->dataSize(), m_jacobianSparse->jacobian()->data(), jacData);
//#define DUMP_SPARSE_JACOBIANS
#ifdef DUMP_SPARSE_JACOBIANS
		std::ofstream jacdump2("jacobian_dfdy_sparse_CSR_from_ILU.txt");
		m_precondMatrix->write(jacdump2, nullptr, false, 15);
		jacdump2.close();
#endif

	// scale with -gamma
	IBKMK::vector_scale_by(m_precondMatrix->dataSize(), -gamma, jacData);

	// add identity matrix
	m_precondMatrix->addIdentityMatrix();

#ifdef DUMP_SPARSE_JACOBIANS
		std::ofstream jacdump1("jacobian_sparse_CSR_from_ILU.txt");
		m_precondMatrix->write(jacdump1, nullptr, false, 15);
		jacdump1.close();
#endif
	// perform incomplete LU-factorization
	m_precondMatrix->ilu();

#ifdef DUMP_SPARSE_JACOBIANS
		std::ofstream jacdump("jacobian_sparse_CSR_from_ILU_factorized.txt");
		m_precondMatrix->write(jacdump, nullptr, false, 15);
		jacdump.close();
		throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondILU::setup]");
#endif
	return 0;
}


int PrecondILU::solve(double t, const double * y, const double * ydot, const double * residuals,
	const double * r, double * z, double gamma, double delta, int lr)
{
	// copy right hand side to z-vector
	IBKMK::vector_copy(m_precondMatrix->n(), r, z);
#if 0
	std::vector<double> vec(r, r + m_model->n());
	std::ofstream dump("ilu_precond.dump", std::ios_base::app);
	dump << std::setw(10) << std::right << t;
	dump << std::setw(10) << std::right << gamma;
	dump << std::setw(10) << std::right << delta;
	dump << std::setw(10) << std::right << lr;
	for (unsigned int i=0; i<vec.size(); ++i) {
		dump << std::setw(10) << std::right << vec[i];
	}
	dump << std::endl;
#else
	(void)t;
	(void)y;
	(void)ydot;
	(void)residuals;
	(void)gamma;
	(void)delta;
	(void)lr;
#endif

	// fill z-vector with solution of the linear system
	m_precondMatrix->backsolveILU(z);
	return 0;
}


std::size_t PrecondILU::serializationSize() const {
	// we only need to serialize the actual data and statistics
	return m_precondMatrix->dataSize() *sizeof(double);
}


void PrecondILU::serialize(void* & dataPtr) const {
	std::memcpy(dataPtr, m_precondMatrix->data(), m_precondMatrix->dataSize()*sizeof(double));
	dataPtr = (char*)dataPtr + m_precondMatrix->dataSize()*sizeof(double);
}


void PrecondILU::deserialize(void* & dataPtr) {
	std::memcpy(m_precondMatrix->data(), dataPtr, m_precondMatrix->dataSize()*sizeof(double));
	dataPtr = (char*)dataPtr + m_precondMatrix->dataSize()*sizeof(double);
}


} // namespace SOLFRA

