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

#ifndef IBKMKC_sparse_matrixH
#define IBKMKC_sparse_matrixH

#include "IBKMK_common_defines.h"

#ifdef __cplusplus  /* wrapper to enable C++ usage */

namespace IBKMK {

extern "C" {

#endif

/*! Generic sparse matrix - vector multiply r = A*b.
	This function will call special implementations based on number of non-zero elements per row.
	\param n Matrix dimension.
	\param m Number of non-zero elements per row.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n.
	\param r Result vector, size n.
*/
void ibkmk_spmat_eid_multiply(	unsigned int n,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									IBKMK_CONST double * A,
									IBKMK_CONST double * b,
									double * r);

/*! Sparse matrix incomplete LU factorization - ILU(0)
	This function relies on correct setup of the index vector, see see description of sparse-matrix Ellpack-Itpack format.
	\param n Matrix dimension.
	\param m Number of non-zero elements per row.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, will hold iLU factors when function finishes, size n*m, see description of sparse-matrix Ellpack-Itpack format.
*/
void ibkmk_spmat_eid_ilu(	unsigned int n,
							unsigned int m,
							IBKMK_CONST unsigned int * index,
							double * A);

/*! Sparse matrix incomplete backsolve iLU x = b.
	This function will call special implementations based on number of non-zero elements per row.
	\param n Matrix dimension.
	\param m Number of non-zero elements per row.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data (already ILU-factorized), size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param b Right-hand side vector, size n.
*/
void ibkmk_spmat_eid_backsolve(	unsigned int n,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									IBKMK_CONST double * A,
									double * b);

/*! Generic block sparse matrix - vector multiply r = A*b.
	This function will call special implementations based on the submatrix dimensions.
	\param n Matrix dimension.
	\param m Number of non-zero blocks per row.
	\param nSubMatrix Dimension of the submatrices/blocks.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*nSubMatrix*nSubMatrix, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n*nSubMatrix.
	\param r Result vector, size n*nSubMatrix.
*/
void ibkmk_blockspmat_eid_multiply(	unsigned int n,
										unsigned int m,
										unsigned int nSubMatrix,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r);


/*! Generic sparse matrix incomplete LU factorization - ILU(0) for symmetric matrix pattern
	This function relies on correct setup of the index vector, see see description of sparse-matrix Ellpack-Itpack format.
	\param n Matrix dimension.
	\param ja Column index vector, size nnz, see description of sparse-matrix Compresses Sparse Row  format.
	\param ia Row index vector, size n+1, see description of block-sparse-matrix Compresses Sparse Row format.
	\param A Matrix data, will hold iLU factors when function finishes, size nnz.
*/
void ibkmk_spmat_csr_ilu_symm(	unsigned int n,
							IBKMK_CONST unsigned int * ia,
							IBKMK_CONST unsigned int * ja,
							double * A);

/*! Generic sparse matrix incomplete LU factorization - ILU(0) for asymmetric matrix pattern
	This function relies on correct setup of the index vector, see see description of sparse-matrix Ellpack-Itpack format.
	\param n Matrix dimension.
	\param ja Column index vector, size nnz, see description of sparse-matrix Compresses Sparse Row  format.
	\param ia Row index vector, size n+1, see description of block-sparse-matrix Compresses Sparse Row format.
	\param jaT Column index vector for the transposed matrix, size nnz, needed for non-symmetric generic matrix format.
	\param iaT Row index vectorfor the transposed matrix, size n+1, needed for non-symmetric generic matrix format.
	\param A Matrix data, will hold iLU factors when function finishes, size nnz.
*/
void ibkmk_spmat_csr_ilu_asymm(	unsigned int n,
							IBKMK_CONST unsigned int * ia,
							IBKMK_CONST unsigned int * ja,
							IBKMK_CONST unsigned int * iaT,
							IBKMK_CONST unsigned int * jaT,
							double * A);

/*! Generic sparse matrix incomplete backsolve iLU x = b.
	This function will call special implementations based on number of non-zero elements per row.
	\param n Matrix dimension.
	\param ja Column index vector, size nnz, see description of sparse-matrix Compresses Sparse Row  format.
	\param ia Row index vector, size n+1, see description of block-sparse-matrix Compresses Sparse Row format.
	\param A Matrix data (already ILU-factorized), size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param b Right-hand side vector, size n.
*/
void ibkmk_spmat_csr_backsolve(	unsigned int n,
								IBKMK_CONST unsigned int * ia,
								IBKMK_CONST unsigned int * ja,
								IBKMK_CONST double * A,
								double * b);

/*! Generic sparse matrix - vector multiply r = A*b.
	\param n Matrix dimension.
	\param nnz Number of non-zero elements.
	\param A Matrix data, size nnz, see description of block-sparse-matrix Compresses Sparse Row format.
	\param ja Column index vector, size nnz, see description of sparse-matrix Compresses Sparse Row  format.
	\param ia Row index vector, size n+1, see description of block-sparse-matrix Compresses Sparse Row format.
	\param b Vector, size n.
	\param r Result vector, size n.
*/
void ibkmk_spmat_csr_multiply(	unsigned int n,
								IBKMK_CONST double * A,
								IBKMK_CONST unsigned int * ia,
								IBKMK_CONST unsigned int * ja,
								IBKMK_CONST double * b,
								double * r);

/*! Generic block sparse matrix - vector multiply r = A*b.
	This function will call special implementations based on the submatrix dimensions.
	\param n Matrix dimension.
	\param nnz Number of non-zero elements.
	\param nSubMatrix Dimension of the submatrices/blocks.
	\param A Matrix data, size nnz*nSubMatrix*nSubMatrix, see description of block-sparse-matrix Block Compresses Sparse Row format.
	\param ja Column index vector, size nnz, see description of sparse-matrix Block Compresses Sparse Row  format.
	\param ia Row index vector, size n+1, see description of block-sparse-matrix Block Compresses Sparse Row format.
	\param b Vector, size n*nSubMatrix.
	\param r Result vector, size n*nSubMatrix.
*/
void ibkmk_blockspmat_csr_multiply(	unsigned int n,
										unsigned int nSubMatrix,
										IBKMK_CONST double * A,
										IBKMK_CONST unsigned int * ia,
										IBKMK_CONST unsigned int * ja,
										IBKMK_CONST double * b,
										double * r);


/*! Sparse matrix incomplete LU factorization - ILU(0)
	This function relies on correct setup of the index vector, see see description of sparse-matrix Ellpack-Itpack format.
	\param n Matrix dimension.
	\param nSubMatrix Dimension of the submatrices/blocks, equals number of balance equations.
	\param m Number of non-zero elements per row.
	\param index Index vector, size n*m, see description of block-sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, will hold iLU factors when function finishes, size n*m, see description of block-sparse-matrix Ellpack-Itpack format.
*/
void ibkmk_blockspmat_eid_ilu(	unsigned int n,
									unsigned int nSubMatrix,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									double * A);

/*! Sparse matrix incomplete backsolve iLU x = b.
	This function will call special implementations based on number of non-zero elements per row.
	\param n Matrix dimension.
	\param nBalanceEquations Dimension of the submatrices/blocks, equals number of balance equations.
	\param m Number of non-zero elements per row.
	\param index Index vector, size n*m, see description of block-sparse-matrix Ellpack-Itpack format.
	\param A Matrix data (already ILU-factorized), size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param b Right-hand side vector, size n.
*/
void ibkmk_blockspmat_eid_backsolve(	unsigned int n,
										unsigned int nSubMatrix,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										double * b);


#ifdef __cplusplus
} // namespace IBKMKC

} // extern "C"
#endif

/*! \file IBKMKC_sparse_matrix.h
	\brief Routines for sparse matrix operations with matrices stored in Ellpack-Itpack format.
*/

#endif // IBKMKC_sparse_matrixH
