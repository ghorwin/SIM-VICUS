/*
 * -----------------------------------------------------------------
 * Programmer(s): Andreas Nicolai @ TU Dresden, Germany
 * -----------------------------------------------------------------
 * Code is based on original cvode_band.h file.
 * -----------------------------------------------------------------
 * This is the header file for a generic BLOCK-TRIDIAGONAL linear 
 * solver package, based on the DlsMat type defined in 
 * sundials_direct.h.
 *
 * There are two sets of block-tridiagonal solver routines listed in
 * this file: one set uses type DlsMat defined below and the
 * other set uses the type realtype ** for block-tridiag matrix 
 * arguments.
 * Routines that work with the type DlsMat begin with "BTridiag".
 * Routines that work with realtype ** begin with "btridiag"
 * -----------------------------------------------------------------
 */

#ifndef _SUNDIALS_BTRIDIAG_H
#define _SUNDIALS_BTRIDIAG_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <sundials/sundials_direct.h>

/*
 * -----------------------------------------------------------------
 * Function : BTridiagBTTRF
 * -----------------------------------------------------------------
 * Usage : ier = BTridiagBTTRF(A);
 *         if (ier != 0) ... A is singular
 * -----------------------------------------------------------------
 * BTridiagBTTRF performs the LU factorization of the N by N 
 * block-tridiagonal matrix A. This is done using a block-based
 * version of Crout's algorithm.
 *
 * A successful LU factorization leaves the "matrix" A
 * with the following information:
 *
 * (1) If the unique LU factorization of A is given by A = LU,
 *     then the matrix U is stored in the upper band with the implicit
 *     definition of a unity main diagonal. The main and lower band
 *     hold the matrix L.
 *
 * BTridiagBTTRF returns 0 if successful. Otherwise it encountered
 * a zero diagonal element during the factorization or the LU 
 * factorization of a block in the main diagonal failed.
 * In this case it returns the block-based column index 
 * (numbered from one) at which it encountered the zero.
 *
 * BTridiagBTTRF is only a wrapper around btridiagBTTRF. All work is done
 * in btridiagBTTRF works directly on the data in the DlsMat A (i.e.,
 * the field cols).
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int BTridiagBTTRF(DlsMat A, long int * pivots);
SUNDIALS_EXPORT int btridiagBTTRF(realtype **a, long int nblocks, long int blocksize, 
                                  long int * pivots);

/*
 * -----------------------------------------------------------------
 * Function : BTridiagBTTRS
 * -----------------------------------------------------------------
 * Usage : BTridiagBTTRS(A, b);
 * -----------------------------------------------------------------
 * BTridiagBTTRS solves the N-dimensional system A x = b using
 * the LU factorization in A and the pivot information in p
 * computed in BTridiagBTTRF. The solution x is returned in b. This
 * routine cannot fail if the corresponding call to BTridiagBTTRF
 * did not fail.
 *
 * BTridiagBTTRS is only a wrapper around btridiagBTTRS which does 
 * all the work directly on the data in the DlsMat A (i.e., the 
 * field cols).
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT void BTridiagBTTRS(DlsMat A, long int * pivots, realtype *b);
SUNDIALS_EXPORT void btridiagBTTRS(realtype **a, long int nblocks, long int blocksize, long int * pivots, realtype *b);

/*
 * -----------------------------------------------------------------
 * Function : BTridiagCopy
 * -----------------------------------------------------------------
 * Usage : BTridiagCopy(A, B);
 * -----------------------------------------------------------------
 * BTridiagCopy copies the block-tridiagonal matrix A into 
 * the block-tridiagonal matrix B.
 * 
 * BTridiagCopy is a wrapper around btridiagCopy which accesses the data
 * in the DlsMat A and B (i.e. the fields cols)
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT void BTridiagCopy(DlsMat A, DlsMat B);
SUNDIALS_EXPORT void btridiagCopy(realtype **a, realtype **b, long int nblocks, long int blocksize );

/*
 * -----------------------------------------------------------------
 * Function: BTridiagScale
 * -----------------------------------------------------------------
 * Usage : BTridiagScale(c, A);
 * -----------------------------------------------------------------
 * A(i,j) <- c*A(i,j),   for all i, j that are part of the 
 *                       block-tridigonal matrix, 
 *                       i.e. fabs(j/blocksize-i/blocksize) <= 1
 *
 * BTridiagScale is a wrapper around btridiagScale which performs 
 * the actual scaling by accessing the data in the DlsMat A (i.e. 
 * the field cols).
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT void BTridiagScale(realtype c, DlsMat A);
SUNDIALS_EXPORT void btridiagScale(realtype c, realtype **a, long int nblocks, long int blocksize);

/*
 * -----------------------------------------------------------------
 * Function: btridiagAddIdentity
 * -----------------------------------------------------------------
 * btridiagAddIdentity adds the identity matrix to the 
 * block-tridiagonal matrix stored in the realtype** arrays.
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT void btridiagAddIdentity(realtype **a, long int nblocks, long int blocksize);

/*
 * -----------------------------------------------------------------
 * Function: BTridiagMult
 * -----------------------------------------------------------------
 * Usage : BTridiagMult(A, b);
 * -----------------------------------------------------------------
 * BTridiagMult is a wrapper around btridiagMult which performs 
 * the matrix-vector multiplication by accessing the data in the 
 * DlsMat A (i.e. the field cols).
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT void BTridiagMult(DlsMat A, realtype * b, realtype * r);
SUNDIALS_EXPORT void btridiagMult(realtype **a, realtype * b, realtype * r, long int nblocks, long int blocksize);

#ifdef __cplusplus
}
#endif

#endif
