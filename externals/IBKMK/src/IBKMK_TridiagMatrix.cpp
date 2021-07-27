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

#include "IBKMK_TridiagMatrix.h"

#include <iomanip>
#include <IBK_InputOutput.h>

#include "IBKMK_common_defines.h"

#include "IBKMKC_tridiag_matrix.h"

namespace IBKMK {

void TridiagMatrix::solveEquationSystem(double * b) {
	double * L = &m_data[0];				// L holds the lower band (L[0] not used)
	double * M = L + m_n;					// M holds main diagonal band
	double * U = M + m_n;					// U holds the upper band (U[m_n-1] not used)
	for (unsigned int i=1; i<m_n; ++i) {
		double fact = L[i]/M[i-1];
		M[i] -= fact*U[i-1];
		b[i] -= fact*b[i-1];
	}
	b[m_n-1] /= M[m_n-1];
	for (int i=m_n-2; i>=0; --i) {
		b[i] -= U[i]*b[i+1];
		b[i] /= M[i];
	}
}

int TridiagMatrix::solveEquationSystemChecked(double * b) {
	double * L = &m_data[0];				// L holds the lower band (L[0] not used)
	double * M = L + m_n;					// M holds main diagonal band
	double * U = M + m_n;					// U holds the upper band (U[m_n-1] not used)
	for (unsigned int i=1; i<m_n; ++i) {
		if (M[i-1]==0.0)		return 1;	// Zero element in main diagonal
		double fact = L[i]/M[i-1];
		M[i] -= fact*U[i-1];
		if (M[i]==0.0)			return 2;	// Matrix is singular or becomes singular by transformation
		b[i] -= fact*b[i-1];
	}
	b[m_n-1] /= M[m_n-1];
	for (int i=m_n-2; i>=0; --i) {
		b[i] -= U[i]*b[i+1];
		b[i] /= M[i];
	}
	return 0; // all ok
}


int TridiagMatrix::lu() {
	int result = ibkmk_tridiag_LU( m_n, &m_data[0]);
	return result;
}


void TridiagMatrix::backsolve(double * b) const {
	ibkmk_tridiag_backsolve( m_n, &m_data[0], b);
}


void TridiagMatrix::multiply(const double * b, double * r) const {
	ibkmk_tridiag_vec_mult(m_n, &m_data[0], b, r);
}


void TridiagMatrix::write(std::ostream & out, double * b, bool eulerFormat, unsigned int width,
						  const char * const matrixLabel, const char * const vectorLabel) const
{
	// re-use generic implementation
	IBK::write_matrix(out, *this, b, eulerFormat, width, matrixLabel, vectorLabel);
}


std::size_t TridiagMatrix::serializationSize() const {
	// the actual data consists of:
	// - matrix identifier (char)
	// - 1 size variable (n),
	// - data storage
	size_t s = sizeof(char) + sizeof(uint32_t) + m_data.size()*sizeof(double);
	return s;
}


void TridiagMatrix::serialize(void* & dataPtr) const {
	// store type
	*(char*)dataPtr = (char)MT_TridiagMatrix;
	dataPtr = (char*)dataPtr + sizeof(char);

	*(uint32_t*)dataPtr = (uint32_t)m_n;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	size_t dataSize = m_data.size()*sizeof(double);
	std::memcpy(dataPtr, &m_data[0], dataSize);
	dataPtr = (char*)dataPtr + dataSize;
}


void TridiagMatrix::deserialize(void* & dataPtr) {
	const char * const FUNC_ID = "[TridiagMatrix::deserialize]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_TridiagMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	if (n != m_n)
		throw IBK::Exception("Invalid matrix dimensions in binary data storage (target matrix not properly resized?).", FUNC_ID);

	size_t dataSize = m_data.size()*sizeof(double);
	std::memcpy(&m_data[0], dataPtr, dataSize);
	dataPtr = (char*)dataPtr + dataSize;
}


void TridiagMatrix::recreate(void* & dataPtr) {
	const char * const FUNC_ID = "[TridiagMatrix::recreate]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_TridiagMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	resize(n);

	size_t dataSize = m_data.size()*sizeof(double);
	std::memcpy(&m_data[0], dataPtr, dataSize);
	dataPtr = (char*)dataPtr + dataSize;
}

} // namespace IBKMK

