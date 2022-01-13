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

#include "IBKMK_BlockTridiagMatrix.h"
#include "IBKMK_DenseMatrix.h"
#include "IBKMKC_tridiag_matrix.h"

#ifdef USE_SUNDIALS_BTTRF

// Borland compiler don't know uint32_t
#if defined(__BORLANDC__)
	typedef unsigned __int32 uint32_t;
#endif

extern "C" {
	int btridiagBTTRF(double **a, long int nblocks, long int blocksize, long int * pivots);
	int btridiagBTTRS(double **a, long int nblocks, long int blocksize, long int * pivots, double *b);
}

#endif // USE_SUNDIALS_BTTRF

namespace IBKMK {


// *** Implementation of BlockTridiagMatrix ***

const BlockTridiagMatrix & BlockTridiagMatrix::operator=(const BlockTridiagMatrix & b) {
	if (m_n == b.m_n && m_m == b.m_m && m_pivots.size() == b.m_pivots.size()) {
		// only copy values into existing memory, no pointers are invalidated
		std::copy(b.m_data.begin(), b.m_data.end(), m_data.begin());
		std::copy(b.m_pivots.begin(), b.m_pivots.end(), m_pivots.begin());
	}
	else {
		m_data = b.m_data;
		m_pivots = b.m_pivots;
		m_n = b.m_n;
		m_m = b.m_m;
		m_blockSize = b.m_blockSize;
		// update pointers
		m_blocks.resize(3*m_n);
		double * A = &m_data[0];
		for (unsigned int i=0; i<3*m_n; ++i)
			m_blocks[i] = A + i*m_blockSize;
	}
	return *this;
}


void BlockTridiagMatrix::resize(unsigned int n, unsigned int m) {
	IBK_ASSERT(n > 0);
	IBK_ASSERT(m > 0);
	m_n = n;
	m_m = m;
	m_blockSize = m_m*m_m;
	m_data.resize(n*3*m_blockSize);
	m_pivots.resize(m*n); // we need storage space for all pivots of the main diagonal elements
	// update pointers
	m_blocks.resize(3*m_n);
	double * A = &m_data[0];
	for (unsigned int i=0; i<3*m_n; ++i)
		m_blocks[i] = A + i*m_blockSize;
}


double & BlockTridiagMatrix::operator()(unsigned int i, unsigned int j) {
	// determine block matrix coordinates
	unsigned int mi = i/m_m;		// block-row
	unsigned int mj = j/m_m;		// block-column
	// submatrix coordinates
	unsigned int smi = i % m_m;		// submatrix-row
	unsigned int smj = j % m_m;		// submatrix-column
	unsigned int smidx = smi + m_m*smj;
	// outside of the three bands?
	if (mi == mj)			return submatrix(mi, Main)[smidx];
	else if (mj+1 == mi)	return submatrix(mi, Lower)[smidx];
	else if (mj == mi+1)	return submatrix(mi, Upper)[smidx];
	else
		throw IBK::Exception(IBK::FormatString("Invalid index (%1,%2) for block-tridiagonal matrix with nblocks = %3 and blocksize = %4")
							 .arg(i).arg(j).arg(m_n).arg(m_m), "[BlockTridiagMatrix::operator()]");
}


double BlockTridiagMatrix::value(unsigned int i, unsigned int j) const {
	// determine block matrix coordinates
	unsigned int mi = i/m_m;		// block-row
	unsigned int mj = j/m_m;		// block-column
	// submatrix coordinates
	unsigned int smi = i % m_m;		// submatrix-row
	unsigned int smj = j % m_m;		// submatrix-column
	unsigned int smidx = smi + m_m*smj;
	// outside of the three bands?
	if (mi == mj)			return submatrix(mi, Main)[smidx];
	else if (mj+1 == mi)	return submatrix(mi, Lower)[smidx];
	else if (mj == mi+1)	return submatrix(mi, Upper)[smidx];
	else					return 0;
}


void BlockTridiagMatrix::setZero() {
	if (m_data.size() > 0)
		std::memset(&m_data[0], 0, m_n*m_blockSize*sizeof(double));
}


void BlockTridiagMatrix::solveEquationSystem(double * b) {
	// use in-place LU and backsolve for right-hand side vector
	if (lu() != 0)
		throw IBK::Exception("Error solving equation system, encountered zero diagonal.", "[BlockTridiagMatrix::solveEquationSystem]");
	backsolve(b);
}


int BlockTridiagMatrix::lu() {


#ifdef USE_SUNDIALS_BTTRF

	// use sundials implementation
	int result;
	if (m_pivots.empty())
		result = btridiagBTTRF(&m_blocks[0], m_n, m_m, NULL);
	else
		result = btridiagBTTRF(&m_blocks[0], m_n, m_m, &m_pivots[0]);
	return result;

#else

	/* must have at least 4 block-rows */
	if (m_n < 4)
		return -1; /* invalid matrix dimension */

	// some more helper pointers, L, U and M are arrays of pointers to the start of the blocks
	double ** L = &m_blocks[0];
	double ** M = &m_blocks[m_n];
	double ** U = &m_blocks[2*m_n];

	// call special implementation for block tridiag lu with or without pivoting
	if (m_pivots.empty())
		return ibkmk_btridiag_LU( (int)m_n, (int)m_m, L, M, U);
	else
		return ibkmk_btridiag_LU_pivot( (int)m_n, (int)m_m, L, M, U, &m_pivots[0] );

#endif

}


void BlockTridiagMatrix::backsolve(double * b) const {

#ifdef USE_SUNDIALS_BTTRF

	// the const-cast is needed only for the c-api, m_pivots does not get modified here!
	double ** A = const_cast<double**>(&m_blocks[0]);
	if (m_pivots.empty())
		btridiagBTTRS(A, m_n, m_m, NULL, b);
	else
		btridiagBTTRS(A, m_n, m_m,
					  const_cast<long int*>(&m_pivots[0]),
					  b);

#else

	// some more helper pointers, L, U and M are constant arrays of constant pointers to the start of the blocks
	const double * const * L = &m_blocks[0];
	const double * const * M = &m_blocks[m_n];
	const double * const * U = &m_blocks[2*m_n];

	if (m_pivots.empty())
		ibkmk_btridiag_backsolve( m_n, m_m, L, M, U, b );
	else
		ibkmk_btridiag_backsolve_pivot( m_n, m_m, L, M, U, &m_pivots[0], b );

#endif

}


void BlockTridiagMatrix::multiply(const double * b, double * r) const {
	// we compute the multiplication block-based
	const double * L = &m_data[0];
	const double * M = L + m_blockSize*m_n;
	const double * U = M + m_blockSize*m_n;
	// special cases for first block sizes
	switch (m_m) {
		case 1 : {
			ibkmk_tridiag_vec_mult(m_n, &m_data[0], b, r);
		} break;

		default : {
			// first row: r[0] = M[0].b[0] + U[0].b[1]  with . being matrix-vector multiplications
			ibkmk_dense_vec_mult(m_m, M, b, r);
			if (m_n == 1) return;

			ibkmk_dense_vec_mult_add(m_m, U, b + m_m, r);
			// all rows except the last:
			// r[i] = L[i].b[i-1] + M.b[i] + U.b[i+1]
			for (unsigned int i=2; i<m_n; ++i) {	// note: we don't use i in the loop, so 1 .. < m_n-1 is the same as 2 .. < m_n
				L += m_blockSize;
				M += m_blockSize;
				U += m_blockSize;
				b += m_m; // points always to b[i]
				r += m_m; // points always to r[i]
				ibkmk_dense_vec_mult(m_m, L, b - m_m, r);
				ibkmk_dense_vec_mult_add(m_m, M, b, r);
				ibkmk_dense_vec_mult_add(m_m, U, b + m_m, r);
			}
			L += m_blockSize;
			M += m_blockSize;
			b += m_m; // points to b[n-1]
			r += m_m; // points to r[n-1]
			// last row: r[n-1] = L[n-1].b[n-2] + M[n-1].b[n-1]  with . being matrix-vector multiplications
			ibkmk_dense_vec_mult(m_m, L, b - m_m, r);
			ibkmk_dense_vec_mult_add(m_m, M, b, r);
		} break;
	} // switch (m_m)
}


void BlockTridiagMatrix::write(std::ostream & out, double * b, bool eulerFormat, unsigned int width,
							   const char * const matrixLabel, const char * const vectorLabel) const
{
	if (width < 1)	width = 10;
	else 			--width; // subtract one for automatically padded " "
	if (eulerFormat) {
		// writes matrix in euler format
		out << ">format(" << width << ",0);" << std::endl;
		out << ">" << matrixLabel << ":=[ " << std::endl;
		for (unsigned int i=0; i<m_n*m_m; ++i) {
			for (unsigned int j=0; j<m_n*m_m; ++j) {
				// determine block matrix coordinates
				unsigned int mi = i/m_m;		// block-row
				unsigned int mj = j/m_m;		// block-column
				// submatrix coordinates
				unsigned int smi = i % m_m;		// submatrix-row
				unsigned int smj = j % m_m;		// submatrix-column
				unsigned int smidx = smi + m_m*smj;
				// outside of the three bands?
				out << std::setw(width) << std::right;
				if (mi == mj)			out << submatrix(mi, Main)[smidx];
				else if (mj+1 == mi)	out << submatrix(mi, Lower)[smidx];
				else if (mj == mi+1)	out << submatrix(mi, Upper)[smidx];
				else {
					// special handling for first and last row
					if (mi == 0 && mj == 2)					out << submatrix(mi, Lower)[smidx];
					else if (mi == m_n-1 && mj == m_n-3)	out << submatrix(mi, Upper)[smidx];
					else out << "0";
				}
				if (j+1<m_n*m_m)
					out << ",";
			}
			if (i != m_n*m_m-1)
				out << ";" << std::endl;
		}
		out << "];" << std::endl << std::endl;
		// check if we need to write rhs
		if (b != NULL) {
			out << ">" << vectorLabel << ":= [";
			for (unsigned int i=0; i<m_n*m_m; ++i) {
				out << b[i];
				if (i != m_n*m_m-1) out << "; ";
			}
			out << "];\n";
		}
	}
	// dump in screen format
	else {
		for (unsigned int i=0; i<m_n*m_m; ++i) {
			out << "[ ";
			for (unsigned int j=0; j<m_n*m_m; ++j) {
				// determine block matrix coordinates
				unsigned int mi = i/m_m;		// block-row
				unsigned int mj = j/m_m;		// block-column
				// submatrix coordinates
				unsigned int smi = i % m_m;		// submatrix-row
				unsigned int smj = j % m_m;		// submatrix-column
				unsigned int smidx = smi + m_m*smj;
				// outside of the three bands?
				out << std::setw(width) << std::right;
				if (mi == mj)			out << submatrix(mi, Main)[smidx];
				else if (mj+1 == mi)	out << submatrix(mi, Lower)[smidx];
				else if (mj == mi+1)	out << submatrix(mi, Upper)[smidx];
				else {
					// special handling for first and last row
					if (mi == 0 && mj == 2)							out << submatrix(mi, Lower)[smidx];
					else if (mi == m_n-1 && mj == m_n-3)			out << submatrix(mi, Upper)[smidx];
					else out << ".";
				}
				out << " ";
			}
			out << "]";
			if (b != NULL)
				out << "  [ " << std::setw(width) << std::right << b[i] << " ]";
			out << std::endl;
		}
		out << std::endl;
	}
}

std::size_t BlockTridiagMatrix::serializationSize() const {
	// the actual data consists of:
	// - matrix identifier (char)
	// - 2 size variables (n and m),
	// - data storage
	size_t s = sizeof(char) + 2*sizeof(uint32_t) + m_data.size()*sizeof(double);
	return s;
}


void BlockTridiagMatrix::serialize(void* & dataPtr) const {
	// store type
	*(char*)dataPtr = (char)MT_BlockTridiagMatrix;
	dataPtr = (char*)dataPtr + sizeof(char);

	*(uint32_t*)dataPtr = (uint32_t)m_n;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	*(uint32_t*)dataPtr = (uint32_t)m_m;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);

	size_t dataSize = m_data.size()*sizeof(double);
	std::memcpy(dataPtr, &m_data[0], dataSize);
	dataPtr = (char*)dataPtr + dataSize;
}


void BlockTridiagMatrix::deserialize(void* & dataPtr) {
	const char * const FUNC_ID = "[BlockTridiagMatrix::deserialize]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_BlockTridiagMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int m = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	if (n != m_n || m != m_m)
		throw IBK::Exception("Invalid matrix dimensions in binary data storage (target matrix not properly resized?).", FUNC_ID);

	size_t dataSize = m_data.size()*sizeof(double);
	std::memcpy(&m_data[0], dataPtr, dataSize);
	dataPtr = (char*)dataPtr + dataSize;
}


void BlockTridiagMatrix::recreate(void* & dataPtr) {
	const char * const FUNC_ID = "[TridiagMatrix::recreate]";
	char matType = *(char*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(char);
	// check for valid matrix type
	if (matType != MT_BlockTridiagMatrix)
		throw IBK::Exception("Invalid matrix type in binary data storage.", FUNC_ID);

	unsigned int n = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	unsigned int m = *(uint32_t*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(uint32_t);
	resize(n,m);

	size_t dataSize = m_data.size()*sizeof(double);
	std::memcpy(&m_data[0], dataPtr, dataSize);
	dataPtr = (char*)dataPtr + dataSize;
}

} // namespace IBKMK

