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

#include "IBKMK_SparseMatrixPattern.h"

#include <IBK_Exception.h>
#include <IBK_FormatString.h>


namespace IBKMK {


void SparseMatrixPattern::calculateTransitiveClosure(
	SparseMatrixPattern &pattern,
	SparseMatrixPattern &transposePattern,
	unsigned int n,
	unsigned int startIndex,
	unsigned int endIndex)
{

	FUNCID(SparseMatrixPattern::calculateTransitiveClosure);
	// error: index exceeds matrix dimension
	if (startIndex > n || endIndex > n) {
		throw IBK::Exception(IBK::FormatString("Error "
			"calculating transitive closure for matrix indexes (%1,%2): maximum "
			"index is %3!")
			.arg(startIndex).arg(endIndex).arg(n),
			FUNC_ID);
	}
	if (startIndex > endIndex) {
		throw IBK::Exception(IBK::FormatString("Error "
			"calculating transitive closure for matrix indexes (%1,%2)!")
			.arg(startIndex).arg(endIndex),
			FUNC_ID);
	}

	// use Warshall algorithm for calculation of tarnsitive closure
	for (unsigned int k = startIndex; k < endIndex; ++k) {
		try {
			// *** generate transitive closure using Warshalls algorithm ***

			// find all indices for column equation
			std::vector<unsigned int> rows;
			std::vector<unsigned int> columns;
			// find all rows and columns
			pattern.indexesPerRow(k, columns);
			transposePattern.indexesPerRow(k, rows);

			// for i: dik == 1
			for (unsigned int iIdx = 0; iIdx < rows.size(); ++iIdx) {
				unsigned int i = rows[iIdx];
				// skip equal rows
				if (i == k)
					continue;
				// for j: dkj == 1
				for (unsigned int jIdx = 0; jIdx < columns.size(); ++jIdx) {

					unsigned int j = columns[jIdx];
					// skip equal columns
					if (j == k)
						continue;
					// fill di,j
					if (!pattern.test(i, j))
						pattern.set(i, j);
					if (!transposePattern.test(j, i))
						transposePattern.set(j, i);
				}
			}
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error eliminating equation %1 from sparse matrix pattern!")
				.arg(k), FUNC_ID);
		}
	}
}


SparseMatrixPattern::SparseMatrixPattern(unsigned int n) {
	FUNCID(SparseMatrixPattern::SparseMatrixPattern);
	// do not allow dimensions == 0
	if (n == 0)
		throw IBK::Exception("Error creating sparse matrix pattern: matrix dimenion '0' is not allowed!", FUNC_ID);
	m_n = n;
}


void SparseMatrixPattern::set(unsigned int i, unsigned int j) {
	FUNCID(SparseMatrixPattern::set);

	// error: index exceeds matrix dimension
	if(i >= m_n || j >= m_n) {
		throw IBK::Exception(IBK::FormatString( "Error "
			"registering index pair (%1,%2): maximum "
			"index is %3!")
			.arg(i).arg(j).arg(m_n),
			FUNC_ID);
	}
#ifdef STANDARD_IMPLEMENTATION
	unsigned int hash = i*m_n + j;
	m_data.insert(hash);
#else // STANDARD_IMPLEMENTATION
	uint64_t chunksPerRow = m_n/32 + ((m_n % 32) != 0 ? 1 : 0);
	uint64_t indj = j / 32;
	// generate index of chunk
	uint64_t hash = i*chunksPerRow + indj;
	// generate index of bit in chunk
	unsigned int modj = j % 32;
	unsigned int bitidx = 1 << modj;
	m_data[hash] |= bitidx;
#endif // STANDARD_IMPLEMENTATION
}


bool SparseMatrixPattern::test(unsigned int i, unsigned int j) const {
	FUNCID(SparseMatrixPattern::test);

	// error: index exceeds matrix dimension
	if(i >= m_n || j >= m_n) {
		throw IBK::Exception(IBK::FormatString( "Error "
			"checking for index pair (%1,%2): maximum "
			"index is %3!")
			.arg(i).arg(j).arg(m_n),
			FUNC_ID);
	}
#ifdef STANDARD_IMPLEMENTATION
	unsigned int hash = i*m_n + j;
	return m_data.find(hash) != m_data.end();
#else // STANDARD_IMPLEMENTATION
	uint64_t chunksPerRow = m_n/32 + ((m_n % 32) != 0 ? 1 : 0);
	uint64_t indj = j / 32;
	// generate index of chunk
	uint64_t hash = i*chunksPerRow + indj;
	std::map<uint64_t, unsigned int>::const_iterator it = m_data.find(hash);
	if (it == m_data.end())
		return false;
	// search bit
	unsigned int val = it->second;
	// generate index of bit in chunk
	unsigned int modj = j % 32;
	unsigned int bitidx = 1 << modj;
	return (val & bitidx) != 0;
#endif // STANDARD_IMPLEMENTATION
}


void SparseMatrixPattern::indexesPerRow(unsigned int i, std::vector<unsigned int> & columnIndexes) const {
	FUNCID(SparseMatrixPattern::indexesPerRow);

	// error: index exceeds matrix dimension
	if(i >= m_n) {
		throw IBK::Exception(IBK::FormatString( "Error "
			"retrieving indexes for row %1: maximum "
			"index size is %2!")
			.arg(i).arg(m_n),
			FUNC_ID);
	}
#ifdef STANDARD_IMPLEMENTATION
	columnIndexes.clear(); // should be a no-op if vector is empty

	// generate index of first chunk in row i
	unsigned int colIndex = 0;
	for (unsigned int hash = m_n*i; hash<m_n*(i+1); ++hash, ++colIndex) {
		std::set<unsigned int>::const_iterator it = m_data.find(hash);
		if (it != m_data.end()) {
			columnIndexes.push_back(colIndex);
		}
	}
#else // STANDARD_IMPLEMENTATION

	columnIndexes.clear(); // should be a no-op if vector is empty

	// generate index of first chunk in row i
	uint64_t chunksPerRow = m_n/32 + ((m_n % 32) != 0 ? 1 : 0);
	unsigned int colIndex = 0;

	// search for next available hash code in map
	uint64_t startHash = chunksPerRow*i;
	uint64_t endHash = chunksPerRow*(i+1);
	std::map<uint64_t, unsigned int>::const_iterator it = m_data.lower_bound(startHash);
	// no valid entry left
	if(it == m_data.end())
		return;

	for ( uint64_t hash = startHash; hash<endHash; ++hash) {
		if (hash != it->first) {
			colIndex += 32; // skip all bits in this chunk
			continue; // no bit set in this chunk
		}
		unsigned int val = it->second;
		// search for bits in chunk
		unsigned int mask = 1;
		for (unsigned int h=0; h<32; ++h) {
			if (val & mask) {
				columnIndexes.push_back(colIndex);
			}
			mask <<= 1;
			++colIndex;
		}
		// search for next valid hash
		++it;
		// no entry left
		if(it == m_data.end() || it->first >= endHash)
			break;
	}
#endif // STANDARD_IMPLEMENTATION
}


} // namespace IBKMK
