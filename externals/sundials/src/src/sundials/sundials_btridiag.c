/*
 * -----------------------------------------------------------------
 * Programmer(s): Andreas Nicolai @ TU Dresden, Germany
 * -----------------------------------------------------------------
 * The implementation is based on the original sundials_band.c file.
 * -----------------------------------------------------------------
 * This is the implementation file for a generic BLOCK-TRIDIAGONAL
 * linear solver package.
 * -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <sundials/sundials_btridiag.h>
#include <sundials/sundials_math.h>

#define ZERO RCONST(0.0)
#define ONE  RCONST(1.0)

/* Uncomment the following define to enable special
   Implementations for low block-dimension */
#define USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS

/*
 * -----------------------------------------------------
 * Functions working on DlsMat
 * -----------------------------------------------------
 */

int BTridiagBTTRF(DlsMat A, long int * pivots)
{
  return(btridiagBTTRF(A->cols, A->N, A->M, pivots));
}

void BTridiagBTTRS(DlsMat A, long int * pivots, realtype *b)
{
  btridiagBTTRS(A->cols, A->N, A->M, pivots, b);
}

void BTridiagCopy(DlsMat A, DlsMat B)
{
  btridiagCopy(A->cols, B->cols, A->N, A->M);
}

void BTridiagScale(realtype c, DlsMat A)
{
  btridiagScale(c, A->cols, A->N, A->M);
}

void BTridiagMult(DlsMat A, realtype * b, realtype * r)
{
  btridiagMult(A->cols, b, r, A->N, A->M);
}

/*
 * -----------------------------------------------------
 * Helper functions using pivoting:

 * Generic functions (any block-dimension) using pivoting
 * and special implementations:
 * -----------------------------------------------------
 */
int denseLUPivot(long int n, realtype *A, long int *p);
void denseBacksolvePivot(long int n, realtype *A, long int *p, realtype *b);
void denseInverseMultPivot(long int n, realtype * A, long int * pivots, realtype * B);


/*
 * -----------------------------------------------------
 * Helper functions without pivoting:

 * Generic functions (any block-dimension) and special
 * implementations:
 * -----------------------------------------------------
 */
int denseLU(long int n, realtype *A);
void denseBacksolve(long int n, realtype *A, realtype *b);
void denseInverseMult(long int n, realtype * A, realtype * B);

void denseMultSub(long int n, realtype * A, realtype * B, realtype * C);
void denseVecMultSub(long int n, realtype * A, realtype * b, realtype * d);
void denseVecMultAdd(long int n, realtype * A, realtype * b, realtype * d);

int denseLU2(realtype * A);
int denseLU3(realtype * A);
int denseLU4(realtype * A);

void denseInverseMult2(realtype * A, realtype * B);
void denseInverseMult3(realtype * A, realtype * B);
void denseInverseMult4(realtype * A, realtype * B);

void denseBacksolve2(realtype * A, realtype * b);
void denseBacksolve3(realtype * A, realtype * b);
void denseBacksolve4(realtype * A, realtype * b);

void denseMultSub2(realtype * A, realtype * B, realtype * C);
void denseMultSub3(realtype * A, realtype * B, realtype * C);
void denseMultSub4(realtype * A, realtype * B, realtype * C);

void denseVecMultSub2(realtype * A, realtype * b, realtype * d);
void denseVecMultSub3(realtype * A, realtype * b, realtype * d);
void denseVecMultSub4(realtype * A, realtype * b, realtype * d);

/*
 * -----------------------------------------------------
 * Functions working on realtype**
 * -----------------------------------------------------
 */

/* Special implementation for tridiagonal matrices. */
int tridiagGTTRF(realtype **a, long int nblocks)
{
  realtype *L, *M, *U;
  long int i;

  L = a[0];
  M = a[nblocks];
  U = a[2*nblocks];

  /* first row */
  if (M[0] == 0)
    return 1;
  U[0] /= M[0];
  L[0] /= M[0];
  /* second row */
  M[1] -= L[1]*U[0];
  U[1] -= L[1]*L[0];
  U[1] /= M[1];
  for (i=2; i<nblocks-1; ++i) {
    M[i] -= L[i]*U[i-1];
    if (M[i] == 0)
      return i+1;
    U[i] /= M[i];
  }
  /* last row */
  L[nblocks-1] -= U[nblocks-1]*U[nblocks-3];
  M[nblocks-1] -= L[nblocks-1]*U[nblocks-2];
  if (M[nblocks-1] == 0)
    return nblocks;

  return 0;
}

/* General block-tridiagonal matrices */
int btridiagBTTRF(realtype **a, long int nblocks, long int blocksize, long int * pivots)
{
  realtype ** L, **M, **U;
  long int i;

  /* must have at least 4 block-rows */
  if (nblocks < 4)
    return -1; /* invalid matrix dimension */

  L = a;              /* points to L */
  M = a + nblocks;    /* points to M */
  U = a + 2*nblocks;  /* points to U */

  /* use pivoting-free version if pivots array is a NULL pointer */
  if (pivots == NULL) {
    /* switch for special cases of smaller blocksizes */
    switch (blocksize) {
      /* call special implementation for tridiagonal systems */
      case 1 : return tridiagGTTRF(a, nblocks);

#ifdef USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS
      case 2:
        /* first row */
        if (denseLU2(M[0]) != 0) return 1;
        denseInverseMult2(M[0], U[0]);
        denseInverseMult2(M[0], L[0]);
        /* second row */
        denseMultSub2(L[1], U[0], M[1]);
        if (denseLU2(M[1]) != 0) return 2;
        denseMultSub2(L[1], L[0], U[1]);
        denseInverseMult2(M[1], U[1]);
        for (i=2; i<nblocks-1; ++i) {
          denseMultSub2(L[i], U[i-1], M[i]);
          if (denseLU2(M[i]) != 0) return i+1;
          denseInverseMult2(M[i], U[i]);
        }
        /* last row */
        denseMultSub2(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        denseMultSub2(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        if (denseLU2(M[nblocks-1]) != 0) return nblocks;
      break;

      case 3:
        /* first row */
        if (denseLU3(M[0]) != 0) return 1;
        denseInverseMult3(M[0], U[0]);
        denseInverseMult3(M[0], L[0]);
        /* second row */
        denseMultSub3(L[1], U[0], M[1]);
        if (denseLU3(M[1]) != 0) return 2;
        denseMultSub3(L[1], L[0], U[1]);
        denseInverseMult3(M[1], U[1]);
        for (i=2; i<nblocks-1; ++i) {
          denseMultSub3(L[i], U[i-1], M[i]);
          if (denseLU3(M[i]) != 0) return i+1;
          denseInverseMult3(M[i], U[i]);
        }
        /* last row */
        denseMultSub3(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        denseMultSub3(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        if (denseLU3(M[nblocks-1]) != 0) return nblocks;
      break;

      case 4:
        /* first row */
        if (denseLU4(M[0]) != 0) return 1;
        denseInverseMult4(M[0], U[0]);
        denseInverseMult4(M[0], L[0]);
        /* second row */
        denseMultSub4(L[1], U[0], M[1]);
        if (denseLU4(M[1]) != 0) return 2;
        denseMultSub4(L[1], L[0], U[1]);
        denseInverseMult4(M[1], U[1]);
        for (i=2; i<nblocks-1; ++i) {
          denseMultSub4(L[i], U[i-1], M[i]);
          if (denseLU4(M[i]) != 0) return i+1;
          denseInverseMult4(M[i], U[i]);
        }
        /* last row */
        denseMultSub4(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        denseMultSub4(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        if (denseLU4(M[nblocks-1]) != 0) return nblocks;
      break;
#endif /* USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS */

      default :
        /* first row */
        /* compute M_0^{-1} U_0 and store in U_0 */
        if (denseLU(blocksize, M[0]) != 0)
          return 1;
        denseInverseMult(blocksize, M[0], U[0]);
        /* compute M_0^{-1} L_0 and store in L_0 */
        denseInverseMult(blocksize, M[0], L[0]);
        /* second row */
        /* compute M_1 - L_1 U_0 and store in M_1 */
        denseMultSub(blocksize, L[1], U[0], M[1]);
        /* compute M_1^{-1}(U_1 - L_1 L_0) and store in U_1 */
        if (denseLU(blocksize, M[1]) != 0)
          return 2;
        denseMultSub(blocksize, L[1], L[0], U[1]);
        denseInverseMult(blocksize, M[1], U[1]);
        for (i=2; i<nblocks-1; ++i) {
          /* compute M_i - L_i U_{i-1} and store in M_i */
          denseMultSub(blocksize, L[i], U[i-1], M[i]);
          /* compute M_i^{-1} U_i and store in L_0 */
          if (denseLU(blocksize, M[i]) != 0)
            return i+1;
          denseInverseMult(blocksize, M[i], U[i]);
        }
        /* last row */
        /* compute L_{n-1} - U_{n-1} U_{n-3} and store in L_{n-1} */
        denseMultSub(blocksize, U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        /* compute M_{n-1} - L_{n-1} U_{n-2} and store in M_{n-1} */
        denseMultSub(blocksize, L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        /* compute M_{n-1}^{-1} U_i and store in L_0 */
        if (denseLU(blocksize, M[nblocks-1]) != 0)
          return nblocks;
    } /* switch */

  }
  else { /* if (pivots == NULL)  */

    /* use pivoting version of algorithm */

    /* switch for special cases of smaller blocksizes */
    switch (blocksize) {
      /* call special implementation for tridiagonal systems */
      case 1 : return tridiagGTTRF(a, nblocks);

#ifdef USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS
      case 2:
        /* first row */
        if (denseLUPivot(2, M[0], pivots) != 0) return 1;
        denseInverseMultPivot(2, M[0], pivots, U[0]);
        denseInverseMultPivot(2, M[0], pivots, L[0]);
        /* second row */
        denseMultSub2(L[1], U[0], M[1]);
        if (denseLUPivot(2, M[1], pivots + 2) != 0) return 2;
        denseMultSub2(L[1], L[0], U[1]);
        denseInverseMultPivot(2, M[1], pivots + 2, U[1]);
        for (i=2; i<nblocks-1; ++i) {
          denseMultSub2(L[i], U[i-1], M[i]);
          if (denseLUPivot(2, M[i], pivots + i*2) != 0) return i+1;
          denseInverseMultPivot(2, M[i], pivots + i*2, U[i]);
        }
        /* last row */
        denseMultSub2(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        denseMultSub2(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        if (denseLUPivot(2, M[nblocks-1], pivots + i*2) != 0) return nblocks;
      break;

      case 3:
        /* first row */
        if (denseLUPivot(3, M[0], pivots) != 0) return 1;
        denseInverseMultPivot(3, M[0], pivots, U[0]);
        denseInverseMultPivot(3, M[0], pivots, L[0]);
        /* second row */
        denseMultSub3(L[1], U[0], M[1]);
        if (denseLUPivot(3, M[1], pivots + 3) != 0) return 2;
        denseMultSub3(L[1], L[0], U[1]);
        denseInverseMultPivot(3, M[1], pivots + 3, U[1]);
        for (i=2; i<nblocks-1; ++i) {
          denseMultSub3(L[i], U[i-1], M[i]);
          if (denseLUPivot(3, M[i], pivots + i*3) != 0) return i+1;
          denseInverseMultPivot(3, M[i], pivots + i*3, U[i]);
        }
        /* last row */
        denseMultSub3(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        denseMultSub3(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        if (denseLUPivot(3, M[nblocks-1], pivots + i*3) != 0) return nblocks;
      break;

      case 4:
        /* first row */
        if (denseLUPivot(4, M[0], pivots) != 0) return 1;
        denseInverseMultPivot(4, M[0], pivots, U[0]);
        denseInverseMultPivot(4, M[0], pivots, L[0]);
        /* second row */
        denseMultSub4(L[1], U[0], M[1]);
        if (denseLUPivot(4, M[1], pivots + 4) != 0) return 2;
        denseMultSub4(L[1], L[0], U[1]);
        denseInverseMultPivot(4, M[1], pivots + 4, U[1]);
        for (i=2; i<nblocks-1; ++i) {
          denseMultSub4(L[i], U[i-1], M[i]);
          if (denseLUPivot(4, M[i], pivots + i*4) != 0) return i+1;
          denseInverseMultPivot(4, M[i], pivots + i*4, U[i]);
        }
        /* last row */
        denseMultSub4(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        denseMultSub4(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        if (denseLUPivot(4, M[nblocks-1], pivots + i*4) != 0) return nblocks;
      break;

#endif /* USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS */

      default:
        if (denseLUPivot(blocksize, M[0], pivots) != 0)
          return 1;
        denseInverseMultPivot(blocksize, M[0], pivots, U[0]);
        denseInverseMultPivot(blocksize, M[0], pivots, L[0]);
        denseMultSub(blocksize, L[1], U[0], M[1]);
        if (denseLUPivot(blocksize, M[1], pivots + blocksize) != 0)
          return 2;
        denseMultSub(blocksize, L[1], L[0], U[1]);
        denseInverseMultPivot(blocksize, M[1], pivots + blocksize, U[1]);
        for (i=2; i<nblocks-1; ++i) {
          denseMultSub(blocksize, L[i], U[i-1], M[i]);
          if (denseLUPivot(blocksize, M[i], pivots + i*blocksize) != 0)
            return i+1;
          denseInverseMultPivot(blocksize, M[i], pivots + i*blocksize, U[i]);
        }
        denseMultSub(blocksize, U[nblocks-1], U[nblocks-3], L[nblocks-1]);
        denseMultSub(blocksize, L[nblocks-1], U[nblocks-2], M[nblocks-1]);
        if (denseLUPivot(blocksize, M[nblocks-1], pivots + (nblocks-1)*blocksize) != 0)
          return nblocks;
    } /* switch */

  } /* end if (pivots == NULL) */

  return(0);
}

/* Special implementation for back-solution of tridiagonal systems. */
void tridiagGTTRS(realtype **a, long int nblocks, realtype *b)
{
  realtype *L, *M, *U;
  long int i;

  L = a[0];
  M = a[nblocks];
  U = a[2*nblocks];

  /* forward elimination */
  b[0] /= M[0];
  for (i=1; i<nblocks-1; ++i) {
    b[i] -= L[i]*b[i-1];
    b[i] /= M[i];
  }
  /* last row */
  b[nblocks-1] -= L[nblocks-1]*b[nblocks-2];
  b[nblocks-1] -= U[nblocks-1]*b[nblocks-3];
  b[nblocks-1] /= M[nblocks-1];

  /* backward elimination */
  for (i=nblocks-2; i>=1; --i) {
    b[i] -= U[i]*b[i+1];
  }
  /* first row */
  b[0] -= U[0]*b[1];
  b[0] -= L[0]*b[2];
}

/* Generic implementation for back-solution of general block-tridiagonal systems. */
void btridiagBTTRS(realtype **a, long int nblocks, long int blocksize, long int * pivots, realtype *b)
{
  realtype **L, **M, **U, *bd;
  long int i;

  L = a;              /* points to L */
  M = a + nblocks;    /* points to M */
  U = a + 2*nblocks;  /* points to U */

  if (pivots == NULL) {
    /* use pivoting-free implementation */
    switch (blocksize) {
      case 1:
        /* call special implementation for tridiagonal systems */
        tridiagGTTRS(a, nblocks, b);
        break;

#ifdef USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS
      case 2:
        /* forward elimination */
        denseBacksolve2(M[0], b);
        for (i=1; i<nblocks-1; ++i) {
          denseVecMultSub2(L[i], b + (i-1)*2, b + i*2);
          denseBacksolve2(M[i], b + i*2);
        }
        /* last row */
        denseVecMultSub2(L[nblocks-1], b + (nblocks-2)*2, b + (nblocks-1)*2);
        denseVecMultSub2(U[nblocks-1], b + (nblocks-3)*2, b + (nblocks-1)*2);
        denseBacksolve2(M[nblocks-1], b + (nblocks-1)*2);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub2(U[i], b + (i+1)*2, b + i*2);
        }
        /* first row */
        denseVecMultSub2(U[0], b + 2, b);
        denseVecMultSub2(L[0], b + 4, b);
        break;

      case 3:
        /* forward elimination */
        denseBacksolve3(M[0], b);
        for (i=1; i<nblocks-1; ++i) {
          denseVecMultSub3(L[i], b + (i-1)*3, b + i*3);
          denseBacksolve3(M[i], b + i*3);
        }
        /* last row */
        denseVecMultSub3(L[nblocks-1], b + (nblocks-2)*3, b + (nblocks-1)*3);
        denseVecMultSub3(U[nblocks-1], b + (nblocks-3)*3, b + (nblocks-1)*3);
        denseBacksolve3(M[nblocks-1], b + (nblocks-1)*3);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub3(U[i], b + (i+1)*3, b + i*3);
        }
        /* first row */
        denseVecMultSub3(U[0], b + 3, b);
        denseVecMultSub3(L[0], b + 6, b);
        break;

      case 4:
        /* forward elimination */
        denseBacksolve4(M[0], b);
        for (i=1; i<nblocks-1; ++i) {
          denseVecMultSub4(L[i], b + (i-1)*4, b + i*4);
          denseBacksolve4(M[i], b + i*4);
        }
        /* last row */
        denseVecMultSub4(L[nblocks-1], b + (nblocks-2)*4, b + (nblocks-1)*4);
        denseVecMultSub4(U[nblocks-1], b + (nblocks-3)*4, b + (nblocks-1)*4);
        denseBacksolve4(M[nblocks-1], b + (nblocks-1)*4);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub4(U[i], b + (i+1)*4, b + i*4);
        }
        /* first row */
        denseVecMultSub4(U[0], b + 4, b);
        denseVecMultSub4(L[0], b + 8, b);
        break;
#endif /* USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS */

      default : ;
        /* forward elimination */
        denseBacksolve(blocksize, M[0], b);
        for (i=1; i<nblocks-1; ++i) {
          denseVecMultSub(blocksize, L[i], b + (i-1)*blocksize, b + i*blocksize);
          denseBacksolve(blocksize, M[i], b + i*blocksize);
        }
        /* last row */
        denseVecMultSub(blocksize, L[nblocks-1], b + (nblocks-2)*blocksize, b + (nblocks-1)*blocksize);
        denseVecMultSub(blocksize, U[nblocks-1], b + (nblocks-3)*blocksize, b + (nblocks-1)*blocksize);
        denseBacksolve(blocksize, M[nblocks-1], b + (nblocks-1)*blocksize);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub(blocksize, U[i], b + (i+1)*blocksize, b + i*blocksize);
        }
        /* first row */
        denseVecMultSub(blocksize, U[0], b + blocksize, b);
        denseVecMultSub(blocksize, L[0], b + 2*blocksize, b);
    } /* switch */

  }
  else { /* if (pivots == NULL) */

    /* use pivoting implementation */
    switch (blocksize) {
      case 1:
        /* call special implementation for tridiagonal systems */
        tridiagGTTRS(a, nblocks, b);
        break;

#ifdef USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS
      case 2:
        /* forward elimination */
        denseBacksolvePivot(2, M[0], pivots, b);
        for (i=1; i<nblocks-1; ++i) {
          denseVecMultSub2(L[i], b + (i-1)*2, b + i*2);
          denseBacksolvePivot(2, M[i], pivots + i*2, b + i*2);
        }
        /* last row */
        denseVecMultSub2(L[nblocks-1], b + (nblocks-2)*2, b + (nblocks-1)*2);
        denseVecMultSub2(U[nblocks-1], b + (nblocks-3)*2, b + (nblocks-1)*2);
        denseBacksolvePivot(2, M[nblocks-1], pivots + (nblocks-1)*2, b + (nblocks-1)*2);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub2(U[i], b + (i+1)*2, b + i*2);
        }
        /* first row */
        denseVecMultSub2(U[0], b + 2, b);
        denseVecMultSub2(L[0], b + 4, b);
        break;

      case 3:
        /* forward elimination */
        denseBacksolvePivot(3, M[0], pivots, b);
        for (i=1; i<nblocks-1; ++i) {
          denseVecMultSub3(L[i], b + (i-1)*3, b + i*3);
          denseBacksolvePivot(3, M[i], pivots + i*3, b + i*3);
        }
        /* last row */
        denseVecMultSub3(L[nblocks-1], b + (nblocks-2)*3, b + (nblocks-1)*3);
        denseVecMultSub3(U[nblocks-1], b + (nblocks-3)*3, b + (nblocks-1)*3);
        denseBacksolvePivot(3, M[nblocks-1], pivots + (nblocks-1)*3, b + (nblocks-1)*3);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub3(U[i], b + (i+1)*3, b + i*3);
        }
        /* first row */
        denseVecMultSub3(U[0], b + 3, b);
        denseVecMultSub3(L[0], b + 6, b);
        break;

      case 4:
        /* forward elimination */
        denseBacksolvePivot(4, M[0], pivots, b);
        for (i=1; i<nblocks-1; ++i) {
          denseVecMultSub4(L[i], b + (i-1)*4, b + i*4);
          denseBacksolvePivot(4, M[i], pivots + i*4, b + i*4);
        }
        /* last row */
        denseVecMultSub4(L[nblocks-1], b + (nblocks-2)*4, b + (nblocks-1)*4);
        denseVecMultSub4(U[nblocks-1], b + (nblocks-3)*4, b + (nblocks-1)*4);
        denseBacksolvePivot(4, M[nblocks-1], pivots + (nblocks-1)*4, b + (nblocks-1)*4);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub4(U[i], b + (i+1)*4, b + i*4);
        }
        /* first row */
        denseVecMultSub4(U[0], b + 4, b);
        denseVecMultSub4(L[0], b + 8, b);
        break;
#endif /* USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS */

      default:
        /* M, L and U have been initialized already */
        bd = b;
        /* forward elimination */
        denseBacksolvePivot(blocksize, M[0], pivots, bd);
        for (i=1; i<nblocks-1; ++i) {
          bd += blocksize;
          denseVecMultSub(blocksize, L[i], bd - blocksize, bd);
          denseBacksolvePivot(blocksize, M[i], pivots + i*blocksize, bd);
        }
        /* last row */
        /* L points to a[0] + (nblocks-2)*blockmemsize */
        /* M points to a[1] + (nblocks-2)*blockmemsize */
        /* bd points to b + (nblocks-2)*blocksize */
        denseVecMultSub(blocksize, L[nblocks-1], bd, bd + blocksize);
        denseVecMultSub(blocksize, U[nblocks-1], bd - blocksize, bd + blocksize);
        denseBacksolvePivot(blocksize, M[nblocks-1], pivots + (nblocks-1)*blocksize, bd + blocksize);

        /* backward elimination */
        for (i=nblocks-2; i>=1; --i) {
          denseVecMultSub(blocksize, U[i], b + (i+1)*blocksize, b + i*blocksize);
        }
        /* first row */
        denseVecMultSub(blocksize, U[0], b + blocksize, b);
        denseVecMultSub(blocksize, L[0], b + 2*blocksize, b);
    } /* switch */

  } /* if (pivots == NULL) */

}

void btridiagCopy(realtype **a, realtype **b, long int nblocks, long int blocksize)
{
  long int i, data_size;
  realtype * a_data, * b_data;

  data_size = 3*nblocks*blocksize*blocksize;
  a_data = a[0];
  b_data = b[0];

  for (i=0; i<data_size; ++i) {
    *b_data++ = *a_data++;
  }
}

void btridiagScale(realtype c, realtype **a, long int nblocks, long int blocksize)
{
  long int i, data_size;
  realtype *a_data;

  a_data = a[0];
  data_size = 3*nblocks*blocksize*blocksize;

  for (i=0; i<data_size; ++i) {
    *a_data++ *= c;
  }
}

void btridiagAddIdentity(realtype **a, long int nblocks, long int blocksize)
{
  long int i,j, blocksize2;
  realtype *a_data;

  /* memory size used by each block */
  blocksize2 = blocksize*blocksize;
  /* a_data points to the main diagonal band */
  a_data = a[1];

  /* i index of main diagonal block */
  for (i=0; i<nblocks; i++, a_data += blocksize2) {
    /* j column in block */
    for (j=0; j<blocksize; j++)
      a_data[j*blocksize + j] += ONE;
  }
}

void btridiagMult(realtype **a, realtype * b, realtype * r, long int nblocks, long int blocksize)
{
  realtype **L, **M, **U;
  long int i;
  L = a;              /* points to L */
  M = a + nblocks;    /* points to M */
  U = a + 2*nblocks;  /* points to U */

  /* clear target vector */
  memset((void*)r, 0, sizeof(realtype)*blocksize*nblocks);

  /* first row: r[0] = M[0].b[0] + U[0].b[1] + L[0].b[2] */
  denseVecMultAdd(blocksize, M[0], b, r);
  denseVecMultAdd(blocksize, U[0], b + blocksize, r);
  denseVecMultAdd(blocksize, L[0], b + 2*blocksize, r);

  /* all rows except the last: */
  /* r[i] = L[i].b[i-1] + M.b[i] + U.b[i+1] */
  for (i=1; i<nblocks-1; ++i) {
    b += blocksize;
    r += blocksize;
    denseVecMultAdd(blocksize, L[i], b - blocksize, r);
    denseVecMultAdd(blocksize, M[i], b, r);
    denseVecMultAdd(blocksize, U[i], b + blocksize, r);
  }
  /* last row: r[n-1] = L[n-1].b[n-2] + M[n-1].b[n-1] + U[n-1].b[n-3] */
  b += blocksize;
  r += blocksize;
  denseVecMultAdd(blocksize, L[nblocks-1], b - blocksize, r);
  denseVecMultAdd(blocksize, M[nblocks-1], b, r);
  denseVecMultAdd(blocksize, U[nblocks-1], b - 2*blocksize, r);
}


/*
 * -----------------------------------------------------
 * Helper functions for btridiagGBTRF and btridiagGBTRS
 * The work on dense matricies, stored in a continuous
 * memory array in column-based order. Thus, the element
 * i,j in matrix A with dimension n x n is accessed
 * via A[i + j*n].
 * -----------------------------------------------------
 */

/* In-place LU Factorization of A (dimension n x n, column-based storage).
   Returns 0 for success, -2 for singular matrix.
   Pivoting version, requires pivot vector of size n.
*/
int denseLUPivot(long int n, realtype *A, long int *p)
{
  long int i, j, k, l;
  realtype *col_i, *col_j, *col_k;
  realtype temp, mult, a_kj;

  /* k-th elimination step number */
  col_k  = A;
  for (k=0; k < n; k++, col_k += n) {

    /* find l = pivot row number */
    l=k;
    for (i=k+1; i < n; i++)
      if (SUNRabs(col_k[i]) > SUNRabs(col_k[l])) l=i;
    p[k] = l;

    /* check for zero pivot element */
    if (col_k[l] == ZERO) return(k+1);

    /* swap a(k,1:n) and a(l,1:n) if necessary */
    if ( l!= k ) {
      col_i = A;
      for (i=0; i<n; i++, col_i += n) {
        temp = col_i[l];
        col_i[l] = col_i[k];
        col_i[k] = temp;
      }
    }

    /* Scale the elements below the diagonal in
     * column k by 1.0/a(k,k). After the above swap
     * a(k,k) holds the pivot element. This scaling
     * stores the pivot row multipliers a(i,k)/a(k,k)
     * in a(i,k), i=k+1, ..., m-1.
     */
    mult = ONE/col_k[k];
    for(i=k+1; i < n; i++) col_k[i] *= mult;

    /* row_i = row_i - [a(i,k)/a(k,k)] row_k, i=k+1, ..., m-1 */
    /* row k is the pivot row after swapping with row l.      */
    /* The computation is done one column at a time,          */
    /* column j=k+1, ..., n-1.                                */

    col_j = A + (k+1)*n;
    for (j=k+1; j < n; j++, col_j += n) {

      a_kj = col_j[k];

      /* a(i,j) = a(i,j) - [a(i,k)/a(k,k)]*a(k,j)  */
      /* a_kj = a(k,j), col_k[i] = - a(i,k)/a(k,k) */

      if (a_kj != ZERO) {
        for (i=k+1; i < n; i++)
          col_j[i] -= a_kj * col_k[i];
        }
    }
  }

  /* return 0 to indicate success */

  return(0);
}

/* Solves the equation system Ax = b using backsolving, A must be
   LU factorized already.
   Pivoting version, requires pivot vector of size n.
*/
void denseBacksolvePivot(long int n, realtype *A, long int *p, realtype *b)
{
  long int i, k, pk;
  realtype *col_k, tmp;

  /* Permute b, based on pivot information in p */
  for (k=0; k<n; k++) {
    pk = p[k];
    if(pk != k) {
      tmp = b[k];
      b[k] = b[pk];
      b[pk] = tmp;
    }
  }

  /* Solve Ly = b, store solution y in b */
  col_k = A;
  for (k=0; k<n-1; k++, col_k += n) {
    for (i=k+1; i<n; i++) b[i] -= col_k[i]*b[k];
  }

  /* Solve Ux = y, store solution x in b */
  col_k = A + (n-1)*n;
  for (k = n-1; k > 0; k--, col_k -= n) {
    b[k] /= col_k[k];
    for (i=0; i<k; i++) b[i] -= col_k[i]*b[k];
  }
  b[0] /= A[0];
}

/* Computes A^-1 * B and stores result in B, A is assumed to be LU factorized.
   Pivoting version, requires pivot vector of size n.
*/
void denseInverseMultPivot(long int n, realtype * A, long int * pivots, realtype * B) {
  long int i;
  for (i=0; i<n; ++i, B += n)
    denseBacksolvePivot(n, A, pivots, B);
}
// ---------------------------------------------------------------------------

/* In-place LU Factorization of A (dimension n x n, column-based storage).
   Returns 0 for success, -2 for singular matrix.
   Non-Pivoting version.
*/
int denseLU(long int n, realtype * A) {
  long int i,j,k;
  for (k=0; k<n-1;++k) {
    for (i=k+1;i<n;++i) {
      if (A[k + k*n] == 0) return -2;
      A[i + k*n] /= A[k + k*n];
      for (j=k+1; j<n; ++j)
        A[i + j*n] -= A[i + k*n]*A[k + j*n];
    }
  }
  if (A[n-1 + (n-1)*n] == 0) return -2;
  return 0;
}

/* Solves the equation system Ax = b using backsolving, A must be
   LU factorized already.
   Non-Pivoting version.
*/
void denseBacksolve(long int n, realtype * A, realtype * b) {
  long int i,k;
  /* forward elimination */
  for (k=1; k<n; ++k) {
    for (i=0; i<k; ++i) {
      b[k] -= A[k + i*n]*b[i];
    }
  }
  /* backward elimination */
  for (k=n-1; k>=0; --k) {
    for (i=k+1; i<n; ++i) {
      b[k] -= A[k + i*n]*b[i];
    }
    b[k] /= A[k + n*k];
  }
}

/* Computes A^-1 * B and stores result in B, A is assumed to be LU factorized.
   Non-Pivoting version.
*/
void denseInverseMult(long int n, realtype * A, realtype * B) {
  long int i;
  for (i=0; i<n; ++i, B += n)
    denseBacksolve(n, A, B);
}

/* Computes C -= A*B. */
void denseMultSub(long int n, realtype * A, realtype * B, realtype * C) {
  long int i,j,k;
  for (j=0; j<n; ++j)
    for (k=0; k<n; ++k)
      for (i=0; i<n; ++i)
        C[i + j*n] -= A[i + k*n] * B[k + j*n];
}

/* Computes d -= A*b. */
void denseVecMultSub(long int n, realtype * A, realtype * b, realtype * d) {
  long int i,k;
  for (k=0; k<n; ++k)
    for (i=0; i<n; ++i)
      d[i] -= A[i + k*n] * b[k];
}

/* Computes d += A*b */
void denseVecMultAdd(long int n, realtype * A, realtype * b, realtype * d) {
  long int k, i;
  for (k=0; k<n; ++k)
    for (i=0; i<n; ++i)
      d[i] += A[i + k*n] * b[k];
}
/* --------------------------------------------------------------------------- */




/*
 * -----------------------------------------------------
 * Special Implementation Versions (manual loop unrolling)
 * for block dimensions 2x2, 3x3 and 4x4.
 * Use of these functions is controlled through the
 * compiler define USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS
 * -----------------------------------------------------
 */



/* Special version of LU algorithm for matricies of dimension 2x2. */
int denseLU2(realtype * A) {
  if (A[0] == 0) return -2;
  A[1] /= A[0];
  A[3] -= A[1]*A[2];
  if (A[3] == 0) return -2;
  return 0;
}

/* Special version of LU algorithm for matricies of dimension 3x3. */
int denseLU3(realtype * A) {
  if (A[0] == 0) return -2;
  // column 0
  A[1] /= A[0];
  A[2] /= A[0];

  // column 1
  A[4] -= A[1]*A[3];
  if (A[4] == 0) return -2;

  A[5] -= A[2]*A[3];
  A[5] /= A[4];

  // column 2
  A[7] -= A[1]*A[6];
  A[8] -= A[2]*A[6] + A[5]*A[7];
  if (A[8] == 0) return -2;

  return 0;
}

/* Special version of LU algorithm for matricies of dimension 4x4. */
int denseLU4(realtype * A) {
  if (A[0] == 0) return -2;
  // column 0
  A[1] /= A[0];
  A[2] /= A[0];
  A[3] /= A[0];

  // column 1
  A[5] -= A[1]*A[4];
  if (A[5] == 0) return -2;

  A[6] -= A[2]*A[4];
  A[6] /= A[5];
  A[7] -= A[3]*A[4];
  A[7] /= A[5];

  // column 2
  A[9] -= A[1]*A[8];
  A[10] -= A[2]*A[8] + A[6]*A[9];
  if (A[10] == 0) return -2;

  A[11] -= A[3]*A[8] + A[7]*A[9];
  A[11] /= A[10];

  // column 3
  A[13] -= A[1]*A[12];
  A[14] -= A[2]*A[12] + A[6]*A[13];
  A[15] -= A[3]*A[12] + A[7]*A[13] + A[11]*A[14];
  if (A[15] == 0) return -2;

  return 0;
}
/* --------------------------------------------------------------------------- */



/* Special version for matricies of dimensions 2x2. */
void denseInverseMult2(realtype * A, realtype * B) {
  denseBacksolve2(A, B);
  denseBacksolve2(A, B+2);
}

/* Special version for matricies of dimensions 3x3. */
void denseInverseMult3(realtype * A, realtype * B) {
  denseBacksolve3(A, B);
  denseBacksolve3(A, B+3);
  denseBacksolve3(A, B+6);
}

/* Special version for matricies of dimensions 4x4. */
void denseInverseMult4(realtype * A, realtype * B) {
  denseBacksolve4(A, B);
  denseBacksolve4(A, B+4);
  denseBacksolve4(A, B+8);
  denseBacksolve4(A, B+12);
}
/* --------------------------------------------------------------------------- */




/* Special version of backsolving algorithm for matricies of dimension 2x2. */
void denseBacksolve2(realtype * A, realtype * b) {
  /* forward elimination */
  b[1] -= A[1]*b[0];
  /* backward elimination */
  b[1] /= A[3];
  b[0] -= A[2]*b[1];
  b[0] /= A[0];
}

/* Special version of backsolving algorithm for matricies of dimension 3x3. */
void denseBacksolve3(realtype * A, realtype * b) {
  /* forward elimination */
  b[1] -= A[1]*b[0];
  b[2] -= A[2]*b[0] + A[5]*b[1];
  /* backward elimination */
  b[2] /= A[8];
  b[1] -= A[7]*b[2];
  b[1] /= A[4];
  b[0] -= A[6]*b[2] + A[3]*b[1];
  b[0] /= A[0];
}

/* Special version of backsolving algorithm for matricies of dimension 4x4. */
void denseBacksolve4(realtype * A, realtype * b) {
  /* forward elimination */
  b[1] -= A[1]*b[0];
  b[2] -= A[2]*b[0] + A[6]*b[1];
  b[3] -= A[3]*b[0] + A[7]*b[1] + A[11]*b[2];
  /* backward elimination */
  b[3] /= A[15];
  b[2] -= A[14]*b[3];
  b[2] /= A[10];
  b[1] -= A[13]*b[3] + A[9]*b[2];
  b[1] /= A[5];
  b[0] -= A[12]*b[3] + A[8]*b[2] + A[4] * b[1];
  b[0] /= A[0];
}
/* --------------------------------------------------------------------------- */



/* Computes C -= A*B with matrix dimension 2x2. */
void denseMultSub2(realtype * A, realtype * B, realtype * C) {
  /** j = 0 **/

  /* i = 0 */
  C[0] -= A[0] * B[0];
  C[0] -= A[2] * B[1];

  /* i = 1 */
  C[1] -= A[1] * B[0];
  C[1] -= A[3] * B[1];

  /** j = 1 **/

  /* i = 0 */
  C[2] -= A[0] * B[2];
  C[2] -= A[2] * B[3];

  /* i = 1 */
  C[3] -= A[1] * B[2];
  C[3] -= A[3] * B[3];
}

/* Computes C -= A*B with matrix dimension 3x3. */
void denseMultSub3(realtype * A, realtype * B, realtype * C) {
  /** j = 0 **/

  /* i = 0 */
  C[0] -= A[0] * B[0];
  C[0] -= A[3] * B[1];
  C[0] -= A[6] * B[2];

  /* i = 1 */
  C[1] -= A[1] * B[0];
  C[1] -= A[4] * B[1];
  C[1] -= A[7] * B[2];

  /* i = 2 */
  C[2] -= A[2] * B[0];
  C[2] -= A[5] * B[1];
  C[2] -= A[8] * B[2];

  /** j = 1 **/

  /* i = 0 */
  C[3] -= A[0] * B[3];
  C[3] -= A[3] * B[4];
  C[3] -= A[6] * B[5];

  /* i = 1 */
  C[4] -= A[1] * B[3];
  C[4] -= A[4] * B[4];
  C[4] -= A[7] * B[5];

  /* i = 2 */
  C[5] -= A[2] * B[3];
  C[5] -= A[5] * B[4];
  C[5] -= A[8] * B[5];

  /** j = 2 **/

  /* i = 0 */
  C[6] -= A[0] * B[6];
  C[6] -= A[3] * B[7];
  C[6] -= A[6] * B[8];

  /* i = 1 */
  C[7] -= A[1] * B[6];
  C[7] -= A[4] * B[7];
  C[7] -= A[7] * B[8];

  /* i = 2 */
  C[8] -= A[2] * B[6];
  C[8] -= A[5] * B[7];
  C[8] -= A[8] * B[8];
}

/* Computes C -= A*B with matrix dimension 4x4. */
void denseMultSub4(realtype * A, realtype * B, realtype * C) {
  /** j = 0 **/

  /* i = 0 */
  C[0] -= A[0] * B[0];
  C[0] -= A[4] * B[1];
  C[0] -= A[8] * B[2];
  C[0] -= A[12] * B[3];

  /* i = 1 */
  C[1] -= A[1] * B[0];
  C[1] -= A[5] * B[1];
  C[1] -= A[9] * B[2];
  C[1] -= A[13] * B[3];

  /* i = 2 */
  C[2] -= A[2] * B[0];
  C[2] -= A[6] * B[1];
  C[2] -= A[10] * B[2];
  C[2] -= A[14] * B[3];

  /* i = 3 */
  C[3] -= A[3] * B[0];
  C[3] -= A[7] * B[1];
  C[3] -= A[11] * B[2];
  C[3] -= A[15] * B[3];

  /** j = 1 **/

  /* i = 0 */
  C[4] -= A[0] * B[4];
  C[4] -= A[4] * B[5];
  C[4] -= A[8] * B[6];
  C[4] -= A[12] * B[7];

  /* i = 1 */
  C[5] -= A[1] * B[4];
  C[5] -= A[5] * B[5];
  C[5] -= A[9] * B[6];
  C[5] -= A[13] * B[7];

  /* i = 2 */
  C[6] -= A[2] * B[4];
  C[6] -= A[6] * B[5];
  C[6] -= A[10] * B[6];
  C[6] -= A[14] * B[7];

  /* i = 3 */
  C[7] -= A[3] * B[4];
  C[7] -= A[7] * B[5];
  C[7] -= A[11] * B[6];
  C[7] -= A[15] * B[7];

  /** j = 2 **/

  /* i = 0 */
  C[8] -= A[0] * B[8];
  C[8] -= A[4] * B[9];
  C[8] -= A[8] * B[10];
  C[8] -= A[12] * B[11];

  /* i = 1 */
  C[9] -= A[1] * B[8];
  C[9] -= A[5] * B[9];
  C[9] -= A[9] * B[10];
  C[9] -= A[13] * B[11];

  /* i = 2 */
  C[10] -= A[2] * B[8];
  C[10] -= A[6] * B[9];
  C[10] -= A[10] * B[10];
  C[10] -= A[14] * B[11];

  /* i = 3 */
  C[11] -= A[3] * B[8];
  C[11] -= A[7] * B[9];
  C[11] -= A[11] * B[10];
  C[11] -= A[15] * B[11];

  /** j = 3 **/

  /* i = 0 */
  C[12] -= A[0] * B[12];
  C[12] -= A[4] * B[13];
  C[12] -= A[8] * B[14];
  C[12] -= A[12] * B[15];

  /* i = 1 */
  C[13] -= A[1] * B[12];
  C[13] -= A[5] * B[13];
  C[13] -= A[9] * B[14];
  C[13] -= A[13] * B[15];

  /* i = 2 */
  C[14] -= A[2] * B[12];
  C[14] -= A[6] * B[13];
  C[14] -= A[10] * B[14];
  C[14] -= A[14] * B[15];

  /* i = 3 */
  C[15] -= A[3] * B[12];
  C[15] -= A[7] * B[13];
  C[15] -= A[11] * B[14];
  C[15] -= A[15] * B[15];
}
/* --------------------------------------------------------------------------- */



/* Computes d -= A*b with matrix dimension 2x2. */
void denseVecMultSub2(realtype * A, realtype * b, realtype * d) {
  /* i = 0 */
  d[0] -= A[0] * b[0];
  d[0] -= A[2] * b[1];

  /* i = 1 */
  d[1] -= A[1] * b[0];
  d[1] -= A[3] * b[1];
}

/* Computes d -= A*b with matrix dimension 3x3. */
void denseVecMultSub3(realtype * A, realtype * b, realtype * d) {
  /* i = 0 */
  d[0] -= A[0] * b[0];
  d[0] -= A[3] * b[1];
  d[0] -= A[6] * b[2];

  /* i = 1 */
  d[1] -= A[1] * b[0];
  d[1] -= A[4] * b[1];
  d[1] -= A[7] * b[2];

  /* i = 2 */
  d[2] -= A[2] * b[0];
  d[2] -= A[5] * b[1];
  d[2] -= A[8] * b[2];
}

/* Computes d -= A*b with matrix dimension 4x4. */
void denseVecMultSub4(realtype * A, realtype * b, realtype * d) {
  /* i = 0 */
  d[0] -= A[0] * b[0];
  d[0] -= A[4] * b[1];
  d[0] -= A[8] * b[2];
  d[0] -= A[12] * b[3];

  /* i = 1 */
  d[1] -= A[1] * b[0];
  d[1] -= A[5] * b[1];
  d[1] -= A[9] * b[2];
  d[1] -= A[13] * b[3];

  /* i = 2 */
  d[2] -= A[2] * b[0];
  d[2] -= A[6] * b[1];
  d[2] -= A[10] * b[2];
  d[2] -= A[14] * b[3];

  /* i = 3 */
  d[3] -= A[3] * b[0];
  d[3] -= A[7] * b[1];
  d[3] -= A[11] * b[2];
  d[3] -= A[15] * b[3];
}
/* --------------------------------------------------------------------------- */
