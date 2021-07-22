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

#ifndef IBKMK_BlockSparseMatrixH
#define IBKMK_BlockSparseMatrixH

#include <vector>
#include <cstring>

#include <IBK_assert.h>

namespace IBKMK {


/*! Class for block sparse matrix storage in Ellpack-Itpack format (IBK-type).
	Data storage is similar to the Ellpack-Itpack format, but with special definitions
	regarding the index table. Each row of the sparse matrix can only hold as much as
	m_blocksPerRow blocks and at minimum one non-zero blocks which must be the main diagonal.
	Total size of the matrix in memory can be m_n*m_blocksPerRow elements.

	Instead of scalar values, the BlockSparseMatrix holds blocks (square matrices) instead.
	Index vector, however, remains the same as for scalar matrices. Thus, size of m_data and
	m_index are no longer the same.

### Ellpack-Itpack Sparse Matrix Format Description ###

Matrix has dimensions n x m, with m being the number of non-zero elements per row.
A matrix

	[  1   2          ]
	[  3   4   5      ]
	[      6   7      ]
	[  8   9  10      ]
	[     11  12  13  ]

will be represented by n=5, m=3

	A     = [ 1 2 0   3 4 5   6 7 0   8 9 10   11 12 13 ]
	index = [ 0 1 1   0 1 2   1 2 2   0 1  2    1  2  3 ]

The index for missing elements should be the preceding index and the corresponding
element must be zero.

Mind that instead of scalars the data vector ususally contains submatrices/blocks of
size m_nSubMatrix*m_nSubMatrix in column-major order.
*/

class BlockSparseMatrix {
public:
	/*! Constructor, creates an empty matrix. */
	BlockSparseMatrix() : m_n(0), m_blocksPerRow(0), m_nSubMatrix(0) {}

	/*! Constructor, creates a matrix with initial size.
		\param number of rows in a block sparse matrix
		\param blocksPerRow number of block elements per row
		\param nSubMatrix side length of a block
		\param indices Vector of size n*blocksPerRow holding the indices.
		\sa resize()
	*/
	BlockSparseMatrix(	unsigned int n,
						unsigned int blocksPerRow,
						unsigned int nSubMatrix,
						const unsigned int * indices = NULL)
	{
		resize(n, blocksPerRow, nSubMatrix, indices);
	}

	/*! Resizes the matrix.
		\param n Number of block-rows of matrix.
		\param blocksPerRow Number of blocks per row.
		\param indices Vector of size n*blocksPerRow holding the indices. If a NULL pointer (the default),
				the index vector is initialized with -1 and need to be clean up afterwards.

		The total dimension of the matrix if given in scalars is n*blocksPerRow x n*blocksPerRow.

		\warning If an invalid index vector is passed, or none at all, populate the index vector
				manually using member function index() before using an multiplication functions.
	*/
	void resize(unsigned int n, unsigned int blocksPerRow, unsigned int nSubMatrix, const unsigned int * indices = NULL);

	/*! Returns size of matrix. */
	unsigned int n() const { return m_n; }

	/*! Returns number of elements per row. */
	unsigned int blocksPerRow() const { return m_blocksPerRow; }

	/*! Returns dimension of submatrices. */
	unsigned int nSubMatrix() const { return m_nSubMatrix; }

	/*! Returns a reference to the element at the coordinates [i, j] (i = row).
		\code
		A(i,j) = 6;
		\endcode
		\warning This function is not necessarily fast, do not use in numerical engines.
	*/
	double &  operator()(unsigned int i, unsigned int j);

	/*! Returns a constant reference to the element at the coordinates [i, j] (i = row).
		\code
		val = A.value(i,j);
		\endcode
		\warning This function is not really fast, do not use in numerical engines.
	*/
	double value(unsigned int i, unsigned int j) const;

	/*! Returns pointer to continuous memory array of size n()*blocksPerRow()*nSubMatrix()*nSubMatrix()
		holding data values.
	*/
	double * data() { return &m_data[0]; }

	/*! Returns pointer to continuous memory array of size n()*blocksPerRow()*nSubMatrix()*nSubMatrix()
		holding data values (const-version).
	*/
	const double * data() const { return &m_data[0]; }

	/*! Returns pointer to continuous memory array of size n()*blocksPerRow() holding index information.
		\warning Mind that index vector has smaller size than data vector!
	*/
	unsigned int * index() { return &m_index[0]; }

	/*! Const-version of above function. */
	const unsigned int * index() const { return &m_index[0]; }

	/*! Returns a adress from one of the cells, or 0 if the cell is not stored in the matrix.
		\param i Matrix row, 0 <= i < m_n
		\param j Matrix column, 0 <= j < m_n
	*/
	double * block(unsigned int i, unsigned int j);

	/*! Same as above (const version).
		\param i Matrix row, 0 <= i < m_n
		\param j Matrix column, 0 <= j < m_n
	*/
	const double * block(unsigned int i, unsigned int j) const;

	/*! Multiplies matrix with block-vector b and stores result in block-vector res. */
	void multiply(const double * b, double * res) const;

	//	unsigned int storageIndex(unsigned int i, unsigned int j) const;

	/*! Performs an in-place ILU.
		Requires the matrix to have the values of the original matrix.
	*/
	void ilu();

	/*! Solves LUx = b as x = U^{-1} L^{-1} b using backward elimination.
		The matrix is expected to hold incomplete LU data as generated by ilu() with
		u_{i,i} = 1.
	*/
	void backsolveILU(double * b) const;

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
	/*! The number of rows of the main matrix */
	unsigned int						m_n;
	/*! The number of columns of the index matrix */
	unsigned int						m_blocksPerRow;
	/*! The number of rows of the SubMatrix */
	unsigned int						m_nSubMatrix;
	/*! The dimension of the sub matrix (nSubMatrix*nSubMatrix) */
	unsigned int						m_blockSize;

	/*! Data storage, size = m_n * m_nSubMatrix*m_nSubMatrix * m_blocksPerRow = m_n * m_blocksPerRow * m_blockSize
		Blocks are stored in column-major order.
	*/
	std::vector<double>					m_data;

	/*! Index vector (block-based), size = m_n * m_blocksPerRow, see documentation for IBK::SparseMatrix for details. */
	std::vector<unsigned int>			m_index;

}; // BlockSparseMatrix

/*! \file IBKMK_BlockSparseMatrix.h
	\brief Contains the class BlockSparseMatrix.
	\example block_sparse_matrix_example.cpp Illustrates usage of class BlockSparseMatrix.
*/

} // namespace IBKMK


#endif // IBKMK_BlockSparseMatrixH
