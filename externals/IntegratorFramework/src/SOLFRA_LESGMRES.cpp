#include "SOLFRA_LESGMRES.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_JacobianInterface.h"
#include "SOLFRA_PrecondInterface.h"

#include <cvode/cvode_spgmr.h>
#include <cvode/cvode_spils.h>
#include <cvode/cvode_serialization.h>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_math.h>
#include <IBK_messages.h>

//#define DEBUG_ERROR_TEST

namespace SOLFRA {

void LESGMRES::init(ModelInterface * model, IntegratorInterface * integrator, PrecondInterface * precond,
					JacobianInterface * jacobian)
{
	const char * const FUNC_ID = "[LESGMRES::init]";

	m_integrator = integrator;
	m_model      = model;
	m_precond    = precond;
	m_jacobian   = jacobian;


	// determine which preconditioner we are using
	if (precond == NULL)
		IBK::IBK_Message(IBK::FormatString("No Preconditioner is set for LESGMRES linear equation solver."), IBK::MSG_WARNING, FUNC_ID, IBK::VL_ALL );

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// create and register CVSpgmr with integrator
		int sundialsPrecType = PREC_NONE; // by default preconditioner is disabled
		// if we have a Jacobian matrix implementation, we need the preconditioner to be a dummy left-sided Preconditioner
		if (jacobian != NULL)
			sundialsPrecType = PREC_LEFT;
		if (precond != NULL) {
			switch (precond->preconditionerType()) {
				case PrecondInterface::Left		: sundialsPrecType = PREC_LEFT; break;
				case PrecondInterface::Right	: sundialsPrecType = PREC_RIGHT; break;
				case PrecondInterface::Both		: sundialsPrecType = PREC_BOTH; break;
				case PrecondInterface::None		:
					throw IBK::Exception(IBK::FormatString("Error initialising preconditioner for LESGMRES linear equation solver: "
														"Preconditioner attribute 'Left', 'Right' or 'Both is enforced."), FUNC_ID);
				default							:  break;
			}
		}
		int result = CVSpgmr(intCVODE->cvodeMem(), sundialsPrecType, m_maxKrylovDim);
		if (result != CVSPILS_SUCCESS) {
			if (result == CVSPILS_MEM_FAIL)
				IBK::IBK_Message( "CVSpgmr memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVSPILS_ILL_INPUT)
				IBK::IBK_Message( "CVSpgmr init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVSpgmr init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}

		// *** Set Linear Iteration Factor (EpsLin) ***
		if (m_linIterConvCoeff != 0) {
			IBK::IBK_Message( IBK::FormatString("Setting EpsLin (eplifac,linIterConvCoeff) to %1.\n").arg(m_linIterConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			CVSpilsSetEpsLin(intCVODE->cvodeMem(), m_linIterConvCoeff);
		}
	}
	else {
		throw IBK::Exception(IBK::FormatString("Error initializing Sundials GMRES linear solver: "
			"Only 'IntegratorSundialsIDA' or 'IntegratorSundialsCVODE' "
			"are suitable integrators."), FUNC_ID);
	}
}


void LESGMRES::setup(const double *, const double *, const double *, double) {
	return;
}


void LESGMRES::solve(double * /*rhs*/) {
	return;
}


std::size_t LESGMRES::serializationSize() const {
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


void LESGMRES::serialize(void* & dataPtr) const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVSpilsSerialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}


void LESGMRES::deserialize(void* & dataPtr) {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVSpilsDeserialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}

} // namespace SOLFRA

