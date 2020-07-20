#include "SOLFRA_PrecondILUT.h"

#include "SOLFRA_IntegratorInterface.h"
#include "SOLFRA_JacobianSparseCSR.h"
#include "SOLFRA_ModelInterface.h"

#include <IBKMK_SparseMatrixCSR.h>
#include <IBKMKC_vector_operations.h>

#include <IBK_assert.h>

#include <IBKMKC_ilut.h>

#include <fstream>
#include <iomanip>
#include <cmath>

namespace SOLFRA {

class ILUTWrapper {
public:
	ILUTWrapper(IBKMK::ILUfac * mat) : m_mat(mat) {}

	unsigned int n() const { return m_mat->n; }
	double value(int i, int j) const {
		// check if value is present in the matrix
		if (i == j)
			return m_mat->D[i];
		else if (i<j) {
			// upper diagonal, search for j-index in current row
			for (int k=0; k< m_mat->U->nzcount[i]; ++k)
				if (m_mat->U->ja[i][k] == j)
					return m_mat->U->ma[i][k];
		}
		else {
			// lower diagonal
			// search for j-index in current row
			for (int k=0; k< m_mat->L->nzcount[i]; ++k)
				if (m_mat->L->ja[i][k] == j)
					return m_mat->L->ma[i][k];
		}

		return 0;
	}

	IBKMK::ILUfac * m_mat;
};

PrecondILUT::PrecondILUT(PreconditionerType precondType, unsigned int maxFillinLevel) :
	PrecondInterface(precondType),
	m_jacobianSparse(nullptr),
	m_precondMatrixCSR(nullptr),
	m_itsolMatrix(nullptr),
	m_factorizedItsolMatrix(nullptr),
	m_maxLevelOfFillIn(maxFillinLevel),
	m_tolerance(1e-3)
{
}


PrecondILUT::~PrecondILUT() {
	if (m_precondMatrixCSR != nullptr)
		delete m_precondMatrixCSR;
	if (m_itsolMatrix != nullptr)
		delete m_itsolMatrix;
	if (m_factorizedItsolMatrix != nullptr)
		delete m_factorizedItsolMatrix;
}


void PrecondILUT::init(SOLFRA::ModelInterface * model, SOLFRA::IntegratorInterface * integrator,
					  const SOLFRA::JacobianInterface *jacobianInterface)
{
	//const char * const FUNC_ID = "[PrecondILUT::init]";

	// initialize all variables needed for Jacobian
	m_integrator = integrator;
	m_model = model;

	// store pointer to jacobian interface - dynamic conversion done to JacobianSparse so that we can
	// access member functions createAndReleaseJacobianCopy() and jacobian().
	m_jacobianSparse = dynamic_cast<const SOLFRA::JacobianSparse*>(jacobianInterface);

	// transfer jacobian matrix and ensure it is a sparse Jacobian implementation
	// do we have a CSR Jacobian interface?
	const SOLFRA::JacobianSparseCSR * jacCSR = dynamic_cast<const SOLFRA::JacobianSparseCSR*>(jacobianInterface);
	IBK_ASSERT (jacCSR != nullptr);

	// let Jacobian implementation construct a copy for us and transfer ownership
	m_precondMatrixCSR = dynamic_cast<IBKMK::SparseMatrixCSR*>(jacCSR->createAndReleaseJacobianCopy());
	IBK_ASSERT(m_precondMatrixCSR != nullptr);

	// create a itsol sparse m,atrix and copy precond matrix pointer
	m_itsolMatrix = new IBKMK::SpaFmt;
	// resize matrix
	unsigned int n = m_precondMatrixCSR->n();
	m_itsolMatrix->n = n;
	m_itsolMatrix->nzcount = (int *)malloc(n * sizeof(int));

	m_itsolMatrix->ja = (int **)std::malloc(n * sizeof(int*));
	m_itsolMatrix->ma = (double **)std::malloc(n * sizeof(double*));

	unsigned int jStart = 0;
	for (unsigned int i = 0; i < n; ++i) {

		unsigned int nzRow = m_precondMatrixCSR->ia()[i + 1] - m_precondMatrixCSR->ia()[i];
		m_itsolMatrix->nzcount[i] = nzRow;

		m_itsolMatrix->ja[i] = (int *)(m_precondMatrixCSR->ja() + jStart);
		m_itsolMatrix->ma[i] = (m_precondMatrixCSR->data() + jStart);

		// update start position
		jStart += nzRow;
	}

	m_factorizedItsolMatrix = new IBKMK::ILUfac;
	m_factorizedItsolMatrix->L = nullptr;
	m_factorizedItsolMatrix->U = nullptr;
	m_factorizedItsolMatrix->D = nullptr;
	m_factorizedItsolMatrix->work = nullptr;
}


int PrecondILUT::setup(double t, const double * y, const double * ydot, const double * residuals,
	bool jacOk, bool & jacUpdated, double gamma)
{
	(void)t;
	(void)y;
	(void)ydot;
	(void)residuals;
	(void)jacUpdated;
	(void)jacOk;

	double * jacData = nullptr;

	// restore partial Jacobian data in our local copy
	jacData = m_precondMatrixCSR->data();
	IBKMK::vector_copy(m_precondMatrixCSR->dataSize(), m_jacobianSparse->jacobian()->data(), jacData);

//#define DUMP_SPARSE_JACOBIANS
#ifdef DUMP_SPARSE_JACOBIANS
	std::ofstream jacdump("jacobian_dfdy_sparse_CSR_from_ILUT.txt");
	m_precondMatrixCSR->write(jacdump, nullptr, false, 15);
	jacdump.close();
#endif

	// scale with -gamma
	IBKMK::vector_scale_by(m_precondMatrixCSR->dataSize(), -gamma, jacData);

	// add identity matrix to get M = I - gamma * J = I - gamma * df/dy
	m_precondMatrixCSR->addIdentityMatrix();

#ifdef DUMP_SPARSE_JACOBIANS
		std::ofstream jacdump1("jacobian_sparse_CSR_from_ILUT.txt");
		m_precondMatrixCSR->write(jacdump1, nullptr, false, 15);
		jacdump1.close();
#endif

	IBKMK::ibkmk_free_ilut(m_factorizedItsolMatrix);

	// perform incomplete incomplete LU-factorization
	IBKMK::ibkmk_ilut(m_itsolMatrix, m_factorizedItsolMatrix, m_maxLevelOfFillIn, m_tolerance, stdout);

#ifdef DUMP_SPARSE_JACOBIANS
		std::ofstream jacdump2("jacobian_sparse_CSR_from_ILUT_factorized.txt");
		ILUTWrapper wrapper(m_factorizedItsolMatrix);
		IBK::write_matrix(jacdump2, wrapper, nullptr, false, 15, "A", "b");
		jacdump2.close();
		throw IBK::Exception("Done with test-dump of Jacobian", "[PrecondILU::setup]");
#endif

	return 0;
}


int PrecondILUT::solve(double t, const double * y, const double * ydot, const double * residuals,
	const double * r, double * z, double gamma, double delta, int lr)
{
	// copy right hand side to z-vector
	IBKMK::vector_copy(m_precondMatrixCSR->n(), r, z);
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
	IBKMK::ibkmk_lutsolve(z, z, m_factorizedItsolMatrix);
	return 0;
}


std::size_t PrecondILUT::serializationSize() const {
	// we only need to serialize the actual data and statistics
	return m_precondMatrixCSR->dataSize() *sizeof(double);
}


void PrecondILUT::serialize(void* & dataPtr) const {
	std::memcpy(dataPtr, m_precondMatrixCSR->data(), m_precondMatrixCSR->dataSize()*sizeof(double));
	dataPtr = (char*)dataPtr + m_precondMatrixCSR->dataSize()*sizeof(double);
}


void PrecondILUT::deserialize(void* & dataPtr) {
	std::memcpy(m_precondMatrixCSR->data(), dataPtr, m_precondMatrixCSR->dataSize()*sizeof(double));
	dataPtr = (char*)dataPtr + m_precondMatrixCSR->dataSize()*sizeof(double);
}


} // namespace SOLFRA




