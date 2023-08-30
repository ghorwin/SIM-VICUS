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
#include <functional>

#include "IBKMK_SparseMatrixCSR.h"
#include "IBKMKC_sparse_matrix.h"
#include <IBK_InputOutput.h>

namespace IBKMK {

#define INVALID_INDEX(k,index,offset)  ((k) > 0 && (index)[(offset) + (k)] == (index)[(offset) + (k) - 1])

void SparseMatrixCSR::addIdentityMatrix() {
	// add identity matrix
	const unsigned int * ia = constIa();
	const unsigned int * ja = constJa();
	double * data_ptr = &m_data[0];
	for (unsigned int i=0; i<m_n; ++i) {
		for (unsigned int jidx = ia[i]; jidx < ia[i+1]; ++jidx) {
			unsigned int j = ja[jidx];
			if (j == i) {
				data_ptr[jidx] += 1.0;
				break;
			}
		}
	}
}


void SparseMatrixCSR::generateTransposedIndex(const std::vector<unsigned int> &ia, const std::vector<unsigned int> &ja,
	std::vector<unsigned int> &iaT, std::vector<unsigned int> &jaT)
{
	// compose the transposed matrix pattern
	iaT.clear();
	iaT.resize(ia.size());
	jaT.resize(ja.size(), std::numeric_limits<unsigned int>::infinity());
	// copy and transpose indices: start with iaT
	for(unsigned int i = 0; i < ia.size() - 1; ++i) {
		for(unsigned int jIdx = ia[i]; jIdx < ia[i + 1]; ++jIdx) {
			// get column index
			IBK_ASSERT(jIdx < ja.size());
			unsigned int j = ja[jIdx];
			// update offset
			IBK_ASSERT(j < iaT.size() - 1);
			++iaT[j + 1];
		}
	}
	// update iaT
	for(unsigned int j = 0; j < iaT.size() - 1; ++j)
	{
		iaT[j + 1] += iaT[j];
	}

	for(unsigned int i = 0; i < ia.size() - 1; ++i) {
		for(unsigned int jIdx = ia[i]; jIdx < ia[i + 1]; ++jIdx) {
			// get column index
			IBK_ASSERT(jIdx < ja.size());
			unsigned int j = ja[jIdx];
			// store at the next free position of the corresponding column
			IBK_ASSERT(j < iaT.size() - 1);
			for(unsigned int k = iaT[j]; k < iaT[j + 1]; ++k) {
				if(jaT[k] == std::numeric_limits<unsigned int>::infinity()) {
					jaT[k] = i;
					break;
				}
			}
		}
	}
	// sort entries inside jaT
	for(unsigned int i = 0; i < iaT.size() - 1; ++i) {
		std::sort(jaT.begin() + iaT[i], jaT.begin() + iaT[i + 1]);
	}
}


void SparseMatrixCSR::resize(unsigned int n, unsigned int nnz, const unsigned int * ia, const unsigned int * ja,
		const unsigned int * iaT, const unsigned int * jaT)
{
	FUNCID(SparseMatrixCSR::resize);
	if (n <= 0 || nnz <= 0)
		throw IBK::Exception("Invalid input in n or nnz.", FUNC_ID);
	m_n = n;
	m_nnz = nnz;
	m_data.resize(m_nnz);
	m_ia.resize(m_n+1);
	m_ja.resize(m_nnz);
	std::memcpy((void*)&m_ia[0], (const void*)ia, sizeof(unsigned int)*(m_n+1));
	std::memcpy((void*)&m_ja[0], (const void*)ja, sizeof(unsigned int)*m_nnz);

	// empty tranpose indices mean symmetric matrix pattern
	if (iaT == nullptr || jaT == nullptr) {
		m_iaT.clear();
		m_jaT.clear();
		m_patternMode = PM_Symmetric;
	}
	// to be treated as asymmetric
	else {
		m_iaT.resize(m_n+1);
		m_jaT.resize(m_nnz);
		std::memcpy((void*)&m_iaT[0], (const void*)iaT, sizeof(unsigned int)*(m_n+1));
		std::memcpy((void*)&m_jaT[0], (const void*)jaT, sizeof(unsigned int)*m_nnz);
		m_patternMode = PM_Asymmetric;
	}
}


void SparseMatrixCSR::resizeFromEID(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices) {
	FUNCID(SparseMatrixCSR::resizeFromEID);
	if (n == 0)
		throw IBK::Exception("Invalid input in n.", FUNC_ID);
	if (elementsPerRow == 0)
		throw IBK::Exception("Invalid input in elementsPerRow.", FUNC_ID);
	if (indices == nullptr)
		throw IBK::Exception("Invalid input in indices.", FUNC_ID);

	m_n = n;
	m_ia.clear();
	m_ia.resize(m_n+1);
	m_ja.clear();
	// eid format is always symmetric
	m_patternMode = PM_Symmetric;
	// fill ja and ia
	for (unsigned int i = 0; i < n; ++i) {
		unsigned int nValidIndices = 0;
		for (unsigned int j = 0; j < elementsPerRow; ++j) {
			// skip invalid indices
			if (INVALID_INDEX(j ,indices, i * elementsPerRow) )
				continue;
			// store column index
			m_ja.push_back(indices[i * elementsPerRow + j]);
			++nValidIndices;
		}
		// store row offset
		std::transform(m_ia.begin() + i + 1, m_ia.end(), m_ia.begin() + i + 1,
			std::bind(std::plus<unsigned int>(), nValidIndices, std::placeholders::_1));
	}
	// set number of nonzero elements
	m_nnz = (unsigned int)m_ja.size();
	// resize data vector
	m_data.resize(m_nnz);
}

void SparseMatrixCSR::parseFromEID(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices, const double *data) {
	FUNCID(SparseMatrixCSR::parseFromEID);
	if (n != m_n || n * elementsPerRow < m_nnz)
		throw IBK::Exception("Wrong matrix size! Call 'resizeFromEID' before parsing!", FUNC_ID);
	if (elementsPerRow == 0)
		throw IBK::Exception("Invalid input in elementsPerRow.", FUNC_ID);
	if (indices == nullptr)
		throw IBK::Exception("Invalid input in indices.", FUNC_ID);
	if (data == nullptr)
		throw IBK::Exception("Invalid input in data.", FUNC_ID);

	// now fill data
	unsigned int colIdx = 0;
	for (unsigned int i = 0; i < n; ++i) {
		for (unsigned int j = 0; j < elementsPerRow; ++j) {
			// skip invalid indices
			if (INVALID_INDEX(j ,indices, i * elementsPerRow) )
				continue;
			// error: wrong matrix size
			if(colIdx == m_nnz) {
				throw IBK::Exception("Wrong matrix size! Call 'resizeFromEID' before parsing!", FUNC_ID);
			}
			// fill data
			m_data[colIdx] = data[i * elementsPerRow + j];
			++colIdx;
		}
	}
	// matrix is too small
	if(colIdx < m_nnz ){
		throw IBK::Exception("Wrong matrix size! Call 'resizeFromEID' before parsing!", FUNC_ID);
	}
}


double & SparseMatrixCSR::operator()(unsigned int i, unsigned int j) {
	FUNCID(SparseMatrixCSR::operator());
	// search all index values in row i until we find the column j
	IBK_ASSERT(i < m_n);
	IBK_ASSERT(j < m_n);
	// search for index position in row i
	for (unsigned int k=m_ia[i]; k < m_ia[i+1]; ++k) {
		if (m_ja[k] == j) {
			return m_data[k];
		}
	}
	throw IBK::Exception( IBK::FormatString("Index out of range. No element at (%1,%2) in sparsity pattern.").arg(i).arg(j), FUNC_ID);
}


double SparseMatrixCSR::value(unsigned int i, unsigned int j) const {
	// search all index values in row i until we find the column j
	IBK_ASSERT(i < m_n);
	IBK_ASSERT(j < m_n);
	// search for index position in row i
	for (unsigned int k=m_ia[i]; k < m_ia[i+1]; ++k) {
		if (m_ja[k] == j) {
			return m_data[k];
		}
	}
	return 0;			// column not found in index row, return 0
}


void SparseMatrixCSR::multiply(const double * b, double * res) const {
	IBKMK::ibkmk_spmat_csr_multiply( m_n, &m_data[0], &m_ia[0], &m_ja[0], &b[0], &res[0]  );
}



unsigned int SparseMatrixCSR::storageIndex(unsigned int i, unsigned int j) const {
	// search all index values in row i until we find the column j
	IBK_ASSERT(i < m_n);
	IBK_ASSERT(j < m_n);
	// k1 is first element of row i
	unsigned int k1 = m_ia[i];
	// k2 is first element of row i+1
	unsigned int k2 = m_ia[i + 1];
	// loop over all elements of row i
	while (k1 < k2) {
		if (m_ja[k1] == j)
			return k1;
		++k1;
	}
	return m_nnz;			// column not found in index row i, return max value (means invalid)
}

void SparseMatrixCSR::ilu() {
	if(m_patternMode == PM_Symmetric) {
		ibkmk_spmat_csr_ilu_symm(m_n, &m_ia[0], &m_ja[0], &m_data[0]);
	}
	else {
		ibkmk_spmat_csr_ilu_asymm(m_n, &m_ia[0], &m_ja[0], &m_iaT[0], &m_jaT[0], &m_data[0]);
	}
}


void SparseMatrixCSR::backsolveILU(double * b) const {
	ibkmk_spmat_csr_backsolve(m_n, &m_ia[0], &m_ja[0], &m_data[0], b);
}


std::size_t SparseMatrixCSR::serializationSize() const {
	// the actual data consists of:
	// - matrix identifier (char)
	// - 2 size variables (n, nnz),
	// - data storage vector
	// - i and j index vectors
	// - iT and jT transposed index vectors ( only if matrix pattern is asymmetric)
	//
	// each vector is serialized as 4-byte integer with size followed by actual data
	size_t s = sizeof(char) + 2*sizeof(uint32_t)
			+ sizeof(uint32_t) + m_data.size()*sizeof(double)
			+ 4*sizeof(uint32_t) // each of the index vector data blocks is preceeded by the size
			+ m_ia.size()*sizeof(unsigned int) + m_ja.size()*sizeof(unsigned int)
			+ m_iaT.size()*sizeof(unsigned int) + m_jaT.size()*sizeof(unsigned int);
	return s;
}


void SparseMatrixCSR::serialize(void* & dataPtr) const {
	// Memory layout
	//
	// 1 char = 5
	// 1 uint32_t = 4 bytes = n
	// 1 uint32_t = 4 bytes = nnz
	// binary vector data
	// binary vector ia
	// binary vector ja
	// binary vector iaT
	// binary vector jaT

	*(char*)dataPtr = (char)MT_SparseMatrixCSR;
	dataPtr = (char*)dataPtr + sizeof(char);

	*(uint32_t*)dataPtr = (uint32_t)m_n;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	*(uint32_t*)dataPtr = (uint32_t)m_nnz;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	IBK::serialize_vector(dataPtr, m_data);
	IBK::serialize_vector(dataPtr, m_ia);
	IBK::serialize_vector(dataPtr, m_ja);
	IBK::serialize_vector(dataPtr, m_iaT);
	IBK::serialize_vector(dataPtr, m_jaT);
}


void SparseMatrixCSR::deserialize(void* & dataPtr) {
	FUNCID(SparseMatrixCSR::deserialize);
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_SparseMatrixCSR)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int  nnz = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	if (n != m_n || nnz != m_nnz)
		throw IBK::Exception("Invalid matrix dimensions in binary data storage (target matrix not properly resized?).", FUNC_ID);

	// Mind: these functions assume correct memory sizes and throw exceptions if mismatching
	IBK::deserialize_vector(dataPtr, m_data);
	IBK::deserialize_vector(dataPtr, m_ia);
	IBK::deserialize_vector(dataPtr, m_ja);
	IBK::deserialize_vector(dataPtr, m_iaT);
	IBK::deserialize_vector(dataPtr, m_jaT);
}


void SparseMatrixCSR::recreate(void* & dataPtr) {
	FUNCID(SparseMatrixCSR::recreate);
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_SparseMatrixCSR)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	m_n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	m_nnz = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	IBK::recreate_vector(dataPtr, m_data);
	IBK::recreate_vector(dataPtr, m_ia);
	IBK::recreate_vector(dataPtr, m_ja);
	IBK::recreate_vector(dataPtr, m_iaT);
	IBK::recreate_vector(dataPtr, m_jaT);

	if (m_data.size() != m_nnz || m_ia.size() != m_n+1 || m_ja.size() != m_nnz)
		throw IBK::Exception("Inconsistent binary matrix data.", FUNC_ID);
}



} // namespace IBKMK

