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

#ifndef IBKMKC_band_matrixH
#define IBKMKC_band_matrixH

#include "IBKMK_common_defines.h"

#ifdef __cplusplus  /* wrapper to enable C++ usage */

namespace IBKMK {

extern "C" {

#endif

/*! Generic matrix vector multiply r = A*b.
	\param n Matrix block-dimension.
	\param m Block-dimension.
	\param ml Block-lower diagonals.
	\param mu Block-upper diagonals.
	\param A Matrix data, size n*m*m*(ml+mu+1), see description of matrix format.
	\param b Vector, size n*m.
	\param r Result vector, size n*m.
*/
void ibkmk_blockband_multiply(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu, double * A, double * b, double * r);


/*! Matrix vector multiply r = A*b for block-dimensions 2x2.
	\param n Matrix block-dimension.
	\param ml Block-lower diagonals.
	\param mu Block-upper diagonals.
	\param A Matrix data, size n*m*m*(ml+mu+1), see description of matrix format.
	\param b Vector, size n*m.
	\param r Result vector, size n*m.
*/
void ibkmk_blockband_multiply2(unsigned int n, unsigned int ml, unsigned int mu, double * A, double * b, double * r);


/*! In-place LU factorization without pivoting.
	\param n Matrix block-dimension.
	\param m Block-dimension.
	\param ml Block-lower diagonals.
	\param mu Block-upper diagonals.
	\param A Matrix data, size n*m*m*(ml+mu+1), see description of matrix format.
*/
int ibkmk_blockband_lu(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu, double * A);


/*! In-place LU factorization without pivoting.
	\param n Matrix block-dimension.
	\param m Block-dimension.
	\param ml Block-lower diagonals.
	\param mu Block-upper diagonals.
	\param A Matrix data, size n*m*m*(ml+mu+1), see description of matrix format.
*/
void ibkmk_blockband_backsolve(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu, double * A, double * b);


#ifdef __cplusplus
} // namespace IBKMKC

} // extern "C"
#endif

/*! \file IBKMKC_band_matrix.c
	\brief Routines for block banded matrix operations.
*/

#endif // IBKMKC_band_matrixH
