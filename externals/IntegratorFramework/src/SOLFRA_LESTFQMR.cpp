#include "SOLFRA_LESTFQMR.h"

#include <iomanip>
#include <iostream>
#include <fstream>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_PrecondInterface.h"

#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>
#include <nvector/nvector_serial.h>

#include <cvode/cvode.h>
#include <cvode/cvode_sptfqmr.h>
#include <cvode/cvode_spils.h>
#include <cvode/cvode_serialization.h>


#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>

namespace SOLFRA {

LESTFQMR::LESTFQMR()
{
}


LESTFQMR::~LESTFQMR() {

}


void LESTFQMR::init(ModelInterface * model, IntegratorInterface * integrator,
					PrecondInterface * precond, JacobianInterface * jacobian)
{
	const char * const FUNC_ID = "[LESTFQMR::init]";

	m_integrator = integrator;
	m_model      = model;
	m_precond    = precond;
	m_jacobian   = jacobian;

	// determine which preconditioner we are using
	if (precond == nullptr)
		IBK::IBK_Message(IBK::FormatString("No Preconditioner is set for LESTFQMR linear equation solver."), IBK::MSG_WARNING, FUNC_ID, IBK::VL_ALL );

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// create and register CVSptfqmr with integrator
		int sundialsPrecType = PREC_NONE; // by default preconditioner is disabled
		// if we have a Jacobian matrix implementation, we need the preconditioner to be a dummy left-sided Preconditioner
		if (jacobian != nullptr)
			sundialsPrecType = PREC_LEFT;
		if (precond != nullptr) {
			switch (precond->preconditionerType()) {
				case PrecondInterface::Left		: sundialsPrecType = PREC_LEFT; break;
				case PrecondInterface::Right	: sundialsPrecType = PREC_RIGHT; break;
				case PrecondInterface::Both		: sundialsPrecType = PREC_BOTH; break;
				case PrecondInterface::None		:
					throw IBK::Exception(IBK::FormatString("Error initialising preconditioner for LESTFQMR linear equation solver: "
														"Preconditioner attribute 'Left', 'Right' or 'Both is enforced."), FUNC_ID);
				default							:  break;
			}
		}
		int result = CVSptfqmr(intCVODE->cvodeMem(), sundialsPrecType, m_maxKrylovDim);
		if (result != CVSPILS_SUCCESS) {
			if (result == CVSPILS_MEM_FAIL)
				IBK::IBK_Message( "CVSptfqmr memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVSPILS_ILL_INPUT)
				IBK::IBK_Message( "CVSptfqmr init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVSptfqmr init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}

		// *** Set Linear Iteration Factor (EpsLin) ***
		if (m_linIterConvCoeff != 0) {
			IBK::IBK_Message( IBK::FormatString("Setting EpsLin (eplifac,linIterConvCoeff) to %1.\n").arg(m_linIterConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			CVSpilsSetEpsLin(intCVODE->cvodeMem(), m_linIterConvCoeff);
		}

		return;
	}

	throw IBK::Exception(IBK::FormatString("Error initializing LESTFQMR linear solver: "
			"Only 'IntegratorSundialsCVODE' is currently supported."), FUNC_ID);

}


void LESTFQMR::setup(const double * y, const double * ydot, const double * residuals, double gamma) {
	(void)y;
	(void)ydot;
	(void)residuals;
	(void)gamma;
	// implementation is not needed, since CVODE uses internal communication to provided LES solver
}


void LESTFQMR::solve(double * rhs) {
	(void)rhs;
	// implementation is not needed, since CVODE uses internal communication to provided LES solver
}


void LESTFQMR::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	LESInterfaceIterative::writeStatisticsHeader(logfilePath, doRestart);
}


void LESTFQMR::writeStatistics(double t)  {
	LESInterfaceIterative::writeStatistics(t);
}


std::size_t LESTFQMR::serializationSize() const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// we are using the CVSpgmr solver
		std::size_t s = CVSpilsSerializationSize(intCVODE->cvodeMem());
		return s;
	}
	else {
		return 0; // serialization not supported
	}
}


void LESTFQMR::serialize(void* & dataPtr) const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVSpilsSerialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}


void LESTFQMR::deserialize(void* & dataPtr) {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVSpilsDeserialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}


} // namespace SOLFRA

