#include "SOLFRA_JacobianDense.h"

#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_Constants.h"

#include <fstream>
#include <iomanip>

#include <IBK_Exception.h>
#include <IBK_assert.h>
#include <IBK_InputOutput.h>

namespace SOLFRA {

JacobianDense::JacobianDense() :
	m_relToleranceDQ(1e-8),
	m_absToleranceDQ(1e-8),
	m_model(nullptr),
	m_nRhsEvals(0)
{
}

void JacobianDense::init(ModelInterface * model) {
//	const char * const FUNC_ID = "[JacobianDense::init]";

	// retreive pointer to the current model
	m_model = model;

	// resize all vectors
	resize(model->n());
	m_yMod.resize(model->n());
	m_ydotMod.resize(model->n());
	m_ydiff.resize(model->n());
}


int JacobianDense::setup(	double t,
							const double * y,
							const double * ydot,
							const double * residuals,
							double gamma
						)
{
	(void)t;
	(void)residuals;
	(void)gamma;

	// recompute Jacobian
	setZero();

	// store current solution guess
	std::memcpy(&m_yMod[0], y, m_yMod.size()*sizeof(double));

	// compute (partial) Jacobian as J = df/dy via directional derivatives

	for (unsigned int i=0; i<n(); ++i) {

		// modify y value in row i
		m_ydiff[i] = std::fabs(y[i])*m_relToleranceDQ + m_absToleranceDQ;
		m_yMod[i] += m_ydiff[i];

		// calculate modified right hand side
		m_model->setY(&m_yMod[0]);
		// calculate modified right hand side of the model, and store f(t,y) in m_ydotMod
		m_model->ydot(&m_ydotMod[0]);
		// update statistics
		++m_nRhsEvals;

		// compute Jacobian elements in all columns
		// df/dy = (f(y+eps) - f(y) )/eps
		for (unsigned int j=0; j<n(); ++j) {
			// compute finite-differences column j in row i
			(*this)(j,i) = ( m_ydotMod[j] - ydot[j] )/m_ydiff[i];
		}

		m_yMod[i] = y[i];

	} // for i

	// m_jacobian now holds df/dy

// dump defines are defined in SOLFRA_JacobianInterface.h
#ifdef DUMP_JACOBIAN_TEXT
	std::ofstream jacdump("jacobian_dense.txt");
	write(jacdump, nullptr, false, 15);
	jacdump.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[JacobianDense::setup]");
#endif
#ifdef DUMP_JACOBIAN_BINARY
	IBK::write_matrix_binary(*this, "jacobian_dense.bin");
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESDense::setup]");
#endif

	return 0;
}


std::size_t JacobianDense::serializationSize() const {
	// we only need to serialize the actual data and statistics
	return IBKMK::DenseMatrix::serializationSize() + sizeof(unsigned int);
}


void JacobianDense::serialize(void* & dataPtr) const {
	IBKMK::DenseMatrix::serialize(dataPtr);
	*(unsigned int*)dataPtr = m_nRhsEvals;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}


void JacobianDense::deserialize(void* & dataPtr) {
	const char * const FUNC_ID = "[JacobianSparseEID::deserialize]";
	try {
		IBKMK::DenseMatrix::deserialize(dataPtr);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error in Dense Jacobian deserialization.", FUNC_ID);
	}
	m_nRhsEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}


void JacobianDense::recreate(void* & dataPtr) {
	const char * const FUNC_ID = "[JacobianSparseEID::recreate]";
	try {
		IBKMK::DenseMatrix::recreate(dataPtr);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error in Dense Jacobian recreation.", FUNC_ID);
	}
	m_nRhsEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}

} // namespace SOLFRA

