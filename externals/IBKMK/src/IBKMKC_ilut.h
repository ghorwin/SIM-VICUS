/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#ifndef IBKMKC_ilutH
#define IBKMKC_ilutH

#ifdef __cplusplus  /* wrapper to enable C++ usage */

#include <cstdio>

namespace IBKMK {

extern "C" {

#else
	#include <stdio.h>
#endif


/*-------------------- matrix structures*/

typedef struct SpaFmt {
/*---------------------------------------------
| C-style CSR format - used internally
| for all matrices in CSR format
|---------------------------------------------*/
  int n;
  int *nzcount;  /* length of each row */
  int **ja;      /* pointer-to-pointer to store column indices  */
  double **ma;   /* pointer-to-pointer to store nonzero entries */
} SparMat, *csptr;

/*---------------------------------------------
| struct for ilu factized matrix
|---------------------------------------------*/
typedef struct ILUfac {
	int n;
	csptr L;      /* L part elements                            */
	double *D;    /* diagonal elements                          */
	csptr U;      /* U part elements                            */
	int *work;    /* working buffer */
} ILUSpar, LDUmat, *iluptr;



/*-------------------- function declaration */

/*----------------------------------------------------------------------------
 * ILUT preconditioner
 * incomplete LU factorization with dual truncation mechanism
 * NOTE : no pivoting implemented as yet in GE for diagonal elements
 *----------------------------------------------------------------------------
 * Parameters
 *----------------------------------------------------------------------------
 * on entry:
 * =========
 * csmat    = matrix stored in SpaFmt format -- see heads.h for details
 * lu       = pointer to a ILUSpar struct -- see heads.h for details
 * lfil     = integer. The fill-in parameter. Each column of L and
 *            each column of U will have a maximum of lfil elements.
 *            NOTE: Setting lfil to a value of less than the original
 *                  non-zero count will effectively drop elements from
 *                  the original matrix pattern.
 *            WARNING: THE MEANING OF LFIL HAS CHANGED WITH RESPECT TO
 *            EARLIER VERSIONS.
 *            lfil must be .ge. 0.
 * tol      = real*8. Sets the threshold for dropping small terms in the
 *            factorization. See below for details on dropping strategy.
 * fp       = file pointer for error log ( might be stdout )
 *
 * on return:
 * ==========
 * ierr     = return value.
 *            ierr  = 0   --> successful return.
 *            ierr  = -1  --> Illegal value for lfil
 *            ierr  = -2  --> zero diagonal or zero col encountered
 * lu->n    = dimension of the matrix
 *   ->L    = L part -- stored in SpaFmt format
 *   ->D    = Diagonals
 *   ->U    = U part -- stored in SpaFmt format
 *----------------------------------------------------------------------------
 * Notes:
 * ======
 * All the diagonals of the input matrix must not be zero
 *----------------------------------------------------------------------------
 * Dual drop-off strategy works as follows.
 *
 * 1) Theresholding in L and U as set by tol. Any element whose size
 *    is less than some tolerance (relative to the norm of current
 *    row in u) is dropped.
 *
 * 2) Keeping only the largest lfil elements in the i-th column of L
 *    and the largest lfil elements in the i-th column of U.
 *
 * Flexibility: one can use tol=0 to get a strategy based on keeping the
 * largest elements in each column of L and U. Taking tol .ne. 0 but lfil=n
 * will give the usual threshold strategy (however, fill-in is then
 * impredictible).
 *--------------------------------------------------------------------------*/
int ibkmk_ilut( csptr csmat, iluptr lu, int lfil, double tol, FILE *fp );

/*----------------------------------------------------------------------
 *    performs a forward followed by a backward solve
 *    for LU matrix as produced by ilut
 *    y  = right-hand-side
 *    x  = solution on return
 *    lu = LU matrix as produced by ilut.
 *--------------------------------------------------------------------*/
int ibkmk_lutsolve( double *y, double *x, iluptr lu );

/*----------------------------------------------------------------------
 *    frees internal memory allocated during call to ilut
 *    lu = LU matrix as produced by ilut.
 *--------------------------------------------------------------------*/
void ibkmk_free_ilut(iluptr lu);

#ifdef __cplusplus
} // namespace IBKMKC

} // extern "C"
#endif

  /*! \file IBKMKC_ilut.h
  \brief Routines for threshold ILU copied from ITSOL library.
  */


#endif // IBKMKC_ilutH
