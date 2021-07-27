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

#ifndef IBKMK_SparseMatrixEIDH
#define IBKMK_SparseMatrixEIDH

#include "IBKMK_SparseMatrix.h"

#include <vector>

#include <IBK_assert.h>

namespace IBKMK {

/*! Class for sparse matrix storage in Ellpack-Itpack format (IBK-type).
	Data storage is similar to the Ellpack-Itpack format, but with special definitions
	regarding the index table. Each row of the sparse matrix can only hold as much as
	m_elementsPerRow elements and at minimum one non-zero elements which must be the main diagonal.
	Total size of the matrix in memory can be m_n*m_elementsPerRow elements.

### Ellpack-Itpack DELPHIN-Variant, Sparse Matrix Format Description ###

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
value must be zero. For example, multiplication of a vector b with
this matrix leads to the following equation for first row:

	r[0] = A[0]*b[index[0]] + A[1]*b[index[1]] + A[2]*b[index[2]]
			= 1*b[0] + 2*b[1] + 0*b[1]

where executing the last multiplication is faster than additional if clauses that
would stop the inner loop once no more non-zero elements are included.

Within each index row column indices must be stored in increasing order (sort by value).
*/
class SparseMatrixEID: public SparseMatrix {
public:
	/*! Constructor, creates an empty matrix.
		Before using the matrix, call resize().
		\sa resize
	*/
	SparseMatrixEID() : m_elementsPerRow(0) {}

	/*! Constructor, creates a matrix with initial size and optionally with index vector.
		\sa resize
	*/
	SparseMatrixEID(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices) {
		resize(n, elementsPerRow, indices);
	}

	/*! Adds an identity matrix to the currently stored matrix (adds 1 to main diagonal). */
	virtual void addIdentityMatrix();

	/*! Returns a reference to the element at the coordinates [i, j] (i = row).
		\param i Matrix row, 0 <= i < m_n
		\param j Matrix column, 0 <= j < m_n
		\code
		A(i,j) = 6;
		\endcode
	*/
	virtual double & operator()(unsigned int i, unsigned int j);

	/*! Returns a constant reference to the element at the coordinates [i, j].
		\param i Matrix row, 0 <= i < n
		\param j Matrix column, 0 <= j < n
		\code
		val = A.value(i,j);
		\endcode
		When an element is requested, that is not part of the
		matrix pattern the function returns a zero instead.
	*/
	virtual double value(unsigned int i, unsigned int j) const;

	/*! Returns the storage index in the m_data vector for a cell in row i, and column j.
		\param i Matrix row, 0 <= i < m_n
		\param j Matrix column, 0 <= j < m_n
		\return Returns a storage index 0 <= index < m_elementsPerRow*m_n if the
				cell is used in the sparse matrix,
				otherwise m_elementsPerRow*m_n, which indicates, that this
				cell is not part of the sparse matrix (=0 cell).

		This function is currently used by the ilu() algorithm and by the write() function.
	*/
	virtual unsigned int storageIndex(unsigned int i, unsigned int j) const;

	/*! Multiplies matrix with vector b and stores result in vector res.
		This version of the function works with dublicated index vector
		entries in the index vector for zero elements in data vector.
	*/
	virtual void multiply(const double * b, double * res) const;

	/*! Performs an in-place ILU.
		Requires the matrix to have the values of the original matrix.
		Uses LU factorization where <math>u_{i,i} = 1</math>.
	*/
	virtual void ilu();

	/*! Solves <math>LUx = b as x = U^{-1} L^{-1} b</math> using backward elimination.
		The matrix is expected to hold incomplete LU data as generated by ilu() with
		<math>u_{i,i} = 1</math>.
	*/
	virtual void backsolveILU(double * b) const;

	/*! Returns pointer to continuous memory array holding index information. */
	const unsigned int * constIndex() const { return &m_index[0]; }

	/*! Returns pointer to continuous memory array holding index information. */
	unsigned int * index() { return &m_index[0]; }

	/*! Returns number of elements per row. */
	unsigned int elementsPerRow() const { return m_elementsPerRow; }

	/*! Resize the matrix.
		\param n Dimension of the matrix, n x n
		\param elementsPerRow Number of values per row
		\param indices Vector of size n*elementsPerRow holding the indices. If a NULL pointer (the default),
			   the index vector is initialized with 0 (first column vector of full matrix).
		\warning If an invalid index vector is passed, or none at all, populate the index vector
				manually using member function index() before using any sparse matrix functions.
	*/
	void resize(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices);

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

protected:
	unsigned int						m_elementsPerRow;
	std::vector<unsigned int>			m_index;
}; // SparseMatrixEID

/*! \file IBKMK_SparseMatrixEID.h
	\brief Contains the class SparseMatrixEID.
	\example sparse_matrix_example.cpp Illustrates usage of class SparseMatrixEID.
*/

} // namespace IBKMK


#endif // IBKMK_SparseMatrixEIDH
