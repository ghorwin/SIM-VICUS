/*
 * -----------------------------------------------------------------
 * Programmer(s): Andreas Nicolai @ TU Dresden, Germany
 * -----------------------------------------------------------------
 * Code is based on original cvode_band.h file.
 * -----------------------------------------------------------------
 * This is the header file for the CVODE block-tridiagonal linear 
 * solver, CVBTRIDIAG.
 * -----------------------------------------------------------------
 */

#ifndef _CVBTRIDIAG_H
#define _CVBTRIDIAG_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <cvode/cvode_direct.h>
#include <sundials/sundials_btridiag.h>
 
/*
 * -----------------------------------------------------------------
 * Function : CVBTridiag
 * -----------------------------------------------------------------
 * A call to the CVBTridiag function links the main CVODE integrator
 * with the CVBTRIDIAG linear solver.
 *
 * cvode_mem is the pointer to the integrator memory returned by
 *           CVodeCreate.
 *
 * nb is the size of the ODE system in blocks/submatricies. The 
 *    totalnumber of unknowns is computed from nb*blocksize.
 *
 * blocksize is the dimension (blocksize x blocksize) of the 
 *           blocks/submatricies in the block-tridiagonal matrix.
 *
 * The return value of CVBTridiag is one of:
 *    CVDLS_SUCCESS   if successful
 *    CVDLS_MEM_NULL  if the cvode memory was NULL
 *    CVDLS_MEM_FAIL  if there was a memory allocation failure
 *    CVDLS_ILL_INPUT if a required vector operation is missing or
 *                    if the size or blocksize has an illegal value.
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int CVBTridiag(void *cvode_mem, long int nb, long int blocksize);

#ifdef __cplusplus
}
#endif

#endif
