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

#ifndef IBKMK_SparseMatrixCSRH
#define IBKMK_SparseMatrixCSRH


#include "IBKMK_SparseMatrix.h"

#include <IBK_assert.h>

namespace IBKMK {

/*! Class for sparse matrix storage in CSR format (IBK-type).

### CSR Sparse Matrix Format Description ###

Matrix nonzero positions are stored by two index vectors, ja (size nnz) and ia (size n+1).

A matrix

	[  1   2             ]
	[  3   4   5         ]
	[      6   7         ]
	[  8   9      10     ]
	[     11  12      13 ]

will be represented by n=5, nnz (number of nonzeros)=14

	A     = [ 1 2   3 4 5   6 7   8 9 10   11 12 13   ]
	ia    = [ 0     2       5     7        10         13 ]
	ja    = [ 0 1   0 1 2   1 2   0 1  3    1  2  4   ]

The transposed matrix is

	A     = [ 1 3 8   2 4 6 9 11   5 7 12   10   13   ]
	ia    = [ 0       3            8        11   12   13 ]
	ja    = [ 0 1 3   0 1 2 3 4    1 2 4    3    4    ]


Note: in the IBK-Version of the CSR matrix, the diagonal element is always used.
*/
class SparseMatrixCSR: public SparseMatrix {
public:

	/*! Defines wether matrix has symmetric or assymetric pattern.
		ilu() and backsolve() operations are faster with a symmetric matrix pattern and less
		memory is used.
		\note Symmetric means that the matrix pattern is symmetric, not the values (not necessarily
			  a symmetric matrix).
	*/
	enum PatternMode {
		PM_Symmetric,
		PM_Asymmetric,
		NUM_PatternMode
	};

	/*! Constructor, creates an empty matrix.
		Before using the matrix, call resize().
		\sa resize
	*/
	SparseMatrixCSR() : m_nnz(0), m_patternMode(NUM_PatternMode) {}

	/*! Constructor, creates a matrix with initial size and index vectors.
		\sa resize
	*/
	SparseMatrixCSR(unsigned int n, unsigned int nnz, const unsigned int * ia, const unsigned int * ja,
		const unsigned int * iaT = nullptr, const unsigned int * jaT = nullptr) {
		resize(n, nnz, ia, ja, iaT, jaT);
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
		\return Returns a storage index 0 <= index < m_nnz if the
				cell is used in the sparse matrix,
				otherwise m_nnz, which indicates, that this
				cell is not part of the sparse matrix (=0 cell).

		This function is currently used by the jacobian implenetation.
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

	/*! Returns number of non-zero eleemnts. */
	unsigned int nnz() const { return m_nnz; }

	/*! Returns pointer to continuous memory array holding row index information. */
	const unsigned int * constIa() const { return &m_ia[0]; }

	/*! Returns pointer to continuous memory array holding row index information. */
	unsigned int * ia() { return &m_ia[0]; }

	/*! Returns pointer to continuous memory array holding column index information. */
	const unsigned int * constJa() const { return &m_ja[0]; }

	/*! Returns pointer to continuous memory array holding column index information. */
	unsigned int * ja() { return &m_ja[0]; }

	/*! Returns pointer to continuous memory array holding column index information of the transposed
		matrix.
		\note Only for asymmetric matrixes.
		\warning Built-in automatism may yield hard-to-track errors. Maybe configure matrix as either
			assymetric or symmetric (additional constructor properties) so that availability of vectors
			and correct sizing can be checked.
	*/
	const unsigned int * constIaT() const {  if(!m_iaT.empty()) return &m_iaT[0]; return &m_ia[0]; }

	/*! Returns pointer to continuous memory array holding row index information of the transposed
		matrix.
		\note Only for asymmetric matrixes.
		\warning Built-in automatism may yield hard-to-track errors. Maybe configure matrix as either
			assymetric or symmetric (additional constructor properties) so that availability of vectors
			and correct sizing can be checked.
	*/
	unsigned int * iaT() { if(!m_iaT.empty()) return &m_iaT[0]; return &m_ia[0];}

	/*! Returns pointer to continuous memory array holding column index information of the transposed
		matrix.
		\note Only for asymmetric matrixes.
	*/
	const unsigned int * constJaT() const {  if(!m_jaT.empty()) return &m_jaT[0]; return &m_ja[0]; }

	/*! Returns pointer to continuous memory array holding column index information of the transposed
		matrix.
		\note Only for asymmetric matrixes.
	*/
	unsigned int * jaT() { if(!m_jaT.empty()) return &m_jaT[0]; return &m_ja[0]; }

	/*! Resizes the matrix.
		If nullptr is passed for arguments iaT and jaT, the matrix has a symmetric pattern.
		For non-symmetric matrix patterns, both pointers iaT and jaT must be valid and not nullptr.
		\param n Dimension of the matrix, n x n
		\param nnz Total number of nonzeros values.
		\param ia Row index vector (size n+1)
		\param ja Column index vector (nnz)
		\param iaT Row index vector of the transposed matrix (only for nonsysmmetric matrices)
		\param jaT Column index vector of the transposed matrix (only for nonsysmmetric matrices)
	*/
	void resize(unsigned int n, unsigned int nnz, const unsigned int * ia, const unsigned int * ja,
		const unsigned int * iaT = nullptr, const unsigned int * jaT = nullptr);

	/*! Resize the matrix from EID format.
		\param n Dimension of the matrix, n x n
		\param elementsPerRow Number of values per row for ellpack-itpack
		\param indices Vector of size n*elementsPerRow holding the indices for ellpack-itpack format.
		If index vector is nullptr pointer this function throws an exception.
	*/
	void resizeFromEID(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices);

	/*! Fills matrix from a given EID representation.
		Ensure that you resized the matrix using resizeFromeid() befor calling parseFromeid().
		\param n Dimension of the matrix, n x n
		\param elementsPerRow Number of values per row
		\param indices Vector of size n*elementsPerRow holding the indices. If a nullptr pointer (the default),
			   the index vector is initialized with 0 (first column vector of full matrix).
		\param Data vector of the elpack-itpack matrix.
	*/
	void parseFromEID(unsigned int n, unsigned int elementsPerRow, const unsigned int * indices, const double *data);

	/*! Generate transpose index vectors
		\param ia Row index vector (size n+1)
		\param ja Column index vector (nnz)
		\param iaT Row index vector of the transposed matrix (size n+1, to be generated)
		\param jaT Column index vector of the transposed matrix (nnz, to be generated)
	*/
	static void generateTransposedIndex(const std::vector<unsigned int> &ia, const std::vector<unsigned int> &ja,
		std::vector<unsigned int> &iaT, std::vector<unsigned int> &jaT);

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
	/*! Total number of non-zero elements in the matrix, equals size of m_data. */
	unsigned int						m_nnz;
	std::vector<unsigned int>			m_ia;
	std::vector<unsigned int>			m_ja;
	/*! i-Indices of the tranposed matrix. Important for the ILU of non-symmetric matrices.*/
	std::vector<unsigned int>			m_iaT;
	/*! j-Indices of the tranposed matrix. Important for the ILU of non-symmetric matrices.*/
	std::vector<unsigned int>			m_jaT;
	PatternMode							m_patternMode;
}; // SparseMatrixCSR

/*! \file IBKMK_SparseMatrixCSR.h
	\brief Contains the class SparseMatrixCSR.
	\example sparse_matrix_csr_example.cpp Illustrates usage of class SparseMatrixCSR.
*/

} // namespace IBKMK


#endif // IBKMK_SparseMatrixCSRH
