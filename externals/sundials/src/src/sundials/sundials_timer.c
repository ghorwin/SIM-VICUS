/*
 * -----------------------------------------------------------------
 * $Revision: 1.21 $
 * $Date: 2014/08/06 21:46:54 $
 * -----------------------------------------------------------------
 * Programmer(s): Andreas Nicolai
 * -----------------------------------------------------------------
 * Copyright (c) 2014, Technische Universität Dresden, Germany
 * For license details, see the LICENSE file.
 * -----------------------------------------------------------------
 * This is the implementation file for the SUNDIALS timers.
 * -----------------------------------------------------------------
 */

#include <sundials/sundials_timer.h>

#include <stdio.h>

#if defined(_WIN32)

  /* on Windows for QueryPerformanceCounter */
  #include <windows.h>

static LARGE_INTEGER timerStartValues_[SUNDIALS_TIMER_COUNT];
static LARGE_INTEGER timerStopValues_[SUNDIALS_TIMER_COUNT];
static LARGE_INTEGER performanceFrequency_;

#else

  /* on Unix systems, use gettimeofday() */
  #include <sys/time.h>

static struct timeval timerStartValues_[SUNDIALS_TIMER_COUNT];
static struct timeval timerStopValues_[SUNDIALS_TIMER_COUNT];

#endif

/* Holds time span of last start-stop interval */
static realtype timerDifference_[SUNDIALS_TIMER_COUNT];
/* Holds sum of all time spans */
static realtype timerSums_[SUNDIALS_TIMER_COUNT];


void TimerStart(int idx) {
#if defined(_WIN32)
  /* Windows */
  if (performanceFrequency_.QuadPart == 0)
	QueryPerformanceFrequency(&performanceFrequency_);

  QueryPerformanceCounter(timerStartValues_ + idx);

#else

  /* Unix/Linux */
  gettimeofday(timerStartValues_ + idx, 0);

#endif

  timerDifference_[idx] = 0;
}


realtype TimerStop(int idx) {
#if defined(_WIN32)

  LARGE_INTEGER secs;

  if (timerDifference_[idx] != 0)
	return timerDifference_[idx];

  QueryPerformanceCounter(timerStopValues_ + idx);
  secs.QuadPart = timerStopValues_[idx].QuadPart - timerStartValues_[idx].QuadPart;
  timerDifference_[idx] = (double)secs.QuadPart;
  timerDifference_[idx] /= performanceFrequency_.QuadPart;
#else

  gettimeofday(timerStopValues_ + idx, 0);
  timerDifference_[idx] = ((timerStopValues_[idx].tv_sec  - timerStartValues_[idx].tv_sec) * 1000000u + timerStopValues_[idx].tv_usec - timerStartValues_[idx].tv_usec) / 1.e6;

#endif

  timerSums_[idx] += timerDifference_[idx];
  return timerDifference_[idx];
}


realtype TimerValue(int idx) {
  return timerDifference_[idx];
}


realtype TimerSum(int idx) {
  return timerSums_[idx];
}


void TimerSumReset(int idx) {
  timerSums_[idx] = 0;
}

void PrintTimings() {
  printf("Integrator timings\n");
  printf("  Function evaluations called from integrator  : FEVAL                     = %g s\n", TimerSum(SUNDIALS_TIMER_FEVAL));
  printf("  Linear system setup                          : LS_SETUP                  = %g s\n", TimerSum(SUNDIALS_TIMER_LS_SETUP));
  printf("  Linear system solve                          : LS_SOLVE                  = %g s\n", TimerSum(SUNDIALS_TIMER_LS_SOLVE));
  printf("\n");
  printf("Direct LES solver timings\n");
  printf("  Composition of Jacobian matrix df/dy (DQ)    : JACOBIAN_GENERATION       = %g s\n", TimerSum(SUNDIALS_TIMER_JACOBIAN_GENERATION));
  printf("  Factorization of system Jacobian M           : JACOBIAN_FACTORIZATION    = %g s\n", TimerSum(SUNDIALS_TIMER_JACOBIAN_FACTORIZATION));
  printf("  Function evaluations for Jacobian generation : FEVAL_JACOBIAN_GENERATION = %g s\n", TimerSum(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION));
  printf("\n");
  printf("Iterative LES solver timings\n");
  printf("  Matrix-vector multiplications                : ATIMES                    = %g s\n", TimerSum(SUNDIALS_TIMER_ATIMES));
  printf("  Function evaluations during iterative solve  : FEVAL_LS_SOLVE            = %g s\n", TimerSum(SUNDIALS_TIMER_FEVAL_LS_SOLVE));
  printf("\n");
  printf("Preconditioner timings\n");
  printf("  Preconditioner setup/generation              : PRE_SETUP                 = %g s\n", TimerSum(SUNDIALS_TIMER_PRE_SETUP));
  printf("  Function evaluations during setup            : FEVAL_PRE_SETUP           = %g s\n", TimerSum(SUNDIALS_TIMER_FEVAL_PRE_SETUP));
  printf("  Preconditioner solves                        : PRE_SOLVE                 = %g s\n", TimerSum(SUNDIALS_TIMER_PRE_SOLVE));

}
