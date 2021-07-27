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

#ifndef IBKMK_BandMatrixH
#define IBKMK_BandMatrixH

#include <vector>
#include <stdexcept>
#include <numeric>
#include <utility>
#include <cmath>
#include <cstring>

#include <IBK_assert.h>
#include <IBK_FormatString.h>
#include <IBK_Exception.h>

namespace IBKMK {

/*! A banded matrix implementation.

	The implementation is aimed at easy and intuitive usage of the
	matrix and internal storage. A decomposition and backsolving
	algorithm is also provided. When creating or resizing a band
	matrix you need to pass the number of elements in the matrix n and
	the number of upper and lower off-diagonal elements mu and ml.
	If you are using the decomposition algorithm you have to ensure
	that there is enough storage space available, because the pivoting
	requires a larger bandwidth than the original matrix has. The
	parameter smu gives the storage upper bandwidth used for
	the storage of the decomposed matrix and is calculated as in the
	following example:
	\code
	// Defined size of matrix and number of off-diagonal elements
	unsigned int n = 10;
	unsigned int mu = 3;	// 3 upper off-diagonal elements
	unsigned int ml = 2;	// 2 lower off-diagonal elements
	// Compute the storage upper bandwidth needed for decomposition.
	// If decomposition is not used, set msu = mu
	unsigned int smu = std::min(n - 1, mu + ml);
	// Now create a banded matrix
	BandMatrix mat(n, mu, ml, smu);
	// or resize an existing matrix object
	mat.resize(n, mu, ml, msu);
	\endcode
*/
class BandMatrix {
public:
	/*! Default constructor (creates an empty matrix). */
	BandMatrix() : m_n(0), m_mu(0), m_ml(0), m_smu(0), m_colSize(0) {}

	/*! Constructor, see class documentation for description of parameters. */
	BandMatrix(unsigned int n, unsigned int mu, unsigned int ml, unsigned int smu) {
		resize(n, mu, ml, smu);
	}

	/*! Copy-Constructor. */
	BandMatrix(const BandMatrix & other) : m_n(0), m_mu(0), m_ml(0), m_smu(0), m_colSize(0)	
	{ *this=other; }

	/*! Assignment operator. */
	const BandMatrix & operator=(const BandMatrix & other) {
		if (m_n != other.m_n || m_mu != other.m_mu || m_ml != other.m_ml || m_smu != other.m_smu)
			resize(other.m_n, other.m_mu, other.m_ml, other.m_smu);
		// copy data from other matrix
		std::memcpy(m_data[0], other.m_data[0], sizeof(double)*m_n*m_colSize);
		// also copy pivots
		std::memcpy(&m_pivots[0], &other.m_pivots[0], sizeof(unsigned int)*m_n);
		return *this;
	}

	/*! Returns a reference to the element at the coordinates [i, j].
		\param i Matrix row, 0 <= i < n
		\param j Matrix column, 0 <= j < n
		\code
		A(i,j) = 6;
		\endcode
	*/
	double &  operator()(unsigned int i, unsigned int j) { return m_data[j][i-j+m_smu]; }

	/*! Returns a constant reference to the element at the coordinates [i, j].
		\param i Matrix row, 0 <= i < n
		\param j Matrix column, 0 <= j < n
		\code
		val = A.value(i,j);
		\endcode
		When an element is requested, that is not part of the
		matrix pattern the function returns a zero instead.
	*/
	double value(unsigned int i, unsigned int j) const {
		IBK_ASSERT(j < m_n);
		IBK_ASSERT(i < m_n);
		if (j+m_ml<i || j>i+m_mu)	return 0.0;
		else								return m_data[j][i-j+m_smu];
	}

	/*! Resizes the matrix (all data will be lost). */
	void resize(unsigned int n, unsigned int mu, unsigned int ml, unsigned int smu);

	/*! Returns the number of rows/cols in the matrix. */
	unsigned int  n() const { return m_n; }

	/*! Returns the upper band width. */
	unsigned int  mu() const { return m_mu; }

	/*! Returns the lower band width. */
	unsigned int  ml() const { return m_ml; }

	/*! Returns the extended upper band width. */
	unsigned int  smu() const { return m_smu; }

	/*! Clears the matrix. */
	void clear() {
		m_data.clear();
		m_dataStorage.clear();
		m_pivots.clear();
		m_n = m_mu = m_ml = m_smu = 0;
	}

	/*! Fills the matrix with zeros. */
	void setZero() { std::memset(m_data[0], 0, m_n*m_colSize*sizeof(double)); }
	/*! Returns the raw data. */
	std::vector<double>& data() { return m_dataStorage; }

	/*! Performs an in-place LU factorization of the matrix with partial pivoting.
		\return Returns 0 if successful.
			- If during the first elimination step a column is found to be completely zero
			(and thus also the pivot element), the column number (1..m_n) is returned.
			- If in the second step the pivot element becomes zero, the function returns the
			row of that element as value between m_n...2*m_n-1.
			- If the final pivot element in cell (m_n-1, m_n-1) is zero, the function returns 2*m_n.
	*/
	int lu();

	/*! Solves the equation system: Ax = b, using the 'pivots'
		from the factorization, the solution is stored in the vector 'b'.
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
	/*! Dimension of the matrix. */
	unsigned int m_n;
	/*! Size of the upper diagonal band (without center diagonal). */
	unsigned int m_mu;
	/*! Size of the lower diagonal band (without center diagonal). */
	unsigned int m_ml;
	/*! Size of the upper diagonal band including space for factorisation. */
	unsigned int m_smu;
	/*! Number of elements per storage column. */
	unsigned int m_colSize;
	/*! This vector of size n contains pointers to the individual columns of the matrix.
		The first element points to the first column, and thus also to the begin
		of the whole linear memory array of size n*m_colSize.
	*/
	std::vector<double*>		m_data;
	/*! This vector holds the actual data. */
	std::vector<double>			m_dataStorage;
	/*! Pivot vector for lu() and backsolve() functions. */
	std::vector<unsigned int>	m_pivots;
};
// ---------------------------------------------------------------------------


} // namespace IBKMK

#endif // IBKMK_BandMatrixH
