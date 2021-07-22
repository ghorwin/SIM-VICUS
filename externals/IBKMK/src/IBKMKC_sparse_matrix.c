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

#include "IBKMKC_sparse_matrix.h"


#include <assert.h>
#include <stdio.h>

#include <IBK_openMP.h>

/*** Core Utility Routines ***/

/*! Generic version of matrix vector add d = d + A*b for all matrix sizes.
	\param n Matrix dimension
	\param A Pointer to matrix data (size = n*n) in column-major storage.
	\param b Vector of size n
	\param d Vector of size n
*/
void dense_vec_mult_add(unsigned int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {
	unsigned int k, i;
	for (k=0; k<n; ++k)
		for (i=0; i<n; ++i)
			d[i] += A[i + k*n] * b[k];
}

/*! Unrolled version of matrix vector add d = d + A*b for matrix size of 2.
	\param A Pointer to matrix data (size = 2*2) in column-major storage.
	\param b Vector of size 2
	\param d Vector of size 2
*/
void dense_vec_mult_add2(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {
	d[0] += A[0] * b[0] + A[2] * b[1];
	d[1] += A[1] * b[0] + A[3] * b[1];
}

/*! Unrolled version of matrix vector add d = d + A*b for matrix size of 3.
	\param A Pointer to matrix data (size = 3*3) in column-major storage.
	\param b Vector of size 3
	\param d Vector of size 3
*/
void dense_vec_mult_add3(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {
	d[0] += A[0] * b[0] + A[3] * b[1] + A[6] * b[2];
	d[1] += A[1] * b[0] + A[4] * b[1] + A[7] * b[2];
	d[2] += A[2] * b[0] + A[5] * b[1] + A[8] * b[2];
}

/*** Prototypes of Special Implementations ***/

/*! Specialized version of matrix vector multiply r = A*b for m=5.
	\param n Matrix dimension.
	\param index Index vector, size n*5, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*5, see description of sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n.
	\param r Result vector, size n.
*/
void ibkmk_spmat_eid_multiply5(	unsigned int n,
									IBKMK_CONST unsigned int * IBKMK_RESTRICT index,
									IBKMK_CONST double * IBKMK_RESTRICT A,
									IBKMK_CONST double * IBKMK_RESTRICT b,
									double * IBKMK_RESTRICT r);

/*! Specialized version of matrix vector multiply r = A*b for m=10.
	\param n Matrix dimension.
	\param index Index vector, size n*10, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*10, see description of sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n.
	\param r Result vector, size n.
*/
void ibkmk_spmat_eid_multiply10(	unsigned int n,
									IBKMK_CONST unsigned int * index,
									IBKMK_CONST double * A,
									IBKMK_CONST double * b,
									double * r);

/*! Specialized version of matrix vector multiply r = A*b for m=7.
	\param n Matrix dimension.
	\param index Index vector, size n*7, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*7, see description of sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n.
	\param r Result vector, size n.
*/
void ibkmk_spmat_eid_multiply7(	unsigned int n,
									IBKMK_CONST unsigned int * IBKMK_RESTRICT index,
									IBKMK_CONST double * IBKMK_RESTRICT A,
									IBKMK_CONST double * IBKMK_RESTRICT b,
									double * IBKMK_RESTRICT r);

/*! Specialized version of matrix vector multiply r = A*b for m=14.
	\param n Matrix dimension.
	\param index Index vector, size n*14, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*14, see description of sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n.
	\param r Result vector, size n.
*/
void ibkmk_spmat_eid_multiply14(	unsigned int n,
									IBKMK_CONST unsigned int * index,
									IBKMK_CONST double * A,
									IBKMK_CONST double * b,
									double * r);

/*! Specialized version of matrix vector multiply r = A*b for m=15.
	\param n Matrix dimension.
	\param index Index vector, size n*15, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*15, see description of sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n.
	\param r Result vector, size n.
*/
void ibkmk_spmat_eid_multiply15(	unsigned int n,
									IBKMK_CONST unsigned int * index,
									IBKMK_CONST double * A,
									IBKMK_CONST double * b,
									double * r);


/*! Specialized version of block sparse matrix vector multiply r = A*b for nSubmatrix = 2.
	\param n Matrix dimension.
	\param m Number of non-zero blocks per row.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*2*2, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n*2.
	\param r Result vector, size n*2.
*/
void ibkmk_blockspmat_eid_multiply2(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r);

/*! Specialized version of block sparse matrix vector multiply r = A*b for nSubmatrix = 3.
	\param n Matrix dimension.
	\param m Number of non-zero blocks per row.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*3*3, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n*3.
	\param r Result vector, size n*3.
*/
void ibkmk_blockspmat_eid_multiply3(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r);



/*! Specialized version of block sparse matrix vector multiply r = A*b for nSubmatrix = 2; m = 5.
	\param n Matrix dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*2*2, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n*2.
	\param r Result vector, size n*2.
*/
void ibkmk_blockspmat_eid_multiply10(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r);


/*! Specialized version of block sparse matrix vector multiply r = A*b for nSubmatrix = 2, m = 7.
	\param n Matrix dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*2*2, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n*2.
	\param r Result vector, size n*2.
*/
void ibkmk_blockspmat_eid_multiply14(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r);

/*! Specialized version of block sparse matrix vector multiply r = A*b for nSubmatrix = 3; m = 5.
	\param n Matrix dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*3*3, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n*3.
	\param r Result vector, size n*3.
*/
void ibkmk_blockspmat_eid_multiply15(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r);


/*! Specialized version of block sparse matrix vector multiply r = A*b for nSubmatrix = 3, m = 7.
	\param n Matrix dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*3*3, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b Vector, size n*3.
	\param r Result vector, size n*3.
*/
void ibkmk_blockspmat_eid_multiply21(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r);


/*! Specialized version of block sparse matrix ilu for nSubmatrix = 2.
	\param n Matrix dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*2*2, see description of block-sparse-matrix Ellpack-Itpack format.
*/
void ibkmk_blockspmat_eid_ilu2(	unsigned int n,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									double * A);



/*! Specialized version of block sparse matrix ilu for nSubmatrix = 3.
	\param n Matrix dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*3*3, see description of block-sparse-matrix Ellpack-Itpack format.
*/
void ibkmk_blockspmat_eid_ilu3(	unsigned int n,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									double * A);

/*! Specialized version of block sparse matrix backsolving for nSubmatrix = 2.
	\param n Matrix dimension (blocks).
	\param m block dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*2*2, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b rhs vector, size n*2.
*/
void ibkmk_blockspmat_eid_backsolve2(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										double * b);

/*! Specialized version of block sparse matrix backsolving for nSubmatrix = 3.
	\param n Matrix dimension (blocks).
	\param m block dimension.
	\param index Index vector, size n*m, see description of sparse-matrix Ellpack-Itpack format.
	\param A Matrix data, size n*m*3*3, see description of block-sparse-matrix Ellpack-Itpack format.
	\param b rhs vector, size n*3.
*/
void ibkmk_blockspmat_eid_backsolve3(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										double * b);


/*** Implementations of public functions ***/

void ibkmk_spmat_eid_multiply( unsigned int n, unsigned int m, IBKMK_CONST unsigned int * index, IBKMK_CONST double * A, IBKMK_CONST double * b, double * r) {
	switch(m) {
		case 5 :
			ibkmk_spmat_eid_multiply5( n, index, A, b, r );
			break;
		case 7 :
			ibkmk_spmat_eid_multiply7( n, index, A, b, r );
			break;
		case 10 :
			ibkmk_spmat_eid_multiply10( n, index, A, b, r );
			break;
		case 14 :
			ibkmk_spmat_eid_multiply14( n, index, A, b, r );
			break;
		case 15 :
			ibkmk_spmat_eid_multiply15( n, index, A, b, r );
			break;

		default: {

			/* generic routine */
			int i = 0;
			unsigned int j = 0;
			unsigned int k = 0;

			/* loop over all rows */
#pragma omp parallel for private(k)
			for (i=0; i<(int)n; ++i) {
				k = i*m;
				r[i] = 0;
				/* process all elements in this row */
				for (j=0; j<m; ++j) {
					r[i] += A[k]*b[ index[k] ];
					++k;
				}
			}

		} break;
	}
}

void ibkmk_spmat_eid_ilu(unsigned int n, unsigned int m, IBKMK_CONST unsigned int * index, double * A) {
	unsigned int i,j,k,s,jidx, jidxfirst, jidxend, iidx, iidxend, n1, sidx, sidxend;
	double A_kk_inv, A_ik;

	A_kk_inv = 0; /* Initialization only necessary to avoid compiler warning */

	/* Main elimination loop, process all rows */
	for (k=0, n1 = n-1; k<n1; ++k) {
		/* Upper triangular matrix part, L_kk is already known, we can compute all U_kj */
		for (jidxfirst=jidx=k*m, jidxend = jidx + m; jidx<jidxend; ++jidx) {
			/* retrieve column index j */
			j = index[jidx];
			/* skip columns below k */
			if (j<k)
				continue;
			/* check for invalid/unused element, "jidxfirst != jidx" ensures that the check is only executed on second index and later */
			if (jidxfirst != jidx && j == index[jidx - 1])
				break;
			/* if we have j == k we found the index for the main diagonal element and store it */
			if (j == k) {
				/* store inverse of diagonal matrix element */
				A_kk_inv = 1.0/A[jidx];
				continue;
			}
			/* we have found a valid element in column j, do A_kj := A_kj/A_kk */
			A[jidx] *= A_kk_inv;

			A_ik = 0; /* Initialization only necessary to avoid compiler warning */
			/* Lower triangular matrix part */
			/* Loop over all rows with elements in column k */
			for (iidx=k*m, iidxend = iidx + m; iidx < iidxend; ++iidx) {
				i = index[iidx]; /* !!!Symmetric matrix pattern required!!! */
				/* skip rows below k */
				if (i<=k)
					continue;
				/* stop when encountering invalid elements,
				   since i > k, and we have always the diagonal element, this check will only be encountered when
				   iidx has been increased at least once.
				*/
				if (i == index[iidx-1])
					break;
				/* We now have a row i, which has an element in column k */
				/* find A_ik and A_ij */
				for (sidx = i*m, sidxend = sidx+m; sidx < sidxend; ++sidx) {
					s = index[sidx];
					if (s==k)
						A_ik = A[sidx]; /* k < j, therefore this will be executed first */
					else if (s==j) {
						A[sidx] -= A_ik*A[jidx];
						break;
					}
				} /* sidx */

			} /* iidx */

		} /* jidx */

	} /* k */

}


void ibkmk_spmat_eid_backsolve(unsigned int n, unsigned int m, IBKMK_CONST unsigned int * index,
	IBKMK_CONST double * A, double * b)
{
	unsigned int i, kIdxEnd, k=0, kIdx;
	/** firstly, L loop (forward elimination) **/

	/* First row, main diagonal element is always at first position */
	b[0] /= A[0]; /* bn_0 = b_0/l_00 */
	/* Remaining n-1 rows */
	for (i=1; i<n; ++i) {
		/* first subtract all known factorials, sum_k l_ik*b_k */
		/* loop over all indices in row i, up to but not including the main diagonal element */
		/* kIdx    -> first column index in row i */
		/* kIdxEnd -> last column index +1 in row i */
		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
			k = index[kIdx];
			/* stop once diagonal element (k == i) found */
			if (k == i)
				break;
			/* subtract product from b[i] */
			b[i] -= A[kIdx]*b[k];
		}
		/* must have stopped multiplication loop at diagonal element */
		assert(i == k);
		assert(kIdx < kIdxEnd);
		/* finally divide by diagonal element */
		b[i] /= A[kIdx];
	}
	/* now b holds L^-1*b */

	/* secondly, U loop (backward elimination), with special case u_i,i = 1 */
	for (i=n-2; (int)i>=0; --i) {
		/* subtract all previously known solutions from */
		/* process all indices in row i backwards, stop when reaching main diagonal element */
		/* kIdx    -> first column index in row i */
		/* kIdxEnd -> last column index +1 in row i */
		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
			k = index[kIdx];
			/* skip columns <= i */
			if (k<=i)
				continue;
			/* subtract product from b[i], don't bother if multiplying invalid elements, because A[kIdx] == 0 for invalid elements. */
			b[i] -= A[kIdx]*b[k];
		}
	}
	/* now b holds U^-1 * L^-1 * b	*/
}


void ibkmk_blockspmat_eid_multiply(	unsigned int n,
										unsigned int m,
										unsigned int nSubMatrix,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r)
{
	IBKMK_CONST double * dataRow = A;
	IBKMK_CONST unsigned int * indexRow = index;
	IBKMK_CONST double * Ablock, * bBlock;

	unsigned int i,j,k;

	unsigned int blockSize;

	switch (nSubMatrix) {
		case 1 :
			ibkmk_spmat_eid_multiply(n, m, index, A, b, r );
			break;

		case 2 :
			switch (m) {
				case 5 :
					ibkmk_blockspmat_eid_multiply10( n, index, A, b, r );
					break;
				case 7 :
					ibkmk_blockspmat_eid_multiply14( n, index, A, b, r );
					break;
				default:
					ibkmk_blockspmat_eid_multiply2(n, m, index, A, b, r );
					break;
			}
			break;

		case 3 :
			switch (m) {
				case 5 :
					ibkmk_blockspmat_eid_multiply15( n, index, A, b, r );
					break;
				case 7 :
					ibkmk_blockspmat_eid_multiply21( n, index, A, b, r );
					break;
				default:
					ibkmk_blockspmat_eid_multiply3(n, m, index, A, b, r );
					break;
			}
			break;

		default : {

			blockSize = nSubMatrix*nSubMatrix;
//#if defined(_OPENMP)
//#pragma omp for ordered schedule(static,1)
//#endif
			for (i=0; i<n; ++i,
				r += nSubMatrix,
				indexRow += m,
				dataRow += m*blockSize)
			{

// we requrire this a small amount of work
//#if defined(_OPENMP)
//#pragma omp ordered
//	{
//#endif
				/* zero out result vector */
				for (k=0; k<nSubMatrix; ++k)
					r[k] = 0;
//#if defined(_OPENMP)
//#pragma omp ordered
//	}
//#enif

// and a larger one
				/* process all indices in this row */
				Ablock = dataRow;
				for (j=0; j<m; ++j, Ablock += blockSize) {
					unsigned int idx = indexRow[j];
					bBlock = b + idx*nSubMatrix;
					dense_vec_mult_add(nSubMatrix, Ablock, bBlock, r);
				}


			}
		} break;

	}
}


void ibkmk_spmat_csr_ilu_symm(unsigned int n,
						IBKMK_CONST unsigned int * ia,
						IBKMK_CONST unsigned int * ja,
						double * A) {
	unsigned int i,j,k,s,jidx, iidx, sidx;
	double A_kk_inv, A_ik;

	A_kk_inv = 0; /* Initialization only necessary to avoid compiler warning */

	/* Main elimination loop, process all rows */
	for (k=0; k < n-1; ++k) {
		/* Upper triangular matrix part, L_kk is already known, we can compute all U_kj */
		for (jidx = ia[k]; jidx < ia[k + 1]; ++jidx) {
			/* retrieve column index j */
			j = ja[jidx];
			/* skip columns below k */
			if (j < k)
				continue;
			/* if we have j == k we found the index for the main diagonal element and store it */
			if (j == k) {
				/* store inverse of diagonal matrix element */
				A_kk_inv = 1.0/A[jidx];
				continue;
			}
			/* we have found a valid element in column j, do A_kj := A_kj/A_kk */
			A[jidx] *= A_kk_inv;

			A_ik = 0; /* Initialization only necessary to avoid compiler warning */
			/* Lower triangular matrix part */
			/* Loop over all rows beginning from k + 1 */
			/* and find A_ik and A_ij */
			for (iidx  = ia[k]; iidx < ia[k + 1]; ++iidx) {
				/* We only accept rows i, which have an entry in column k
				   In the case of symmetric pattern we can use the current index information
				   swapping column and row*/
				i = ja[iidx];
				/* skip rows below k */
				if (i <= k)
					continue;
				/* find A_ik and A_ij */
				for (sidx = ia[i]; sidx < ia[i + 1]; ++sidx) {
					s = ja[sidx];
					if (s==k)
						A_ik = A[sidx]; /* k < j, therefore this will be executed first */
					else if (s==j) {
						/* do A_ij := A_ij - sum_k A_ik * A_kj*/
						A[sidx] -= A_ik*A[jidx];
						break;
					}
				} /* sidx */

			} /* iidx */

		} /* jidx */

	} /* k */

}


void ibkmk_spmat_csr_ilu_asymm(unsigned int n,
						IBKMK_CONST unsigned int * ia,
						IBKMK_CONST unsigned int * ja,
						IBKMK_CONST unsigned int * iaT,
						IBKMK_CONST unsigned int * jaT,
						double * A) {
	unsigned int i,j,k,s,jidx, iidx, sidx;
	double A_kk_inv, A_ik;

	A_kk_inv = 0; /* Initialization only necessary to avoid compiler warning */

	/* Main elimination loop, process all rows */
	for (k=0; k < n-1; ++k) {
		/* Upper triangular matrix part, L_kk is already known, we can compute all U_kj */
		for (jidx = ia[k]; jidx < ia[k + 1]; ++jidx) {
			/* retrieve column index j */
			j = ja[jidx];
			/* skip columns below k */
			if (j < k)
				continue;
			/* if we have j == k we found the index for the main diagonal element and store it */
			if (j == k) {
				/* store inverse of diagonal matrix element */
				A_kk_inv = 1.0/A[jidx];
				continue;
			}
			/* we have found a valid element in column j, do A_kj := A_kj/A_kk */
			A[jidx] *= A_kk_inv;

			A_ik = 0; /* Initialization only necessary to avoid compiler warning */
			/* Lower triangular matrix part */
			/* Loop over all rows beginning from k + 1 */
			/* and find A_ik and A_ij */
			for (iidx  = iaT[k]; iidx < iaT[k + 1]; ++iidx) {
				/* Ensure the following:
				 We only accept rows i, which have an entry in column k */
				i = jaT[iidx];
				/* skip rows below k */
				if (i <= k)
					continue;
				/* find A_ik and A_ij */
				for (sidx = ia[i]; sidx < ia[i + 1]; ++sidx) {
					s = ja[sidx];
					if (s==k)
						A_ik = A[sidx]; /* k < j, therefore this will be executed first */
					else if (s==j) {
						/* do A_ij := A_ij - sum_k A_ik * A_kj*/
						A[sidx] -= A_ik*A[jidx];
						break;
					}
				} /* sidx */

			} /* iidx */

		} /* jidx */

	} /* k */

}


void ibkmk_spmat_csr_backsolve(unsigned int n, IBKMK_CONST unsigned int * ia, IBKMK_CONST unsigned int * ja,
	IBKMK_CONST double * A, double * b)
{
	unsigned int i, k=0, kIdx;
	/** firstly, L loop (forward elimination) **/

	/* First row, main diagonal element is always at first position */
	b[0] /= A[0]; /* bn_0 = b_0/l_00 */
	/* Remaining n-1 rows */
	for (i=1; i<n; ++i) {
		/* first subtract all known factorials, sum_k l_ik*b_k */
		/* loop over all indices in row i, up to but not including the main diagonal element */
		for (kIdx = ia[i]; kIdx < ia[i + 1]; ++kIdx) {
			k = ja[kIdx];
			/* stop once diagonal element (k == i) found */
			if (k == i)
				break;
			/* subtract product from b[i] */
			b[i] -= A[kIdx]*b[k];
		}
		/* must have stopped multiplication loop at diagonal element */
		assert(i == k);
		/* finally divide by diagonal element */
		b[i] /= A[kIdx];
	}
	/* now b holds L^-1*b */

	/* secondly, U loop (backward elimination), with special case u_i,i = 1 */
	for (i=n-2; (int)i>=0; --i) {
		/* subtract all previously known solutions from */
		/* process all indices in row i backwards, stop when reaching main diagonal element */
		for (kIdx = ia[i]; kIdx < ia[i + 1]; ++kIdx) {
			k = ja[kIdx];
			/* skip columns <= i */
			if (k <= i)
				continue;
			/* subtract product from b[i], don't bother if multiplying invalid elements, because A[kIdx] == 0 for invalid elements. */
			b[i] -= A[kIdx]*b[k];
		}
	}
	/* now b holds U^-1 * L^-1 * b	*/
}


void ibkmk_spmat_csr_multiply(	unsigned int n,
								IBKMK_CONST double * A,
								IBKMK_CONST unsigned int * ia,
								IBKMK_CONST unsigned int * ja,
								IBKMK_CONST double * b,
								double * r)
{
	unsigned int i,k;
	double t;
	for (i=0; i<n; ++i) {
		t = 0;
		for (k=ia[i]; k<ia[i+1]; ++k) {
			t += A[k]*b[ja[k]];
		}
		r[i] = t;
	}
}



void ibkmk_blockspmat_csr_multiply(	unsigned int n,
										unsigned int nSubMatrix,
										IBKMK_CONST double * A,
										IBKMK_CONST unsigned int * ia,
										IBKMK_CONST unsigned int * ja,
										IBKMK_CONST double * b,
										double * r)
{
	unsigned int i,k;

	unsigned int blockSize;

	switch (nSubMatrix) {
		case 1 :
			ibkmk_spmat_csr_multiply(n, A, ia, ja, b, r);
			break;

		default : {

			blockSize = nSubMatrix*nSubMatrix;

			for (i=0; i<n; ++i) {
				/* zero out result vector */
				for (k=0; k<nSubMatrix; ++k)
					r[i*nSubMatrix + k] = 0;
				for (k=ia[i]; k<ia[i+1]; ++k) {
					dense_vec_mult_add(nSubMatrix, A + k*blockSize, b + ja[k]*nSubMatrix, r+i*nSubMatrix);
				}
			}
		} break;

	}
}




/* Special Implementations */

void ibkmk_spmat_eid_multiply5(unsigned int n, IBKMK_CONST unsigned int * IBKMK_RESTRICT index, IBKMK_CONST double * IBKMK_RESTRICT A, IBKMK_CONST double * IBKMK_RESTRICT b, double * IBKMK_RESTRICT r) {
	int i, k;
#pragma omp parallel for schedule(static) private(k)
	for (i=0; i<(int)n; ++i) {
		k = i*5;
		/* process all elements in this row */
		r[i]   = A[k  ]*b[ index[k  ] ] +
				 A[k+1]*b[ index[k+1] ] +
				 A[k+2]*b[ index[k+2] ] +
				 A[k+3]*b[ index[k+3] ] +
				 A[k+4]*b[ index[k+4] ];
	}
}


void ibkmk_spmat_eid_multiply10(unsigned int n, IBKMK_CONST unsigned int * index, IBKMK_CONST double * A, IBKMK_CONST double * b, double * r) {
	int i, k;
#pragma omp parallel for schedule(static) private(k)
	for (i=0; i<(int)n; ++i ) {
		k = i*10;
		/* process all elements in this row */
		r[i]   = A[k  ]*b[ index[k  ] ] +
				 A[k+1]*b[ index[k+1] ] +
				 A[k+2]*b[ index[k+2] ] +
				 A[k+3]*b[ index[k+3] ] +
				 A[k+4]*b[ index[k+4] ] +
				 A[k+5]*b[ index[k+5] ] +
				 A[k+6]*b[ index[k+6] ] +
				 A[k+7]*b[ index[k+7] ] +
				 A[k+8]*b[ index[k+8] ] +
				 A[k+9]*b[ index[k+9] ];
	}
}

void ibkmk_spmat_eid_multiply7(unsigned int n, IBKMK_CONST unsigned int * IBKMK_RESTRICT index, IBKMK_CONST double * IBKMK_RESTRICT A, IBKMK_CONST double * IBKMK_RESTRICT b, double * IBKMK_RESTRICT r) {
	int i, k;
#pragma omp parallel for schedule(static) private(k)
	for (i=0; i<(int)n; ++i) {
		k = i*7;
		/* process all elements in this row */
		r[i] =	A[k  ]*b[ index[k  ] ] +
				A[k+1]*b[ index[k+1] ] +
				A[k+2]*b[ index[k+2] ] +
				A[k+3]*b[ index[k+3] ] +
				A[k+4]*b[ index[k+4] ] +
				A[k+5]*b[ index[k+5] ] +
				A[k+6]*b[ index[k+6] ];
	}
}


void ibkmk_spmat_eid_multiply14(unsigned int n, IBKMK_CONST unsigned int * index, IBKMK_CONST double * A, IBKMK_CONST double * b, double * r) {
	int i, k;
#pragma omp parallel for schedule(static) private(k)
	for (i=0; i<(int)n; ++i) {
		k=i*14;
		/* process all elements in this row */
		r[i] =	A[k  ]*b[ index[k  ] ] +
				A[k+1]*b[ index[k+1] ] +
				A[k+2]*b[ index[k+2] ] +
				A[k+3]*b[ index[k+3] ] +
				A[k+4]*b[ index[k+4] ] +
				A[k+5]*b[ index[k+5] ] +
				A[k+6]*b[ index[k+6] ] +
				A[k+7]*b[ index[k+7] ] +
				A[k+8]*b[ index[k+8] ] +
				A[k+9]*b[ index[k+9] ] +
				A[k+10]*b[ index[k+10] ] +
				A[k+11]*b[ index[k+11] ] +
				A[k+12]*b[ index[k+12] ] +
				A[k+13]*b[ index[k+13] ];
	}
}

void ibkmk_spmat_eid_multiply15(unsigned int n, IBKMK_CONST unsigned int * index, IBKMK_CONST double * A, IBKMK_CONST double * b, double * r) {
	int i, k;
#pragma omp parallel for schedule(static) private(k)
	for (i=0; i<(int)n; ++i) {
		k=i*15;
		/* process all elements in this row */
		r[i] =	A[k  ]*b[ index[k  ] ] +
				A[k+1]*b[ index[k+1] ] +
				A[k+2]*b[ index[k+2] ] +
				A[k+3]*b[ index[k+3] ] +
				A[k+4]*b[ index[k+4] ] +
				A[k+5]*b[ index[k+5] ] +
				A[k+6]*b[ index[k+6] ] +
				A[k+7]*b[ index[k+7] ] +
				A[k+8]*b[ index[k+8] ] +
				A[k+9]*b[ index[k+9] ] +
				A[k+10]*b[ index[k+10] ] +
				A[k+11]*b[ index[k+11] ] +
				A[k+12]*b[ index[k+12] ] +
				A[k+13]*b[ index[k+13] ] +
				A[k+14]*b[ index[k+14] ];
	}
}


void ibkmk_blockspmat_eid_multiply2(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r)
{
	IBKMK_CONST double *		Ablock = A;
	IBKMK_CONST unsigned int *	indexRow = index;
	IBKMK_CONST double *		bBlock;

	int i,j;

/// \todo initialization of private vars #pragma omp parallel for schedule(static) private(Ablock,indexRow,j,bBlock,r)
	for (i=0; i<(int)n; ++i)
	{
		/* zero out result vector */
		r[0] = 0;
		r[1] = 0;
		/* process all indices in this row */
		for (j=0; j<(int)m; ++j) {
			bBlock = b + indexRow[j]*2;
			r[0] += Ablock[0] * bBlock[0] + Ablock[2] * bBlock[1];
			r[1] += Ablock[1] * bBlock[0] + Ablock[3] * bBlock[1];
			Ablock += 4;
		}
		r += 2;
		indexRow += m;
	}
}


void ibkmk_blockspmat_eid_multiply3(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r)
{
	IBKMK_CONST double * Ablock = A;
	IBKMK_CONST unsigned int * indexRow = index;
	int i,j;

/// \todo initialization of private vars #pragma omp parallel for schedule(static) private(Ablock,indexRow,j)
	for (i=0; i<(int)n; ++i)
	{
		/* zero out result vector */
		r[0] = 0;
		r[1] = 0;
		r[2] = 0;
		/* process all indices in this row */
		for (j=0; j<(int)m; ++j) {
			r[0] += Ablock[0] * b[indexRow[j]*3] + Ablock[3] * b[indexRow[j]*3+1] + Ablock[6] * b[indexRow[j]*3+2];
			r[1] += Ablock[1] * b[indexRow[j]*3] + Ablock[4] * b[indexRow[j]*3+1] + Ablock[7] * b[indexRow[j]*3+2];
			r[2] += Ablock[2] * b[indexRow[j]*3] + Ablock[5] * b[indexRow[j]*3+1] + Ablock[8] * b[indexRow[j]*3+2];
			Ablock += 9;
		}
		r += 3;
		indexRow += m;
	}
}



void ibkmk_blockspmat_eid_multiply10(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r)
{
	IBKMK_CONST double *		Ablock = A;
	IBKMK_CONST unsigned int *	indexRow = index;
	int i;

/// \todo initialization of private vars #pragma omp parallel for schedule(static)  private(Ablock,indexRow)
	for (i=0; i<(int)n; ++i)
	{
		/* zero out result vector */
		/* process all indices in this row */
		r[0] =	Ablock[ 0] * b[indexRow[0]*2] + Ablock[ 2] * b[indexRow[0]*2+1] +
				Ablock[ 4] * b[indexRow[1]*2] + Ablock[ 6] * b[indexRow[1]*2+1] +
				Ablock[ 8] * b[indexRow[2]*2] + Ablock[10] * b[indexRow[2]*2+1] +
				Ablock[12] * b[indexRow[3]*2] + Ablock[14] * b[indexRow[3]*2+1] +
				Ablock[16] * b[indexRow[4]*2] + Ablock[18] * b[indexRow[4]*2+1];

		r[1] =	Ablock[ 1] * b[indexRow[0]*2] + Ablock[ 3] * b[indexRow[0]*2+1] +
				Ablock[ 5] * b[indexRow[1]*2] + Ablock[ 7] * b[indexRow[1]*2+1] +
				Ablock[ 9] * b[indexRow[2]*2] + Ablock[11] * b[indexRow[2]*2+1] +
				Ablock[13] * b[indexRow[3]*2] + Ablock[15] * b[indexRow[3]*2+1] +
				Ablock[17] * b[indexRow[4]*2] + Ablock[19] * b[indexRow[4]*2+1];

		r += 2;
		Ablock += 20;
		indexRow += 5;
	}
}


void ibkmk_blockspmat_eid_multiply14(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r)
{
	IBKMK_CONST double *		Ablock = A;
	IBKMK_CONST unsigned int *	indexRow = index;
	double * rIndex = r;
	int i;

/// \todo initialization of private vars #pragma omp parallel for schedule(static)  private(Ablock,indexRow,j,bBlock)
	for (i=0; i<(int)n; ++i)
	{
		/* process all indices in this row */
		rIndex[0] =	Ablock[ 0] * b[indexRow[0]*2] + Ablock[ 2] * b[indexRow[0]*2+1] +
					Ablock[ 4] * b[indexRow[1]*2] + Ablock[ 6] * b[indexRow[1]*2+1] +
					Ablock[ 8] * b[indexRow[2]*2] + Ablock[10] * b[indexRow[2]*2+1] +
					Ablock[12] * b[indexRow[3]*2] + Ablock[14] * b[indexRow[3]*2+1] +
					Ablock[16] * b[indexRow[4]*2] + Ablock[18] * b[indexRow[4]*2+1] +
					Ablock[20] * b[indexRow[5]*2] + Ablock[22] * b[indexRow[5]*2+1] +
					Ablock[24] * b[indexRow[6]*2] + Ablock[26] * b[indexRow[6]*2+1];

		rIndex[1] =	Ablock[ 1] * b[indexRow[0]*2] + Ablock[ 3] * b[indexRow[0]*2+1] +
					Ablock[ 5] * b[indexRow[1]*2] + Ablock[ 7] * b[indexRow[1]*2+1] +
					Ablock[ 9] * b[indexRow[2]*2] + Ablock[11] * b[indexRow[2]*2+1] +
					Ablock[13] * b[indexRow[3]*2] + Ablock[15] * b[indexRow[3]*2+1] +
					Ablock[17] * b[indexRow[4]*2] + Ablock[19] * b[indexRow[4]*2+1] +
					Ablock[21] * b[indexRow[5]*2] + Ablock[23] * b[indexRow[5]*2+1] +
					Ablock[25] * b[indexRow[6]*2] + Ablock[27] * b[indexRow[6]*2+1];

		rIndex += 2;
		Ablock += 28;
		indexRow += 7;
	}
}


void ibkmk_blockspmat_eid_multiply15(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r)
{
	IBKMK_CONST double * Ablock = A;
	IBKMK_CONST unsigned int * indexRow = index;
	int i;

/// \todo initialization of private vars #pragma omp parallel for schedule(static) private(k)
	for (i=0; i<(int)n; ++i)
	{
		/* process all indices in this row */
		r[0] =	Ablock[ 0] * b[indexRow[0]*3] + Ablock[ 3] * b[indexRow[0]*3+1] + Ablock[ 6] * b[indexRow[0]*3+2] +
				Ablock[ 9] * b[indexRow[1]*3] + Ablock[12] * b[indexRow[1]*3+1] + Ablock[15] * b[indexRow[1]*3+2] +
				Ablock[18] * b[indexRow[2]*3] + Ablock[21] * b[indexRow[2]*3+1] + Ablock[24] * b[indexRow[2]*3+2] +
				Ablock[27] * b[indexRow[3]*3] + Ablock[30] * b[indexRow[3]*3+1] + Ablock[33] * b[indexRow[3]*3+2] +
				Ablock[36] * b[indexRow[4]*3] + Ablock[39] * b[indexRow[4]*3+1] + Ablock[42] * b[indexRow[4]*3+2];

		r[1] =	Ablock[ 1] * b[indexRow[0]*3] + Ablock[ 4] * b[indexRow[0]*3+1] + Ablock[ 7] * b[indexRow[0]*3+2] +
				Ablock[10] * b[indexRow[1]*3] + Ablock[13] * b[indexRow[1]*3+1] + Ablock[16] * b[indexRow[1]*3+2] +
				Ablock[19] * b[indexRow[2]*3] + Ablock[22] * b[indexRow[2]*3+1] + Ablock[25] * b[indexRow[2]*3+2] +
				Ablock[28] * b[indexRow[3]*3] + Ablock[31] * b[indexRow[3]*3+1] + Ablock[34] * b[indexRow[3]*3+2] +
				Ablock[37] * b[indexRow[4]*3] + Ablock[40] * b[indexRow[4]*3+1] + Ablock[43] * b[indexRow[4]*3+2];

		r[2] =	Ablock[ 2] * b[indexRow[0]*3] + Ablock[ 5] * b[indexRow[0]*3+1] + Ablock[ 8] * b[indexRow[0]*3+2] +
				Ablock[11] * b[indexRow[1]*3] + Ablock[14] * b[indexRow[1]*3+1] + Ablock[17] * b[indexRow[1]*3+2] +
				Ablock[20] * b[indexRow[2]*3] + Ablock[23] * b[indexRow[2]*3+1] + Ablock[26] * b[indexRow[2]*3+2] +
				Ablock[29] * b[indexRow[3]*3] + Ablock[32] * b[indexRow[3]*3+1] + Ablock[35] * b[indexRow[3]*3+2] +
				Ablock[38] * b[indexRow[4]*3] + Ablock[41] * b[indexRow[4]*3+1] + Ablock[44] * b[indexRow[4]*3+2];
		r += 3;
		Ablock += 45;
		indexRow += 5;
	}
}


void ibkmk_blockspmat_eid_multiply21(	unsigned int n,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										IBKMK_CONST double * b,
										double * r)
{
	IBKMK_CONST double * Ablock = A;
	IBKMK_CONST unsigned int * indexRow = index;
	int i;

/// \todo initialization of private vars #pragma omp parallel for schedule(static) private(k)
	for (i=0; i<(int)n; ++i)
	{
		/* process all indices in this row */
		r[0] =	Ablock[ 0] * b[indexRow[0]*3] + Ablock[ 3] * b[indexRow[0]*3+1] + Ablock[ 6] * b[indexRow[0]*3+2] +
				Ablock[ 9] * b[indexRow[1]*3] + Ablock[12] * b[indexRow[1]*3+1] + Ablock[15] * b[indexRow[1]*3+2] +
				Ablock[18] * b[indexRow[2]*3] + Ablock[21] * b[indexRow[2]*3+1] + Ablock[24] * b[indexRow[2]*3+2] +
				Ablock[27] * b[indexRow[3]*3] + Ablock[30] * b[indexRow[3]*3+1] + Ablock[33] * b[indexRow[3]*3+2] +
				Ablock[36] * b[indexRow[4]*3] + Ablock[39] * b[indexRow[4]*3+1] + Ablock[42] * b[indexRow[4]*3+2] +
				Ablock[45] * b[indexRow[5]*3] + Ablock[48] * b[indexRow[5]*3+1] + Ablock[51] * b[indexRow[5]*3+2] +
				Ablock[54] * b[indexRow[6]*3] + Ablock[57] * b[indexRow[6]*3+1] + Ablock[60] * b[indexRow[6]*3+2];

		r[1] =	Ablock[ 1] * b[indexRow[0]*3] + Ablock[ 4] * b[indexRow[0]*3+1] + Ablock[ 7] * b[indexRow[0]*3+2] +
				Ablock[10] * b[indexRow[1]*3] + Ablock[13] * b[indexRow[1]*3+1] + Ablock[16] * b[indexRow[1]*3+2] +
				Ablock[19] * b[indexRow[2]*3] + Ablock[22] * b[indexRow[2]*3+1] + Ablock[25] * b[indexRow[2]*3+2] +
				Ablock[28] * b[indexRow[3]*3] + Ablock[31] * b[indexRow[3]*3+1] + Ablock[34] * b[indexRow[3]*3+2] +
				Ablock[37] * b[indexRow[4]*3] + Ablock[40] * b[indexRow[4]*3+1] + Ablock[43] * b[indexRow[4]*3+2] +
				Ablock[46] * b[indexRow[5]*3] + Ablock[49] * b[indexRow[5]*3+1] + Ablock[52] * b[indexRow[5]*3+2] +
				Ablock[55] * b[indexRow[6]*3] + Ablock[58] * b[indexRow[6]*3+1] + Ablock[61] * b[indexRow[6]*3+2];

		r[2] =	Ablock[ 2] * b[indexRow[0]*3] + Ablock[ 5] * b[indexRow[0]*3+1] + Ablock[ 8] * b[indexRow[0]*3+2] +
				Ablock[11] * b[indexRow[1]*3] + Ablock[14] * b[indexRow[1]*3+1] + Ablock[17] * b[indexRow[1]*3+2] +
				Ablock[20] * b[indexRow[2]*3] + Ablock[23] * b[indexRow[2]*3+1] + Ablock[26] * b[indexRow[2]*3+2] +
				Ablock[29] * b[indexRow[3]*3] + Ablock[32] * b[indexRow[3]*3+1] + Ablock[35] * b[indexRow[3]*3+2] +
				Ablock[38] * b[indexRow[4]*3] + Ablock[41] * b[indexRow[4]*3+1] + Ablock[44] * b[indexRow[4]*3+2] +
				Ablock[47] * b[indexRow[5]*3] + Ablock[50] * b[indexRow[5]*3+1] + Ablock[53] * b[indexRow[5]*3+2] +
				Ablock[56] * b[indexRow[6]*3] + Ablock[59] * b[indexRow[6]*3+1] + Ablock[62] * b[indexRow[6]*3+2];

		r += 3;
		Ablock += 63;
		indexRow += 7;
	}
}


void ibkmk_blockspmat_eid_ilu(	unsigned int n,
									unsigned int nSubMatrix,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									double * A)
{
	switch (nSubMatrix) {
		case 1 :
			ibkmk_spmat_eid_ilu( n, m, index, A );
			break;

		case 2 :
			ibkmk_blockspmat_eid_ilu2( n, m, index, A );
			break;

		case 3 :
			ibkmk_blockspmat_eid_ilu3( n, m, index, A );
			break;

		default : {
			/// \todo implement
			}
		break;
	}
}




void ibkmk_blockspmat_eid_ilu3(	unsigned int n,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									double * A)
{

	unsigned int i,j,k,s,jidx, jidxfirst, jidxend, iidx, iidxend, n1, sidx, sidxend;


	double A_kk_inv[9] = {0,0,0,0,0,0,0,0,0};/* Initialization only necessary to avoid compiler warning */
	double A_ik[9] = {0,0,0,0,0,0,0,0,0};
	double A_kj[9] = {0,0,0,0,0,0,0,0,0};

	/* Main elimination loop, process all blocks (rows) */
	for (k=0, n1 = n-1; k<n1; ++k) {

		/* Upper triangular matrix part, L_kk is already known, we can compute all U_kj */
		for (jidxfirst=jidx=k*m, jidxend = jidx + m; jidx<jidxend; ++jidx) {

			/* retrieve column index j */
			j = index[jidx];

			/* skip columns below k */
			if (j<k)
				continue;

			/* check for invalid/unused element, "jidxfirst != jidx" ensures that the check is only executed on second index and later */
			if (jidxfirst != jidx && j == index[jidx - 1])
				break;

			/* if we have j == k we found the index for the main diagonal element and store it */
			if (j == k) {
				// perform LU-factorization without use of ibkmk_dense routines
				// ibkmk_dense_LU3(&A_kk_inv[0])
				// (missing exception handling)
				A_kk_inv[0] = A[jidx*9];
				A_kk_inv[1] = A[jidx*9+1];
				A_kk_inv[2] = A[jidx*9+2];
				A_kk_inv[3] = A[jidx*9+3];
				A_kk_inv[4] = A[jidx*9+4];
				A_kk_inv[5] = A[jidx*9+5];
				A_kk_inv[6] = A[jidx*9+6];
				A_kk_inv[7] = A[jidx*9+7];
				A_kk_inv[8] = A[jidx*9+8];

				// 1st row
				A_kk_inv[3] /= A_kk_inv[0];
				A_kk_inv[6] /= A_kk_inv[0];

				// 2nd row
				A_kk_inv[4] -= A_kk_inv[1]*A_kk_inv[3];
				A_kk_inv[7] -= A_kk_inv[1]*A_kk_inv[6];
				A_kk_inv[7] /= A_kk_inv[4];

				// 3rd row
				A_kk_inv[5] -= A_kk_inv[2]*A_kk_inv[3];
				A_kk_inv[8] -= A_kk_inv[2]*A_kk_inv[6] + A_kk_inv[5]*A_kk_inv[7];

				A[jidx *9]     = A_kk_inv[0];
				A[jidx *9 + 1] = A_kk_inv[1];
				A[jidx *9 + 2] = A_kk_inv[2];
				A[jidx *9 + 3] = A_kk_inv[3];
				A[jidx *9 + 4] = A_kk_inv[4];
				A[jidx *9 + 5] = A_kk_inv[5];
				A[jidx *9 + 6] = A_kk_inv[6];
				A[jidx *9 + 7] = A_kk_inv[7];
				A[jidx *9 + 8] = A_kk_inv[8];
				continue;
			}
			/* we have found a valid element in column j, do A_kj := A_kj/A_kk */
			//ibkmk_dense_backsolve3(&A_kk_inv[0], &A[jidx*9]);
			// L - loop
			A[jidx*9]   /= A_kk_inv[0];
			A[jidx*9+1] -= A_kk_inv[1] * A[jidx*9];
			A[jidx*9+1] /= A_kk_inv[4];
			A[jidx*9+2] -= A_kk_inv[2] * A[jidx*9] + A_kk_inv[5] * A[jidx*9+1];
			A[jidx*9+2] /= A_kk_inv[8];
			// U - loop
			A[jidx*9+1] -= A_kk_inv[7] * A[jidx*9+2];
			A[jidx*9]   -= A_kk_inv[6] * A[jidx*9+2] + A_kk_inv[3] * A[jidx*9+1];

			// L - loop
			A[jidx*9+3] /= A_kk_inv[0];
			A[jidx*9+4] -= A_kk_inv[1] * A[jidx*9+3];
			A[jidx*9+4] /= A_kk_inv[4];
			A[jidx*9+5] -= A_kk_inv[2] * A[jidx*9+3] + A_kk_inv[5] * A[jidx*9+4];
			A[jidx*9+5] /= A_kk_inv[8];
			// U - loop
			A[jidx*9+4] -= A_kk_inv[7] * A[jidx*9+5];
			A[jidx*9+3] -= A_kk_inv[6] * A[jidx*9+5] + A_kk_inv[3] * A[jidx*9+4];

			// L - loop
			A[jidx*9+6] /= A_kk_inv[0];
			A[jidx*9+7] -= A_kk_inv[1] * A[jidx*9+6];
			A[jidx*9+7] /= A_kk_inv[4];
			A[jidx*9+8] -= A_kk_inv[2] * A[jidx*9+6] + A_kk_inv[5] * A[jidx*9+7];
			A[jidx*9+8] /= A_kk_inv[8];
			// U - loop
			A[jidx*9+7] -= A_kk_inv[7] * A[jidx*9+8];
			A[jidx*9+6] -= A_kk_inv[6] * A[jidx*9+8] + A_kk_inv[3] * A[jidx*9+7];

			A_ik[0] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[1] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[2] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[3] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[4] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[5] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[6] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[7] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[8] = 0; /* Initialization only necessary to avoid compiler warning */

			A_kj[0] = A[jidx*9];
			A_kj[1] = A[jidx*9+1];
			A_kj[2] = A[jidx*9+2];
			A_kj[3] = A[jidx*9+3];
			A_kj[4] = A[jidx*9+4];
			A_kj[5] = A[jidx*9+5];
			A_kj[6] = A[jidx*9+6];
			A_kj[7] = A[jidx*9+7];
			A_kj[8] = A[jidx*9+8];

			/* Lower triangular matrix part */
			/* Loop over all rows with elements in column k */
			for (iidx=k*m, iidxend = iidx + m; iidx < iidxend; ++iidx) {

				i = index[iidx]; /* !!!Symmetric matrix pattern required!!! */

				/* skip rows below k */
				if (i<=k)
					continue;

				/* stop when encountering invalid elements,
				   since i > k, and we have always the diagonal element, this check will only be encountered when
				   iidx has been increased at least once.
				*/
				if (i == index[iidx-1])
					break;

				/* We now have a row i, which has an element in column k */
				/* find A_ik and A_ij */
				for (sidx = i*m, sidxend = sidx+m; sidx < sidxend; ++sidx) {

					s = index[sidx];

					if (s==k){
						A_ik[0] = A[sidx*9]; /* k < j, therefore this will be executed first */
						A_ik[1] = A[sidx*9+1]; /* k < j, therefore this will be executed first */
						A_ik[2] = A[sidx*9+2]; /* k < j, therefore this will be executed first */
						A_ik[3] = A[sidx*9+3]; /* k < j, therefore this will be executed first */
						A_ik[4] = A[sidx*9+4]; /* k < j, therefore this will be executed first */
						A_ik[5] = A[sidx*9+5]; /* k < j, therefore this will be executed first */
						A_ik[6] = A[sidx*9+6]; /* k < j, therefore this will be executed first */
						A_ik[7] = A[sidx*9+7]; /* k < j, therefore this will be executed first */
						A_ik[8] = A[sidx*9+8]; /* k < j, therefore this will be executed first */
					} else if (s==j) {
						//ibkmk_dense_mat_mult_sub3(&A_ik[0], &A_kj[0], &A[sidx*9]);
						A[sidx*9]   -= A_ik[0]*A_kj[0] + A_ik[3]*A_kj[1] + A_ik[6]*A_kj[2];
						A[sidx*9+1] -= A_ik[1]*A_kj[0] + A_ik[4]*A_kj[1] + A_ik[7]*A_kj[2];
						A[sidx*9+2] -= A_ik[2]*A_kj[0] + A_ik[5]*A_kj[1] + A_ik[8]*A_kj[2];
						A[sidx*9+3] -= A_ik[0]*A_kj[3] + A_ik[3]*A_kj[4] + A_ik[6]*A_kj[5];
						A[sidx*9+4] -= A_ik[1]*A_kj[3] + A_ik[4]*A_kj[4] + A_ik[7]*A_kj[5];
						A[sidx*9+5] -= A_ik[2]*A_kj[3] + A_ik[5]*A_kj[4] + A_ik[8]*A_kj[5];
						A[sidx*9+6] -= A_ik[0]*A_kj[6] + A_ik[3]*A_kj[7] + A_ik[6]*A_kj[8];
						A[sidx*9+7] -= A_ik[1]*A_kj[6] + A_ik[4]*A_kj[7] + A_ik[7]*A_kj[8];
						A[sidx*9+8] -= A_ik[2]*A_kj[6] + A_ik[5]*A_kj[7] + A_ik[8]*A_kj[8];
						break;
					}
				} /* sidx */

			} /* iidx */

		} /* jidx */

	} /* k */
	// factorize last diagonal
	for (jidxfirst=jidx=(n-1)*m, jidxend = jidx + m; jidx<jidxend; ++jidx) {

		/* retrieve column index j */
		j = index[jidx];

		/* skip columns below k */
		if (j<k)
			continue;
		/* if we have j == k we found the index for the main diagonal element and store it */
		if (j == k) {
			// perform LU-factorization without use of ibkmk_dense routines
			// ibkmk_dense_LU3(&A[jidx *9])
			// (missing exception handling)
			// 1st row
			A[jidx*9+3] /= A[jidx*9];
			A[jidx*9+6] /= A[jidx*9];

			// 2nd row
			A[jidx*9+4] -= A[jidx*9+1]*A[jidx*9+3];
			A[jidx*9+7] -= A[jidx*9+1]*A[jidx*9+6];
			A[jidx*9+7] /= A[jidx*9+4];

			// 3rd row
			A[jidx*9+5] -= A[jidx*9+2]*A[jidx*9+3];
			A[jidx*9+8] -= A[jidx*9+2]*A[jidx*9+6] + A[jidx*9+5]*A[jidx*9+7];
			break;
		}
	}


}

void ibkmk_blockspmat_eid_ilu2(	unsigned int n,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									double * A)
{

	unsigned int i,j,k,s,jidx, jidxfirst, jidxend, iidx, iidxend, n1, sidx, sidxend;

	double A_kk_inv[4] = {0,0,0,0};/* Initialization only necessary to avoid compiler warning */
	double A_ik[4] = {0,0,0,0};
	double A_kj[4] = {0,0,0,0};

	/* Main elimination loop, process all blocks (rows) */
	for (k=0, n1 = n-1; k<n1; ++k) {

		/* Upper triangular matrix part, L_kk is already known, we can compute all U_kj */
		for (jidxfirst=jidx=k*m, jidxend = jidx + m; jidx<jidxend; ++jidx) {

			/* retrieve column index j */
			j = index[jidx];

			/* skip columns below k */
			if (j<k)
				continue;

			/* check for invalid/unused element, "jidxfirst != jidx" ensures that the check is only executed on second index and later */
			if (jidxfirst != jidx && j == index[jidx - 1])
				break;

			/* if we have j == k we found the index for the main diagonal element and store it */
			if (j == k) {
				/* store inverse of diagonal matrix element */
				A_kk_inv[0] = A[jidx *4];
				A_kk_inv[1] = A[jidx *4 + 1];
				A_kk_inv[2] = A[jidx *4 + 2];
				A_kk_inv[3] = A[jidx *4 + 3];

				// perform LU-factorization without use of ibkmk_dense routines
				// ibkmk_dense_LU2(&A_kk_inv[0])
				// (missing exception handling)
				A_kk_inv[2] /= A_kk_inv[0];
				A_kk_inv[3] -= A_kk_inv[1]*A_kk_inv[2];


				A[jidx *4]     = A_kk_inv[0];
				A[jidx *4 + 1] = A_kk_inv[1];
				A[jidx *4 + 2] = A_kk_inv[2];
				A[jidx *4 + 3] = A_kk_inv[3];
				continue;
			}
			/* we have found a valid element in column j, do A_kj := A_kj/A_kk */
			//ibkmk_dense_backsolve2(&A_kk_inv[0], &A[jidx*4]);
			A[jidx*4]   /= A_kk_inv[0];
			A[jidx*4+1] -= A_kk_inv[1]*A[jidx*4];
			A[jidx*4+1] /= A_kk_inv[3];
			A[jidx*4]   -= A_kk_inv[2]*A[jidx*4+1];

			//ibkmk_dense_backsolve2(&A_kk_inv[0], &A[jidx*4 + 2]);
			A[jidx*4+2] /= A_kk_inv[0];
			A[jidx*4+3] -= A_kk_inv[1]*A[jidx*4+2];
			A[jidx*4+3] /= A_kk_inv[3];
			A[jidx*4+2] -= A_kk_inv[2]*A[jidx*4+3];

			A_ik[0] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[1] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[2] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[3] = 0; /* Initialization only necessary to avoid compiler warning */

			A_kj[0] = A[jidx*4];
			A_kj[1] = A[jidx*4+1];
			A_kj[2] = A[jidx*4+2];
			A_kj[3] = A[jidx*4+3];

			/* Lower triangular matrix part */
			/* Loop over all rows with elements in column k */
			for (iidx=k*m, iidxend = iidx + m; iidx < iidxend; ++iidx) {

				i = index[iidx]; /* !!!Symmetric matrix pattern required!!! */

				/* skip rows below k */
				if (i<=k)
					continue;

				/* stop when encountering invalid elements,
				   since i > k, and we have always the diagonal element, this check will only be encountered when
				   iidx has been increased at least once.
				*/
				if (i == index[iidx-1])
					break;

				/* We now have a row i, which has an element in column k */
				/* find A_ik and A_ij */
				for (sidx = i*m, sidxend = sidx+m; sidx < sidxend; ++sidx) {

					s = index[sidx];

					if (s==k){
						A_ik[0] = A[sidx*4]; /* k < j, therefore this will be executed first */
						A_ik[1] = A[sidx*4+1]; /* k < j, therefore this will be executed first */
						A_ik[2] = A[sidx*4+2]; /* k < j, therefore this will be executed first */
						A_ik[3] = A[sidx*4+3]; /* k < j, therefore this will be executed first */
					} else if (s==j) {
						//ibkmk_dense_mat_mult_sub2(&A_ik[0], &A[jidx*4], &A[sidx*4]);
						A[sidx*4]   -= ( A_ik[0] * A_kj[0] + A_ik[2] * A_kj[1] );
						A[sidx*4+1] -= ( A_ik[1] * A_kj[0] + A_ik[3] * A_kj[1] );
						A[sidx*4+2] -= ( A_ik[0] * A_kj[2] + A_ik[2] * A_kj[3] );
						A[sidx*4+3] -= ( A_ik[1] * A_kj[2] + A_ik[3] * A_kj[3] );
						break;
					}
				} /* sidx */

			} /* iidx */

		} /* jidx */

	} /* k */
	// factorize last diagonal
	for (jidxfirst=jidx=(n-1)*m, jidxend = jidx + m; jidx<jidxend; ++jidx) {

		/* retrieve column index j */
		j = index[jidx];

		/* skip columns below k */
		if (j<k)
			continue;
		/* if we have j == k we found the index for the main diagonal element and store it */
		if (j == k) {
			// perform LU-factorization without use of ibkmk_dense routines
			// ibkmk_dense_LU2(&A[jidx *4])
			// (missing exception handling)
			A[jidx *4 + 2] /= A[jidx *4];
			A[jidx *4 + 3] -= A[jidx *4 + 1]*A[jidx *4 + 2];
			break;
		}
	}
}

void ibkmk_blockspmat_eid_ilu2Misch(	unsigned int n,
									unsigned int m,
									IBKMK_CONST unsigned int * index,
									double * A)
{

	unsigned int i,j,k,s,jidx, jidxfirst, jidxend, iidx, iidxend, n1, sidx, sidxend;

	double A_kk_inv[4] = {0,0,0,0};/* Initialization only necessary to avoid compiler warning */
	double A_ik[4] = {0,0,0,0};
	double A_kj_1[2] = {0,0};

	/* Main elimination loop, process all blocks (rows) */
	for (k=0, n1 = n-1; k<n1; ++k) {

		/* Upper triangular matrix part, L_kk is already known, we can compute all U_kj */
		for (jidxfirst=jidx=k*m, jidxend = jidx + m; jidx<jidxend; ++jidx) {

			/* retrieve column index j */
			j = index[jidx];

			/* skip columns below k */
			if (j<k)
				continue;

			/* check for invalid/unused element, "jidxfirst != jidx" ensures that the check is only executed on second index and later */
			if (jidxfirst != jidx && j == index[jidx - 1])
				break;

			/* if we have j == k we found the index for the main diagonal element and store it */
			if (j == k) {
				/* store inverse of diagonal matrix element */
				A_kk_inv[0] = 1.0/A[jidx*4];
				A_kk_inv[1] = 1.0/A[jidx*4+1];
				A_kk_inv[2] = 1.0/A[jidx*4+2];
				A_kk_inv[3] = 1.0/A[jidx*4+3];
				continue;
			}
			/* we have found a valid element in column j, do A_kj := A_kj/A_kk */
			A[jidx*4]   *= A_kk_inv[0];
			A[jidx*4+1] *= A_kk_inv[1];
			A[jidx*4+2] *= A_kk_inv[2];
			A[jidx*4+3] *= A_kk_inv[3];

			A_ik[0] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[1] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[2] = 0; /* Initialization only necessary to avoid compiler warning */
			A_ik[3] = 0; /* Initialization only necessary to avoid compiler warning */

			/* Lower triangular matrix part */
			/* Loop over all rows with elements in column k */
			for (iidx=k*m, iidxend = iidx + m; iidx < iidxend; ++iidx) {

				i = index[iidx]; /* !!!Symmetric matrix pattern required!!! */

				/* skip rows below k */
				if (i<=k)
					continue;

				/* stop when encountering invalid elements,
				   since i > k, and we have always the diagonal element, this check will only be encountered when
				   iidx has been increased at least once.
				*/
				if (i == index[iidx-1])
					break;

				/* We now have a row i, which has an element in column k */
				/* find A_ik and A_ij */
				for (sidx = i*m, sidxend = sidx+m; sidx < sidxend; ++sidx) {

					s = index[sidx];

					if (s==k){
						A_ik[0] = A[sidx*4]; /* k < j, therefore this will be executed first */
						A_ik[1] = A[sidx*4+1]; /* k < j, therefore this will be executed first */
						A_ik[2] = A[sidx*4+2]; /* k < j, therefore this will be executed first */
						A_ik[3] = A[sidx*4+3]; /* k < j, therefore this will be executed first */
					} else if (s==j) {
						// if so the exeution order is not selectable at this point...
						A[sidx*4+2] -= A_ik[2]*A[jidx*4+2] - A_kj_1[0]; // ->  + we need access to previous block in j direction!?
						A[sidx*4+3] -= A_ik[3]*A[jidx*4+3] - A_kj_1[1]; // ->  + we need access to previous block in j direction!?
						A[sidx*4]   -= A_ik[0]*A[jidx*4]   - A_ik[2]*A[jidx*4+2];
						A[sidx*4+1] -= A_ik[1]*A[jidx*4+1] - A_ik[3]*A[jidx*4+3];
						// save old block in j direction
						A_kj_1[0]    = A[sidx*4];
						A_kj_1[1]    = A[sidx*4+1];
						break;
					}
				} /* sidx */

			} /* iidx */

		} /* jidx */

	} /* k */

}



void ibkmk_blockspmat_eid_backsolve(	unsigned int n,
										unsigned int nSubMatrix,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										double * b)
{

	switch (nSubMatrix) {
		case 1 :
			ibkmk_spmat_eid_backsolve( n, m, index, A, b );
			break;
		case 2 :
			ibkmk_blockspmat_eid_backsolve2( n, m, index, A, b );
			break;
		case 3 :
			ibkmk_blockspmat_eid_backsolve3( n, m, index, A, b );
			break;

		default : {
			/// \todo implement
			}
		break;
	}
//	unsigned int i, kIdxEnd, k=0, kIdx;
//	/** firstly, L loop (forward elimination) **/

//	/* First row, main diagonal element is always at first position */
//	b[0] /= A[0]; /* bn_0 = b_0/l_00 */
//	/* Remaining n-1 rows */
//	for (i=1; i<n; ++i) {
//		/* first subtract all known factorials, sum_k l_ik*b_k */
//		/* loop over all indices in row i, up to but not including the main diagonal element */
//		/* kIdx    -> first column index in row i */
//		/* kIdxEnd -> last column index +1 in row i */
//		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
//			k = index[kIdx];
//			/* stop once diagonal element (k == i) found */
//			if (k == i)
//				break;
//			/* subtract product from b[i] */
//			b[i] -= A[kIdx]*b[k];
//		}
//		/* must have stopped multiplication loop at diagonal element */
//		assert(i == k);
//		assert(kIdx < kIdxEnd);
//		/* finally divide by diagonal element */
//		b[i] /= A[kIdx];
//	}
//	/* now b holds L^-1*b */

//	/* secondly, U loop (backward elimination), with special case u_i,i = 1 */
//	for (i=n-2; (int)i>=0; --i) {
//		/* subtract all previously known solutions from */
//		/* process all indices in row i backwards, stop when reaching main diagonal element */
//		/* kIdx    -> first column index in row i */
//		/* kIdxEnd -> last column index +1 in row i */
//		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
//			k = index[kIdx];
//			/* skip columns <= i */
//			if (k<=i)
//				continue;
//			/* subtract product from b[i], don't bother if multiplying invalid elements, because A[kIdx] == 0 for invalid elements. */
//			b[i] -= A[kIdx]*b[k];
//		}
//	}
//	/* now b holds U^-1 * L^-1 * b	*/

}

void ibkmk_blockspmat_eid_backsolve2(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										double * b)
{
	unsigned int i, kIdxEnd, k=0, kIdx;
	/** firstly, L loop (forward elimination) **/

	/* First row, main diagonal element is always at first position */
	//ibkmk_dense_backsolve2(&A[0], &b[0]);
	// L - loop
	b[0] /= A[0];
	b[1] -= A[1]*b[0];
	b[1] /= A[3];
	// U - loop
	b[0] -= A[2]*b[1];

	/* Remaining n-1 rows */
	for (i=1; i<n; ++i) {
		/* first subtract all known factorials, sum_k l_ik*b_k */
		/* loop over all indices in row i, up to but not including the main diagonal element */
		/* kIdx    -> first column index in row i */
		/* kIdxEnd -> last column index +1 in row i */
		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
			k = index[kIdx];
			/* stop once diagonal element (k == i) found */
			if (k == i)
				break;
			/* subtract product from b[i] */
			//ibkmk_dense_vec_mult_sub2(&A[kIdx * 4], &b[k * 2], &b[i * 2]);
			b[i * 2]     -= ( A[kIdx * 4] *     b[k * 2] + A[kIdx * 4 + 2] * b[k * 2 + 1] );
			b[i * 2 + 1] -= ( A[kIdx * 4 + 1] * b[k * 2] + A[kIdx * 4 + 3] * b[k * 2 + 1] );
		}
		/* must have stopped multiplication loop at diagonal element */
		assert(i == k);
		assert(kIdx < kIdxEnd);
		/* finally divide by diagonal element */
		/* calculate inverse diagonal matrix*/

		//ibkmk_dense_backsolve2(&A[kIdx * 4], &b[i * 2]);
		// L - loop
		b[i * 2]     /= A[kIdx * 4];
		b[i * 2 + 1] -= A[kIdx * 4 + 1]*b[i * 2];
		b[i * 2 + 1] /= A[kIdx * 4 + 3];
		// U - loop
		b[i * 2]     -= A[kIdx * 4 + 2]*b[i * 2 + 1];
	}
	/* now b holds L^-1*b */

	/* secondly, U loop (backward elimination), with special case u_i,i = 1 */
	for (i=n-2; (int)i>=0; --i) {
		/* subtract all previously known solutions from */
		/* process all indices in row i backwards, stop when reaching main diagonal element */
		/* kIdx    -> first column index in row i */
		/* kIdxEnd -> last column index +1 in row i */
		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
			k = index[kIdx];
			/* skip columns <= i */
			if (k<=i)
				continue;
			/* subtract product from b[i] */
			//ibkmk_dense_vec_mult_sub2(&A[kIdx * 4], &b[k * 2], &b[i * 2]);
			b[i * 2]     -= ( A[kIdx * 4] *     b[k * 2] + A[kIdx * 4 + 2] * b[k * 2 + 1] );
			b[i * 2 + 1] -= ( A[kIdx * 4 + 1] * b[k * 2] + A[kIdx * 4 + 3] * b[k * 2 + 1] );
		}
	}
	/* now b holds U^-1 * L^-1 * b	*/
}

void ibkmk_blockspmat_eid_backsolve3(	unsigned int n,
										unsigned int m,
										IBKMK_CONST unsigned int * index,
										IBKMK_CONST double * A,
										double * b)
{
	unsigned int i, kIdxEnd, k=0, kIdx;
	/** firstly, L loop (forward elimination) **/

	/* First row, main diagonal element is always at first position */
	//ibkmk_dense_backsolve2(&A[0], &b[0]);
	// L - loop
	b[0] /= A[0];
	b[1] -= A[1]*b[0];
	b[1] /= A[4];
	b[2] -= A[2]*b[0] + A[5]*b[1];
	b[2] /= A[8];
	// U - loop
	b[1] -= A[7]*b[2];
	b[0] -= A[6]*b[2] + A[3]*b[1];

	/* Remaining n-1 rows */
	for (i=1; i<n; ++i) {
		/* first subtract all known factorials, sum_k l_ik*b_k */
		/* loop over all indices in row i, up to but not including the main diagonal element */
		/* kIdx    -> first column index in row i */
		/* kIdxEnd -> last column index +1 in row i */
		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
			k = index[kIdx];
			/* stop once diagonal element (k == i) found */
			if (k == i)
				break;
			/* subtract product from b[i] */
			//ibkmk_dense_vec_mult_sub3(&A[kIdx * 9], &b[k * 3], &b[i * 3]);
			b[i * 3]     -= ( A[kIdx * 9] *     b[k * 3] + A[kIdx * 9 + 3] * b[k * 3 + 1]  + A[kIdx * 9 + 6] * b[k * 3 + 2] );
			b[i * 3 + 1] -= ( A[kIdx * 9 + 1] * b[k * 3] + A[kIdx * 9 + 4] * b[k * 3 + 1]  + A[kIdx * 9 + 7] * b[k * 3 + 2] );
			b[i * 3 + 2] -= ( A[kIdx * 9 + 2] * b[k * 3] + A[kIdx * 9 + 5] * b[k * 3 + 1]  + A[kIdx * 9 + 8] * b[k * 3 + 2] );
		}
		/* must have stopped multiplication loop at diagonal element */
		assert(i == k);
		assert(kIdx < kIdxEnd);
		/* finally divide by diagonal element */
		/* calculate inverse diagonal matrix*/

		//ibkmk_dense_backsolve2(&A[kIdx * 4], &b[i * 2]);
		// L - loop
		b[i * 3]     /= A[kIdx * 9];
		b[i * 3 + 1] -= A[kIdx * 9 + 1]*b[i * 3];
		b[i * 3 + 1] /= A[kIdx * 9 + 4];
		b[i * 3 + 2] -= A[kIdx * 9 + 2]*b[i * 3] + A[kIdx * 9 + 5]*b[i * 3 + 1];
		b[i * 3 + 2] /= A[kIdx * 9 + 8];
		// U - loop
		b[i * 3 + 1] -= A[kIdx * 9 + 7]*b[i * 3 + 2];
		b[i * 3]     -= A[kIdx * 9 + 6]*b[i * 3 + 2] + A[kIdx * 9 + 3]*b[i * 3 + 1];

	}
	/* now b holds L^-1*b */

	/* secondly, U loop (backward elimination), with special case u_i,i = 1 */
	for (i=n-2; (int)i>=0; --i) {
		/* subtract all previously known solutions from */
		/* process all indices in row i backwards, stop when reaching main diagonal element */
		/* kIdx    -> first column index in row i */
		/* kIdxEnd -> last column index +1 in row i */
		for (kIdx = i*m, kIdxEnd = kIdx + m; kIdx < kIdxEnd; ++kIdx) {
			k = index[kIdx];
			/* skip columns <= i */
			if (k<=i)
				continue;
			/* subtract product from b[i] */
			//ibkmk_dense_vec_mult_sub3(&A[kIdx * 9], &b[k * 3], &b[i * 3]);
			b[i * 3]     -= ( A[kIdx * 9] *     b[k * 3] + A[kIdx * 9 + 3] * b[k * 3 + 1]  + A[kIdx * 9 + 6] * b[k * 3 + 2] );
			b[i * 3 + 1] -= ( A[kIdx * 9 + 1] * b[k * 3] + A[kIdx * 9 + 4] * b[k * 3 + 1]  + A[kIdx * 9 + 7] * b[k * 3 + 2] );
			b[i * 3 + 2] -= ( A[kIdx * 9 + 2] * b[k * 3] + A[kIdx * 9 + 5] * b[k * 3 + 1]  + A[kIdx * 9 + 8] * b[k * 3 + 2] );
		}
	}
	/* now b holds U^-1 * L^-1 * b	*/
}
