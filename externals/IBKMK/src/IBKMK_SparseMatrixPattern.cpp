/*	IBK Math Kernel Library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

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

SparseMatrixPattern::SparseMatrixPattern(unsigned int n) {
	const char * const FUNC_ID = "[SparseMatrixPattern::SparseMatrixPattern]";
	// do not allow dimensions == 0
	if (n == 0)
		throw IBK::Exception("Error creating sparse matrix pattern: matrix dimenion '0' is not allowed!", FUNC_ID);
	m_n = n;
}


void SparseMatrixPattern::set(unsigned int i, unsigned int j) {
	const char * const FUNC_ID = "[SparseMatrixPattern::set]";

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
	unsigned int chunksPerRow = m_n/32 + ((m_n % 32) != 0 ? 1 : 0);
	unsigned int indj = j / 32;
	// generate index of chunk
	unsigned int hash = i*chunksPerRow + indj;
	// generate index of bit in chunk
	unsigned int modj = j % 32;
	unsigned int bitidx = 1 << modj;
	m_data[hash] |= bitidx;
#endif // STANDARD_IMPLEMENTATION
}


bool SparseMatrixPattern::test(unsigned int i, unsigned int j) const {
	const char * const FUNC_ID = "[SparseMatrixPattern::test]";

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
	unsigned int chunksPerRow = m_n/32 + ((m_n % 32) != 0 ? 1 : 0);
	unsigned int indj = j / 32;
	// generate index of chunk
	unsigned int hash = i*chunksPerRow + indj;
	std::map<unsigned int, unsigned int>::const_iterator it = m_data.find(hash);
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
	const char * const FUNC_ID = "[SparseMatrixPattern::indexesPerRow]";

	// error: index exceeds matrix dimension
	if(i >= m_n) {
		throw IBK::Exception(IBK::FormatString( "Error "
			"retrieving indexes for row %1: maximum "
			"index size is %2!")
			.arg(i).arg(m_n),
			FUNC_ID);
	}
#ifdef STANDARD_IMPLEMENTATION
#else // STANDARD_IMPLEMENTATION

	columnIndexes.clear(); // should be a no-op if vector is empty

	// generate index of first chunk in row i
	unsigned int chunksPerRow = m_n/32 + ((m_n % 32) != 0 ? 1 : 0);
	unsigned int colIndex = 0;
	for (unsigned int hash = chunksPerRow*i; hash<chunksPerRow*(i+1); ++hash) {
		std::map<unsigned int, unsigned int>::const_iterator it = m_data.find(hash);
		if (it == m_data.end()) {
			colIndex += 32; // skip all bits in this chunk
			continue; // no bit set in this chunk
		}
		unsigned int val = it->second;
		// search for bits in chunk
		register unsigned int mask = 1;
		for (unsigned int h=0; h<32; ++h) {
			if (val & mask)
				columnIndexes.push_back(colIndex);
			mask <<= 1;
			++colIndex;
		}
	}
#endif // STANDARD_IMPLEMENTATION
}


} // namespace IBKMK
