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

#ifndef IBKMKC_vector_operationsH
#define IBKMKC_vector_operationsH


#include "IBKMK_common_defines.h"

#ifdef __cplusplus  /* wrapper to enable C++ usage */

namespace IBKMK {

extern "C" {

#endif /* __cplusplus */

/*! \file IBKMKC_vector_operations.c
	\brief Routines for vector operations.
*/

/*! Fills an unsigned int vector y = alpha */
void vectorUInt_fill( IBKMK_CONST unsigned int vectorSize, unsigned int * targetVector, IBKMK_CONST unsigned int value);


/*! Fills an vector y = alpha */
void vector_fill( IBKMK_CONST unsigned int vectorSize, double * targetVector, IBKMK_CONST double value);


/*! Operation: y = x. */
void vector_copy(IBKMK_CONST unsigned int n, IBKMK_CONST double * x, double * y);

/*! Operation: y = a*x.
	\param n Size of vectors
	\param a Factor to muplity vector source with.
	\param x Pointer to read-only memory of size n doubles
	\param y Pointer to write-only memory of size n doubles
*/
void vector_scale(IBKMK_CONST unsigned int n, double a, IBKMK_CONST double * x, double * y);

/*! Operation: x *= a.
	\param n Size of vectors
	\param a Factor to muplity vector source with.
	\param x Pointer to read/write memory of size n doubles
*/
void vector_scale_by(IBKMK_CONST unsigned int n, double a, double * x);

/*! Operation: y = a*x.
	Note: for a == IBKMK_ONE a special implementation for y += x is used.
	\param n Size of vectors
	\param a Scale factor for vector x.
	\param x Pointer to read-only memory of size n doubles
	\param y Pointer to read/write memory of size n doubles
*/
void vector_add(IBKMK_CONST unsigned int n, double a, IBKMK_CONST double * x, double * y);

/*! Operation: y -= x.
	\param n Size of vectors
	\param x Pointer to read-only memory of size n doubles
	\param y Pointer to read/write memory of size n doubles
*/
void vector_sub(IBKMK_CONST unsigned int n, IBKMK_CONST double * x, double * y);


/*! Operation: z = a*x + b*y, with special implementations for cases when a or b are equal to IBKMK
	\code
	// effectively for all n vector elements
	z[i] = a*x[i] + b*y[i];
	\endcode
	\param n Size of vectors
	\param a Factor to multiply vector x with.
	\param b Factor to multiply vector y with.
	\param x Pointer to read-only memory of size n doubles
	\param y Pointer to read-only memory of size n doubles
	\param z Pointer to write-only memory of size n doubles
*/
void vector_linear_sum(IBKMK_CONST unsigned int n, double a, IBKMK_CONST double * x, double b, IBKMK_CONST double * y, double * z);

#ifdef __cplusplus  /* wrapper to enable C++ usage */
} // namespace IBKMKC

} // extern "C"
#endif /* __cplusplus */

#endif // IBKMKC_vector_operationsH
