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

#include "IBKMKC_tridiag_matrix.h"

#include "IBKMKC_dense_matrix.h"

/* Uncomment the following define to enable special
   implementations for low block-dimension */
#define USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS


/* Special implementation for tridiagonal matrices. */
int ibkmk_tridiag_LU(IBKMK_CONST int n, double *A) {
	double *L, *M, *U;
	int i;

	L = A;
	M = A + n;
	U = M + n;

	/* first row */
	if (M[0] == 0)						return 1;
	U[0] /= M[0];
	L[0] /= M[0];
	/* second row */
	M[1] -= L[1]*U[0];
	if (M[1] == 0)						return 2;
	U[1] -= L[1]*L[0];
	U[1] /= M[1];
	for (i=2; i<n-1; ++i) {
		M[i] -= L[i]*U[i-1];
		if (M[i] == 0)					return i+1;
		U[i] /= M[i];
	}
	/* last row */
	L[n-1] -= U[n-1]*U[n-3];
	M[n-1] -= L[n-1]*U[n-2];
	if (M[n-1] == 0)					return n;

	return 0;
}


/* Special implementation for tridiagonal matrices. */
void ibkmk_tridiag_backsolve(IBKMK_CONST int n, IBKMK_CONST double * A, double * b) {
	double *L, *M, *U;
	int i;

	L = A;
	M = A + n;
	U = M + n;

	/* forward elimination */
	b[0] /= M[0];
	for (i=1; i<n-1; ++i) {
		b[i] -= L[i]*b[i-1];
		b[i] /= M[i];
	}
	/* last row */
	b[n-1] -= L[n-1]*b[n-2];
	b[n-1] /= M[n-1];

	/* backward elimination */
	for (i=n-2; i>=1; --i) {
		b[i] -= U[i]*b[i+1];
	}
	/* first row */
	b[0] -= U[0]*b[1];
}


/* Special implementation for tridiagonal matrices. */
void ibkmk_tridiag_vec_mult(IBKMK_CONST int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * r) {
	IBKMK_CONST double * L = A;
	IBKMK_CONST double * M = A + n;
	IBKMK_CONST double * U = M + n;
	int i;

	r[0] = M[0]*b[0];
	if (n == 1) return; // special case, scalar equation
	r[0] += U[0]*b[1];
	for (i=1; i<(n-1); i++)
		r[i] = L[i]*b[i-1] + M[i]*b[i] + U[i]*b[i+1];
	r[n-1] = L[n-1]*b[n-2] + M[n-1]*b[n-1];
}


/* --- Block-Tridiagonal Function Implementations ---- */

/* Special implementation of lu() for block-tridiagonal matrices without block-level pivoting. */
int ibkmk_btridiag_LU( IBKMK_CONST int nblocks, IBKMK_CONST int blocksize, double ** L, double ** M, double ** U ) {
	int i;

	/* switch for special cases of smaller blocksizes */
	switch (blocksize) {

		/* special implementation for tridiagonal systems */
		case 1 :
			return ibkmk_tridiag_LU(nblocks, *L);
		break;

#ifdef USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS
		case 2:
			/* first row */
			if (ibkmk_dense_LU2(M[0]) != 0)			return 1;
			ibkmk_dense_inverse_mult2(M[0], U[0]);
			ibkmk_dense_inverse_mult2(M[0], L[0]);
			/* second row */
			ibkmk_dense_mat_mult_sub2(L[1], U[0], M[1]);
			if (ibkmk_dense_LU2(M[1]) != 0)			return 2;
			ibkmk_dense_mat_mult_sub2(L[1], L[0], U[1]);
			ibkmk_dense_inverse_mult2(M[1], U[1]);
			for (i=2; i<nblocks-1; ++i) {
				ibkmk_dense_mat_mult_sub2(L[i], U[i-1], M[i]);
				if (ibkmk_dense_LU2(M[i]) != 0)		return i+1;
				ibkmk_dense_inverse_mult2(M[i], U[i]);
			}
			/* last row */
			ibkmk_dense_mat_mult_sub2(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
			ibkmk_dense_mat_mult_sub2(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
			if (ibkmk_dense_LU2(M[nblocks-1]) != 0)	return nblocks;
		break;

		case 3:
			/* first row */
			if (ibkmk_dense_LU3(M[0]) != 0)			return 1;
			ibkmk_dense_inverse_mult3(M[0], U[0]);
			ibkmk_dense_inverse_mult3(M[0], L[0]);
			/* second row */
			ibkmk_dense_mat_mult_sub3(L[1], U[0], M[1]);
			if (ibkmk_dense_LU3(M[1]) != 0)			return 2;
			ibkmk_dense_mat_mult_sub3(L[1], L[0], U[1]);
			ibkmk_dense_inverse_mult3(M[1], U[1]);
			for (i=2; i<nblocks-1; ++i) {
				ibkmk_dense_mat_mult_sub3(L[i], U[i-1], M[i]);
				if (ibkmk_dense_LU3(M[i]) != 0)		return i+1;
				ibkmk_dense_inverse_mult3(M[i], U[i]);
			}
			/* last row */
			ibkmk_dense_mat_mult_sub3(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
			ibkmk_dense_mat_mult_sub3(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
			if (ibkmk_dense_LU3(M[nblocks-1]) != 0)	return nblocks;
		break;

		case 4:
			/* first row */
			if (ibkmk_dense_LU4(M[0]) != 0)			return 1;
			ibkmk_dense_inverse_mult4(M[0], U[0]);
			ibkmk_dense_inverse_mult4(M[0], L[0]);
			/* second row */
			ibkmk_dense_mat_mult_sub4(L[1], U[0], M[1]);
			if (ibkmk_dense_LU4(M[1]) != 0)			return 2;
			ibkmk_dense_mat_mult_sub4(L[1], L[0], U[1]);
			ibkmk_dense_inverse_mult4(M[1], U[1]);
			for (i=2; i<nblocks-1; ++i) {
				ibkmk_dense_mat_mult_sub4(L[i], U[i-1], M[i]);
				if (ibkmk_dense_LU4(M[i]) != 0)		return i+1;
				ibkmk_dense_inverse_mult4(M[i], U[i]);
			}
			/* last row */
			ibkmk_dense_mat_mult_sub4(U[nblocks-1], U[nblocks-3], L[nblocks-1]);
			ibkmk_dense_mat_mult_sub4(L[nblocks-1], U[nblocks-2], M[nblocks-1]);
			if (ibkmk_dense_LU4(M[nblocks-1]) != 0)	return nblocks;
		break;
#endif /* USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS */

		default :
			/* Generic Implementation for arbitrary block sizes */

			/* first row */
			/* compute M_0^{-1} U_0 and store in U_0 */
			if (ibkmk_dense_LU(blocksize, M[0]) != 0)			return 1;
			ibkmk_dense_inverse_mult(blocksize, M[0], U[0]);
			/* compute M_0^{-1} L_0 and store in L_0 */
			ibkmk_dense_inverse_mult(blocksize, M[0], L[0]);
			/* second row */
			/* compute M_1 - L_1 U_0 and store in M_1 */
			ibkmk_dense_mat_mult_sub(blocksize, L[1], U[0], M[1]);
			/* compute M_1^{-1}(U_1 - L_1 L_0) and store in U_1 */
			if (ibkmk_dense_LU(blocksize, M[1]) != 0)			return 2;
			ibkmk_dense_mat_mult_sub(blocksize, L[1], L[0], U[1]);
			ibkmk_dense_inverse_mult(blocksize, M[1], U[1]);
			for (i=2; i<nblocks-1; ++i) {
				/* compute M_i - L_i U_{i-1} and store in M_i */
				ibkmk_dense_mat_mult_sub(blocksize, L[i], U[i-1], M[i]);
				/* compute M_i^{-1} U_i and store in L_0 */
				if (ibkmk_dense_LU(blocksize, M[i]) != 0)		return i+1;
				ibkmk_dense_inverse_mult(blocksize, M[i], U[i]);
			}
			/* last row */
			/* compute L_{n-1} - U_{n-1} U_{n-3} and store in L_{n-1} */
			ibkmk_dense_mat_mult_sub(blocksize, U[nblocks-1], U[nblocks-3], L[nblocks-1]);
			/* compute M_{n-1} - L_{n-1} U_{n-2} and store in M_{n-1} */
			ibkmk_dense_mat_mult_sub(blocksize, L[nblocks-1], U[nblocks-2], M[nblocks-1]);
			/* compute M_{n-1}^{-1} U_i and store in L_0 */
			if (ibkmk_dense_LU(blocksize, M[nblocks-1]) != 0)	return nblocks;
	} /* switch */

	return 0;
}
// ---------------------------------------------------------------------------


int ibkmk_btridiag_LU_pivot( IBKMK_CONST int nblocks, IBKMK_CONST int blocksize,
							 double ** L, double ** M, double ** U, long int * pivots )
{
	int i;

	/* switch for special cases of smaller blocksizes */
	switch ( blocksize ) {

		case 1:
			return ibkmk_btridiag_LU( nblocks, 1, L, M, U );
			break;

#if 0
		case 2 :

			if (ibkmk_dense_LU2( &M[0] ) != 0)
				return 1;
			ibkmk_dense_inverse_mult2( &M[0], &U[0] );
			ibkmk_dense_inverse_mult2( &M[0], &L[0] );
			ibkmk_dense_mat_mult_sub2( &L[1], &U[0], &M[1] );
			if ( ibkmk_dense_LU2( &M[1] ) != 0 )
				return 2;
			ibkmk_dense_mat_mult_sub2( &L[1], &L[0], &U[1] );
			ibkmk_dense_inverse_mult_pivot( 2, &M[1], pivots + 2, &U[1] );
			for ( i=2; i<(int)n-1; ++i ) {
				ibkmk_dense_mat_mult_sub2( &L[i], &U[i-1], &M[i] );
				if (ibkmk_dense_LU2( &M[i] ) != 0)
					return i+1;
					ibkmk_dense_inverse_mult_pivot( 2, &M[i], pivots + i*2, &U[i] );
			}
			ibkmk_dense_mat_mult_sub2( &U[n-1], &U[n-3], &L[n-1] );
			ibkmk_dense_mat_mult_sub2( &L[n-1], &U[n-2], &M[n-1] );
			if (ibkmk_dense_LU2( &M[n-1] ) != 0)
				return 2;

		break;

		case 3 :

			if (ibkmk_dense_LU3( &M[0] ) != 0)
				return 1;
			ibkmk_dense_inverse_mult3( &M[0], &U[0]);
			ibkmk_dense_inverse_mult3( &M[0], &L[0]);
			ibkmk_dense_mat_mult_sub3( &L[1], &U[0], &M[1]);
			if (ibkmk_dense_LU3( &M[1] ) != 0)
				return 2;
			ibkmk_dense_mat_mult_sub3( &L[1], &L[0], &U[1]);
			ibkmk_dense_inverse_mult_pivot(3, &M[1], pivots + 3, &U[1]);
			for (i=2; i<(int)n-1; ++i) {
				ibkmk_dense_mat_mult_sub3( &L[i], &U[i-1], &M[i]);
				if (ibkmk_dense_LU3( &M[i] ) != 0)
					return i+1;
					ibkmk_dense_inverse_mult_pivot(3, &M[i], pivots + i*3, &U[i]);
			}
			ibkmk_dense_mat_mult_sub3( &U[n-1], &U[n-3], &L[n-1]);
			ibkmk_dense_mat_mult_sub3( &L[n-1], &U[n-2], &M[n-1]);
			if (ibkmk_dense_LU3( &M[n-1] ) != 0)
				return 3;

		break;
#endif
		default:

			if (ibkmk_dense_LU_pivot(blocksize, M[0], pivots) != 0)
				return 1;
			ibkmk_dense_inverse_mult_pivot(blocksize, M[0], pivots, U[0]);
			ibkmk_dense_inverse_mult_pivot(blocksize, M[0], pivots, L[0]);
			ibkmk_dense_mat_mult_sub(blocksize, L[1], U[0], M[1]);
			if (ibkmk_dense_LU_pivot(blocksize, M[1], pivots + blocksize) != 0)
				return 2;
			ibkmk_dense_mat_mult_sub(blocksize, L[1], L[0], U[1]);
			ibkmk_dense_inverse_mult_pivot(blocksize, M[1], pivots + blocksize, U[1]);
			for (i=2; i<nblocks-1; ++i) {
				ibkmk_dense_mat_mult_sub(blocksize, L[i], U[i-1], M[i]);
				if (ibkmk_dense_LU_pivot(blocksize, M[i], pivots + i*blocksize) != 0)
					return i+1;
				ibkmk_dense_inverse_mult_pivot(blocksize, M[i], pivots + i*blocksize, U[i]);
			}
			ibkmk_dense_mat_mult_sub(blocksize, U[nblocks-1], U[nblocks-3], L[nblocks-1]);
			ibkmk_dense_mat_mult_sub(blocksize, L[nblocks-1], U[nblocks-2], M[nblocks-1]);
			if (ibkmk_dense_LU_pivot(blocksize, M[nblocks-1], pivots + (nblocks-1)*blocksize) != 0)
				return nblocks;

		break;

	} /* switch */

	return 0;
}
// ---------------------------------------------------------------------------


void ibkmk_btridiag_backsolve( IBKMK_CONST int nblocks, IBKMK_CONST int blocksize,
							   IBKMK_CONST double * IBKMK_CONST * L,
							   IBKMK_CONST double * IBKMK_CONST * M,
							   IBKMK_CONST double * IBKMK_CONST * U,
							   double * b)
{
	long int i;

	switch ( blocksize ) {
		case 1 :
			ibkmk_tridiag_backsolve(nblocks, *L, b);
			break;

#ifdef USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS
		case 2 :
			/* forward elimination */
			ibkmk_dense_backsolve2(M[0], b);
			for (i=1; i<nblocks-1; ++i) {
				ibkmk_dense_vec_mult_sub2(L[i], b + (i-1)*2, b + i*2);
				ibkmk_dense_backsolve2(M[i], b + i*2);
			}
			/* last row */
			ibkmk_dense_vec_mult_sub2(L[nblocks-1], b + (nblocks-2)*2, b + (nblocks-1)*2);
			ibkmk_dense_vec_mult_sub2(U[nblocks-1], b + (nblocks-3)*2, b + (nblocks-1)*2);
			ibkmk_dense_backsolve2(M[nblocks-1], b + (nblocks-1)*2);

			/* backward elimination */
			for (i=nblocks-2; i>=1; --i) {
				ibkmk_dense_vec_mult_sub2(U[i], b + (i+1)*2, b + i*2);
			}
			/* first row */
			ibkmk_dense_vec_mult_sub2(U[0], b + 2, b);
			ibkmk_dense_vec_mult_sub2(L[0], b + 4, b);
			break;

		case 3 :
			/* forward elimination */
			ibkmk_dense_backsolve3(M[0], b);
			for (i=1; i<nblocks-1; ++i) {
				ibkmk_dense_vec_mult_sub3(L[i], b + (i-1)*3, b + i*3);
				ibkmk_dense_backsolve3(M[i], b + i*3);
			}
			/* last row */
			ibkmk_dense_vec_mult_sub3(L[nblocks-1], b + (nblocks-2)*3, b + (nblocks-1)*3);
			ibkmk_dense_vec_mult_sub3(U[nblocks-1], b + (nblocks-3)*3, b + (nblocks-1)*3);
			ibkmk_dense_backsolve3(M[nblocks-1], b + (nblocks-1)*3);

			/* backward elimination */
			for (i=nblocks-2; i>=1; --i) {
				ibkmk_dense_vec_mult_sub3(U[i], b + (i+1)*3, b + i*3);
			}
			/* first row */
			ibkmk_dense_vec_mult_sub3(U[0], b + 3, b);
			ibkmk_dense_vec_mult_sub3(L[0], b + 6, b);
			break;

		case 4 :
			/* forward elimination */
			ibkmk_dense_backsolve4(M[0], b);
			for (i=1; i<nblocks-1; ++i) {
				ibkmk_dense_vec_mult_sub4(L[i], b + (i-1)*4, b + i*4);
				ibkmk_dense_backsolve4(M[i], b + i*4);
			}
			/* last row */
			ibkmk_dense_vec_mult_sub4(L[nblocks-1], b + (nblocks-2)*4, b + (nblocks-1)*4);
			ibkmk_dense_vec_mult_sub4(U[nblocks-1], b + (nblocks-3)*4, b + (nblocks-1)*4);
			ibkmk_dense_backsolve4(M[nblocks-1], b + (nblocks-1)*4);

			/* backward elimination */
			for (i=nblocks-2; i>=1; --i) {
				ibkmk_dense_vec_mult_sub4(U[i], b + (i+1)*4, b + i*4);
			}
			/* first row */
			ibkmk_dense_vec_mult_sub4(U[0], b + 4, b);
			ibkmk_dense_vec_mult_sub4(L[0], b + 8, b);
			break;
#endif /* USE_SPECIAL_BTRIDIAG_IMPLEMENTATIONS */

		default :
			/* forward elimination */
			ibkmk_dense_backsolve(blocksize, M[0], b);
			for (i=1; i<nblocks-1; ++i) {
				ibkmk_dense_vec_mult_sub(blocksize, L[i], b + (i-1)*blocksize, b + i*blocksize);
				ibkmk_dense_backsolve(blocksize, M[i], b + i*blocksize);
			}
			/* last row */
			ibkmk_dense_vec_mult_sub(blocksize, L[nblocks-1], b + (nblocks-2)*blocksize, b + (nblocks-1)*blocksize);
			ibkmk_dense_vec_mult_sub(blocksize, U[nblocks-1], b + (nblocks-3)*blocksize, b + (nblocks-1)*blocksize);
			ibkmk_dense_backsolve(blocksize, M[nblocks-1], b + (nblocks-1)*blocksize);

			/* backward elimination */
			for (i=nblocks-2; i>=1; --i) {
				ibkmk_dense_vec_mult_sub(blocksize, U[i], b + (i+1)*blocksize, b + i*blocksize);
			}
			/* first row */
			ibkmk_dense_vec_mult_sub(blocksize, U[0], b + blocksize, b);
			ibkmk_dense_vec_mult_sub(blocksize, L[0], b + 2*blocksize, b);
			break;
	}

}
// ---------------------------------------------------------------------------


void ibkmk_btridiag_backsolve_pivot(IBKMK_CONST int nblocks,
									IBKMK_CONST int blocksize,
									IBKMK_CONST double * IBKMK_CONST * L,
									IBKMK_CONST double * IBKMK_CONST * M,
									IBKMK_CONST double * IBKMK_CONST * U,
									IBKMK_CONST long int * pivots,
									double * b)
{

	int i;
	double * bd;

	switch (blocksize) {
		case 1:
			/* call special implementation for tridiagonal systems */
			ibkmk_tridiag_backsolve(nblocks, *L, b);
			break;

		default:
			/* M, L and U have been initialized already */
			bd = b;
			/* forward elimination */
			ibkmk_dense_backsolve_pivot(blocksize, M[0], pivots, bd);
			for (i=1; i<nblocks-1; ++i) {
				bd += blocksize;
				ibkmk_dense_vec_mult_sub(blocksize, L[i], bd - blocksize, bd);
				ibkmk_dense_backsolve_pivot(blocksize, M[i], pivots + i*blocksize, bd);
			}
			/* last row */
			/* L points to a[0] + (nblocks-2)*blockmemsize */
			/* M points to a[1] + (nblocks-2)*blockmemsize */
			/* bd points to b + (nblocks-2)*blocksize */
			ibkmk_dense_vec_mult_sub(blocksize, L[nblocks-1], bd, bd + blocksize);
			ibkmk_dense_vec_mult_sub(blocksize, U[nblocks-1], bd - blocksize, bd + blocksize);
			ibkmk_dense_backsolve_pivot(blocksize, M[nblocks-1], pivots + (nblocks-1)*blocksize, bd + blocksize);

			/* backward elimination */
			for (i=nblocks-2; i>=1; --i) {
				ibkmk_dense_vec_mult_sub(blocksize, U[i], b + (i+1)*blocksize, b + i*blocksize);
			}
			/* first row */
			ibkmk_dense_vec_mult_sub(blocksize, U[0], b + blocksize, b);
			ibkmk_dense_vec_mult_sub(blocksize, L[0], b + 2*blocksize, b);

	} /* switch */

}


