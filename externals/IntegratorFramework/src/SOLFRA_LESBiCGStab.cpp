#include "SOLFRA_LESBiCGStab.h"

#include <cstring>
#include <cmath>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_JacobianInterface.h"
#include "SOLFRA_PrecondInterface.h"

#include <cvode/cvode_spbcgs.h>
#include <cvode/cvode_spils.h>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>

namespace SOLFRA {


void LESBiCGStab::init(ModelInterface *  model, IntegratorInterface * integrator, PrecondInterface * precond, JacobianInterface *jacobian) {
	const char * const FUNC_ID = "[LESBiCGStab::init]";

	m_integrator	= integrator;
	m_jacobian		= jacobian;
	m_model			= model;
	m_precond		= precond;

	// determine which preconditioner we are using
	if (precond == NULL)
		IBK::IBK_Message(IBK::FormatString("No Preconditioner is set for LESBiCGStab linear equation solver."), IBK::MSG_WARNING, FUNC_ID, IBK::VL_ALL );

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// create and register CVSpbcg with integrator
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
					throw IBK::Exception(IBK::FormatString("Error initialising preconditioner for LESBiCGStab linear equation solver: "
														"Preconditioner attribute 'Left', 'Right' or 'Both is enforced."), FUNC_ID);
				default							:  break;
			}
		}
		int result = CVSpbcg(intCVODE->cvodeMem(), sundialsPrecType, m_maxKrylovDim);
		if (result != CVSPILS_SUCCESS) {
			if (result == CVSPILS_MEM_FAIL)
				IBK::IBK_Message( "CVSpbcg memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVSPILS_ILL_INPUT)
				IBK::IBK_Message( "CVSpbcg init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVSpbcg init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}

		// *** Set Linear Iteration Factor (EpsLin) ***
		if (m_linIterConvCoeff != 0) {
			IBK::IBK_Message( IBK::FormatString("Setting EpsLin (eplifac,linIterConvCoeff) to %1.\n").arg(m_linIterConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			CVSpilsSetEpsLin(intCVODE->cvodeMem(), m_linIterConvCoeff);
		}

		return;
	}

	throw IBK::Exception(IBK::FormatString("Error initializing LESBiCGStab linear solver: "
			"Only 'IntegratorSundialsCVODE' is currently supported."), FUNC_ID);
}


void LESBiCGStab::setup(const double *, const double *, const double *, double) {
	return;
}


void LESBiCGStab::solve( double * ) {
	return;
}



} // namespace SOLFRA

