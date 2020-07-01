/*
 * -----------------------------------------------------------------
 * $Revision: 1.2 $
 * $Date: 2006/11/29 00:05:07 $
 * -----------------------------------------------------------------
 * Programmer(s): Andreas Nicolai
 * -----------------------------------------------------------------
 * Copyright (c) 2014, Technische Universität Dresden, Germany
 * All rights reserved.
 * For details, see the LICENSE file.
 * -----------------------------------------------------------------
 * This is the header file for the SUNDIALS timers.
 *------------------------------------------------------------------
 */

#ifndef _SUNDIALSTIMERS_H
#define _SUNDIALSTIMERS_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#ifndef _SUNDIALS_CONFIG_H
#define _SUNDIALS_CONFIG_H
#include <sundials/sundials_config.h>
#endif

#include <sundials/sundials_types.h>

/*
 * -----------------------------------------------------------------
 * Function : TimerStart
 * -----------------------------------------------------------------
 * A call to the TimerStart function starts the timer with index idx.
 *
 * idx is the number of the timer to start (idx < MAX_TIMER_INDEX).
 *     If the timer with this index is already in use, it is restarted.
 *
 * The function does not return anything.
 * -----------------------------------------------------------------
 */
void TimerStart(int idx);

/*
 * -----------------------------------------------------------------
 * Function : TimerStop
 * -----------------------------------------------------------------
 * A call to the TimerStop function stops the timer with index idx.
 *
 * idx is the number of the timer to stop (idx < MAX_TIMER_INDEX).
 *     The value of the timer is stored and accumulated to the global
 *     time summation value.
 *
 * Returns the wall clock time in seconds elapsed since the call to
 * TimerStart().
 * -----------------------------------------------------------------
 */
realtype TimerStop(int idx);

/*
 * -----------------------------------------------------------------
 * Function : TimerValue
 * -----------------------------------------------------------------
 * A call to the TimerValue function returns the timer value currently
 * stored for the given timer. The timer must have been stopped previously.
 *
 * idx is the number of the timer to query (idx < MAX_TIMER_INDEX).
 *
 * Returns the elapsed time in seconds between the last TimerStart()
 * and TimerStop() calls. Returns 0 if timer is still running.
 * -----------------------------------------------------------------
 */
realtype TimerValue(int idx);

/*
 * -----------------------------------------------------------------
 * Function : TimerSum
 * -----------------------------------------------------------------
 * A call to the TimerSum function returns the accumulated timer values
 * currently stored for the given timer.
 *
 * idx is the number of the timer to query (idx < MAX_TIMER_INDEX).
 *
 * Returns the total elapsed time in seconds between all calls of
 * TimerStart() and TimerStop() for this timer index since the call
 * of TimerSumReset().
 * -----------------------------------------------------------------
 */
realtype TimerSum(int idx);

/*
 * -----------------------------------------------------------------
 * Function : TimerSumReset
 * -----------------------------------------------------------------
 * A call to the TimerSumReset function resets the global timer sum.
 *
 * idx is the number of the timer to query (idx < MAX_TIMER_INDEX).
 * -----------------------------------------------------------------
 */
void TimerSumReset(int idx);


/*
 * -----------------------------------------------------------------
 * Function : PrintTimings
 * -----------------------------------------------------------------
 * Prints timer statistics.
 * -----------------------------------------------------------------
 */
void PrintTimings();

/* Comment-out the define SUNDIALS_USE_INSTRUMENTATION to disable instrumentation  */
#define SUNDIALS_USE_INSTRUMENTATION
#ifdef SUNDIALS_USE_INSTRUMENTATION



/* Macro for convenient timed execution of a function */

#if defined(_OPENMP)

#if defined(_MSC_VER)

#define SUNDIALS_TIMED_FUNCTION(COUNTER_IDX, FUNCTION_CALL) \
__pragma(omp master) \
  TimerStart(COUNTER_IDX); \
  FUNCTION_CALL; \
__pragma(omp master) \
  TimerStop(COUNTER_IDX);

#else /* defined(_MSC_VER) */

#define SUNDIALS_TIMED_FUNCTION(COUNTER_IDX, FUNCTION_CALL) \
_Pragma("omp master") \
  TimerStart(COUNTER_IDX); \
  FUNCTION_CALL; \
_Pragma("omp master") \
  TimerStop(COUNTER_IDX);

#endif /* defined(_MSC_VER) */


#else /* defined(_OPENMP) */

#define SUNDIALS_TIMED_FUNCTION(COUNTER_IDX, FUNCTION_CALL) \
  TimerStart(COUNTER_IDX); \
  FUNCTION_CALL; \
  TimerStop(COUNTER_IDX);

#endif


#else /* SUNDIALS_USE_INSTRUMENTATION */

#define SUNDIALS_TIMED_FUNCTION(COUNTER_IDX, FUNCTION_CALL) \
  FUNCTION_CALL;

#endif /* SUNDIALS_USE_INSTRUMENTATION */



/* Integrators/Solvers */

/* Time spend in function evaluations called from integrator. */
#define SUNDIALS_TIMER_FEVAL 0
/* Time spend in calls to linear system setup. */
#define SUNDIALS_TIMER_LS_SETUP 1
/* Time spend in calls to linear system solve. */
#define SUNDIALS_TIMER_LS_SOLVE 2

/* Timers for LES solvers */

/* Time spend in composition of Jacobian matrix df/dy (via finite differences etc.). */
#define SUNDIALS_TIMER_JACOBIAN_GENERATION 3
/* Time spend in factorization system Jacobian M. */
#define SUNDIALS_TIMER_JACOBIAN_FACTORIZATION 4
/* Time spend in function evaluations for Jacobian generation. */
#define SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION 5

/* Time spend in matrix-vector multiplications. */
#define SUNDIALS_TIMER_ATIMES 6
/* Time spend in function evaluations during iterative solve of linear system. */
#define SUNDIALS_TIMER_FEVAL_LS_SOLVE 7

/* Time spend in preconditioner setup/generation. */
#define SUNDIALS_TIMER_PRE_SETUP 8
/* Time spend in function evaluations for Jacobian generation. */
#define SUNDIALS_TIMER_FEVAL_PRE_SETUP 9
/* Time spend in preconditioner solves. */
#define SUNDIALS_TIMER_PRE_SOLVE 10

/* Number of timers to be used (add a few for user-code instrumentation). */
#define SUNDIALS_TIMER_COUNT 25

#ifdef __cplusplus
}
#endif

#endif
