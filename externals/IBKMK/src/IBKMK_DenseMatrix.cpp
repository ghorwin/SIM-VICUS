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

#include "IBKMK_DenseMatrix.h"

#include <IBK_StringUtils.h>
#include <IBK_Exception.h>
#include <IBK_InputOutput.h>

#include "IBKMK_common_defines.h"
#include "IBKMKC_dense_matrix.h"

namespace IBKMK {

void DenseMatrix::resize(unsigned int n) {
	m_n = n;
	m_data.resize(m_n*m_n);
	m_pivots.resize(n);
}

void DenseMatrix::swap(DenseMatrix & mat) {
	std::swap(m_n, mat.m_n);
	m_data.swap(mat.m_data);    // this is faster then normal swapping!
	m_pivots.swap(mat.m_pivots);
}

int DenseMatrix::lu() {
	return ibkmk_dense_LU_pivot(m_n, &m_data[0], &m_pivots[0]);
}


void DenseMatrix::backsolve(double * b) const {
	ibkmk_dense_backsolve_pivot(m_n, &m_data[0], &m_pivots[0], b);
}


void DenseMatrix::multiply(const double * b, double * res) const {
	ibkmk_dense_vec_mult(m_n, &m_data[0], b, res);
}


void DenseMatrix::write(std::ostream & out, double * b, bool eulerFormat, unsigned int width,
						const char * const matrixLabel, const char * const vectorLabel) const
{
	// re-use generic implementation
	IBK::write_matrix(out, *this, b, eulerFormat, width, matrixLabel, vectorLabel);
}


std::size_t DenseMatrix::serializationSize() const {
	// the actual data consists of:
	// - matrix identifier (char)
	// - 1 size variable (n),
	// - data storage, and
	// - pivot array
	size_t s = sizeof(char) + sizeof(uint32_t)
			+ 2*sizeof(uint32_t)
			+ m_data.size()*sizeof(double) + m_n*sizeof(long int);
	return s;
}


void DenseMatrix::serialize(void* & dataPtr) const {
	// store type
	*(char*)dataPtr = (char)MT_DenseMatrix;
	dataPtr = (char*)dataPtr + sizeof(char);

	*(uint32_t*)dataPtr = (uint32_t)m_n;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	IBK::serialize_vector(dataPtr, m_data);
	IBK::serialize_vector(dataPtr, m_pivots);
}


void DenseMatrix::deserialize(void* & dataPtr) {
	FUNCID(DenseMatrix::deserialize);
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_DenseMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	if (n != m_n)
		throw IBK::Exception("Invalid matrix dimensions in binary data storage (target matrix not properly resized?).", FUNC_ID);

	IBK::deserialize_vector(dataPtr, m_data);
	IBK::deserialize_vector(dataPtr, m_pivots);
}


void DenseMatrix::recreate(void* & dataPtr) {
	FUNCID(DenseMatrix::recreate);
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_DenseMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	m_n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	IBK::recreate_vector(dataPtr, m_data);
	IBK::recreate_vector(dataPtr, m_pivots);

	if (m_data.size() != m_n*m_n || m_pivots.size() != m_n)
		throw IBK::Exception("Inconsistent binary matrix data.", FUNC_ID);
}


} // namespace IBKMK

