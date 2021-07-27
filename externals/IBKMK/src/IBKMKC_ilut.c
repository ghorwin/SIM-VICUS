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

#include "IBKMKC_ilut.h"

// ILUT Implementation taken from ITSOL (Saad)
// See OTHER_LICENSES for details

#ifdef __cplusplus  /* wrapper to enable C++ usage */

namespace IBKMK {

#else
	#include <stdlib.h>
	#include <math.h>
#endif

/* prototypes */

static int qsplitC(double *a, int *ind, int n, int Ncut);
static int setupCS(csptr amat, int len, int job);

/*-------------------- function definition*/

/*----------------------------------------------------------------------
| Initialize ILUSpar structs.
|----------------------------------------------------------------------
| on entry:
|==========
|   ( lu )  =  Pointer to a ILUSpar struct.
|       n   =  size of matrix
|
| On return:
|===========
|
|    lu->n
|      ->L     L matrix, SpaFmt format
|      ->D     Diagonals
|      ->U     U matrix, SpaFmt format
|      ->work  working buffer of length n
|      ->bf    buffer
|
| integer value returned:
|             0   --> successful return.
|            -1   --> memory allocation error.
|--------------------------------------------------------------------*/
static int setupILU( iluptr lu, int n ) {
	lu->n  = n;
	lu->D = (double *)malloc( sizeof(double) * n);
	lu->L = (csptr)malloc( sizeof(SparMat));
	setupCS( lu->L, n, 1 );
	lu->U = (csptr)malloc( sizeof(SparMat));
	setupCS( lu->U, n, 1 );
	lu->work = (int *)malloc( sizeof(int) * n);
	return 0;
}
/*---------------------------------------------------------------------
|     end of setupILU
|--------------------------------------------------------------------*/

/*----------------------------------------------------------------------
| Initialize SpaFmt structs.
|----------------------------------------------------------------------
| on entry:
|==========
| ( amat )  =  Pointer to a SpaFmt struct.
|     len   =  size of matrix
|     job   =  0: pattern only
|              1: data and pattern
|
| On return:
|===========
|
|  amat->n
|      ->*nzcount
|      ->**ja
|      ->**ma
|
| integer value returned:
|             0   --> successful return.
|             1   --> memory allocation error.
|--------------------------------------------------------------------*/
static int setupCS(csptr amat, int len, int job) {
	amat->n = len;
	amat->nzcount = (int *)malloc( len*sizeof(int));
	amat->ja = (int **) malloc( len*sizeof(int *));
	if( job == 1 )
		amat->ma = (double **) malloc( len*sizeof(double *));
	else
		amat->ma = NULL;
	return 0;
}
/*---------------------------------------------------------------------
|     end of setupCS
|--------------------------------------------------------------------*/


int ibkmk_ilut( csptr csmat, iluptr lu, int lfil, double tol, FILE *fp ) {
	int n = csmat->n;
	int len, lenu, lenl;
	int nzcount, *ja, *jbuf, *iw, i, j, k;
	int col, jpos, jrow, upos;
	double t, tnorm, tolnorm, fact, lxu, *wn, *ma, *w;
	csptr L, U;
	double *D;

	if( lfil < 0 ) {
		fprintf( fp, "ilut: Illegal value for lfil.\n" );
		return -1;
	}

	setupILU( lu, n );
	L = lu->L;
	U = lu->U;
	D = lu->D;

	iw = (int *)malloc( n*sizeof(int));
	jbuf = (int *)malloc( n*sizeof(int));
	wn = (double *)malloc( n * sizeof(double));
	w = (double *)malloc( n * sizeof(double));

	/* set indicator array jw to -1 */
	for( i = 0; i < n; i++ ) iw[i] = -1;

	/* beginning of main loop */
	for( i = 0; i < n; i++ ) {
		nzcount = csmat->nzcount[i];
		ja = csmat->ja[i];
		ma = csmat->ma[i];
		/* calculate error norm: tol * 1/n sum_j |aij|  */
		tnorm = 0;
		for( j = 0; j < nzcount; j++ ) {
			tnorm += fabs( ma[j] );
		}
		if( tnorm == 0.0 ) {
			fprintf( fp, "ilut: zero row encountered.\n" );
			return -2;
		}
		tnorm /= (double)nzcount;
		tolnorm = tol * tnorm;

		/* unpack L-part and U-part of column of A in arrays w */
		lenu = 0;
		lenl = 0;
		jbuf[i] = i;
		w[i] = 0;
		iw[i] = i;
		for( j = 0; j < nzcount; j++ ) {
			col = ja[j];
			t = ma[j];
			if( col < i ) {
				iw[col] = lenl;
				jbuf[lenl] = col;
				w[lenl] = t;
				lenl++;
			} else if( col == i ) {
				w[i] = t;
			} else {
				lenu++;
				jpos = i + lenu;
				iw[col] = jpos;
				jbuf[jpos] = col;
				w[jpos] = t;
			}
		}

		j = -1;
		len = 0;
		/* eliminate previous rows: */
		while( ++j < lenl ) { // lower part: j < i
			/*----------------------------------------------------------------------------
			 *  in order to do the elimination in the correct order we must select the
			 *  smallest column index among jbuf[k], k = j+1, ..., lenl
			 *--------------------------------------------------------------------------*/
			jrow = jbuf[j];
			jpos = j;
			/* determine smallest column index */
			for( k = j + 1; k < lenl; k++ ) {
				if( jbuf[k] < jrow ) {
					jrow = jbuf[k];
					jpos = k;
				}
			}
			if( jpos != j ) {
				col = jbuf[j];
				jbuf[j] = jbuf[jpos];
				jbuf[jpos] = col;
				iw[jrow] = j;
				iw[col]  = jpos;
				t = w[j];
				w[j] = w[jpos];
				w[jpos] = t;
			}

			/* get the multiplier: */
			/* fact = aij/djj (j < i)*/
			fact = w[j] * D[jrow];
			/* lij := aij/djj (j < i)*/
			w[j] = fact;
			/* zero out element in row by resetting iw(n+jrow) to -1 */
			iw[jrow] = -1;

			/* combine current row and row jrow */
			nzcount = U->nzcount[jrow];
			ja = U->ja[jrow];
			ma = U->ma[jrow];
			for( k = 0; k < nzcount; k++ ) {
				col = ja[k];
				jpos = iw[col];
				/* lxu = - lij * ujk (j < i, j < k)*/
				lxu = - fact * ma[k];
				/* if fill-in element is small then disregard */
				if( fabs( lxu ) < tolnorm && jpos == -1 ) continue;

				if( col < i ) {
					/* dealing with lower part */
					/* lik := lik - sum_j<k lij * ujk (k < i)*/
					if( jpos == -1 ) {
						/* this is a fill-in element */
						jbuf[lenl] = col;
						iw[col] = lenl;
						w[lenl] = lxu;
						lenl++;
					} else {
						w[jpos] += lxu;
					}

				} else {
					/* dealing with upper part */
					/* uik := aik - sum_j<i lij * ujk  (k > i)*/
					//          if( jpos == -1 ) {
					if( jpos == -1 && fabs(lxu) > tolnorm) {
						/* this is a fill-in element */
						lenu++;
						upos = i + lenu;
						jbuf[upos] = col;
						iw[col] = upos;
						w[upos] = lxu;
					} else {
						w[jpos] += lxu;
					}
				}
			}
		}

		/* restore iw */
		iw[i] = -1;
		for( j = 0; j < lenu; j++ ) {
			iw[jbuf[i+j+1]] = -1;
		}

		/*---------- case when diagonal is zero */
		if( w[i] == 0.0 ) {
			fprintf( fp, "zero diagonal encountered.\n" );
			for( j = i; j < n; j++ ) {
				L->ja[j] = NULL;
				L->ma[j] = NULL;
				U->ja[j] = NULL;
				U->ma[j] = NULL;
			}
			return -2;
		}
		/*-----------Update diagonal */
		/* dii := 1/dii*/
		D[i] = 1 / w[i];

		/* update L-matrix */
		//    len = min( lenl, lfil );
		len = lenl < lfil ? lenl : lfil;
		for( j = 0; j < lenl; j++ ) {
			wn[j] = fabs( w[j] );
			iw[j] = j;
		}
		qsplitC( wn, iw, lenl, len );
		L->nzcount[i] = len;
		if( len > 0 ) {
			ja = L->ja[i] = (int *)malloc( len*sizeof(int));
			ma = L->ma[i] = (double *)malloc( len*sizeof(double));
		}
		for( j = 0; j < len; j++ ) {
			jpos = iw[j];
			ja[j] = jbuf[jpos];
			ma[j] = w[jpos];
		}
		for( j = 0; j < lenl; j++ ) iw[j] = -1;

		/* update U-matrix */
		//    len = min( lenu, lfil );
		len = lenu < lfil ? lenu : lfil;
		for( j = 0; j < lenu; j++ ) {
			wn[j] = fabs( w[i+j+1] );
			iw[j] = i+j+1;
		}
		qsplitC( wn, iw, lenu, len );
		U->nzcount[i] = len;
		if( len > 0 ) {
			ja = U->ja[i] = (int *)malloc( len*sizeof(int));
			ma = U->ma[i] = (double *)malloc( len*sizeof(double));
		}
		for( j = 0; j < len; j++ ) {
			jpos = iw[j];
			ja[j] = jbuf[jpos];
			ma[j] = w[jpos];
		}
		for( j = 0; j < lenu; j++ ) {
			iw[j] = -1;
		}
	}

	free( iw );
	free( jbuf );
	free( wn );
	free(w);

	return 0;
}


int ibkmk_lutsolve( double *y, double *x, iluptr lu ) {
	int n = lu->n, i, j, nzcount, *ja;
	double *D, *ma;
	csptr L, U;

	L = lu->L;
	U = lu->U;
	D = lu->D;

	/* Block L solve */
	for( i = 0; i < n; i++ ) {
		x[i] = y[i];
		nzcount = L->nzcount[i];
		ja = L->ja[i];
		ma = L->ma[i];
		for( j = 0; j < nzcount; j++ ) {
			x[i] -= x[ja[j]] * ma[j];
		}
	}
	/* Block -- U solve */
	for( i = n-1; i >= 0; i-- ) {
		nzcount = U->nzcount[i];
		ja = U->ja[i];
		ma = U->ma[i];
		for( j = 0; j < nzcount; j++ ) {
			x[i] -= x[ja[j]] * ma[j];
		}
		x[i] *= D[i];
	}

	return 0;
}

/*----------------------------------------------------------------------
|     does a quick-sort split of a real array.
|     on input a[0 : (n-1)] is a real array
|     on output is permuted such that its elements satisfy:
|
|     abs(a[i]) >= abs(a[Ncut-1]) for i < Ncut-1 and
|     abs(a[i]) <= abs(a[Ncut-1]) for i > Ncut-1
|
|     ind[0 : (n-1)] is an integer array permuted in the same way as a.
|---------------------------------------------------------------------*/
static int qsplitC(double *a, int *ind, int n, int Ncut) {
	double tmp, abskey;
	int j, itmp, first, mid, last, ncut;
	ncut = Ncut - 1;

	first = 0;
	last = n-1;
	if (ncut<first || ncut>last) return 0;
	/* outer loop -- while mid != ncut */
	do {
		mid = first;
		abskey = fabs(a[mid]);
		for (j=first+1; j<=last; j++) {
			if (fabs(a[j]) > abskey) {
				mid = mid+1;
				tmp = a[mid];
				itmp = ind[mid];
				a[mid] = a[j];
				ind[mid] = ind[j];
				a[j]  = tmp;
				ind[j] = itmp;
			}
		}
		/*-------------------- interchange */
		tmp = a[mid];
		a[mid] = a[first];
		a[first]  = tmp;
		itmp = ind[mid];
		ind[mid] = ind[first];
		ind[first] = itmp;
		/*-------------------- test for while loop */
		if (mid == ncut) break;
		if (mid > ncut)
			last = mid-1;
		else
			first = mid+1;
	} while(mid != ncut);

	return 0;
}


void ibkmk_free_ilut(iluptr lu) {
	int i;
	// free( factorized matrix (ilut will allocate new memory)
	if (lu->L != NULL) {
		for (i = 0; i < lu->n; i++) {
			if (lu->L->nzcount[i] > 0) {
				free( lu->L->ja[i] );
				free( lu->L->ma[i] );
			}
			if (lu->U->nzcount[i] > 0) {
				free( lu->U->ja[i] );
				free( lu->U->ma[i] );
			}
		}
		free( lu->L->ja );
		free( lu->L->nzcount );
		free( lu->L->ma );
		free( lu->U->ja );
		free( lu->U->ma );
		free( lu->U->nzcount );

		free( lu->L );
		free( lu->U );
		free( lu->D );
		free( lu->work );

		lu->L = NULL;
		lu->U = NULL;
		lu->D = NULL;
		lu->work = NULL;
	}

}

#ifdef __cplusplus
} // namespace IBKMKC

#endif
