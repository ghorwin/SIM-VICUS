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

#include "IBKMKC_band_matrix.h"
#include "IBKMKC_dense_matrix.h"

void ibkmk_blockband_multiply(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu, double * A, double * b, double * r) {
	unsigned int i,bw,m2,stride;

	bw = ml + mu + 1;
	m2 = m*m;
	stride = bw*m2;
	/* loop over all block-rows with ml < i*/
	for (i=0; i<ml; ++i) {
		/* sum matrices in block row i, matrix length growing to left */
		ibkmk_dense_row_matvecsum(A + i*stride + (ml - i)*m2, m, bw-ml+i, b, r+m*i);
	}

	/* loop over all block-rows with i < n-mu, i := i - ml - mu*/
	for (; i<n-mu; ++i) {
		/* sum matrices in block row i, full matrix length */
		ibkmk_dense_row_matvecsum(A + i*stride, m, bw, b+(i-ml)*m, r+m*i);
	}

	/* loop over all block-rows with i >= n-mu*/
	for (; i<n; ++i) {
		ibkmk_dense_row_matvecsum(A + i*stride, m, ml+n-i, b+(i-ml)*m, r+m*i);
	}
}



void ibkmk_blockband_multiply2(unsigned int n, unsigned int ml, unsigned int mu, double * A, double * b, double * r) {
	unsigned int i,bw,stride;

	bw = ml + mu + 1;
	stride = bw*2*2;
	/* loop over all block-rows with ml < i*/
	for (i=0; i<ml; ++i) {
		/* sum matrices in block row i, matrix length growing to left */
		ibkmk_dense_row_matvecsum(A + i*stride + (ml - i)*4, 2, bw-ml+i, b, r+2*i);
	}

	/* loop over all block-rows with i < n-mu, i := i - ml - mu*/
	for (; i<n-mu; ++i) {
		/* sum matrices in block row i, full matrix length */
		ibkmk_dense_row_matvecsum(A + i*stride, 2, bw, b+(i-ml)*2, r+2*i);
	}

	/* loop over all block-rows with i >= n-mu*/
	for (; i<n; ++i) {
		ibkmk_dense_row_matvecsum(A + i*stride, 2, ml+n-i, b+(i-ml)*2, r+2*i);
	}
}


int ibkmk_blockband_lu(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu, double * A) {
	unsigned int i, j, k, stride, bw, m2;
	double * Adiag;

	/* block band-width */
	bw = ml + mu + 1;
	/* block-size */
	m2 = m*m;
	/* stride between matrix rows */
	stride = m2*bw;

	/* k - Elimination loop */
	Adiag = A + ml*m2; /* Adiag points to the main diagonal element in the current elimination row */
	for (k=0; k<n; ++k, Adiag += stride) {
		/* Perform in-place LU factorization of block [k,k] */
		ibkmk_dense_LU(m, Adiag);

		/* row k, columns k+1 .. k+mu, j is index in data storage matrix */
		for (j=1; j<mu; ++j) {
//			ibkmk_dense_backsolve(m, Adiag + m2*j

		}

		/* column k, rows k+1 .. k+ml */
		for (i=k+1; i<k+ml; ++i) {

		}


	}
	return 0;
}


void ibkmk_blockband_backsolve(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu,
							   double * A, double * b)
{
	unsigned int i, j, stride, bw, m2;
	double * Arow, *brow;
	double w[10]; /* workspace */
	/* block band-width */
	bw = ml + mu + 1;
	/* block-size */
	m2 = m*m;
	/* stride between matrix rows */
	stride = m2*bw;

	/*
	   ----------------------
	   Solve L*z = b
	   ----------------------

	   We utilize block-columns 0..ml from the matrix data storage.

	*/

	Arow = A;
	brow = b;

	/* first row, compute block-vector z_0 = L(0,0)^-1 * b_0
	   L(0,0) is already LU-factored, so this is a back-solving operation
	*/
	ibkmk_dense_backsolve(m, Arow+m2*ml, brow);
	Arow += stride;
	brow += m;

	/* first 1..ml-1 rows, here we use in each row more blocks left of the main
	   diagonal until full lower half-bandwidth is reached, each row is multiplied
	   with the right-hand side vector b[0...i]
	*/
	for (i=1; i<ml; ++i, Arow += stride, brow += m) {
		/* compute row sum of matrix row i with first i rows of vector b (holding z values) */
		ibkmk_dense_row_matvecsum(Arow+m2*(ml-i), m, i, b, w);
		/* subtract sum from block b_i */
		for (j=0; j<m; ++j)
			brow[j] -= w[j];
		/* compute z_i = L(i,i)^-1 * (b_i - sum(L_i*b_0..i-1) */
		ibkmk_dense_backsolve(m, Arow+m2*ml, brow);
	}

	/* full rows, ml...n-1, here all block columns 0..ml of the matrix storage data are used,
	   each row is multiplied with the right-hand side vector b[i-ml...i-ml+bw]
	*/
	for (; i<n; ++i, Arow += stride, brow += m) {
		/* compute row sum of matrix row i with first i rows of vector b (holding z values) */
		ibkmk_dense_row_matvecsum(Arow, m, ml, b+(i-ml)*m, w);
		/* subtract sum from block b_i */
		for (j=0; j<m; ++j)
			brow[j] -= w[j];
		/* compute z_i = L(i,i)^-1 * (b_i - sum(L_i*b_0..i-1) */
		ibkmk_dense_backsolve(m, Arow+m2*ml, brow);
	}

	/*
	   ----------------------
	   Solve U*x = z
	   ----------------------

	   Arow -> points to last row+1
	   brow -> points to last row+1
	*/


	/* skip last row: nothing to do x_n-1 = z_n-1 */
	Arow -= 2*stride;
	brow -= 2*m;

	/* from now on we only need to multiply from the main diagonal + 1,
	   so we shift the Arow pointer accordingly. */
	Arow += (ml+1)*m2;

	/*
	   Arow -> points to row n-2, main diagonal + 1 block
	   brow -> points to row n-2
	*/

	/* last rows i = n-2...n-mu, note: loop counter i := n-1-i */
	for (i = 1; i<mu; ++i, Arow -= stride, brow -= m) {
		/* compute row sum of matrix row (n-i) with first i rows of vector b (holding z values) */
		ibkmk_dense_row_matvecsum(Arow, m, i, brow+m, w);
		/* subtract sum from block b_i */
		for (j=0; j<m; ++j)
			brow[j] -= w[j];
		/* since U(i,i) = I -> x_i = z_i - sum(U_i,z) */
	}

	/* full rows from n-mu...0, note: loop counter i := n-1-i */
	for (; i<n; ++i, Arow -= stride, brow -= m) {
		/* compute row sum of matrix row i with first mu rows of vector b (holding z values) */
		ibkmk_dense_row_matvecsum(Arow, m, mu, brow+m, w);
		/* subtract sum from block b_i */
		for (j=0; j<m; ++j)
			brow[j] -= w[j];
		/* since U(i,i) = I -> x_i = z_i - sum(U_i,z) */
	}
}
