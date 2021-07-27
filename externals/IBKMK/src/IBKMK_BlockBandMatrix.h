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

#ifndef IBKMK_BlockBandMatrixH
#define IBKMK_BlockBandMatrixH

#include <iostream>
#include <iomanip>

#include <IBK_assert.h>

#include "IBKMKC_dense_matrix.h"

namespace IBKMK {

/*! Implementation of a block-banded matrix block-based LU factorization and backsolving routine
	for solution of linear equation systems.

	The block-banded matrix is designed as variable size container, meaning
	that the sizes of the sub-matricies are defined during runtime. However,
	the time-critical portions (LU factorization and backsolution) are implemented
	using spezialized versions based on sub-matrix sizes.

	Internally, the data in the matrix is stored in a continuous memory array of size
	n*m*m*(ml+mu+1), with an overall row-major ordering of the blocks.
*/
class BlockBandMatrix {
public:

	/*! Default constructor, creates an empty block-banded matrix. */
	BlockBandMatrix() : m_n(0), m_m(0), m_ml(0), m_mu(0), m_blockSize(0)
	{
	}

	/*! Initializing constructor, creates a block-tridiag matrix.
		\param n Dimension of matrix in blocks.
		\param m Size of sub-matricies.
		\param ml Number of block-lower diagonals.
		\param mu Number of block-upper diagonals.
	*/
	BlockBandMatrix(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu) {
		resize(n, m, ml, mu);
	}

	/*! Copy constructor. */
	BlockBandMatrix(const BlockBandMatrix & other) { *this = other; }

	/*! Assignment operator. */
	const BlockBandMatrix & operator=(const BlockBandMatrix & b);

	/*! Resizes the matrix.
		\param n Dimension of matrix in blocks.
		\param m Size of sub-matricies.
		\param ml Number of block-lower diagonals.
		\param mu Number of block-upper diagonals.
	*/
	void resize(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu);

	/*! Returns the size of the matrix. */
	unsigned int	n() const { return m_n; }
	/*! Returns the size of sub-matrices. */
	unsigned int	m() const { return m_m; }
	/*! Returns the number of block-lower-diagonals. */
	unsigned int	ml() const { return m_ml; }
	/*! Returns the number of block-upper-diagonals. */
	unsigned int	mu() const { return m_mu; }

	const std::vector<double> & data() const { return m_data; }

	/*! Returns a pointer to the submatrix data.
		The submatrix data is organized as column based linear storage array with
		order elem = submatrix[row + col*m) with m as dimension of submatrix.
		\param i The row of the submatrix in the block banded matrix.
		\param j The row of the submatrix in the block banded matrix.
	*/
	double * submatrix(unsigned int i, unsigned int j);

	/*! Returns a pointer to the submatrix data (const-version).
		Works just as the function above, but returns a const pointer.
	*/
	const double * submatrix(unsigned int i, unsigned int j) const;

	/*! Does an in-place LU factorization.
		Depending on the size of the sub-matricies, a specialized version
		of the algorithm may be used.
		\return Returns 0 on success, a negative value on general failure.
	*/
	int lu();

	/*! Solves the equation system Ax = b with LU matrix data.
		\param b Vector with rhs vector, size n()*m().
	*/
	void backsolve(double * b) const;

	/*! Computes r = A*b.
		\param b Vector of size n() (read-only).
		\param r Vector of size n(), holds results.
	*/
	void multiply(const double * b, double * r) const;

	/*! Dumps the matrix to an output stream in human-readibly.
		\param out Output stream (ASCII).
		\param b Pointer to vector to print alongside matrix, size = n, NULL if no vector to print.
		\param eulerFormat If true, prints in Euler-Mathtoolbox format
		\param width Column width for matrix output (precision is expected to be set as stream property)
		\param matrixLabel The label to be used for defining the matrix in Euler format, defaults to 'A'
		\param vectorLabel The label to be used for defining the vector b in Euler format, defaults to 'b'
	*/
	void write(std::ostream & out, double * b = NULL, bool eulerFormat = false, unsigned int width=4,
					   const char * const matrixLabel = "A", const char * const vectorLabel = "b") const;

private:
	/*! The dimension of the block banded matrix (m_n x m_n). */
	unsigned int						m_n;
	/*! The dimension of the sub-matrices (m_m x m_m). */
	unsigned int						m_m;
	/*! Number of block-lower diagonals. */
	unsigned int						m_ml;
	/*! Number of block-upper diagonals. */
	unsigned int						m_mu;
	/*! Size of each blocks in number of doubles. */
	unsigned int						m_blockSize;

	/*! Data storage, matrix is stored in continuous memory array,
		block-row-major ordering, within subblock we have column-major ordering.
	*/
	std::vector<double>					m_data;

}; // class BlockBandMatrix


} // namespace IBKMK

#endif // IBKMK_BlockBandMatrixH
