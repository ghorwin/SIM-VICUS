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

#include "IBKMKC_dense_matrix.h"

#include <math.h>

#ifdef __cplusplus  /* wrapper to enable C++ usage */

namespace IBKMK {

#endif

int ibkmk_dense_LU_pivot(int n, double * A, long int * p) {
	int i, j, k, l;
	double *col_i, *col_j, *col_k;
	double temp, mult, a_kj;

	/* k-th elimination step number */
	col_k  = A;
	for (k=0; k < n; k++, col_k += n) {

		/* find l = pivot row number */
		l=k;
		for (i=k+1; i < n; i++)
			if (fabs(col_k[i]) > fabs(col_k[l])) l=i;
		p[k] = l;

		/* check for zero pivot element */
		if (col_k[l] == 0.0) return(k+1);

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
		mult = 1.0/col_k[k];
		for(i=k+1; i < n; i++)
			col_k[i] *= mult;

		/* row_i = row_i - [a(i,k)/a(k,k)] row_k, i=k+1, ..., m-1 */
		/* row k is the pivot row after swapping with row l.      */
		/* The computation is done one column at a time,          */
		/* column j=k+1, ..., n-1.                                */

		col_j = A + (k+1)*n;
		for (j=k+1; j < n; j++, col_j += n) {

			a_kj = col_j[k];

			/* a(i,j) = a(i,j) - [a(i,k)/a(k,k)]*a(k,j)  */
			/* a_kj = a(k,j), col_k[i] = - a(i,k)/a(k,k) */

			if (a_kj != 0.0) {
				for (i=k+1; i < n; i++)
					col_j[i] -= a_kj * col_k[i];
			}
		}
	}

	/* return 0 to indicate success */

	return 0;
}
// ---------------------------------------------------------------------------


void ibkmk_dense_backsolve_pivot(int n, IBKMK_CONST double * A, IBKMK_CONST long int * p, double * b) {
	int i, k, pk;
	double *col_k, tmp;

	/* Permute b, based on pivot information in p */
	for (k=0; k<n; k++) {
		pk = p[k];
		if (pk != k) {
			tmp = b[k];
			b[k] = b[pk];
			b[pk] = tmp;
		}
	}

	/* Solve Ly = b, store solution y in b */
	col_k = A;
	for (k=0; k<n-1; k++, col_k += n) {
		for (i=k+1; i<n; i++)
			b[i] -= col_k[i]*b[k];
	}

	/* Solve Ux = y, store solution x in b */
	col_k = A + (n-1)*n;
	for (k = n-1; k > 0; k--, col_k -= n) {
		b[k] /= col_k[k];
		for (i=0; i<k; i++)
			b[i] -= col_k[i]*b[k];
	}
	b[0] /= A[0];
}
// ---------------------------------------------------------------------------


void ibkmk_dense_inverse_mult_pivot(int n, IBKMK_CONST double * A, IBKMK_CONST long int * p, double * B) {
	int i;
	for (i=0; i<n; ++i, B += n)
		ibkmk_dense_backsolve_pivot(n, A, p, B);
}
// ---------------------------------------------------------------------------


int ibkmk_dense_LU(int n, double * A) {
	int i,j,k;
	for (k=0; k<n-1;++k) {
		for (i=k+1;i<n;++i) {
			if (A[k + k*n] == 0) return -2;
			A[i + k*n] /= A[k + k*n];
			for (j=k+1; j<n; ++j)
				A[i + j*n] -= A[i + k*n]*A[k + j*n];
		}
	}
	/* check that the last computed main diagonal element is not zero */
	if (A[n-1 + (n-1)*n] == 0) return -2;
	return 0;
}
// ---------------------------------------------------------------------------


int ibkmk_dense_LU2(double * A) {
	if (A[0] == 0) return -2;
	A[1] /= A[0];
	A[3] -= A[1]*A[2];
	if (A[3] == 0) return -2;
	return 0;
}
// ---------------------------------------------------------------------------


int ibkmk_dense_LU3(double * A) {
	if (A[0] == 0) return -2;
	/* column 0 */
	A[1] /= A[0];
	A[2] /= A[0];

	/* column 1 */
	A[4] -= A[1]*A[3];
	if (A[4] == 0) return -2;

	A[5] -= A[2]*A[3];
	A[5] /= A[4];

	/* column 2 */
	A[7] -= A[1]*A[6];
	A[8] -= A[2]*A[6] + A[5]*A[7];
	if (A[8] == 0) return -2;

	return 0;
}
// ---------------------------------------------------------------------------


int ibkmk_dense_LU4(double * A) {

	if (A[0] == 0) return -2;
	/* column 0 */
	A[1] /= A[0];
	A[2] /= A[0];
	A[3] /= A[0];

	/* column 1 */
	A[5] -= A[1]*A[4];
	if (A[5] == 0) return -2;

	A[6] -= A[2]*A[4];
	A[6] /= A[5];
	A[7] -= A[3]*A[4];
	A[7] /= A[5];

	/* column 2 */
	A[9] -= A[1]*A[8];
	A[10] -= A[2]*A[8] + A[6]*A[9];
	if (A[10] == 0) return -2;

	A[11] -= A[3]*A[8] + A[7]*A[9];
	A[11] /= A[10];

	/* column 3 */
	A[13] -= A[1]*A[12];
	A[14] -= A[2]*A[12] + A[6]*A[13];
	A[15] -= A[3]*A[12] + A[7]*A[13] + A[11]*A[14];
	if (A[15] == 0) return -2;

	return 0;

}
// ---------------------------------------------------------------------------


void ibkmk_dense_backsolve(int n, IBKMK_CONST double * A, double * b) {

	int i,k;

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
// ---------------------------------------------------------------------------


void ibkmk_dense_backsolve2(IBKMK_CONST double * A, double * b) {

	/* forward elimination */
	b[1] -= A[1]*b[0];

	/* backward elimination */
	b[1] /= A[3];
	b[0] -= A[2]*b[1];
	b[0] /= A[0];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_backsolve3(IBKMK_CONST double * A, double * b) {

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
// ---------------------------------------------------------------------------


void ibkmk_dense_backsolve4(IBKMK_CONST double * A, double * b) {

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
	b[0] -= A[12]*b[3] + A[8]*b[2] + A[4]*b[1];
	b[0] /= A[0];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult(int n, IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	int i,j,k;
	for (i=0; i<n*n; ++i)
		C[i] = 0;
	for (j=0; j<n; ++j)
		for (k=0; k<n; ++k)
			for (i=0; i<n; ++i)
				C[i + j*n] += A[i + k*n] * B[k + j*n];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult2(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	/** j = 0 **/
	C[0] = A[0] * B[0] + A[2] * B[1];
	C[1] = A[1] * B[0] + A[3] * B[1];

	/** j = 1 **/
	C[2] = A[0] * B[2] + A[2] * B[3];
	C[3] = A[1] * B[2] + A[3] * B[3];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult3(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	/** j = 0 **/
	C[0] = A[0] * B[0] + A[3] * B[1] + A[6] * B[2];
	C[1] = A[1] * B[0] + A[4] * B[1] + A[7] * B[2];
	C[2] = A[2] * B[0] + A[5] * B[1] + A[8] * B[2];

	/** j = 1 **/
	C[3] = A[0] * B[3] + A[3] * B[4] + A[6] * B[5];
	C[4] = A[1] * B[3] + A[4] * B[4] + A[7] * B[5];
	C[5] = A[2] * B[3] + A[5] * B[4] + A[8] * B[5];

	/** j = 2 **/
	C[6] = A[0] * B[6] + A[3] * B[7] + A[6] * B[8];
	C[7] = A[1] * B[6] + A[4] * B[7] + A[7] * B[8];
	C[8] = A[2] * B[6] + A[5] * B[7] + A[8] * B[8];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult4(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	/** j = 0 **/
	C[0] = A[0] * B[0] + A[4] * B[1] + A[8] * B[2] + A[12] * B[3];
	C[1] = A[1] * B[0] + A[5] * B[1] + A[9] * B[2] + A[13] * B[3];
	C[2] = A[2] * B[0] + A[6] * B[1] + A[10] * B[2] + A[14] * B[3];
	C[3] = A[3] * B[0] + A[7] * B[1] + A[11] * B[2] + A[15] * B[3];

	/** j = 1 **/
	C[4] = A[0] * B[4] + A[4] * B[5] + A[8] * B[6] + A[12] * B[7];
	C[5] = A[1] * B[4] + A[5] * B[5] + A[9] * B[6] + A[13] * B[7];
	C[6] = A[2] * B[4] + A[6] * B[5] + A[10] * B[6] + A[14] * B[7];
	C[7] = A[3] * B[4] + A[7] * B[5] + A[11] * B[6] + A[15] * B[7];

	/** j = 2 **/
	C[8] = A[0] * B[8] + A[4] * B[9] + A[8] * B[10] + A[12] * B[11];
	C[9] = A[1] * B[8] + A[5] * B[9] + A[9] * B[10] + A[13] * B[11];
	C[10] = A[2] * B[8] + A[6] * B[9] + A[10] * B[10] + A[14] * B[11];
	C[11] = A[3] * B[8] + A[7] * B[9] + A[11] * B[10] + A[15] * B[11];

	/** j = 3 **/
	C[12] = A[0] * B[12] + A[4] * B[13] + A[8] * B[14] + A[12] * B[15];
	C[13] = A[1] * B[12] + A[5] * B[13] + A[9] * B[14] + A[13] * B[15];
	C[14] = A[2] * B[12] + A[6] * B[13] + A[10] * B[14] + A[14] * B[15];
	C[15] = A[3] * B[12] + A[7] * B[13] + A[11] * B[14] + A[15] * B[15];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_inverse_mult(int n, IBKMK_CONST double * A, double * B) {
	int i;
	for (i=0; i<n; ++i)
		ibkmk_dense_backsolve(n, A, B + i*n);
}
// ---------------------------------------------------------------------------


void ibkmk_dense_inverse_mult2(IBKMK_CONST double * A, double * B) {
	ibkmk_dense_backsolve2(A, B);
	ibkmk_dense_backsolve2(A, B+2);
}
// ---------------------------------------------------------------------------


void ibkmk_dense_inverse_mult3(IBKMK_CONST double * A, double * B) {
	ibkmk_dense_backsolve3(A, B);
	ibkmk_dense_backsolve3(A, B+3);
	ibkmk_dense_backsolve3(A, B+6);
}
// ---------------------------------------------------------------------------


void ibkmk_dense_inverse_mult4(IBKMK_CONST double * A, double * B) {
	ibkmk_dense_backsolve4(A, B);
	ibkmk_dense_backsolve4(A, B+4);
	ibkmk_dense_backsolve4(A, B+8);
	ibkmk_dense_backsolve4(A, B+12);
}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult_sub(int n, IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	int i,j,k;
	for (j=0; j<n; ++j)
		for (k=0; k<n; ++k)
			for (i=0; i<n; ++i)
				C[i + j*n] -= A[i + k*n] * B[k + j*n];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult_sub2(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	/** j = 0 **/
	C[0] -= ( A[0] * B[0] + A[2] * B[1] );
	C[1] -= ( A[1] * B[0] + A[3] * B[1] );

	/** j = 1 **/
	C[2] -= ( A[0] * B[2] + A[2] * B[3] );
	C[3] -= ( A[1] * B[2] + A[3] * B[3] );

}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult_sub3(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	/** j = 0 **/
	C[0] -= ( A[0] * B[0] + A[3] * B[1] + A[6] * B[2]);
	C[1] -= ( A[1] * B[0] + A[4] * B[1] + A[7] * B[2]);
	C[2] -= ( A[2] * B[0] + A[5] * B[1] + A[8] * B[2]);

	/** j = 1 **/
	C[3] -= ( A[0] * B[3] + A[3] * B[4] + A[6] * B[5] );
	C[4] -= ( A[1] * B[3] + A[4] * B[4] + A[7] * B[5] );
	C[5] -= ( A[2] * B[3] + A[5] * B[4] + A[8] * B[5] );

	/** j = 2 **/
	C[6] -= ( A[0] * B[6] + A[3] * B[7] + A[6] * B[8] );
	C[7] -= ( A[1] * B[6] + A[4] * B[7] + A[7] * B[8] );
	C[8] -= ( A[2] * B[6] + A[5] * B[7] + A[8] * B[8] );

}
// ---------------------------------------------------------------------------


void ibkmk_dense_mat_mult_sub4(IBKMK_CONST double * A, IBKMK_CONST double * B, double * C) {

	/** j = 0 **/
	C[0] -= ( A[0] * B[0] + A[4] * B[1] + A[8] * B[2] + A[12] * B[3] );
	C[1] -= ( A[1] * B[0] + A[5] * B[1] + A[9] * B[2] + A[13] * B[3] );
	C[2] -= ( A[2] * B[0] + A[6] * B[1] + A[10] * B[2] + A[14] * B[3] );
	C[3] -= ( A[3] * B[0] + A[7] * B[1] + A[11] * B[2] + A[15] * B[3] );

	/** j = 1 **/
	C[4] -= ( A[0] * B[4] + A[4] * B[5] + A[8] * B[6] + A[12] * B[7] );
	C[5] -= ( A[1] * B[4] + A[5] * B[5] + A[9] * B[6] + A[13] * B[7] );
	C[6] -= ( A[2] * B[4] + A[6] * B[5] + A[10] * B[6] + A[14] * B[7] );
	C[7] -= ( A[3] * B[4] + A[7] * B[5] + A[11] * B[6] + A[15] * B[7] );

	/** j = 2 **/
	C[8] -= ( A[0] * B[8] + A[4] * B[9] + A[8] * B[10] + A[12] * B[11] );
	C[9] -= ( A[1] * B[8] + A[5] * B[9] + A[9] * B[10] + A[13] * B[11] );
	C[10] -= ( A[2] * B[8] + A[6] * B[9] + A[10] * B[10] + A[14] * B[11] );
	C[11] -= ( A[3] * B[8] + A[7] * B[9] + A[11] * B[10] + A[15] * B[11] );

	/** j = 3 **/
	C[12] -= ( A[0] * B[12] + A[4] * B[13] + A[8] * B[14] + A[12] * B[15] );
	C[13] -= ( A[1] * B[12] + A[5] * B[13] + A[9] * B[14] + A[13] * B[15] );
	C[14] -= ( A[2] * B[12] + A[6] * B[13] + A[10] * B[14] + A[14] * B[15] );
	C[15] -= ( A[3] * B[12] + A[7] * B[13] + A[11] * B[14] + A[15] * B[15] );

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_add(int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	int k, i;
	for (k=0; k<n; ++k)
		for (i=0; i<n; ++i)
			d[i] += A[i + k*n] * b[k];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_add2(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] += A[0] * b[0] + A[2] * b[1];
	d[1] += A[1] * b[0] + A[3] * b[1];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_add3(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] += A[0] * b[0] + A[3] * b[1] + A[6] * b[2];
	d[1] += A[1] * b[0] + A[4] * b[1] + A[7] * b[2];
	d[2] += A[2] * b[0] + A[5] * b[1] + A[8] * b[2];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_add4(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] += A[0] * b[0] + A[4] * b[1] + A[8] * b[2] + A[12] * b[3];
	d[1] += A[1] * b[0] + A[5] * b[1] + A[9] * b[2] + A[13] * b[3];
	d[2] += A[2] * b[0] + A[6] * b[1] + A[10] * b[2] + A[14] * b[3];
	d[3] += A[3] * b[0] + A[7] * b[1] + A[11] * b[2] + A[15] * b[3];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_sub(int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	int i,k;
	for (k=0; k<n; ++k)
		for (i=0; i<n; ++i)
			d[i] -= A[i + k*n] * b[k];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_sub2(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] -= ( A[0] * b[0] + A[2] * b[1] );
	d[1] -= ( A[1] * b[0] + A[3] * b[1] );

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_sub3(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] -= ( A[0] * b[0] + A[3] * b[1] + A[6] * b[2] );
	d[1] -= ( A[1] * b[0] + A[4] * b[1] + A[7] * b[2] );
	d[2] -= ( A[2] * b[0] + A[5] * b[1] + A[8] * b[2] );

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult_sub4(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] -= (A[0] * b[0] + A[4] * b[1] + A[8] * b[2] + A[12] * b[3]);
	d[1] -= (A[1] * b[0] + A[5] * b[1] + A[9] * b[2] + A[13] * b[3]);
	d[2] -= (A[2] * b[0] + A[6] * b[1] + A[10] * b[2] + A[14] * b[3]);
	d[3] -= (A[3] * b[0] + A[7] * b[1] + A[11] * b[2] + A[15] * b[3]);

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult(int n, IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	int i,j;
	switch (n) {

		case 2:
			d[0] = A[0] * b[0] + A[2] * b[1];
			d[1] = A[1] * b[0] + A[3] * b[1];
			break;

		case 3:
			d[0] = A[0] * b[0] + A[3] * b[1] + A[6] * b[2];
			d[1] = A[1] * b[0] + A[4] * b[1] + A[7] * b[2];
			d[2] = A[2] * b[0] + A[5] * b[1] + A[8] * b[2];
			break;

		case 4:
			d[0] = A[0] * b[0] + A[4] * b[1] + A[8] * b[2] + A[12] * b[3];
			d[1] = A[1] * b[0] + A[5] * b[1] + A[9] * b[2] + A[13] * b[3];
			d[2] = A[2] * b[0] + A[6] * b[1] + A[10] * b[2] + A[14] * b[3];
			d[3] = A[3] * b[0] + A[7] * b[1] + A[11] * b[2] + A[15] * b[3];
			break;

		default:
			for (i=0; i<n; ++i)
				d[i] = 0;
			// Note: if the following lines are swapped, we loose about 30% performance!!!
			for (j=0; j<n; ++j) {
				for (i=0; i<n; ++i) {
					d[i] += A[i + j*n] * b[j];
				}
			}
	}


}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult2(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] = A[0] * b[0] + A[2] * b[1];
	d[1] = A[1] * b[0] + A[3] * b[1];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult3(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] = A[0] * b[0] + A[3] * b[1] + A[6] * b[2];
	d[1] = A[1] * b[0] + A[4] * b[1] + A[7] * b[2];
	d[2] = A[2] * b[0] + A[5] * b[1] + A[8] * b[2];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_vec_mult4(IBKMK_CONST double * A, IBKMK_CONST double * b, double * d) {

	d[0] = A[0] * b[0] + A[4] * b[1] + A[8] * b[2] + A[12] * b[3];
	d[1] = A[1] * b[0] + A[5] * b[1] + A[9] * b[2] + A[13] * b[3];
	d[2] = A[2] * b[0] + A[6] * b[1] + A[10] * b[2] + A[14] * b[3];
	d[3] = A[3] * b[0] + A[7] * b[1] + A[11] * b[2] + A[15] * b[3];

}
// ---------------------------------------------------------------------------


void ibkmk_dense_row_matvecsum(IBKMK_CONST double * Arow, int m, int n, double * b, double * r) {

	int j,stride;
	stride = m*m;
	/* Zero out resultant vector */
	for (j=0; j<m; ++j)
		r[j] = 0;
	/* Multiplication loop */
	for (j=0; j<n; ++j, Arow += stride, b += m) {
		/* multiply block-submatrix A[j] with block-vector b[j] and add to r */
		ibkmk_dense_vec_mult_add(m, Arow, b, r);
	}

}
// ---------------------------------------------------------------------------


#ifdef __cplusplus
} // namespace IBKMKC

#endif
