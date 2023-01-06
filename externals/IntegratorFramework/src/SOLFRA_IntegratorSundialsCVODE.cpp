#include "SOLFRA_IntegratorSundialsCVODE.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_assert.h>
#include <IBK_Time.h>
#include <IBK_StopWatch.h>
#include <IBK_FileUtils.h>

#include <sundials/sundials_types.h>
#include <sundials/sundials_direct.h>


// OpenMP NVector only makes sense for really really huge vectors,
// and for medium sized and small vectors (n<250000) it harms performance
#if 0 && defined(_OPENMP)
	#include <nvector/nvector_openmp.h>
	#define N_VNew(x) N_VNew_OpenMP(x, m_numThreads)
	#define NV_DATA(y) NV_DATA_OMP(y)
	#define N_VDestroy(z) N_VDestroy_OpenMP(z)
#else
	#include <nvector/nvector_serial.h>
	#define N_VNew(x) N_VNew_Serial(x)
	#define NV_DATA(y) NV_DATA_S(y)
	#define N_VDestroy(z) N_VDestroy_Serial(z)
#endif


#include <cvode/cvode.h>
#include <cvode/cvode_band.h>
#include <cvode/cvode_dense.h>
#include <cvode/cvode_spgmr.h>
#include <cvode/cvode_spbcgs.h>
#include <cvode/cvode_sptfqmr.h>
#include <cvode/cvode_bandpre.h>
#include <cvode/cvode_serialization.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_band.h>
#include <sundials/sundials_timer.h>

#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_LESInterface.h"
#include "SOLFRA_LESInterfaceIterative.h"
#include "SOLFRA_PrecondInterface.h"
#include "SOLFRA_JacobianInterface.h"

#include "SOLFRA_IntegratorSundialsCVODEImpl.h"

using namespace std;
using namespace IBK;

//#define DUMP_ERROR_ESTIMATES
#ifdef DUMP_ERROR_ESTIMATES
std::map<int,unsigned int> maxErrorCounters; // key = balance equation index, value = counter of how often this BE had highest error estimate
IBK::StopWatch maxErrorDumpCounter; // maxErrorCounters are dumped only every 20 seconds
#endif // DUMP_ERROR_ESTIMATES

namespace SOLFRA {


// *** CVODE WRAPPER FUNCTIONS ***

/*! Wrapper function called from CVode solver which calls the actual solver routine in
	the solver class.
	\param t Current simulation time point in [s].
	\param y Estimated solution vector at time point t.
	\param ydot Vector for the calculated divergences.
	\param user_data Pointer to the IntegratorSundialsCVODEImpl object.
*/
inline int IntegratorSundialsCVODE_f(realtype t, N_Vector y, N_Vector ydot, void *user_data) {
	FUNCID(IntegratorSundialsCVODE_f);
	IntegratorSundialsCVODEImpl * cvodeWrapper = static_cast<IntegratorSundialsCVODEImpl*>(user_data);
	ModelInterface * model = cvodeWrapper->m_model;

	// set new time point
	ModelInterface::CalculationResult res = model->setTime(t);
	if (res != ModelInterface::CalculationSuccess) {
		if (res == ModelInterface::CalculationRecoverableError) {
			IBK::IBK_Message(IBK::FormatString("setTime() function (in physical model) returned with 'recoverable error' at time t=%1").arg(t),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_INFO);
			return 1; // tell CVODE to try again with smaller time step
		}
		else
			return -1; // stop CVODE
	}

	// set new solution variables
	res = model->setY(NV_DATA(y));
	if (res != ModelInterface::CalculationSuccess) {
		if (res == ModelInterface::CalculationRecoverableError) {
			IBK::IBK_Message(IBK::FormatString("setY() function (in physical model) returned with 'recoverable error' at time t=%1").arg(t),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_INFO);
			return 1; // tell CVODE to try again with smaller time step
		}
		else
			return -1; // stop CVODE
	}

	// retrieve divergences
	res = model->ydot(NV_DATA(ydot));
	if (res != ModelInterface::CalculationSuccess) {
		if (res == ModelInterface::CalculationRecoverableError) {
			IBK::IBK_Message(IBK::FormatString("ydot() function (in physical model) returned with 'recoverable error' at time t=%1").arg(t),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_INFO);
			return 1; // tell CVODE to try again with smaller time step
		}
		else
			return -1; // stop CVODE
	}
//	std::cout << "t = " << t << " ydot[0] = " << NV_DATA(ydot)[0] << std::endl;
	return 0; // Success
}
// ---------------------------------------------------------------------------


// Wrapper function for the error weights calculation function.
int CVEwtFn_f(N_Vector y, N_Vector ewt, void *user_data) {
	IntegratorSundialsCVODEImpl * cvodeWrapper =
		static_cast<IntegratorSundialsCVODEImpl*>(user_data);
	// ensure that the model really has error weights defined
	IBK_ASSERT(cvodeWrapper->m_model->hasErrorWeightsFunction() );
	// retrieve error weights from the model
	SOLFRA::ModelInterface::CalculationResult res = cvodeWrapper->m_model->calculateErrorWeights(NV_DATA(y), NV_DATA(ewt));
	(void)res; // to make compiler happy
	// ensure validity of the implementation
	IBK_ASSERT(res == SOLFRA::ModelInterface::CalculationSuccess);
	return 0; //cvodeWrapper->calculateErrorWeights(NV_DATA(y), NV_DATA(ewt));
}
// ---------------------------------------------------------------------------


// Wrapper function for the error handler
void CVErrHandlerFn_f(int error_code, const char * module, const char * function,
	char *msg, void * /*eh_data*/)
{
	if (error_code < 0) {
		IBK::IBK_Message( IBK::FormatString("Error in CVODE Integrator module '%1', function '%2':\n%3")
			.arg(module).arg(function).arg(msg), IBK::MSG_ERROR, "[CVErrHandlerFn_f]");
	}
	else if (error_code > 0) {
		IBK::IBK_Message( IBK::FormatString("CVODE Integrator module '%1', function '%2': %3\n")
			.arg(module).arg(function).arg(msg), IBK::MSG_WARNING, "[CVErrHandlerFn_f]");
	}
}
// ---------------------------------------------------------------------------


int CVSpilsPrecSetupFn_f(realtype t, N_Vector y, N_Vector fy,
	booleantype jok, booleantype *jcurPtr,
	realtype gamma, void *user_data,
	N_Vector tmp1, N_Vector tmp2,
	N_Vector tmp3)
{
	(void)tmp1;
	(void)tmp2;
	(void)tmp3;
	int res = 0;

	IntegratorSundialsCVODEImpl * cvodeMem = reinterpret_cast<IntegratorSundialsCVODEImpl *>(user_data);
	IBK_ASSERT(cvodeMem != nullptr);
	// cast user data to jacobian interface
	JacobianInterface * jacobian = cvodeMem->m_jacobian;
	// update Jacobian if dedicated implementation is provided and if CVODE signals
	// that Jacobian is outdated
	bool jacGenerated = false;
	if (jacobian != nullptr && jok != TRUE) {
		// update df/dy
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_JACOBIAN_GENERATION,
			res = jacobian->setup(t, NV_DATA(y), NV_DATA(fy), nullptr, gamma);
		);
		jacGenerated = true;
	}
	// cast user data to pre-conditioner interface
	PrecondInterface * precond = cvodeMem->m_precond;
	bool jacUpdated = false;
	if (precond != nullptr) {
		// now call setup in precond
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_PRE_SETUP,
			res = precond->setup(t, NV_DATA(y), NV_DATA(fy), nullptr, (jok == TRUE), jacUpdated, gamma);
		);
	}
	if (jacUpdated || jacGenerated )		*jcurPtr |= TRUE;
	else									*jcurPtr = FALSE;
	return res;
}
// ---------------------------------------------------------------------------


int CVSpilsPrecSolveFn_f(realtype t, N_Vector y, N_Vector fy,
	N_Vector r, N_Vector z,
	realtype gamma, realtype delta,
	int lr, void *user_data, N_Vector tmp)
{
	(void)tmp;
	IntegratorSundialsCVODEImpl * cvodeMem = reinterpret_cast<IntegratorSundialsCVODEImpl *>(user_data);
	IBK_ASSERT(cvodeMem != nullptr);
	// cast user data to pre-conditioner interface
	PrecondInterface * precond = cvodeMem->m_precond;
	if (precond != nullptr) {
		// now call setup in precond
		return precond->solve(t, NV_DATA(y), NV_DATA(fy), nullptr, NV_DATA(r), NV_DATA(z), gamma, delta, lr);
	}

	// no preconditioner, simply copy r to z
	//N_VScale(ONE, V[0], vtemp);
	std::memcpy(NV_DATA(z), NV_DATA(r), cvodeMem->m_model->n()*sizeof(double));

	return 0;
}
// ---------------------------------------------------------------------------


int CVSpilsJacTimesVecFn_f(N_Vector v, N_Vector Jv, realtype t,
	N_Vector y, N_Vector fy,
	void *user_data, N_Vector tmp)
{
	(void)tmp;
	(void)fy;
	(void)y;
	(void)t;
	// cast user data to pre-conditioner interface
	JacobianInterface * jacobian = reinterpret_cast<IntegratorSundialsCVODEImpl *>(user_data)->m_jacobian;
	IBK_ASSERT(jacobian != nullptr);
	// now call J*v in precond
	return jacobian->jacTimesVec(NV_DATA(v), NV_DATA(Jv));
}
// ---------------------------------------------------------------------------


// *** Implementation of IntegratorSundialsCVODE ***


IntegratorSundialsCVODE::IntegratorSundialsCVODE() :
	m_impl(nullptr)
{
	m_dtMin				= 0;
	m_dtMax				= 3600;
	m_maxOrder			= 5;
	m_maxSteps			= 100000000;
	m_maxNonLinIters	= 0; // use defaults
	m_nonLinConvCoeff	= 0; // use defaults
	m_stabilityLimitDetectionEnabled = false;

#ifdef DUMP_ERROR_ESTIMATES
	maxErrorDumpCounter.setIntervalLength(15); // every 15 seconds
	maxErrorDumpCounter.start();
#endif
}


IntegratorSundialsCVODE::~IntegratorSundialsCVODE() {
	delete m_impl;
}


void IntegratorSundialsCVODE::init(ModelInterface * model, double t0,
	const double *y0, LESInterface *lesSolver, PrecondInterface *precond, JacobianInterface *jacobian)
{
	FUNCID(IntegratorSundialsCVODE::init);

	if (m_impl != nullptr) {
		delete m_impl;
		m_impl = nullptr;
	}
	m_impl = new IntegratorSundialsCVODEImpl;

	m_impl->m_precond = precond;
	m_impl->m_jacobian = jacobian;

	// store model pointer for later access
	m_impl->m_model = model;
	IBK_ASSERT(m_impl->m_model != nullptr);

	// determine number of elements per balance equation

	// create the vector for the unknowns
	unsigned int n = model->n();
	m_impl->m_yStorage = N_VNew(n);
	m_impl->m_yStorageOutput = N_VNew(n);
	m_impl->m_yStorageLast = N_VNew(n);
	m_impl->m_errEstimates = N_VNew(n);
	m_impl->m_errWeights = N_VNew(n);

	// initialise solver
	IBK_Message( "Initializing CVODE integrator\n", MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;
	m_impl->m_mem = CVodeCreate(CV_BDF, CV_NEWTON);

	// transfer simulation start time
	m_impl->m_t = t0;
	m_impl->m_dt = model->dt0();

	// transfer initial condition
	std::copy(y0, y0+n, NV_DATA(m_impl->m_yStorage));

	// Initialize cvode memory with equation specific absolute tolerances
	int result = CVodeInit(m_impl->m_mem, IntegratorSundialsCVODE_f, m_impl->m_t, m_impl->m_yStorage);
	if (result != CV_SUCCESS)
		throw IBK::Exception("CVodeInit init error.", FUNC_ID);

	// *** set the CVODE options ***
	// set CVODE data pointer to solver object
	CVodeSetUserData(m_impl->m_mem, (void*)m_impl);
	// set CVODE Max-order
	CVodeSetMaxOrd(m_impl->m_mem, (int)m_maxOrder);
	// set CVODE maximum steps before reaching tout
	CVodeSetMaxNumSteps(m_impl->m_mem, m_maxSteps);
	// set CVODE initial step size
	CVodeSetInitStep(m_impl->m_mem, m_impl->m_dt);
	// set CVODE maximum step size
	CVodeSetMaxStep(m_impl->m_mem, m_dtMax);
	// set CVODE minimum step size
	CVodeSetMinStep(m_impl->m_mem, m_dtMin);
	// set CVODE tolerances, first check if we have an error weights calculation function
	if (m_impl->m_model->hasErrorWeightsFunction() ) {
		CVodeWFtolerances(m_impl->m_mem, CVEwtFn_f);
	}
	//check if we have a vector of tolerances
	else if (m_absTolVec.empty())
		CVodeSStolerances(m_impl->m_mem, m_relTol, m_absTol);
	else {
		// resize abstol vector and copy values
		m_impl->m_absTolVec = N_VNew(n);
		if (m_absTolVec.size() < n)
			throw IBK::Exception("Invalid size of absTolVec.", FUNC_ID);
		std::copy(m_absTolVec.begin(), m_absTolVec.end(), NV_DATA(m_impl->m_absTolVec));
		CVodeSVtolerances(m_impl->m_mem, m_relTol, m_impl->m_absTolVec);
	}
	// set CVODE error handler function
	CVodeSetErrHandlerFn(m_impl->m_mem, CVErrHandlerFn_f, (void*)m_impl);
	if (m_nonLinConvCoeff != 0) {
		IBK::IBK_Message( IBK::FormatString("Setting NonlinConvCoef to %1.\n").arg(m_nonLinConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		CVodeSetNonlinConvCoef(m_impl->m_mem, m_nonLinConvCoeff);
	}
	if (m_maxNonLinIters != 0) {
		IBK::IBK_Message( IBK::FormatString("Setting MaxNonLinIters (maxcor) to %1.\n").arg(m_maxNonLinIters), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		CVodeSetMaxNonlinIters(m_impl->m_mem, m_maxNonLinIters);
	}

	if (m_stabilityLimitDetectionEnabled) {
		CVodeSetStabLimDet(m_impl->m_mem, 1);
	}

	// *** Initialize Jacobian matrix generator ***
	if (jacobian != nullptr) {
		IBK::IBK_Message( IBK::FormatString("Initializing Jacobian implementation\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		jacobian->init(model);
	}

	// *** Initialize Linear Equation Solver ***
	IBK::IBK_Message( IBK::FormatString("Initializing LES solver implementation\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	lesSolver->init(model, this, precond, jacobian);

	// *** Initialize Preconditioner and Jacobian matrix ***

	// The SUNDIALS iterative solvers will only use the Jacobian matrix during
	// the setup phase, if a preconditioner is specified.
	// So, even if we de-selected a preconditioner (precond == nullptr), we need
	// to call CVSpilsSetPreconditioner() if we have a jacobian matrix generator.
	// The call back functions CVSpilsPrecSetupFn_f() and CVSpilsPrecSolveFn_f()
	// distinguish automatically between the case jac+precond and jac alone.
	if (dynamic_cast<SOLFRA::LESInterfaceIterative*>(lesSolver) != nullptr &&
			(precond != nullptr || jacobian != nullptr))
	{
		// connect pre-conditioner to CVode
		int res = CVSpilsSetPreconditioner(m_impl->m_mem,
				  CVSpilsPrecSetupFn_f,
				  CVSpilsPrecSolveFn_f);
		if (res != CV_SUCCESS)
			throw IBK::Exception("Error registering CVSpilsPrecSetupFn_f and/or CVSpilsPrecSolveFn_f.", FUNC_ID);
		// set JacTimesVec function when available
		if (jacobian != nullptr) {
			IBK::IBK_Message( IBK::FormatString("Registering own Jacobian implementation with J*v function.\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			res = CVSpilsSetJacTimesVecFn(m_impl->m_mem, CVSpilsJacTimesVecFn_f);
		}
		else {
			IBK::IBK_Message( IBK::FormatString("Using DQ-Approximation for J*v function.\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}

		// now initialize pre-conditioner
		if (precond != nullptr) {
			IBK::IBK_Message( IBK::FormatString("Initializing Preconditioner implementation\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			precond->init(model, this, jacobian);
		}
	}


}


void IntegratorSundialsCVODE::setLinearSetupFrequency(int msbp) {
	CVodeSetLSetupFrequency(m_impl->m_mem, msbp);
}


IntegratorInterface::StepResultType IntegratorSundialsCVODE::step() {

	// if a stop time has been set, set it in CVode with CVodeSetStopTime()...
	if (m_stopTime != 0.0)
		CVodeSetStopTime(m_impl->m_mem, m_stopTime);

	// tell cvode to take a step
	int res = CVode(m_impl->m_mem, m_impl->m_model->tEnd(), m_impl->m_yStorage, &m_impl->m_t, CV_ONE_STEP);

	// if step was successful, collect statistics
	if (res == CV_SUCCESS || res == CV_TSTOP_RETURN) {
		CVodeGetNumSteps(m_impl->m_mem, &m_impl->m_statNumSteps);
		CVodeGetNumRhsEvals(m_impl->m_mem, &m_impl->m_statNumRHSEvals);
		CVodeGetNumLinSolvSetups(m_impl->m_mem, &m_impl->m_statNumLinSetups);
		CVodeGetNonlinSolvStats(m_impl->m_mem, &m_impl->m_statNumNIters, &m_impl->m_statNumNCFails);
		CVodeGetNumErrTestFails(m_impl->m_mem, &m_impl->m_statNumErrFails);
		CVodeGetLastOrder(m_impl->m_mem, &m_impl->m_statMethodOrder);
		CVodeGetLastStep(m_impl->m_mem, &m_impl->m_statTimeStepSize);
		m_impl->m_dt = m_impl->m_statTimeStepSize;

#ifdef DUMP_ERROR_ESTIMATES
//		CVodeGetErrWeights(m_impl->m_mem, m_impl->m_errWeights);
		CVodeGetEstLocalErrors(m_impl->m_mem, m_impl->m_errEstimates);
		// find largest value
		int largestIndex = -1;
		double largest = 0;
		unsigned int l = NV_LENGTH_S(m_impl->m_errEstimates);
		double * data = NV_DATA_S(m_impl->m_errEstimates);

		for (unsigned int i=0; i<l; ++i) {
			double d = std::fabs(data[i]);
			if (d > largest) {
				largestIndex = i;
				largest = d;
			}
		}
		++maxErrorCounters[largestIndex];
		if (maxErrorDumpCounter.intervalCompleted()) {
			for (std::map<int,unsigned int>::const_iterator it = maxErrorCounters.begin(); it != maxErrorCounters.end(); ++it) {
				std::cout << std::setw(6) << std::right << it->first << " : " << it->second << "\n";
			}
			std::cout.flush();
		}
#endif

	}
	else {
		return IntegratorInterface::StepCriticalError;
	}

	return IntegratorInterface::StepSuccess;
}


double IntegratorSundialsCVODE::t() const {
	// return cached time point
	return m_impl->m_t;
}


double IntegratorSundialsCVODE::dt() const {
	// return cached time step
	return m_impl->m_dt;
}


const double * IntegratorSundialsCVODE::yOut(double t_out) const {
	if (t_out == m_impl->m_t)
		return NV_DATA(m_impl->m_yStorage);
	// interpolate data
	int res = CVodeGetDky(m_impl->m_mem, t_out, 0, m_impl->m_yStorageOutput);
	if (res != 0)
		throw IBK::Exception("Cannot interpolate values.", "[IntegratorSundialsCVODE::y_out]");
	return NV_DATA(m_impl->m_yStorageOutput);
}


void IntegratorSundialsCVODE::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	if (doRestart) {
		m_impl->m_statsFileStream = IBK::create_ofstream(logfilePath / "integrator_cvode_stats.tsv", ios_base::app);
	}
	else {
		m_impl->m_statsFileStream = IBK::create_ofstream(logfilePath / "integrator_cvode_stats.tsv");
		std::ostream & out = *(m_impl->m_statsFileStream);
		out << setw(25) << right << "Time [s]" << "\t";
		out << setw(10) << right << "Steps" << "\t";
		out << setw(10) << right << "RhsEvals" << "\t";
		out << setw(10) << right << "LinSetups" << "\t";
		out << setw(8) << right << "NIters" << "\t";
		out << setw(11) << right << "NConvFails" << "\t";
		out << setw(11) << right << "NErrFails" << "\t";
		out << setw(6) << right << "Order" << "\t";
		out << setw(14) << right << "StepSize [s]";
		out << endl;
	}
}


void IntegratorSundialsCVODE::writeStatistics() {
	if (m_impl->m_statNumSteps == 0) return; // nothing to write before the first step
	std::ostream & out = *(m_impl->m_statsFileStream);
	// Time after last step was completed
	out << fixed << setprecision(10) << setw(25) << right << m_impl->m_t << "\t";
	// NSteps
	out << fixed << setprecision(0) << setw(10) << right << m_impl->m_statNumSteps << "\t";
	// NRhsEvals
	out << fixed << setprecision(0) << setw(10) << right << m_impl->m_statNumRHSEvals << "\t";
	// NLinSetups
	out << fixed << setprecision(0) << setw(10) << right << m_impl->m_statNumLinSetups << "\t";
	// NNIters
	out << fixed << setprecision(0) << setw(8) << right << m_impl->m_statNumNIters << "\t";
	// NNCFails
	out << fixed << setprecision(0) << setw(11) << right << m_impl->m_statNumNCFails << "\t";
	// ErrFails
	out << fixed << setprecision(0) << setw(11) << right << m_impl->m_statNumErrFails << "\t";
	// Order
	out << setw(6) << right << m_impl->m_statMethodOrder << "\t";
	// StepSize
	if (m_impl->m_statTimeStepSize < 1e-5)
		out << setprecision(8) << scientific;
	else
		out << setprecision(6) << fixed;
	out << setw(14) << right << m_impl->m_statTimeStepSize;

	out << std::endl;
}


void IntegratorSundialsCVODE::writeMetrics(double simtime, std::ostream * metricsFile) {
	FUNCID(IntegratorSundialsCVODE::writeMetrics);
	std::string ustr = IBK::Time::suitableTimeUnit(simtime);
	IBK::IBK_Message( IBK::FormatString("Integrator: Steps                          =                          %1\n")
		.arg((unsigned int)m_impl->m_statNumSteps,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Integrator: Newton iterations              =                          %1\n")
		.arg((unsigned int)m_impl->m_statNumNIters,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Integrator: Newton convergence failures    =                          %1\n")
		.arg((unsigned int)m_impl->m_statNumNCFails,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Integrator: Error test failures            =                          %1\n")
		.arg((unsigned int)m_impl->m_statNumErrFails,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tfeval = TimerSum(SUNDIALS_TIMER_FEVAL);
	IBK::IBK_Message( IBK::FormatString("Integrator: Function evaluation (Newton)   = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tfeval, ustr, true),13)
		.arg(tfeval/simtime*100, 5, 'f', 2)
		.arg(stats(StatNumRHSEvals),8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tsetup = TimerSum(SUNDIALS_TIMER_LS_SETUP);
	IBK::IBK_Message( IBK::FormatString("Integrator: LES setup                      = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tsetup, ustr, true),13)
		.arg(tsetup/simtime*100, 5, 'f', 2)
		.arg((unsigned int)m_impl->m_statNumLinSetups,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tsolve = TimerSum(SUNDIALS_TIMER_LS_SOLVE);
	IBK::IBK_Message( IBK::FormatString("Integrator: LES solve                      = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tsolve, ustr, true),13)
		.arg(tsolve/simtime*100, 5, 'f', 2)
		.arg((unsigned int)m_impl->m_statNumNIters,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	if (metricsFile != nullptr) {
		*metricsFile << "IntegratorSteps=" << m_impl->m_statNumSteps << std::endl;
		*metricsFile << "IntegratorErrorTestFails=" << m_impl->m_statNumErrFails << std::endl;
		*metricsFile << "IntegratorNonLinearConvFails=" << m_impl->m_statNumNCFails << std::endl;
		*metricsFile << "IntegratorFunctionEvals=" << stats(StatNumRHSEvals) << std::endl;
		*metricsFile << "IntegratorTimeFunctionEvals=" << tfeval << std::endl;
		*metricsFile << "IntegratorLESSetup=" << m_impl->m_statNumLinSetups << std::endl;
		*metricsFile << "IntegratorTimeLESSetup=" << tsetup << std::endl;
		*metricsFile << "IntegratorLESSolve=" << m_impl->m_statNumNIters << std::endl;
		*metricsFile << "IntegratorTimeLESSolve=" << tsolve << std::endl;
	}
}


void * IntegratorSundialsCVODE::cvodeMem() const {
	if (m_impl == nullptr)
		return nullptr;
	else
		return m_impl->m_mem;
}



unsigned int IntegratorSundialsCVODE::stats(StatisticsType statType) const {
	long n = 0, n2 = 0;
	switch (statType) {
		case StatNumSteps			: CVodeGetNumSteps(m_impl->m_mem, &n); return n;
		case StatNumNIters			: CVodeGetNonlinSolvStats(m_impl->m_mem, &n, &n2); return n;
		case StatNumNCFails			: CVodeGetNonlinSolvStats(m_impl->m_mem, &n, &n2); return n2;
		case StatNumNErrFails		: CVodeGetNumErrTestFails(m_impl->m_mem, &n); return n;
		case StatNumRHSEvals		: CVodeGetNumRhsEvals(m_impl->m_mem, &n); return n;
		case StatNumDlsJacEvals		: CVDlsGetNumJacEvals(m_impl->m_mem, &n); return n;
		case StatNumDlsRHSEvals		: CVDlsGetNumRhsEvals(m_impl->m_mem, &n); return n;
		case StatNumLinIters		: CVSpilsGetNumLinIters(m_impl->m_mem, &n); return n;
		case StatNumLinCFails		: CVSpilsGetNumConvFails(m_impl->m_mem, &n); return n;
		case StatNumLinPrecEvals	: CVSpilsGetNumPrecEvals(m_impl->m_mem, &n); return n;
		case StatNumLinPrecSolves	: CVSpilsGetNumPrecSolves(m_impl->m_mem, &n); return n;
		case StatNumLinRHSEvals		: CVSpilsGetNumRhsEvals(m_impl->m_mem, &n); return n;
		case StatNumLinJtimesEvals	: CVSpilsGetNumJtimesEvals(m_impl->m_mem, &n); return n;
	}
	return 0;
}


std::size_t IntegratorSundialsCVODE::serializationSize() const {
	std::size_t s = CVodeSerializationSize(m_impl->m_mem);
	s += 2*sizeof(double); // also cache m_t and m_dt
	// store m_yStorage vector
	CVODE_SERIALIZE_NVECTOR(SUNDIALS_SERIALIZATION_OPERATION_SIZE, nullptr, m_impl->m_yStorage, s);

	return s;
}


void IntegratorSundialsCVODE::serialize(void* & dataPtr) const {
	CVodeSerialize(m_impl->m_mem, &dataPtr); // ignoring return value for now
	// store m_yStorage vector
	size_t s = 0; // not use during serialization
	CVODE_SERIALIZE_NVECTOR(SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE, &dataPtr, m_impl->m_yStorage, s);
	*(double*)dataPtr = m_impl->m_t;
	dataPtr = (char*)dataPtr + sizeof(double);
	*(double*)dataPtr = m_impl->m_dt;
	dataPtr = (char*)dataPtr + sizeof(double);
}


void IntegratorSundialsCVODE::deserialize(void* & dataPtr) {
	CVodeDeserialize(m_impl->m_mem, &dataPtr); // ignoring return value for now
	size_t s=0; // not use during de-serialization
	CVODE_SERIALIZE_NVECTOR(SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE, &dataPtr, m_impl->m_yStorage, s);
	// update cached m_t and m_dt
	m_impl->m_t = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
	m_impl->m_dt = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
}



// *** Implementation of IntegratorSundialsCVODEImpl ***

IntegratorSundialsCVODEImpl::IntegratorSundialsCVODEImpl() :
	m_mem(nullptr),
	m_yStorage(nullptr),
	m_yStorageOutput(nullptr),
	m_yStorageLast(nullptr),
	m_errEstimates(nullptr),
	m_absTolVec(nullptr),
	m_errWeights(nullptr),
	m_statsFileStream(nullptr),
	m_statNumSteps(0),
	m_statNumRHSEvals(0),
	m_statNumLinSetups(0),
	m_statNumNIters(0),
	m_statNumNCFails(0),
	m_statNumErrFails(0)
{
}


IntegratorSundialsCVODEImpl::~IntegratorSundialsCVODEImpl() {
	if (m_mem!=nullptr)
		CVodeFree(&m_mem);
	if (m_yStorage!=nullptr)
		N_VDestroy(m_yStorage);
	if (m_yStorageOutput!=nullptr)
		N_VDestroy(m_yStorageOutput);
	if (m_yStorageLast!=nullptr)
		N_VDestroy(m_yStorageLast);
	if (m_errEstimates!=nullptr)
		N_VDestroy(m_errEstimates);
	if (m_absTolVec!=nullptr)
		N_VDestroy(m_absTolVec);
	if (m_errWeights!=nullptr)
		N_VDestroy(m_errWeights);

	m_mem = nullptr;
	m_yStorage = nullptr;
	m_yStorageOutput = nullptr;
	m_yStorageLast = nullptr;
	m_errEstimates = nullptr;
	m_absTolVec = nullptr;
	m_errWeights = nullptr;

	delete m_statsFileStream;
}


} // namespace SOLFRA

