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

#ifndef IBKMKC_dense_matrixH
#define IBKMKC_dense_matrixH

#include "IBKMK_common_defines.h"

#ifdef __cplusplus  /* wrapper to enable C++ usage */

namespace IBKMK {

extern "C" {

#endif

/*! In-place LU Factorization of A (dimension n x n, column-based storage).
	Pivoting version, requires pivot vector of size n.
	\param n Matrix dimension
	\param A Matrix, size n*n, stored in column-major format.
	\param p Pivot vector, size n
	\return Returns 0 for success, otherwise index of row where an error occured.
*/
int  ibkmk_dense_LU_pivot        ( int n, double * A, long int * p);

/*! Solves the equation system Ax = b using backsolving, A must be
	LU factorized already.
	Pivoting version, requires pivot vector of size n.
	\param n Matrix dimension
	\param A Matrix, size n*n, stored in column-major format, LU factorized.
	\param p Pivot vector, size n
	\param b Right-hand-side vector, size n
*/
void ibkmk_dense_backsolve_pivot ( int n, IBKMK_CONST double * A, IBKMK_CONST long int * p, double * b);

/*! Computes A^-1 * B and stores result in B, A is assumed to be LU factorized.
	Pivoting version, requires pivot vector of size n.
	\param n Matrix dimension
	\param A Matrix, size n*n, stored in column-major format, LU factorized.
	\param p Pivot vector, size n
	\param B Matrix, size n*n to be backsolved
*/
void ibkmk_dense_inverse_mult_pivot( int n, IBKMK_CONST double * A, IBKMK_CONST long int * p, double * B);


/*! In-place LU Factorization of A (dimension n x n, column-based storage).
	Non-Pivoting version.
	\param n Matrix dimension
	\param A Matrix, size n*n, stored in column-major format.
	\return Returns 0 for success, otherwise index of row where an error occured.
*/
int  ibkmk_dense_LU              ( int n, double * A);

/*! Special version of LU algorithm for matrices of dimension 2x2. */
int  ibkmk_dense_LU2             ( double * A);

/*! Special version of LU algorithm for matrices of dimension 3x3. */
int  ibkmk_dense_LU3             ( double * A);

/*! Special version of LU algorithm for matrices of dimension 4x4. */
int  ibkmk_dense_LU4             ( double * A);


/*! Solves the equation system Ax = b using backsolving, A must be
	LU factorized already.
	Non-Pivoting version.
*/
void ibkmk_dense_backsolve       (int n, IBKMK_CONST double * A, double * b);

/*! Special version of backsolving algorithm for matrices of dimension 2x2. */
void ibkmk_dense_backsolve2      (IBKMK_CONST double * A, double * b);

/*! Special version of backsolving algorithm for matrices of dimension 3x3. */
void ibkmk_dense_backsolve3      (IBKMK_CONST double * A, double * b);

/*! Special version of backsolving algorithm for matrices of dimension 4x4. */
void ibkmk_dense_backsolve4      (IBKMK_CONST double * A, double * b);


/*! Performs the matrix multiplication C = A*B.

	The matrices must be square matrices and must be stored column-based in
	a linear memory block of size n*n.
	\param n Size of matrix
	\param A Input matrix A, as linear memory array.
	\param B Input matrix B, as linear memory array.
	\param C Result matrix C, as linear memory array.
*/
void ibkmk_dense_mat_mult(int n, IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);

/*! Performs the matrix multiplication C = A*B for matrices of dimension 2x2. */
void ibkmk_dense_mat_mult2(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);

/*! Performs the matrix multiplication C = A*B for matrices of dimension 3x3. */
void ibkmk_dense_mat_mult3(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);

/*! Performs the matrix multiplication C = A*B for matrices of dimension 4x4. */
void ibkmk_dense_mat_mult4(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);


/*! Solves B = A^-1*B.

	The function does a backsolve operation for each column in the matrix.

	A must contain the decomposed matrix A. Matrices A and B must be stored in
	column-based linear memory blocks and have the size n*n. The vector pivots
	must contain the pivoting information from the decomposition of the matrix A.
*/
void ibkmk_dense_inverse_mult(int n, IBKMK_CONST double * A, double * B);

/*! Solves B = A^-1*B for a matrix of dimension 2x2.

	The function does a backsolve operation for each column in the matrix.

	A must contain the decomposed matrix A. Matrices A and B must be stored in
	column-based linear memory blocks and have the size 4.
*/
void ibkmk_dense_inverse_mult2(IBKMK_CONST double * A, double * B);

/*! Solves B = A^-1*B for a matrix of dimension 3x3.

	The function does a backsolve operation for each column in the matrix.

	A must contain the decomposed matrix A. Matrices A and B must be stored in
	column-based linear memory blocks and have the size 9.
*/
void ibkmk_dense_inverse_mult3(IBKMK_CONST double * A, double * B);

/*! Solves B = A^-1*B for a matrix of dimension 4x4.

	The function does a backsolve operation for each column in the matrix.

	A must contain the decomposed matrix A. Matrices A and B must be stored in
	column-based linear memory blocks and have the size 16.
*/
void ibkmk_dense_inverse_mult4(IBKMK_CONST double * A, double * B);


/*! Computes C -= A*B. */
void ibkmk_dense_mat_mult_sub(int n, IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);

/*! Computes C -= A*B with matrix dimension 2x2. */
void ibkmk_dense_mat_mult_sub2(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);

/*! Computes C -= A*B with matrix dimension 3x3. */
void ibkmk_dense_mat_mult_sub3(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);

/*! Computes C -= A*B with matrix dimension 4x4. */
void ibkmk_dense_mat_mult_sub4(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C);


/*! Computes d += A*b */
void ibkmk_dense_vec_mult_add(int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Computes d += A*b with matrix dimension 2x2. */
void ibkmk_dense_vec_mult_add2(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Computes d += A*b with matrix dimension 3x3. */
void ibkmk_dense_vec_mult_add3(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Computes d += A*b with matrix dimension 4x4. */
void ibkmk_dense_vec_mult_add4(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);


/*! Computes d -= A*b */
void ibkmk_dense_vec_mult_sub(int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Computes d -= A*b with matrix dimension 2x2. */
void ibkmk_dense_vec_mult_sub2(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Computes d -= A*b with matrix dimension 3x3. */
void ibkmk_dense_vec_mult_sub3(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Computes d -= A*b with matrix dimension 4x4. */
void ibkmk_dense_vec_mult_sub4(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);



/*! Performs the matrix vector multiplication A b = d. */
void ibkmk_dense_vec_mult(int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Special implementation of the matrix vector multiplication a matrix of dimension 2x2. */
void ibkmk_dense_vec_mult2(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Special implementation of the matrix vector multiplication a matrix of dimension 3x3. */
void ibkmk_dense_vec_mult3(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);

/*! Special implementation of the matrix vector multiplication a matrix of dimension 4x4. */
void ibkmk_dense_vec_mult4(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d);



/*! Computes the matrix x vector product of a block-matrix row multiplied with the block-vector b and
	stores the result in the result vector r.
	\param Arow Pointer to a memory block of at least size n*m*m holding the block-matrices, each stored in column-major order
	\param m Dimension of the sub-matrices.
	\param n Number of blocks to multiply.
	\param b  Vector of at least size n*m
	\param r  Vector of at least size m

	\code
	// for m == 1
	r[0] = Arow[0]*b[0] + Arow[1]*b[1] + ... + Arow[n-1]*b[n-1]
	\endcode

*/
void ibkmk_dense_row_matvecsum(IBKMK_CONST double * Arow, int m, int n, IBKMK_CONST double * b, double * r);

#ifdef __cplusplus
} // namespace IBKMKC

} // extern "C"
#endif

/*! \file IBKMKC_dense_matrix.h
	\brief Routines for dense matrix operations with matrices stored in column-major format.
*/

#endif // IBKMKC_dense_matrixH
