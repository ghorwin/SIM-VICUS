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

#ifndef IBKMK_TridiagMatrixH
#define IBKMK_TridiagMatrixH


#include <vector>
#include <stdexcept>
#include <algorithm>
#include <ostream>


#include <IBK_configuration.h>
#include <IBK_assert.h>
#include <IBK_FormatString.h>
#include <IBK_Exception.h>

namespace IBKMK {

/*! The class TridiagMatrix stores the data of a tridiagonal matrix while allowing easy and
	convenient access to the elements.

	The data of the matrix is stored in a continuous memory array, with the memory layout: lower band, main band,
	upper band. The elements (1,3) and (n,n-2) are used by the lu() and backsolving routines but not by
	the multiply routines.
*/
class TridiagMatrix {
public:
	/*! Default constructor (creates an empty matrix). */
	TridiagMatrix() : m_n(0) {}
	/*! Constructor (creates a matrix of dimension n). */
	explicit TridiagMatrix(unsigned int n) : m_n(n), m_data(3*n) {}
	/*! Constructor (creates a matrix of dimension n and initialises all elements with the value 'value'). */
	TridiagMatrix(unsigned int n, const double value) : m_n(n), m_data(3*n, value) {}
	/*! Copy constructor. */
	TridiagMatrix(const TridiagMatrix & other) : m_n(0) { *this = other; }
	/*! Assignment operator. */
	const TridiagMatrix & operator=(const TridiagMatrix & b) {
		if (m_n == b.m_n)
			std::copy(b.m_data.begin(), b.m_data.end(), m_data.begin());
		else {
			m_data = b.m_data;
			m_n = b.m_n;
		}
		return *this;
	}

	/*! Returns a reference to the element at the coordinates [i, j].
		\param i Matrix row, 0 <= i < n
		\param j Matrix column, 0 <= j < n
		\code
		A(i,j) = 6;
		\endcode
	*/
	double & operator()(unsigned int i, unsigned int j) {
		IBK_ASSERT(j < m_n);
		IBK_ASSERT(i < m_n);
		IBK_ASSERT(int(i - j) >= -1 && int(i - j) <= 1);
		return m_data[i + m_n*(j-i+1)];
	}

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
		int diff=static_cast<int>(j-i);
		if (diff==-1)   return m_data[i];
		if (diff==0)    return m_data[i + m_n];
		if (diff==1)    return m_data[i + 2*m_n];
		return 0.0;
	}

	/*! Sets all elements of the lower band (and element L[0] = A(2,0) = 0). */
	void setLower(const double & value)  { std::fill(lower(), lower()+m_n, value); lower()[0] = 0; }
	/*! Sets all elements of the main band. */
	void setMain(const double & value)   { std::fill(main(),  main()+m_n, value); }
	/*! Sets all elements of the upper band  (and element U[n-1] = A(n-3,n-1) = 0). */
	void setUpper(const double & value)  { std::fill(upper(), upper()+m_n, value); upper()[m_n-1] = 0; }

	/*! Returns a pointer to the start of the lower band
		(mind that it starts in row 0, even though lower[0] is not used). */
	double *            lower()  { return &m_data[0]; }
	/*! Returns a pointer to the start of the lower band (const version)
		(mind that it starts in row 0, even though lower[0] is not used). */
	const double *      lower()  const { return &m_data[0]; }
	/*! Returns a pointer to the start of the main band. */
	double *            main()   { return &m_data[0] +   m_n; }
	/*! Returns a pointer to the start of the main band (const version). */
	const double *      main()   const { return &m_data[0] +   m_n; }
	/*! Returns a pointer to the start of the upper band. */
	double *            upper()  { return &m_data[0] + 2*m_n; }
	/*! Returns a pointer to the start of the upper band (const version). */
	const double *      upper()  const { return &m_data[0] + 2*m_n; }

	/*! Adds a number to all elements of the matrix. */
	const TridiagMatrix & operator+=(const double & value) {
		double * p = &m_data[0];
		for (unsigned int i=0; i<m_data.size(); ++i)
			p[i] += value;
		return *this;
	}
	/*! Multiplies all elements of the matrix with a number. */
	const TridiagMatrix & operator*=(const double & value)  {
		double * p = &m_data[0];
		for (unsigned int i=0; i<m_data.size(); ++i)
			p[i] *= value;
		return *this;
	}

	/*! Resizes the matrix (all data will be lost). */
	void resize(unsigned int n) { m_data.resize(3*n); m_n=n; }
	/*! Returns the number of rows/columns in the matrix. */
	unsigned int  n() const { return m_n; }
	/*! Clears the matrix. */
	void clear() { m_data.clear(); m_n=0; }

	/*! Adds the matrix 'other' to this matrix. */
	TridiagMatrix & operator+=(const TridiagMatrix & other) {
		std::transform(m_data.begin(), m_data.end(), other.m_data.begin(),
					   m_data.begin(), std::plus<double>() );
		return *this;
	}


	/*! Solves the linear system of equations A*x = b and stores the resultant x vector
		in the place of vector b.
		In this solver routine the tridiagonal matrix is changed because this algorithm
		works in-place. The advantage is that it does not require additional workspace
		and is the fastest code (on single processor machines) without using vectorization
		(which is not supported for C++ compilers anyway, yet).
		This function modifies the matrix.
	*/
	void solveEquationSystem(double * b);

	/*! Solves the linear system of equations A*x = b and stores the resultant x vector
		in the place of vector b.
		This function works essentially like the function above, but includes error checking.
		\return The return codes are as follows:
		* Return value = 0 : All ok
		* Return value > 0 : Error - Zero element in main diagonal in row value-1
		* Return value -1 : Error - Matrix is singular or becomes singular by transformation
	*/
	int solveEquationSystemChecked(double * b);

	/*! Performs an in-place LU factorization.
		The parts of the L and U triangular matricies are stored in place of the original matrix.
		Once the LU factorization is complete, you can use backsolve() to solve linear systems of
		equations.
		\sa backsolve
	*/
	int lu();

	/*! Backsolving of an equation system Ax = b with A = LU stored in this matrix.
		The matrix is not modified.
		Call lu() prior to using this function on the original matrix A. Once the LU
		factorization is done, you can call backsolve() many times with different
		solution vectors.
		\sa lu()
		\param b The right-hand-side vector b (size n).
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
	unsigned int m_n;			///< Dimension of the matrix.
	/*! Holds the data of the three diagonals in a continuous memory array.
		The data is stored using the following order lower, main, upper band.
		The first element in the lower band and the last element in the upper band are
		not used.
	*/
	std::vector<double> m_data;
};

} // namespace IBKMK

#endif // IBKMK_TridiagMatrixH
