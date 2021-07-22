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

#ifndef IBKMK_BlockTridiagMatrixH
#define IBKMK_BlockTridiagMatrixH

#include <iostream>
#include <iomanip>

#include <IBK_assert.h>

#include "IBKMKC_dense_matrix.h"

namespace IBKMK {

/*! Implementation of a block-tridiagonal matrix including
	block-based Thomas algorithm for solution of linear equation systems.
	The block-tridiagonal matrix implementation is meant to simplify usage by
	providing access based on lower, main or upper band. The sub-matricies
	are then stored in column-based order.

	The block-tridiag matrix is designed as variable size container, meaning
	that the sizes of the sub-matricies are defined during runtime. However,
	the time-critical portions (LU factorization and backsolution) are implemented
	using specialized versions based on sub-matrix sizes.

	Internally, the data in the matrix is stored in a continuous memory array of size
	3*n*m, with the elements of the lower band first, then the elements of the main band
	and last the elements of the upper band.

	\warning The lu() and backsolve() routines can use SUNDIALS implementations internally, select this via
			compile-time define.
	If the pivot vector is empty, the non-pivoting factorization routines are used,
	otherwise the pivoting version of the factorization routines are used. If you want to
	use the non-pivoting version of the matrix, clearPivots() after resizing/creating
	the matrix. Mind that the pivot vector is resized/recreated in a call to resize().
*/
class BlockTridiagMatrix {
public:
	/*! The type of the band, used to access the respective matrix elements.
		The bands are numbered 0..2 corresponding to the order of the bands
		in the linear storage array.
	*/
	enum BandType {
		Lower   = 0,
		Main    = 1,
		Upper   = 2,
		/*! Invalid, returned by bandType() query function, if indexes are outside sparsity pattern. */
		Invalid = -1
	};

	/*! Default constructor, creates an empty block-tridiagonal matrix. */
	BlockTridiagMatrix() : m_n(0), m_m(0), m_blockSize(0)
	{
	}

	/*! Initializing constructor, creates a block-tridiag matrix.
		\param n Dimension of matrix in blocks.
		\param m Size of sub-matricies.
	*/
	BlockTridiagMatrix(unsigned int n, unsigned int m) {
		resize(n,m);
	}

	/*! Copy constructor. */
	BlockTridiagMatrix(const BlockTridiagMatrix & other) { *this = other; }

	/*! Assignment operator. */
	const BlockTridiagMatrix & operator=(const BlockTridiagMatrix & b);

	/*! Resizes the matrix and the pivots vector.
		\param n Dimension of matrix in blocks.
		\param m Size of sub-matricies.
	*/
	void resize(unsigned int n, unsigned int m);

	/*! Clear the pivot vector and force matrix to use non-pivoting factorization
		and backsolving routines.
	*/
	void clearPivots() { m_pivots.clear(); }

	/*! Returns the size of the matrix. */
	unsigned int	n() const { return m_n; }

	/*! Returns the size of sub-matrices. */
	unsigned int	m() const { return m_m; }

	/*! Determines band type of the element in row i, and column j. */
	BandType bandType(unsigned int i, unsigned int j) const {
		if (i==j) return BlockTridiagMatrix::Main;
		if (i==j-1) return BlockTridiagMatrix::Upper;
		if (i==j+1) return BlockTridiagMatrix::Lower;
		return BlockTridiagMatrix::Invalid;
	}

	/*! Returns a reference to the element at the coordinates [i, j].
		\param i Matrix row, 0 <= i < n
		\param j Matrix column, 0 <= j < n
		\code
		A(i,j) = 6;
		\endcode
		When an element is requested, that is not part of the
		matrix pattern the function throws an IBK::Exception.
	*/
	double & operator()(unsigned int i, unsigned int j);

	/*! Returns a constant reference to the element at the coordinates [i, j].
		\param i Matrix row, 0 <= i < n
		\param j Matrix column, 0 <= j < n
		\code
		val = A.value(i,j);
		\endcode
		When an element is requested, that is not part of the
		matrix pattern the function returns a zero instead.
	*/
	double value(unsigned int i, unsigned int j) const;

	/*! Returns a pointer to the submatrix data.
		The submatrix data is organized as column based linear storage array with
		order elem = submatrix[row + col*m) with m as dimension of submatrix.
		\param row	The row of the submatrix in the block banded matrix.
		\param bandType	The type of the band of the matrix.
	*/
	double * submatrix(unsigned int row, BandType bandType) {
		IBK_ASSERT(row < m_n);
		IBK_ASSERT((unsigned int)bandType < 3);
		return &m_data[row*m_blockSize + bandType*m_blockSize*m_n];
	}

	/*! Returns a pointer to the submatrix data (const-version).
		Works just as the function above, but returns a const pointer.
	*/
	const double * submatrix(unsigned int row, BandType bandType) const {
		IBK_ASSERT(row < m_n);
		IBK_ASSERT((unsigned int)bandType < 3);
		return &m_data[row*m_blockSize + bandType*m_blockSize*m_n];
	}

	/*! Fills the matrix with zeros. */
	void setZero();

	/*! Solves equation system Ax = b (A is this matrix, b is the right-hand-side of the equation system)
		using the block-tridiagonal Thomas algorithm by
		first computing the lu factorization, then backsolving with the right-hand side vector.
		\param b Vector with rhs vector, size n()*m().
		\note If the pivots vector is empty (call clearPivots() beforehand), the
		non-pivoting version is used. This is faster,
		yet gives less accurated solutions of the equation system, when the submatrices
		are badly ordered/conditioned.

		\warning After the call to the matrix it is left in factorized form. Calling the function twice
		to compute two equation systems without restoring the original matrix will give garbage.
	*/
	void solveEquationSystem(double * b);

	/*! Does an in-place LU factorization.
		Depending on the size of the sub-matricies, a specialized version
		of the algorithm may be used.
		\return Returns 0 on success, a negative value on general failure.
		\note If the pivots vector is empty (call clearPivots() beforehand), the
		non-pivoting version is used. This is faster,
		yet gives less accurated solutions of the equation system, when the submatrices
		are badly ordered/conditioned.
	*/
	int lu();

	/*! Solves the equation system Ax = b with LU matrix data.
		\param b Vector with rhs vector, size n()*m().
		\note If the pivots vector is empty (call clearPivots() beforehand), the
		non-pivoting version is used. This is faster,
		yet gives less accurated solutions of the equation system, when the submatrices
		are badly ordered/conditioned.
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
		\param eulerFormat If true, prints in Euler-Mathtoolbox format A = [x,x,x; y,y,y; z,z,z];
		\param width Column width for matrix output (precision is expected to be set as stream property)
		\param matrixLabel The label to be used for defining the matrix in Euler format, defaults to 'A'
		\param vectorLabel The label to be used for defining the vector b in Euler format, defaults to 'b'
	*/
	void write(std::ostream & out, double * b = NULL, bool eulerFormat = false, unsigned int width=4,
					   const char * const matrixLabel = "A", const char * const vectorLabel = "b") const;

	/*! Computes and returns serialization size in bytes.
		Includes memory needed for identification of matrix type.
	*/
	std::size_t serializationSize() const;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Starts with matrix identification number, followed by matrix-format-specific data.
	*/
	void serialize(void* & dataPtr) const;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Copies only actual data and pivots, requires matrix to be properly resized beforehand.
		Tests sizes stored in binary memory with sizes set in matrix (must match, otherwise exception is thrown).
	*/
	void deserialize(void* & dataPtr);

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Resizes matrix to requested dimensions and populates matrix with stored content.
	*/
	void recreate(void* & dataPtr);

private:
	/*! The dimension of the block banded matrix (m_n*m_n). */
	unsigned int						m_n;
	/*! The dimension of the sub-matrices (m_m*m_m). */
	unsigned int						m_m;
	/*! Size of each blocks in number of doubles = m_m*m_m, cached since it needs to be accessed many times. */
	unsigned int						m_blockSize;

	/*! Data storage, bands are stored in continuous memory array, each band
		has the size m_n*m_blockSize.
	*/
	std::vector<double>					m_data;

	/*! Vector with pointers to start of all blocks, with m_blocks[0] pointing to L[0], m_blocks[m_n] pointing to M[0] etc. */
	std::vector<double*>				m_blocks;

	/*! Pivot vector (size m_n*m_m), used by factorization and backsolving routines.
		If empty (after a call to clearPivots()) the non-pivoting factorization routines are used.
	*/
	std::vector<long int>				m_pivots;

}; // class BlockTridiagMatrix


} // namespace IBKMK

#endif // IBKMK_BlockTridiagMatrixH
