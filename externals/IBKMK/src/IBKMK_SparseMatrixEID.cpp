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

#include <vector>
#include <cstring>
#include <fstream>
#include <algorithm>

#include "IBKMK_SparseMatrixEID.h"
#include "IBKMKC_sparse_matrix.h"

// k - column in index vector (_not_ in original matrix)
// index - Pointer to index vector
// offset - offset to first element in current row in index vector
#define INVALID_INDEX(k,index,offset)  ((k) > 0 && (index)[(offset) + (k)] == (index)[(offset) + (k) - 1])

namespace IBKMK {

void SparseMatrixEID::addIdentityMatrix() {
	unsigned int kidx = 0;
	double * data_ptr = &m_data[0];
	for (unsigned int i=0; i<m_n; ++i, kidx += m_elementsPerRow) {
		for (unsigned int j=0; j<m_elementsPerRow; ++j) {
			if (m_index[kidx + j] == i) {
				data_ptr[kidx + j] += 1.0;
				break;
			}
		}
	}
}


void SparseMatrixEID::resize(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices) {
	const char * const FUNC_ID = "[SparseMatrixEID::resize]";
	if (n <= 0 || elementsPerRow <= 0)
		throw IBK::Exception("Invalid input in n or elementsPerRow.", FUNC_ID);
	m_n = n;
	m_elementsPerRow = elementsPerRow;
	m_data.resize(m_n*m_elementsPerRow);
	m_index.resize(m_n*m_elementsPerRow);
	// populate the index vector if indices have been passed
	if (indices != NULL) {
		std::memcpy((void*)&m_index[0], (const void*)indices, sizeof(unsigned int)*m_n*m_elementsPerRow);
		// check for valid index vector
		for (unsigned int i=0; i<n; ++i) {
			for (unsigned int j=0; j<m_elementsPerRow; ++j) {
				// check for invalid indices
				if (m_index[j] == (unsigned int)-1) {
					throw IBK::Exception("Invalid index array, use of -1 is not allowed (anylonger).", FUNC_ID);
				}
				// check that index values increase monotically
				if (j>0) {
					if (m_index[j] < m_index[j-1])
						throw IBK::Exception("Invalid index array, column indices must increase monotonically within each matrix row.", FUNC_ID);
					if (m_index[j] == m_index[j-1]) {
						// found first invalid index, check that all subsequent indices are the same
						for (;j<m_elementsPerRow; ++j) {
							if (m_index[j] != m_index[j-1])
								throw IBK::Exception("Invalid index array, once an invalid (double) column matrix has found, all following column indices must be the same.", FUNC_ID);
						}
					}
				}
			}
		}
	}
	else {
		for (unsigned int i=0; i<m_n*m_elementsPerRow; ++i)
			m_index[i] = 0;
	}
}


double & SparseMatrixEID::operator()(unsigned int i, unsigned int j) {
	const char * const FUNC_ID = "[SparseMatrixEID::setValue]";
	// search all index values in row i until we find the column j
	IBK_ASSERT(i < m_n);
	IBK_ASSERT(j < m_n);
	// offset is first element of row i
	unsigned int offset = i*m_elementsPerRow;
	// loop over all elements of row i, if unused element is found break the loop
	for (unsigned int k=0; k<m_elementsPerRow; ++k) {
		// check for unused element, i.e. same index as previous
		//		if (INVALID_INDEX(k,m_index,offset))
		if ( (k > 0) && (m_index[offset + k] == m_index[offset + k - 1]) )
			throw IBK::Exception( IBK::FormatString("Index out of range. No element at (%1,%2) in sparsity pattern.").arg(i).arg(j), FUNC_ID);
		// check if current element is selected element
		if (m_index[offset+k] == j) {
			return m_data[offset+k];
		}
	}
	throw IBK::Exception( IBK::FormatString("Index out of range. No element at (%1,%2) in sparsity pattern.").arg(i).arg(j), FUNC_ID);
}


double SparseMatrixEID::value(unsigned int i, unsigned int j) const {
	// search all index values in row i until we find the column j
	IBK_ASSERT(i < m_n);
	IBK_ASSERT(j < m_n);
	// offset is first element of row i
	unsigned int offset = i*m_elementsPerRow;
	// loop over all elements of row i, if unused element is found break the loop
	for (unsigned int k=0; k<m_elementsPerRow; ++k) {
		// check for unused element, i.e. same index as previous
		//		if (INVALID_INDEX(k,m_index,offset))
		if ( (k > 0) && (m_index[offset + k] == m_index[offset + k - 1]) )
			return 0;			// return 0
		// check if current element is selected element
		if (m_index[offset+k] == j) {
			return m_data[offset+k];
		}
	}
	return 0;			// column not found in index row, return 0
}


void SparseMatrixEID::multiply(const double * b, double * res) const {
	ibkmk_spmat_eid_multiply( m_n, m_elementsPerRow, &m_index[0], &m_data[0], &b[0], &res[0]  );
}


unsigned int SparseMatrixEID::storageIndex(unsigned int i, unsigned int j) const {
	// search all index values in row i until we find the column j
	IBK_ASSERT(i < m_n);
	IBK_ASSERT(j < m_n);
	// k1 is first element of row i
	unsigned int k1 = i*m_elementsPerRow;
	// k2 is first element of row i+1
	unsigned int k2 = k1 + m_elementsPerRow;
	// loop over all elements of row i
	while (k1 < k2) {
		if (m_index[k1] == j)
			return k1;
		++k1;
	}
	return m_n*m_elementsPerRow;			// column not found in index row i, return max value (means invalid)
}

void SparseMatrixEID::ilu() {
	ibkmk_spmat_eid_ilu(m_n, m_elementsPerRow, &m_index[0], &m_data[0]);
}


void SparseMatrixEID::backsolveILU(double * b) const {
	ibkmk_spmat_eid_backsolve(m_n, m_elementsPerRow, &m_index[0], &m_data[0], b);
}


std::size_t SparseMatrixEID::serializationSize() const {
	// the actual data consists of:
	// - matrix identifier (char)
	// - 2 size variables (n, elementsPerRow),
	// - data storage, and
	// - index array
	size_t s = sizeof(char) + 2*sizeof(uint32_t) + m_data.size()*sizeof(double) + m_index.size()*sizeof(unsigned int);
	return s;
}


void SparseMatrixEID::serialize(void* & dataPtr) const {
	// Memory layout
	//
	// 1 char = 4
	// 1 uint32_t = 4 bytes = n
	// 1 uint32_t = 4 bytes = elementsPerRow
	// binary vector data
	// binary vector index

	*(char*)dataPtr = (char)MT_SparseMatrixEID;
	dataPtr = (char*)dataPtr + sizeof(char);

	*(uint32_t*)dataPtr = (uint32_t)m_n;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	*(uint32_t*)dataPtr = (uint32_t)m_elementsPerRow;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	IBK::serialize_vector(dataPtr, m_data);
	IBK::serialize_vector(dataPtr, m_index);
}


void SparseMatrixEID::deserialize(void* & dataPtr) {
	const char * const FUNC_ID = "[SparseMatrixEID::deserialize]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_SparseMatrixEID)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int  elementsPerRow = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	if (n != m_n || elementsPerRow != m_elementsPerRow)
		throw IBK::Exception("Invalid matrix dimensions in binary data storage (target matrix not properly resized?).", FUNC_ID);

	IBK::deserialize_vector(dataPtr, m_data);
	IBK::deserialize_vector(dataPtr, m_index);
}


void SparseMatrixEID::recreate(void* & dataPtr) {
	const char * const FUNC_ID = "[SparseMatrixEID::recreate]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_SparseMatrixEID)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	m_n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	m_elementsPerRow = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	IBK::recreate_vector(dataPtr, m_data);
	IBK::recreate_vector(dataPtr, m_index);

	if (m_data.size() != m_n*m_elementsPerRow || m_index.size() != m_n*m_elementsPerRow)
		throw IBK::Exception("Inconsistent binary matrix data.", FUNC_ID);
}



} // namespace IBKMK

