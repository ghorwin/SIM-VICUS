/*
 * -----------------------------------------------------------------
 * $Revision: 1.00 $
 * $Date: 2011/06/18 12:00:00 $
 * -----------------------------------------------------------------
 * Programmer(s): Andreas Nicolai @ TU Dresden, Germany
 * -----------------------------------------------------------------
 * Implementation is based on the file cvode_band.c
 * -----------------------------------------------------------------
 * This is the implementation file for the CVBTRIDIAG linear solver.
 * -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include <cvode/cvode_btridiag.h>
#include "cvode_direct_impl.h"
#include "cvode_impl.h"

#include <sundials/sundials_math.h>
#include <sundials/sundials_timer.h>

/* Constants */

#define ZERO         RCONST(0.0)
#define ONE          RCONST(1.0)
#define TWO          RCONST(2.0)

/* CVBAND linit, lsetup, lsolve, and lfree routines */

static int cvBTridiagInit(CVodeMem cv_mem);

static int cvBTridiagSetup(CVodeMem cv_mem, int convfail, N_Vector ypred,
                       N_Vector fpred, booleantype *jcurPtr, N_Vector vtemp1,
                       N_Vector vtemp2, N_Vector vtemp3);

static int cvBTridiagSolve(CVodeMem cv_mem, N_Vector b, N_Vector weight,
                       N_Vector ycur, N_Vector fcur);

static int cvBTridiagFree(CVodeMem cv_mem);

/* Readability Replacements */

#define lmm       (cv_mem->cv_lmm)
#define f         (cv_mem->cv_f)
#define nst       (cv_mem->cv_nst)
#define tn        (cv_mem->cv_tn)
#define h         (cv_mem->cv_h)
#define gamma     (cv_mem->cv_gamma)
#define gammap    (cv_mem->cv_gammap)
#define gamrat    (cv_mem->cv_gamrat)
#define ewt       (cv_mem->cv_ewt)
#define nfe       (cv_mem->cv_nfe)
#define linit     (cv_mem->cv_linit)
#define lsetup    (cv_mem->cv_lsetup)
#define lsolve    (cv_mem->cv_lsolve)
#define lfree     (cv_mem->cv_lfree)
#define lmem      (cv_mem->cv_lmem)
#define vec_tmpl      (cv_mem->cv_tempv)
#define setupNonNull  (cv_mem->cv_setupNonNull)

#define mtype      (cvdls_mem->d_type)
#define n          (cvdls_mem->d_n)
#define jacDQ      (cvdls_mem->d_jacDQ)
#define jac        (cvdls_mem->d_btjac)
#define M          (cvdls_mem->d_M)
#define lpivots    (cvdls_mem->d_lpivots)
#define savedJ     (cvdls_mem->d_savedJ)
#define nstlj      (cvdls_mem->d_nstlj)
#define nje        (cvdls_mem->d_nje)
#define nfeDQ       (cvdls_mem->d_nfeDQ)
#define J_data     (cvdls_mem->d_J_data)
#define last_flag  (cvdls_mem->d_last_flag)

/*
 * -----------------------------------------------------------------
 * CVBTridiag
 * -----------------------------------------------------------------
 * This routine initializes the memory record and sets various
 * function fields specific to the block-tridiagonal linear solver
 * module.  CVBTridiag first calls the existing lfree routine if
 * this is not NULL.  It then sets the cv_linit, cv_lsetup,
 * cv_lsolve, and cv_lfree fields in (*cvode_mem)
 * to be cvBTridiagInit, cvBTridiagSetup, cvBTridiagSolve,
 * and cvBTridiagFree, respectively.  It allocates memory for a
 * structure of type CVDlsMemRec and sets the cv_lmem field in
 * (*cvode_mem) to the address of this structure.  It sets
 * setupNonNull in (*cvode_mem) to be TRUE, d_M to be M, and
 * the d_jac field to be cvDlsBTridiagDQJac.
 * Finally, it allocates memory for M, and savedJ.  The
 * CVBTridiag return value is SUCCESS = 0, LMEM_FAIL = -1, or
 * LIN_ILL_INPUT = -2.
 *
 * NOTE: The block-tridiagonal linear solver assumes a serial
 *       implementation of the NVECTOR package.
 *       Therefore, CVBTridiag will first
 *       test for compatible a compatible N_Vector internal
 *       representation by checking that the function
 *       N_VGetArrayPointer exists.
 * -----------------------------------------------------------------
 */

int CVBTridiag(void *cvode_mem, long int nb, long int blocksize) {
  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  /* Return immediately if cvode_mem is NULL */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVDLS_MEM_NULL, "CVBTRIDIAG", "CVBTridiag", MSGD_CVMEM_NULL);
    return(CVDLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  /* Test if the NVECTOR package is compatible with the BLOCK-TRIDIAG solver */
  if (vec_tmpl->ops->nvgetarraypointer == NULL) {
    cvProcessError(cv_mem, CVDLS_ILL_INPUT, "CVBTRIDIAG", "CVBTridiag", MSGD_BAD_NVECTOR);
    return(CVDLS_ILL_INPUT);
  }

  if (lfree != NULL) lfree(cv_mem);

  /* Set four main function fields in cv_mem */
  linit  = cvBTridiagInit;
  lsetup = cvBTridiagSetup;
  lsolve = cvBTridiagSolve;
  lfree  = cvBTridiagFree;

  /* Get memory for CVDlsMemRec */
  cvdls_mem = NULL;
  cvdls_mem = (CVDlsMem) malloc(sizeof(struct CVDlsMemRec));
  if (cvdls_mem == NULL) {
    cvProcessError(cv_mem, CVDLS_MEM_FAIL, "CVBTRIDIAG", "CVBTridiag", MSGD_MEM_FAIL);
    return(CVDLS_MEM_FAIL);
  }

  /* Set matrix type */
  mtype = SUNDIALS_BTRIDIAG;

  /* Initialize Jacobian-related data */
  jacDQ = TRUE;
  jac = NULL;
  J_data = NULL;

  last_flag = CVDLS_SUCCESS;

  setupNonNull = TRUE;

  /* Load problem dimension, this is not block-based, but full dimension */
  n = nb*blocksize;

  /* Test for valid blocksize  */
  if (blocksize <= 0) {
    cvProcessError(cv_mem, CVDLS_ILL_INPUT, "CVBTRIDIAG", "CVBTridiag", MSGD_BAD_SIZES);
    return(CVDLS_ILL_INPUT);
  }

  /* Allocate memory for M, and savedJ */
  M = NULL;
  M = NewBTridiagMat(nb, blocksize);
  if (M == NULL) {
    cvProcessError(cv_mem, CVDLS_MEM_FAIL, "CVBTRIDIAG", "CVBTridiag", MSGD_MEM_FAIL);
    free(cvdls_mem); cvdls_mem = NULL;
    return(CVDLS_MEM_FAIL);
  }
  savedJ = NULL;
  savedJ = NewBTridiagMat(nb, blocksize);
  if (savedJ == NULL) {
    cvProcessError(cv_mem, CVDLS_MEM_FAIL, "CVBTRIDIAG", "CVBTridiag", MSGD_MEM_FAIL);
    DestroyMat(M);
    free(cvdls_mem); cvdls_mem = NULL;
    return(CVDLS_MEM_FAIL);
  }
  lpivots = NULL;
  /* pivots = NewIntArray(nb*blocksize);
  if (pivots == NULL) {
    cvProcessError(cv_mem, CVDLS_MEM_FAIL, "CVBTRIDIAG", "CVBTridiag", MSGD_MEM_FAIL);
    DestroyMat(M);
    DestroyMat(savedJ);
    free(cvdls_mem); cvdls_mem = NULL;
    return(CVDLS_MEM_FAIL);
  }
  */

  /* Attach linear solver memory to integrator memory */
  lmem = cvdls_mem;

  return(CVDLS_SUCCESS);
}

/*
 * -----------------------------------------------------------------
 * cvBTridiagInit
 * -----------------------------------------------------------------
 * This routine does remaining initializations specific to the
 * block-tridiagonal linear solver.
 * -----------------------------------------------------------------
 */

static int cvBTridiagInit(CVodeMem cv_mem)
{
  CVDlsMem cvdls_mem;

  cvdls_mem = (CVDlsMem) lmem;

  nje   = 0;
  nfeDQ  = 0;
  nstlj = 0;

  /* Set Jacobian function and data, depending on jacDQ */
  if (jacDQ) {
    jac = cvDlsBTridiagDQJac;
    J_data = cv_mem;
  } else {
    J_data = cv_mem->cv_user_data;
  }

  last_flag = CVDLS_SUCCESS;
  return(0);
}

/*
 * -----------------------------------------------------------------
 * cvBTridiagSetup
 * -----------------------------------------------------------------
 * This routine does the setup operations for the block-tridiagonal
 * linear solver.
 * It makes a decision whether or not to call the Jacobian evaluation
 * routine based on various state variables, and if not it uses the
 * saved copy.  In any case, it constructs the Newton matrix
 * M = I - gamma*J, updates counters, and calls the band LU
 * factorization routine.
 * -----------------------------------------------------------------
 */

static int cvBTridiagSetup(CVodeMem cv_mem, int convfail, N_Vector ypred,
                       N_Vector fpred, booleantype *jcurPtr, N_Vector vtemp1,
                       N_Vector vtemp2, N_Vector vtemp3)
{
  booleantype jbad, jok;
  realtype dgamma;
  long int ier;
  CVDlsMem cvdls_mem;
  int retval;

  cvdls_mem = (CVDlsMem) lmem;

  /* Use nst, gamma/gammap, and convfail to set J eval. flag jok */

  dgamma = SUNRabs((gamma/gammap) - ONE);
  jbad = (nst == 0) || (nst > nstlj + CVD_MSBJ) ||
         ((convfail == CV_FAIL_BAD_J) && (dgamma < CVD_DGMAX)) ||
         (convfail == CV_FAIL_OTHER);
  jok = !jbad;

  if (jok) {

    /* If jok = TRUE, use saved copy of J */
    *jcurPtr = FALSE;
    BTridiagCopy(savedJ, M);

  } else {

    /* If jok = FALSE, call jac routine for new J value */
    nje++;
    nstlj = nst;
    *jcurPtr = TRUE;
    SetToZero(M);

    SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_JACOBIAN_GENERATION,
      retval = jac(n, tn, ypred, fpred, M, J_data, vtemp1, vtemp2, vtemp3);
  );
    if (retval < 0) {
      cvProcessError(cv_mem, CVDLS_JACFUNC_UNRECVR, "CVBTRIDIAG", "cvBTridiagSetup", MSGD_JACFUNC_FAILED);
      last_flag = CVDLS_JACFUNC_UNRECVR;
      return(-1);
    }
    if (retval > 0) {
      last_flag = CVDLS_JACFUNC_RECVR;
      return(1);
    }

    BTridiagCopy(M, savedJ);

  }

  /* Scale and add I to get M = I - gamma*J */
  BTridiagScale(-gamma, M);
  AddIdentity(M);

  /* Do LU factorization of M */
  SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_JACOBIAN_FACTORIZATION,
    ier = BTridiagBTTRF(M, lpivots);
  );
  /* Return 0 if the LU was complete; otherwise return 1 */
  if (ier > 0) {
    last_flag = ier;
    return(1);
  }
  last_flag = CVDLS_SUCCESS;
  return(0);
}

/*
 * -----------------------------------------------------------------
 * cvBTridiagSolve
 * -----------------------------------------------------------------
 * This routine handles the solve operation for the band linear solver
 * by calling the band backsolve routine.  The return value is 0.
 * -----------------------------------------------------------------
 */

static int cvBTridiagSolve(CVodeMem cv_mem, N_Vector b, N_Vector weight,
                       N_Vector ycur, N_Vector fcur)
{
  CVDlsMem cvdls_mem;
  realtype *bd;
  (void)weight; (void)ycur; (void)fcur;

  cvdls_mem = (CVDlsMem) lmem;

  bd = N_VGetArrayPointer(b);

  BTridiagBTTRS(M, lpivots, bd);

  /* If CV_BDF, scale the correction to account for change in gamma */
  if ((lmm == CV_BDF) && (gamrat != ONE)) {
    N_VScale(TWO/(ONE + gamrat), b, b);
  }

  last_flag = CVDLS_SUCCESS;
  return(0);
}

/*
 * -----------------------------------------------------------------
 * cvBTridiagFree
 * -----------------------------------------------------------------
 * This routine frees memory specific to the band linear solver.
 * -----------------------------------------------------------------
 */

static int cvBTridiagFree(CVodeMem cv_mem)
{
  CVDlsMem cvdls_mem;

  cvdls_mem = (CVDlsMem) lmem;

  DestroyMat(M);
  DestroyMat(savedJ);
  DestroyArray(lpivots);
  free(cvdls_mem);
  cv_mem->cv_lmem = NULL;

  return(0);
}

