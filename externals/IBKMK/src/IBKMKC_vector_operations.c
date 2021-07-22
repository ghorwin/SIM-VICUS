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

#include "IBKMKC_vector_operations.h"

#include <memory.h>
#include <IBK_openMP.h>

#ifdef __cplusplus
namespace IBKMK {
#endif


void vectorUInt_fill( IBKMK_CONST unsigned int vectorSize, unsigned int * targetVector, IBKMK_CONST unsigned int value){

	int i=0;
	int j;

#pragma omp single copyprivate(i)
	{
		for (; i<(int)vectorSize % 8; ++i) {
			targetVector[i] = value;
		}
	}

#pragma omp for
	for ( j = i; j<(int)vectorSize; j+=8) {
		targetVector[j  ] = value;
		targetVector[j+1] = value;
		targetVector[j+2] = value;
		targetVector[j+3] = value;
		targetVector[j+4] = value;
		targetVector[j+5] = value;
		targetVector[j+6] = value;
		targetVector[j+7] = value;
	}
}


void vector_fill( IBKMK_CONST unsigned int vectorSize, double * targetVector, IBKMK_CONST double value){

	int i=0;
	int j;

#pragma omp single copyprivate(i)
	{
		for (; i<(int)vectorSize % 8; ++i) {
			targetVector[i] = value;
		}
	}

#pragma omp for
	for ( j = i; j<(int)vectorSize; j+=8) {
		targetVector[j  ] = value;
		targetVector[j+1] = value;
		targetVector[j+2] = value;
		targetVector[j+3] = value;
		targetVector[j+4] = value;
		targetVector[j+5] = value;
		targetVector[j+6] = value;
		targetVector[j+7] = value;
	}
}


void vector_copy( IBKMK_CONST unsigned int n, IBKMK_CONST double * x, double * y) {

	// openmp doesn't support memcopy since it is not thread safe (static variable use).
	// for serial code, always use memcopy
#if defined(_OPENMP)

	unsigned int i=0;
	int j;
#pragma omp single copyprivate(i)
	{
		for (; i<n % 8; ++i) {
			y[i] = x[i];
		}
	}

#pragma omp for
	for ( j = i; j<(int)n; j+=8) {
		y[j  ] = x[j  ];
		y[j+1] = x[j+1];
		y[j+2] = x[j+2];
		y[j+3] = x[j+3];
		y[j+4] = x[j+4];
		y[j+5] = x[j+5];
		y[j+6] = x[j+6];
		y[j+7] = x[j+7];
	}

#else

	/// \todo Think about checking against x == y --> might indicate programming error in
	/// calling code.
	// nobody beats memcopy as long he uses compiler intrinsics
	memcpy(y, x, n*sizeof(double) );

#endif // defined(_OPENMP)
}


void vector_scale(IBKMK_CONST unsigned int n, double a, IBKMK_CONST double * x, double * y) {

	unsigned int i=0;
	int j;

	/* align data */
#pragma omp single copyprivate(i)
{
	for (; i<n % 8; ++i) {
		y[i] = a*x[i];
	}
}

	/* use loop unrolling for 8 bytes */
#pragma omp for
	for ( j=i; j<(int)n; j+=8) {
		y[j  ] = a*x[j  ];
		y[j+1] = a*x[j+1];
		y[j+2] = a*x[j+2];
		y[j+3] = a*x[j+3];
		y[j+4] = a*x[j+4];
		y[j+5] = a*x[j+5];
		y[j+6] = a*x[j+6];
		y[j+7] = a*x[j+7];
	}

}


void vector_scale_by(IBKMK_CONST unsigned int n, double a, double * x) {

	unsigned int i=0;
	int j;

	/* align data */
#pragma omp single copyprivate(i)
{
	for (; i<n % 8; ++i) {
		x[i] *= a;
	}
}

	/* use loop unrolling for 8 bytes */
#pragma omp for
	for (j=i; j<(int)n; j+=8) {
		x[j  ] *= a;
		x[j+1] *= a;
		x[j+2] *= a;
		x[j+3] *= a;
		x[j+4] *= a;
		x[j+5] *= a;
		x[j+6] *= a;
		x[j+7] *= a;
	}

}


void vector_add(IBKMK_CONST unsigned int n, double a, IBKMK_CONST double * x, double * y) {

	unsigned int i=0;
	int j;

	if (a == IBKMK_ONE) {

		/* align data */
#pragma omp single copyprivate(i)
	{
		for (; i<n % 8; ++i) {
			y[i] += x[i];
		}
	}

		/* use loop unrolling for 8 bytes */
#pragma omp for
		for ( j=i; j<(int)n; j+=8) {
			y[j  ] += x[j  ];
			y[j+1] += x[j+1];
			y[j+2] += x[j+2];
			y[j+3] += x[j+3];
			y[j+4] += x[j+4];
			y[j+5] += x[j+5];
			y[j+6] += x[j+6];
			y[j+7] += x[j+7];
		}

	}
	else {

		/* align data */
#pragma omp single copyprivate(i)
	{
		for (; i<n % 8; ++i) {
			y[i] += a*x[i];
		}
	}

		/* use loop unrolling for 8 bytes */
#pragma omp for
		for ( j=i; j<(int)n; j+=8) {
			y[j  ] += a*x[j  ];
			y[j+1] += a*x[j+1];
			y[j+2] += a*x[j+2];
			y[j+3] += a*x[j+3];
			y[j+4] += a*x[j+4];
			y[j+5] += a*x[j+5];
			y[j+6] += a*x[j+6];
			y[j+7] += a*x[j+7];
		}

	}
}


void vector_sub(IBKMK_CONST unsigned int n, IBKMK_CONST double * x, double * y) {

	unsigned int i=0;
	int j;

	/* align data */
#pragma omp single copyprivate(i)
	{
		for (; i<n % 8; ++i) {
			y[i] -= x[i];
		}
	}

	/* use loop unrolling for 8 bytes */
#pragma omp for
	for (j=i; j<(int)n; j+=8) {
		y[j  ] -= x[j  ];
		y[j+1] -= x[j+1];
		y[j+2] -= x[j+2];
		y[j+3] -= x[j+3];
		y[j+4] -= x[j+4];
		y[j+5] -= x[j+5];
		y[j+6] -= x[j+6];
		y[j+7] -= x[j+7];
	}

}


void vector_linear_sum(IBKMK_CONST unsigned int n, double a, IBKMK_CONST double * x, double b, IBKMK_CONST double * y, double * z)  {

	unsigned int i=0;
	int j;

#pragma omp single copyprivate(i)
	{
		/* align data */
		for (; i<n % 8; ++i) {
			z[i] = a*x[i] + b*y[i];
		}
	}

	/* use loop unrolling for 8 bytes */
#pragma omp for
	for ( j=i; j<(int)n; j+=8) {
		z[j  ] = a*x[j  ] + b*y[j  ];
		z[j+1] = a*x[j+1] + b*y[j+1];
		z[j+2] = a*x[j+2] + b*y[j+2];
		z[j+3] = a*x[j+3] + b*y[j+3];
		z[j+4] = a*x[j+4] + b*y[j+4];
		z[j+5] = a*x[j+5] + b*y[j+5];
		z[j+6] = a*x[j+6] + b*y[j+6];
		z[j+7] = a*x[j+7] + b*y[j+7];
	}

}


#ifdef __cplusplus
} // namespace IBKMK
#endif
