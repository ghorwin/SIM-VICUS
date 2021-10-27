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

#ifndef IBKMK_SparseMatrixPatternH
#define IBKMK_SparseMatrixPatternH

#include <set>
#include <map>
#include <vector>

#include <cstdint>

namespace IBKMK {

/*! Encodes and decodes an 2D sparse matrix pattern
	by a hash code. The standard implementation uses arithmetic operations
	for index encoding, the non-standard implementation bitwise operations.
	The class is created for high performance matrix pattern manipulations
	i.e. the calculation of the transitive closure for a large matrix.

	A sparse matrix pattern nxn is randomely composed and tranposed by the following example
	\code
	int main() {
		unsigned int n = 10;
		// create a random matrix pattern
		SparseMatrixPattern d(n);

		srand((unsigned int)time(NULL));

		for (unsigned int k=0; k<n*5; ++k) {
			d.set((unsigned int) double(rand())*n/RAND_MAX, (unsigned int) double(rand())*n/RAND_MAX);
		}
		// create transpose pattern
		SparseMatrixPattern dT(n);
		for(unsigned int i = 0; i < n; ++i) {
			for(unsigned int j = 0; j < n; ++j) {
				// an entry exists at i,j
				if(d.test(i,j))
					dT.set(j,i);
			}
		}
		// retrieve a special row
		std::vector<unsigned int> columns;
		d.indexesPerRow(1, columns);

		std::cout << std::endl << "Row 1:" << std::endl;
		std::cout << "[ ";
		for (unsigned int i=0; i<columns.size(); ++i) {
			if(i == 0)
				std::cout << columns[i] ;
			else
				std::cout << ", " << columns[i] ;
		}
		std::cout << " ]";
		std::cout << std::endl;
		// retrieve a special column
		std::vector<unsigned int> rows;
		dT.indexesPerRow(1, rows);

		std::cout << std::endl << "Column 1:" << std::endl;
		for (unsigned int i=0; i<rows.size(); ++i)
			std::cout << "[ " << rows[i] << " ]" << std::endl;
		}
	}
	\endcode
*/

class SparseMatrixPattern {
public:
	/*! Static function for calculation of transitive closure of a
		matrix part.
	\param pattern pattern whose transitive closure should be calculated.
	\param transposePattern transpose pattern.
	\param n matrix size.
	\param startIndex start index of matrix part whose transitive closure should be calculated.
	\param endIndex end index of matrix part whose transitive closure should be calculated.
	*/
	static void calculateTransitiveClosure(
		SparseMatrixPattern &pattern,
		SparseMatrixPattern &transposePattern,
		unsigned int n,
		unsigned int startIndex,
		unsigned int endIndex);

	/*! Default constructor, registers size of index space.
		\param n Matrix dimension.
	*/
	SparseMatrixPattern(unsigned int n);

	/*! Registers a pattern entry at a given row and column and
		throws an exception if one of the indexes exceeds current index space.
		\param i pattern row.
		\param j pattern column.
	*/
	void set(unsigned int i, unsigned int j);

	/*! Checks whether an entry for a given row and column is
		regsiters and returns 'true' in this case. The function
		throws an exception if one of the indexes exceeds current index space.
		\param i requested pattern row.
		\param j requested pattern column.
	*/
	bool test(unsigned int i, unsigned int j) const;

	/*! Fills a vector with the pattern information at a given row and
		throws an exception if one the row index exceeds current index space.
		\param i pattern row.
		\param columnIndexes vector with all registered column indexes at row i.
	*/
	void indexesPerRow(unsigned int i, std::vector<unsigned int> & columnIndexes) const;

private:
	unsigned int			m_n;
#ifdef STANDARD_IMPLEMENTATION
	std::set<unsigned int>				m_data;
#else // STANDARD_IMPLEMENTATION
	std::map<uint64_t, unsigned int>	m_data;
#endif // STANDARD_IMPLEMENTATION
};

} // namespace IBKMK

/*! \file IBKMK_SparseMatrixPattern.h
	\brief Contains the declaration of class SparseMatrixPattern.
*/

#endif // IBKMK_SparseMatrixPatternH
