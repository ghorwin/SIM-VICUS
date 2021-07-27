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

#ifndef IBKMK_BlockVectorH
#define IBKMK_BlockVectorH

#include <vector>

namespace IBKMK {

/*! Represents a vector that is composed of n sub-vectors, each of size m.
	In comparison to a linear vector the index operator[] returns a pointer to
	the begin of a block, not to the element.
	*/
class BlockVector {
public:
	/*! Standard constructur, creates an empty vector. */
	BlockVector()  : m_n(0), m_m(0)
	{
	}

	/*! Constructor, creates a vector with n blocks of size m.
		The total size of the vector will be n*m. */
	BlockVector(unsigned int n, unsigned int m) {
		resize(n, m);
	}

	/*! Resizes the vector to hold n sub-vectors of size m. */
	void resize(unsigned int n, unsigned int m) {
		m_data.resize(n*m);
		m_n = n;
		m_m = m;
	}

	/*! Returns pointer to begin of sub-vector i.
		Access sub-vector elements like a two-dimensional matrix.
		\code
		// retrieve element k from sub-vector i
		double val_ik = blockvec[i][k];
		\endcode
	*/
	double * operator[](unsigned int i) {
		return &m_data[i*m_m];
	}

private:
	unsigned int		m_n;	///< Number of sub-vectors.
	unsigned int		m_m;	///< Size of sub-vectors.
	std::vector<double>	m_data;	///< Linear data storage member.

}; // class BlockVector

} // namespace IBKMK


#endif // IBKMK_BlockVectorH
