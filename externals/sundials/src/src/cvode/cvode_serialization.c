/*
 * -----------------------------------------------------------------
 * $Revision: 1.0 $
 * $Date: 2015/03/24 00:05:07 $
 * -----------------------------------------------------------------
 * Programmer(s): Jens Bastian, Andreas Nicolai
 * -----------------------------------------------------------------
 * Copyright (c) 2015, Technische Universität Dresden, Germany
 * For license details, see the LICENSE file.
 * -----------------------------------------------------------------
 * This is the implementation file for the CVODE serialization
 * functions.
 *------------------------------------------------------------------
 */

/*=================================================================*/
/*             Import Header Files                                 */
/*=================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "cvode_impl.h"
#include "cvode_direct_impl.h"
#include "cvode_spils_impl.h"
#include <cvode/cvode_serialization.h>
#include <sundials/sundials_serialization.h>

/* Defines repeated from cvode.c */
#define CV_SS  1
#define CV_SV  2
#define CV_WF  3


/*=================================================================*/
/*             Defines                                             */
/*=================================================================*/

/* Magic header to identify serialization memory block  = "FMI2" */
#define CVODE_SERIALIZATION_MEM_HEADER 0x464D4932

/* The current version number */
#define CVODE_SERIALIZATION_VERSION 1

/*=================================================================*/
/*             Private Helper Functions Prototypes                 */
/*=================================================================*/

size_t CVodeSerializationPrivate(int op, void *cvode_mem, void ** storageDataPtr) {
  CVodeMem cv_mem = (CVodeMem)cvode_mem;
  /* Variable to count memory size if operation is SUNDIALS_SERIALIZATION_OPERATION_SIZE */
  size_t memSize = 0;
  void * storageDataPtrStart;
  int version = CVODE_SERIALIZATION_VERSION;  /* current version when writing. */
  int magic_header = CVODE_SERIALIZATION_MEM_HEADER;
  int i, lmax;

  /*! Store start address of memory block, initialize always to make compiler happy */
  if (storageDataPtr != NULL)
      storageDataPtrStart = *storageDataPtr;
  else
      storageDataPtrStart = NULL;

  /* Read/write magic header */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, magic_header, memSize)

  /* Version number */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, version, memSize)

  /* If we deserialize, check that magic header matches and version number makes sense*/
  if (op == SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE) {
    if (magic_header != CVODE_SERIALIZATION_MEM_HEADER) {
      /* TODO : print error message */
      return (size_t)CVODE_SERIALIZATION_INVALID_MAGIC_HEADER;
    }
    if (version <=0 || version > 1) {
      /* TODO : print error message */
      return (size_t)CVODE_SERIALIZATION_INVALID_VERSION;
    }

  }

  /* When using de-serialization, the value of the version number can now be used to
   * enable/disable certain variables. */

  /* machine unit roundoff */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_uround, memSize)
  /* lmm = CV_ADAMS or CV_BDF */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_lmm, memSize)
  switch (cv_mem->cv_lmm) {
    case CV_ADAMS :
      lmax = ADAMS_Q_MAX + 1; break;
    case CV_BDF   :
    default :
      lmax = BDF_Q_MAX + 1; break;
  }
  /* iter = CV_FUNCTIONAL or CV_NEWTON */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_iter, memSize)
  /* itol = CV_SS, CV_SV, CV_WF, CV_NN */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_itol, memSize)
  /* relative tolerance */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_reltol, memSize)
  switch (cv_mem->cv_itol) {
    case CV_SS :
      /* scalar absolute tolerance */
      CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_Sabstol, memSize)
      break;
    case CV_SV :
      /* vector absolute tolerance */
      CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, cv_mem->cv_Vabstol, memSize)
      break;

    /* user supplied error weights function does not need vector to be stored */
    case CV_WF :
    default:
      break;

  }

  /* Nordsieck array, of size N x (q+1). */
  for (i = 0; i < lmax; ++i)
    CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, cv_mem->cv_zn[i], memSize)

  /* error weight vector */
  CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, cv_mem->cv_ewt, memSize)
  /* y is used as temporary storage by the solver */
/*  CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, &cv_mem->cv_y, memSize)*/
  /* In the context of the solution of the nonlinear equation, acor = y_n(m) - y_n(0).
     On return, this vector is scaled to give the est. local err. */
  CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, cv_mem->cv_acor, memSize)
  /* temporary storage vector */
  CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, cv_mem->cv_tempv, memSize)
  /* temporary storage vector */
  CVODE_SERIALIZE_NVECTOR(op, storageDataPtr, cv_mem->cv_ftemp, memSize)

  CVODE_SERIALIZE_A(op, booleantype, *storageDataPtr, cv_mem->cv_tstopset, memSize)
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tstop, memSize)

  /* current order */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_q, memSize)
  /* order to be used on the next step = q-1, q, or q+1 */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_qprime, memSize)
  /* order to be used on the next step */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_next_q, memSize)
  /* number of internal steps to wait before considering a change in q */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_qwait, memSize)
  /* L = q + 1 */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_L, memSize)
  /* initial step size */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_hin, memSize)
  /* current step size */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_h, memSize)
  /* step size to be used on the next step */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_hprime, memSize)
  /* step size to be used on the next step */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_next_h, memSize)
  /* eta = hprime / h */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_eta, memSize)
  /* value of h used in zn */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_hscale, memSize)
  /* current internal value of t */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tn, memSize)
  /* value of tret last returned by CVode */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tretlast, memSize)
  /* array of previous q+1 successful step sizes indexed from 1 to q+1 */
  for (i = 0; i < lmax+1; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tau[i], memSize)
  /* array of test quantities indexed from 1 to NUM_TESTS(=5) */
  for (i = 0; i < NUM_TESTS+1; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tq[i], memSize)
  /* coefficients of l(x) (degree q poly) */
  for (i = 0; i < lmax; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_l[i], memSize)
  /* the scalar 1/l[1] */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_rl1, memSize)
  /* gamma = h * rl1 */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_gamma, memSize)
  /* gamma at the last setup call */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_gammap, memSize)
  /* gamma / gammap */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_gamrat, memSize)
  /* estimated corrector convergence rate */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_crate, memSize)
  /* | acor | wrms */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_acnrm, memSize)
  /* coeficient in nonlinear convergence test */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_nlscoef, memSize)
  /* Newton iteration counter */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_mnewt, memSize)

  /* q <= qmax */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_qmax, memSize)
  /* maximum number of internal steps for one user call */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_mxstep, memSize)
  /* maximum number of corrector iterations for the solution of the nonlinear equation */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_maxcor, memSize)
  /* maximum number of warning messages issued to the user that t + h == t for the next internal step */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_mxhnil, memSize)
  /* maximum number of error test failures */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_maxnef, memSize)
  /* maximum number of nonlinear convergence failures */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_maxncf, memSize)
  /* |h| >= hmin */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_hmin, memSize)
  /* |h| <= 1/hmax_inv */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_hmax_inv, memSize)
  /* eta <= etamax */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_etamax, memSize)

  /* number of internal steps taken */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_nst, memSize)
  /* number of f calls */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_nfe, memSize)
  /* number of corrector convergence failures */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_ncfn, memSize)
  /* number of error test failures */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_netf, memSize)
  /* number of Newton iterations performed */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_nni, memSize)
  /* number of setup calls */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_nsetups, memSize)
  /* number of messages issued to the user that t + h == t for the next iternal step */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_nhnil, memSize)
  /* ratio of new to old h for order q-1 */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_etaqm1, memSize)
  /* ratio of new to old h for order q */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_etaq, memSize)
  /* ratio of new to old h for order q+1 */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_etaqp1, memSize)

  /* no. of realtype words in 1 N_Vector */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_lrw1, memSize)
  /* no. of integer words in 1 N_Vector */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_liw1, memSize)
  /* no. of realtype words in CVODE work vectors */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_lrw, memSize)
  /* no. of integer words in CVODE work vectors */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_liw, memSize)

  /* last successful q value used */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_qu, memSize)
  /* step number of last setup call */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_nstlp, memSize)
  /* actual initial stepsize */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_h0u, memSize)
  /* last successful h value used */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_hu, memSize)
  /* saved value of tq[5] */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_saved_tq5, memSize)
  /* is Jacobian info. for lin. solver current? */
  CVODE_SERIALIZE_A(op, booleantype, *storageDataPtr, cv_mem->cv_jcur, memSize)
  /* tolerance scale factor */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tolsf, memSize)
  /* value of qmax used when allocating memory */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_qmax_alloc, memSize)
  /* index of the zn vector with saved acor */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_indx_acor, memSize)

  /* does setup do anything? */
  CVODE_SERIALIZE_A(op, booleantype, *storageDataPtr, cv_mem->cv_setupNonNull, memSize)

  /* is Stability Limit Detection on? */
  CVODE_SERIALIZE_A(op, booleantype, *storageDataPtr, cv_mem->cv_sldeton, memSize)
  /* scaled data array for STALD */
  for (i = 0; i < 6; ++i) {
    size_t j;
    for (j = 0; i < 4; ++i)
      CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_ssdat[i][j], memSize)
  }
  /* counter for STALD method */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_nscon, memSize)
  /* counter for number of order reductions */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_nor, memSize)

  /* number of components of g */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_nrtfn, memSize)
  /* array for root information */
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_iroots[i], memSize)
  /* nearest endpoint of interval in root search */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tlo, memSize)
  /* farthest endpoint of interval in root search */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_thi, memSize)
  /* t value returned by rootfinding routine */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_trout, memSize)
  /* saved array of g values at t = tlo */
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_glo[i], memSize)
  /* saved array of g values at t = thi */
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_ghi[i], memSize)
  /* array of g values at t = trout */
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_grout[i], memSize)
  /* copy of tout (if NORMAL mode) */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_toutc, memSize)
  /* tolerance on root location */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_ttol, memSize)
  /* copy of parameter itask */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_taskc, memSize)
  /* flag showing whether last step had a root */
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_irfnd, memSize);
  /* counter for g evaluations */
  CVODE_SERIALIZE_A(op, long, *storageDataPtr, cv_mem->cv_nge, memSize);

//  int cv_nrtfn;            /* number of components of g                       */
//  int *cv_iroots;          /* array for root information                      */
//  int *cv_rootdir;         /* array specifying direction of zero-crossing     */
//  realtype cv_tlo;         /* nearest endpoint of interval in root search     */
//  realtype cv_thi;         /* farthest endpoint of interval in root search    */
//  realtype cv_trout;       /* t value returned by rootfinding routine         */
//  realtype *cv_glo;        /* saved array of g values at t = tlo              */
//  realtype *cv_ghi;        /* saved array of g values at t = thi              */
//  realtype *cv_grout;      /* array of g values at t = trout                  */
//  realtype cv_toutc;       /* copy of tout (if NORMAL mode)                   */
//  realtype cv_ttol;        /* tolerance on root location                      */
//  int cv_taskc;            /* copy of parameter itask                         */
//  int cv_irfnd;            /* flag showing whether last step had a root       */
//  size_t cv_nge;         /* counter for g evaluations                       */
//  booleantype *cv_gactive; /* array with active/inactive event functions      */
//  int cv_mxgnull;          /* number of warning messages about possible g==0  */

  /*----------------
    Rootfinding Data
    ----------------*/

  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_iroots[i], memSize);
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_rootdir[i], memSize);
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_tlo, memSize);
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_thi, memSize);
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_trout, memSize);
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_glo[i], memSize);
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_ghi[i], memSize);
  for (i = 0; i < cv_mem->cv_nrtfn; ++i)
    CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_grout[i], memSize);

  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_toutc, memSize);
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cv_mem->cv_ttol, memSize);
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_taskc, memSize);
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_irfnd, memSize);
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cv_mem->cv_nge, memSize);
  CVODE_SERIALIZE_A(op, int, *storageDataPtr, cv_mem->cv_mxgnull, memSize);

  switch (op) {
    case SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE :
    case SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE :
      memSize = (size_t)((char *)*storageDataPtr - (char *)storageDataPtrStart); break;
    default: ;
  }

  return memSize;
}


size_t CVDlsSerializationPrivate(int op, void *dls_mem, void ** storageDataPtr) {
  CVDlsMem cvdls_mem = (CVDlsMem)dls_mem;
  /* Variable to count memory size if operation is SUNDIALS_SERIALIZATION_OPERATION_SIZE */
  size_t memSize = 0;
  void * storageDataPtrStart;
  int i;

  /*! Store start address of memory block, initialize always to make compiler happy */
  if (storageDataPtr != NULL)
      storageDataPtrStart = *storageDataPtr;
  else
      storageDataPtrStart = NULL;

//  long int d_n;           /* problem dimension (_NOT_ block-based in case of BTRIDIAG */

//  long int d_ml;          /* lower bandwidth of Jacobian                  */
//  long int d_mu;          /* upper bandwidth of Jacobian                  */
//  long int d_smu;         /* upper bandwith of M = MIN(N-1,d_mu+d_ml)     */

//  booleantype d_jacDQ;    /* TRUE if using internal DQ Jacobian approx.   */
//  CVDlsDenseJacFn d_djac; /* dense Jacobian routine to be called          */
//  CVDlsBandJacFn d_bjac;  /* band Jacobian routine to be called           */
//  CVDlsBTridiagJacFn d_btjac; /* block-tridiagonal Jacobian routine to be called  */
//  void *d_J_data;         /* user data is passed to djac or bjac          */

//  DlsMat d_M;             /* M = I - gamma * df/dy                        */
//  DlsMat d_savedJ;        /* savedJ = old Jacobian                        */

//  int *d_pivots;          /* pivots = int pivot array for PM = LU         */
//  long int *d_lpivots;    /* lpivots = long int pivot array for PM = LU   */

//  long int  d_nstlj;      /* nstlj = nst at last Jacobian eval.           */

//  long int d_nje;         /* nje = no. of calls to jac                    */

//  long int d_nfeDQ;       /* no. of calls to f due to DQ Jacobian approx. */

//  long int d_last_flag;   /* last error return flag                       */

  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_n, memSize);

  // band solver specific quantities
  if(cvdls_mem->d_type == SUNDIALS_BAND) {
    CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_ml, memSize);
    CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_mu, memSize);
    CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_smu, memSize);
  }

  CVODE_SERIALIZE_A(op, booleantype, *storageDataPtr, cvdls_mem->d_jacDQ, memSize);

  CVODE_SERIALIZE_DLSMAT(op, storageDataPtr, cvdls_mem->d_M, memSize);
  CVODE_SERIALIZE_DLSMAT(op, storageDataPtr, cvdls_mem->d_savedJ, memSize);

  for (i = 0; i < cvdls_mem->d_n; ++i)
    CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_lpivots[i], memSize);

  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_nstlj, memSize);
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_nje, memSize);
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_nfeDQ, memSize);

  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvdls_mem->d_last_flag, memSize);

  switch (op) {
    case SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE :
    case SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE :
      memSize = (size_t)((char *)*storageDataPtr - (char *)storageDataPtrStart); break;
    default: ;
  }

  return memSize;
}


size_t CVSpilsSerializationPrivate(int op, void *spils_mem, void ** storageDataPtr) {
  CVSpilsMem cvspils_mem = (CVSpilsMem)spils_mem;
  /* Variable to count memory size if operation is SUNDIALS_SERIALIZATION_OPERATION_SIZE */
  size_t memSize = 0;
  void * storageDataPtrStart;
//  int i, lmax;

  /*! Store start address of memory block, initialize always to make compiler happy */
  if (storageDataPtr != NULL)
      storageDataPtrStart = *storageDataPtr;
  else
      storageDataPtrStart = NULL;

  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cvspils_mem->s_sqrtN, memSize);      /* sqrt(N)                                      */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cvspils_mem->s_eplifac, memSize);    /* eplifac = user specified or EPLIN_DEFAULT    */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cvspils_mem->s_deltar, memSize);     /* deltar = delt * tq4                          */
  CVODE_SERIALIZE_A(op, realtype, *storageDataPtr, cvspils_mem->s_delta, memSize);      /* delta = deltar * sqrtN                       */
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_nstlpre, memSize);    /* value of nst at the last pset call           */
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_npe, memSize);        /* npe = total number of pset calls             */
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_nli, memSize);        /* nli = total number of linear iterations      */
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_nps, memSize);        /* nps = total number of psolve calls           */
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_ncfl, memSize);       /* ncfl = total number of convergence failures  */
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_njtimes, memSize);    /* njtimes = total number of calls to jtimes    */
  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_nfes, memSize);       /* nfeSG = total number of calls to f for
                                                                                           difference quotient Jacobian-vector products */

  CVODE_SERIALIZE_A(op, long int, *storageDataPtr, cvspils_mem->s_last_flag, memSize);  /* last error flag returned by any function     */

  switch (op) {
    case SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE :
    case SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE :
      memSize = (size_t)((char *)*storageDataPtr - (char *)storageDataPtrStart); break;
    default: ;
  }

  return memSize;
}

/*
 * =================================================================
 * EXPORTED FUNCTIONS IMPLEMENTATION
 * =================================================================
 */

size_t CVodeSerializationSize(void *cvode_mem) {
  return CVodeSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_SIZE, cvode_mem, NULL);
}

size_t CVodeSerialize(void *cvode_mem, void ** storageDataPtr) {
  return CVodeSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE, cvode_mem, storageDataPtr);
}

size_t CVodeDeserialize(void *cvode_mem, void ** storageDataPtr) {
  return CVodeSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE, cvode_mem, storageDataPtr);
}


size_t CVDlsSerializationSize(void *cvode_mem) {
  return CVDlsSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_SIZE, ((CVodeMem)cvode_mem)->cv_lmem, NULL);
}

size_t CVDlsSerialize(void *cvode_mem, void ** storageDataPtr) {
  return CVDlsSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE, ((CVodeMem)cvode_mem)->cv_lmem, storageDataPtr);
}

size_t CVDlsDeserialize(void *cvode_mem, void ** storageDataPtr) {
  return CVDlsSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE, ((CVodeMem)cvode_mem)->cv_lmem, storageDataPtr);
}


size_t CVSpilsSerializationSize(void *cvode_mem) {
  return CVSpilsSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_SIZE, ((CVodeMem)cvode_mem)->cv_lmem, NULL);
}

size_t CVSpilsSerialize(void *cvode_mem, void ** storageDataPtr) {
  return CVSpilsSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_SERIALIZE, ((CVodeMem)cvode_mem)->cv_lmem, storageDataPtr);
}

size_t CVSpilsDeserialize(void *cvode_mem, void ** storageDataPtr) {
  return CVSpilsSerializationPrivate(SUNDIALS_SERIALIZATION_OPERATION_DESERIALIZE, ((CVodeMem)cvode_mem)->cv_lmem, storageDataPtr);
}
