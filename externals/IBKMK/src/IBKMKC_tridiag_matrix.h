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

#ifndef IBKMKC_tridiag_matrixH
#define IBKMKC_tridiag_matrixH

#include "IBKMK_common_defines.h"

#ifdef __cplusplus  /* wrapper to enable C++ usage */

namespace IBKMK {

extern "C" {

#endif

/*! In-place LU factorization for tridiagonal matrices.
 * \param n Matrix dimension.
 * \param A Pointer to start of tridiagonal memory array.
 * \return Returns 0 for success, otherwise index (1-based) of row where a division by zero occured.
*/
int ibkmk_tridiag_LU(	IBKMK_CONST int n,
						double * A);


/*! Solves the equation system Ax = b using backsolving for tridiagonal matrices, A must be LU factorized already.
 * \param n Matrix dimension.
 * \param A Pointer to start of tridiagonal memory array.
 * \param b Right-hand-side vector of size n. Holds x vector on return.
*/
void ibkmk_tridiag_backsolve(	IBKMK_CONST int n,
								IBKMK_CONST double * A,
								double *b );

/*! Performs the matrix vector multiplication A b = r.
 * \param n Matrix dimension.
 * \param A Pointer to start of tridiagonal memory array (3 times n).
 * \param b Vector (size n) to multiply matrix with.
 * \param r Vector (size n) to hold the result.
*/
void ibkmk_tridiag_vec_mult(	IBKMK_CONST int n,
								IBKMK_CONST double * A,
								IBKMK_CONST double * b,
								double * r);


/*! In-place LU factorization for block tridiagonal matrices with non-pivoting LU decomposition of blocks.
 * \param nblocks Blockbased matrix dimension
 * \param blocksize Dimension of a subblock, e.g. 2 means 2x2 matrix.
 * \param L Lower (block) diagonal (array of pointers to individual block memory arrays).
 * \param M Main (block) diagonal (array of pointers to individual block memory arrays).
 * \param U Upper (block) diagonal (array of pointers to individual block memory arrays).
 * \return Returns 0 for success, otherwise index (1-based) of row where a division by zero occured.
*/
int ibkmk_btridiag_LU(	IBKMK_CONST int nblocks,
						IBKMK_CONST int blocksize,
						double ** L,
						double ** M,
						double ** U );

/*! In-place LU factorization for block tridiagonal matrices with pivoting LU decomposition of blocks.
 * \param nblocks Blockbased matrix dimension
 * \param blocksize Dimension of a subblock, e.g. 2 means 2x2 matrix.
 * \param L Lower (block) diagonal (array of pointers to individual block memory arrays).
 * \param M Main (block) diagonal (array of pointers to individual block memory arrays).
 * \param U Upper (block) diagonal (array of pointers to individual block memory arrays).
 * \param p Pivot vector of size n * nblocks.
 * \return Returns 0 for success, otherwise index (1-based) of row where a division by zero occured.
*/
int ibkmk_btridiag_LU_pivot(	IBKMK_CONST int nblocks,
								IBKMK_CONST int blocksize,
								double ** L,
								double ** M,
								double ** U,
								long int * p );



/*! Solves the equation system (LMU)x = b using backsolving, A must be LU factorized already.
 * \param nblocks Blockbased matrix dimension
 * \param blocksize Dimension of a subblock, e.g. 2 means 2x2 matrix.
 * \param L Lower (block) diagonal (const array of pointers to individual const block memory arrays).
 * \param M Main (block) diagonal (const array of pointers to individual const block memory arrays).
 * \param U Upper (block) diagonal (const array of pointers to individual const block memory arrays).
 * \param b Right-hand-side vector of size n * nblocks.
*/
void ibkmk_btridiag_backsolve(	IBKMK_CONST int nblocks,
								IBKMK_CONST int blocksize,
								IBKMK_CONST double * IBKMK_CONST * L,
								IBKMK_CONST double * IBKMK_CONST * M,
								IBKMK_CONST double * IBKMK_CONST * U,
								double * b);


/*! Solves the equation system (LMU)x = b using backsolving, A must be LU factorized already.
 * \param nblocks Blockbased matrix dimension
 * \param blocksize Dimension of a subblock, e.g. 2 means 2x2 matrix.
 * \param L Lower (block) diagonal (const array of pointers to individual const block memory arrays).
 * \param M Main (block) diagonal (const array of pointers to individual const block memory arrays)..
 * \param U Upper (block) diagonal (const array of pointers to individual const block memory arrays)..
 * \param p Pivot vector of size n * nblocks
 * \param b Right-hand-side vector of size n * nblocks.
*/
void ibkmk_btridiag_backsolve_pivot(	IBKMK_CONST int nblocks,
										IBKMK_CONST int blocksize,
										IBKMK_CONST double * IBKMK_CONST * L,
										IBKMK_CONST double * IBKMK_CONST * M,
										IBKMK_CONST double * IBKMK_CONST * U,
										IBKMK_CONST long int * p,
										double *b);


#ifdef __cplusplus
} // namespace IBKMKC

} // extern "C"
#endif

/*! \file IBKMKC_tridiag_matrix.h
	\brief Routines for tridiagonal matrix operations with matrices stored in column-major format.
*/

#endif // IBKMKC_tridiag_matrixH
