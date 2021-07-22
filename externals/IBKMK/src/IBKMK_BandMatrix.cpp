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

#include "IBKMK_BandMatrix.h"
#include "IBKMK_common_defines.h"

#include <IBK_InputOutput.h>

#include <algorithm>

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
#else
	#include <stdint.h>
#endif


namespace IBKMK {

void BandMatrix::resize(unsigned int n, unsigned int mu, unsigned int ml, unsigned int smu) {
	m_n = n;
	m_mu = mu;
	m_ml = ml;
	m_smu = smu;
	m_colSize = smu + ml + 1;
	m_dataStorage.resize(n*m_colSize);
	m_data.resize(n);
	m_data[0] = &m_dataStorage[0];
	// store pointers to columns
	for (unsigned int i=1; i<n; ++i)
		m_data[i] = &m_dataStorage[0] + i*m_colSize;
	m_pivots.resize(n);
}


int BandMatrix::lu() {
	const char * const FUNC_ID = "[BandMatrix::lu]";
	if (m_n == 0)
		throw IBK::Exception("Matrix is empty.", FUNC_ID);
	unsigned int n_1 = static_cast<unsigned int>(m_n) - 1;
	if (m_pivots.size() != m_n)
		throw IBK::Exception(IBK::FormatString("Mismatching sizes of matrix (n = %1) and pivots vector (size = %2).")
							 .arg(m_n).arg((unsigned int)m_pivots.size()),
							 FUNC_ID);
	unsigned int *p = &m_pivots[0];
	double ** a = &m_data[0];

	// zero out the first smu - mu rows of the rectangular array a
	unsigned int num_rows = m_smu - m_mu;
	if (num_rows > 0) {
		for (unsigned int c=0; c<m_n; ++c) {
			double * a_c = a[c];
			for (unsigned int r=0; r<num_rows; ++r)
				a_c[r] = 0;
		}
	}

	// k = elimination step number
	for (unsigned int k=0; k<n_1; ++k, ++p) {
		double * col_k               = a[k];
		double * diag_k              = col_k + m_smu;
		double * sub_diag_k          = diag_k + 1;

		unsigned int last_row_k = std::min<unsigned int>(n_1,k+m_ml);

		// find l = pivot row number
		unsigned int l=k;
		double max = std::fabs(*diag_k);
		double * kptr = sub_diag_k;
		for (unsigned int i=k+1; i<=last_row_k; ++i, ++kptr)
			if (std::fabs(*kptr) > max) {
				l=i;
				max = std::fabs(*kptr);
			}

		unsigned int storage_l = l-k+m_smu;  // ROW(l,k,smu) : calculate row index in column
		*p = l;                             // store row of pivot element

		// check for zero pivot element
		if (col_k[storage_l] == 0) {
			return k+1;
		}

		// swap a(l,k) and a(k,k) if necessary
		bool swap = (l != k);
		if (swap)
			std::swap(col_k[storage_l], *diag_k);

		// Scale the elements below the diagonal in
		// column k by -1.0 / a(k,k). After the above swap,
		// a(k,k) holds the pivot element. This scaling
		// stores the pivot row multipliers -a(i,k)/a(k,k)
		// in a(i,k), i=k+1, ..., MIN(n-1,k+ml).

		double mult = -1.0 / (*diag_k);
		kptr = sub_diag_k;
		for (unsigned int i=k+1; i<=last_row_k; ++i, ++kptr)
			if (*kptr != 0)
				*kptr *= mult;

		// row_i = row_i - [a(i,k)/a(k,k)] row_k, i=k+1, ..., MIN(n-1,k+ml)
		// row k is the pivot row after swapping with row l.
		// The computation is done one column at a time,
		// column j=k+1, ..., MIN(k+smu,n-1).

		double * col_j;
		unsigned int last_col_k = std::min<unsigned int>(k+m_smu, n_1);
		unsigned int storage_k;
		double a_kj;
		for (unsigned int j=k+1; j<=last_col_k; ++j) {
			col_j = a[j];
			storage_l = l-j+m_smu; // ROW(l,j,smu)
			storage_k = k-j+m_smu; // ROW(k,j,smu)
			a_kj = col_j[storage_l];

			// Swap the elements a(k,j) and a(k,l) if l!=k.
			if (swap) {
				col_j[storage_l] = col_j[storage_k];
				col_j[storage_k] = a_kj;
			}

			// a(i,j) = a(i,j) - [a(i,k)/a(k,k)]*a(k,j)
			// a_kj = a(k,j), *kptr = - a(i,k)/a(k,k), *jptr = a(i,j)

			if (a_kj != 0.0) {
				kptr=sub_diag_k;
				// pointer to the element in row k+1 and column j: ROW(k+1,j,smu)
				double * jptr = col_j + k+1-j+m_smu;
				for (unsigned int i=k+1; i<=last_row_k; ++i, ++kptr, ++jptr)
					*jptr += a_kj * *kptr;
			}
		}
	}

	// set the last pivot row to be n-1 and check for a zero pivot
	*p = n_1;
	if (a[n_1][m_smu] == 0.0)
		return 2*m_n;

	return 0;
}


void BandMatrix::backsolve(double * b) const {
	// get direct access to data memory in vectors
	unsigned int n_1 = static_cast<unsigned int>(m_n)-1;
	IBK_ASSERT(m_pivots.size() == m_n);
	const unsigned int *p = &m_pivots[0];
	const double *const *a = &m_data[0];

	// Solve Ly = b, store solution y in b
	for (unsigned int k=0; k < n_1; ++k) {
		unsigned int l = p[k];
		double mult = b[l];
		if (l != k) {
			b[l] = b[k];
			b[k] = mult;
		}
		const double *diag_k = a[k]+m_smu;
		unsigned int last_row_k = std::min<unsigned int>(n_1,k+m_ml);
		for (unsigned int i=k+1; i<=last_row_k; ++i)
			b[i] += mult * diag_k[i-k];
	}

	// Solve Ux = y, store solution x in b
	for (int k=n_1; k>=0; --k) {
		const double * diag_k = a[k]+m_smu;      // pointer to the diagonal element of column/row k
		int first_row_k = std::max<int>(0, k-static_cast<int>(m_smu));
		b[k] /= (*diag_k);
		double mult = -b[k];
		for (int i=first_row_k; i<=k-1; ++i)
			b[i] += mult*diag_k[i-k];
	}
}


void BandMatrix::multiply(const double * b, double * r) const {
	// first initialize target vector with 0
	std::memset(r, 0, sizeof(double)*m_n);
	// now loop over all columns of the matrix
	for (unsigned int j=0; j<m_n; ++j) {
		// now we loop over all rows in this column and add the corresponding contribution
		// to the resultant vector
		for (unsigned int i=std::max<int>(0,j-m_mu); (int)i<=std::min<int>(m_n-1, j+m_ml); ++i) {
			// std::cout << "i,j = " << i << "," << j << std::endl;
			//std::cout << "r[" << i << "] += " << m_data[j][i - j + m_smu] << "*" << "b[" << j << "]("<< b[j]<< ")" << std::endl;
			r[i] += m_data[j][i - j + m_smu]*b[j];
		}
	}
}


void BandMatrix::write(std::ostream & out, double * b, bool eulerFormat, unsigned int width,
					   const char * const matrixLabel, const char * const vectorLabel) const
{
	// re-use generic implementation
	IBK::write_matrix(out, *this, b, eulerFormat, width, matrixLabel, vectorLabel);
}


std::size_t BandMatrix::serializationSize() const {
	// the actual data consists of:
	// - matrix identifier (char)
	// - 4 size variables (n, mu, ml, smu),
	// - data storage, and
	// - pivot array
	size_t s = sizeof(char) + 4*sizeof(uint32_t) + m_dataStorage.size()*sizeof(double) + m_n*sizeof(unsigned int);
	return s;
}


void BandMatrix::serialize(void* & dataPtr) const {
	// store type
	*(char*)dataPtr = (char)MT_BandMatrix;
	dataPtr = (char*)dataPtr + sizeof(char);

	*(uint32_t*)dataPtr = (uint32_t)m_n;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	*(uint32_t*)dataPtr = (uint32_t)m_mu;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	*(uint32_t*)dataPtr = (uint32_t)m_ml;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	*(uint32_t*)dataPtr = (uint32_t)m_smu;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	size_t dataSize = m_dataStorage.size()*sizeof(double);
	std::memcpy(dataPtr, &m_dataStorage[0], dataSize);
	dataPtr = (char*)dataPtr + dataSize;

	size_t pivotSize = m_n*sizeof(unsigned int);
	std::memcpy(dataPtr, &m_pivots[0], pivotSize);
	dataPtr = (char*)dataPtr + pivotSize;
}


void BandMatrix::deserialize(void* & dataPtr) {
	const char * const FUNC_ID = "[BandMatrix::deserialize]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_BandMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int  mu = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int ml = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int smu = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	if (n != m_n || mu != m_mu || ml != m_ml || smu != m_smu)
		throw IBK::Exception("Invalid matrix dimensions in binary data storage (target matrix not properly resized?).", FUNC_ID);

	size_t dataSize = m_dataStorage.size()*sizeof(double);
	std::memcpy(&m_dataStorage[0], dataPtr, dataSize);
	dataPtr = (char*)dataPtr + dataSize;

	size_t pivotSize = m_n*sizeof(unsigned int);
	std::memcpy(&m_pivots[0], dataPtr, pivotSize);
	dataPtr = (char*)dataPtr + pivotSize;
}


void BandMatrix::recreate(void* & dataPtr) {
	const char * const FUNC_ID = "[BandMatrix::recreate]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_BandMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int  mu = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int ml = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int smu = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	resize(n, mu, ml, smu);

	size_t dataSize = m_dataStorage.size()*sizeof(double);
	std::memcpy(&m_dataStorage[0], dataPtr, dataSize);
	dataPtr = (char*)dataPtr + dataSize;

	size_t pivotSize = m_n*sizeof(unsigned int);
	std::memcpy(&m_pivots[0], dataPtr, pivotSize);
	dataPtr = (char*)dataPtr + pivotSize;
}

} // namespace IBKMK

