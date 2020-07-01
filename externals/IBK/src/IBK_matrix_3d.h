/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

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


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_matrix_3dH
#define IBK_matrix_3dH

#include "IBK_configuration.h"

#include <vector>
#include <stdexcept>
#include <cmath>
#include <cstring>

#include "IBK_assert.h"
#include "IBK_FormatString.h"

namespace IBK {

/*! A simple 3d matrix implementation.

	This is an implementation for a data matrix and supports element access.
	Resizing is possible, see resize() for details.

	\note If you intend to implement/use math operation please check IBKMK::DenseMatrix.
*/
template <typename T>
class matrix_3d {
public:
	typedef T					value_type;			///< The type of the values.
	typedef T&					reference;			///< Reference to one element.
	typedef const T&			const_reference;	///< Constant reference to one element.

	/*! Default construction.
		Creates an empty matrix.
	*/
	matrix_3d() : m_rows(0), m_cols(0), m_stacks(0) {}

	/*! Initialisation constructor.
		Creates a matrix with the dimensions: cols x rows
	*/
	matrix_3d( unsigned int cols, unsigned int rows, unsigned int stacks = 0 );

	/*! Initialisation constructor with initial value.
		Creates a matrix with the dimensions: rows x cols and sets all elements to 'value'.
	*/
	matrix_3d( unsigned int cols, unsigned int rows, unsigned int stacks, const T& value);

	/*! Copy constructor for a matrix with an other type. */
	template <typename R> matrix_3d(const matrix_3d<R>& val);

#ifndef IBK_DEBUG

	/*! Returns a reference to the matrix element at [col][row].
		In debug mode it throws an IBK::Exception if the
		indexes are invalid.
	*/
	reference  operator() (unsigned int col, unsigned int row, unsigned int stack ) { return m_data[stack][row][col]; }

	/*! Returns a constant reference to the matrix element at [col][row].
		In debug mode it throws an IBK::Exception if the
		indexes are invalid.
	*/
	const_reference  operator() (unsigned int col, unsigned int row, unsigned int stack ) const { return m_data[stack][row][col]; }

#else

	reference  operator() (unsigned int col, unsigned int row, unsigned int stack ) { return at(col,row, stack); }

	const_reference  operator() (unsigned int col, unsigned int row, unsigned int stack ) const { return at(col,row, stack); }

#endif // IBK_DEBUG

	/*! Returns a reference to the matrix element at [col][row].
		It throws an IBK::Exception exception when the indexes are invalid.
	*/
	reference  at(unsigned int col, unsigned int row, unsigned int stack);

	/*! Returns a constant reference to the matrix element at [col][row].
		It throws an IBK::Exception when the indexes are invalid.
	*/
	const_reference  at(unsigned int col, unsigned int row, unsigned int stack) const;

	/*! Returns the number of rows in the matrix. */
	unsigned int rows() const { return m_rows; }

	/*! Returns the number of columns in the matrix. */
	unsigned int cols() const { return m_cols; }

	/*! Returns the number of columns in the matrix. */
	unsigned int stacks() const { return m_stacks; }

	/*! Resizes the matrix (all data is lost). */
	void resize(unsigned int col, unsigned int row, unsigned int stacks);

	/*! Swaps the content of the matrix with the content of another matrix 'mat'. */
	void swap(matrix_3d<T>& mat);

	/*! Empties the matrix and sets its dimensions to 0. */
	void clear() { m_data.clear(); m_rows = m_cols = m_stacks = 0; }

	/*! Fills the matrix with the given value. */
	void fill(const T& value);

	/*! Fills the matrix from another matrix.
		\todo Replace with assignment operator.
	*/
	template <typename R> void fill(const matrix_3d<R>& rhs);

	/*! Sets the matrix values to zero. */
	void setZero();

	/*! Returns true is matrix is empty. */
	bool isEmpty() const { return m_data.empty(); }

private:

	std::vector< std::vector< std::vector<T> > >	m_data;		///< the main data member, stores data as [stack][row][col]
	unsigned int									m_rows;		///< the rows of the matrix
	unsigned int									m_cols;		///< the columns of the matrix
	unsigned int									m_stacks;	///< the stack size of the matrix
};
// ---------------------------------------------------------------------------


// *** IMPLEMENTATIONS OF TEMPLATED MEMBER FUNCTIONS ***

template <typename T>
matrix_3d<T>::matrix_3d(unsigned int cols, unsigned int rows, unsigned int stacks):
	m_data(stacks),
	m_rows(rows),
	m_cols(cols),
	m_stacks(stacks)
{
	for (unsigned int i=0; i<stacks; ++i){
		m_data[i].resize(rows);
		for (unsigned int j=0; j<rows;++j)
			m_data[i][j].resize(cols);
	}
}

template <typename T>
matrix_3d<T>::matrix_3d(unsigned int cols, unsigned int rows, unsigned int stacks, const T& value):
	m_data(stacks),
	m_rows(rows),
	m_cols(cols),
	m_stacks(stacks)
{
	for (unsigned int i=0; i<stacks; ++i){
		m_data[i].resize(rows);
		for (unsigned int j=0; j<rows;++j)
			m_data[i][j].resize( cols, value );
	}
}

template <typename T>
template <typename R>
matrix_3d<T>::matrix_3d(const matrix_3d<R>& val) {
	fill(val);
}



template <typename T>
inline typename matrix_3d<T>::reference matrix_3d<T>::at(unsigned int col, unsigned int row, unsigned int stack) {

	if (col>=m_cols || row>=m_rows|| stack>=m_stacks)
		throw IBK::Exception( FormatString("Matrix index [%1,%2,%3] out of range! Matrix dimensions are [%4,%5,%6].")
							  .arg(col) .arg(row) .arg(stack) .arg(m_cols) .arg(m_rows) .arg(m_stacks),
							  "[IBK::matrix_3d::at()]");
	return m_data[stack][row][col];

}

template <typename T>
inline typename matrix_3d<T>::const_reference  matrix_3d<T>::at(unsigned int col, unsigned int row, unsigned int stack ) const {

	if (col>=m_cols || row>=m_rows|| stack>=m_stacks)
		throw IBK::Exception( FormatString("Matrix index [%1,%2,%3] out of range! Matrix dimensions are [%4,%5,%6].")
							  .arg(col) .arg(row) .arg(stack) .arg(m_cols) .arg(m_rows) .arg(m_stacks),
							  "[IBK::matrix_3d::at() const]");
	return m_data[stack][row][col];

}

template <typename T>
void matrix_3d<T>::resize(unsigned int cols, unsigned int rows, unsigned int stacks) {

	m_cols = cols;
	m_rows = rows;
	m_stacks = stacks;
	m_data.resize(m_stacks);
	for (unsigned int i=0; i<m_stacks; ++i){
		m_data[i].resize(m_rows);
		for (unsigned int j=0; j<m_rows; ++j)
			m_data[i][j].resize(m_cols);
		}

}

template <typename T>
inline void matrix_3d<T>::swap(matrix_3d<T>& matrix) {

	std::swap(m_cols, matrix.m_cols);
	std::swap(m_rows, matrix.m_rows);
	std::swap(m_stacks, matrix.m_stacks);
	// this is faster then normal swapping!
	m_data.swap(matrix.m_data);

}

template <typename T>
void matrix_3d<T>::fill(const T& value) {
	for (unsigned int i=0; i<m_stacks; ++i)
		for (unsigned int j=0; j<m_rows; ++j)
			std::fill(m_data[i][j].begin(), m_data[i][j].end(), value);
}

template <typename T>
template <typename R>
void matrix_3d<T>::fill(const matrix_3d<R>& rhs) {

	resize( rhs.cols(), rhs.rows(), rhs.stacks() );
	for (unsigned int i=0; i<stacks(); ++i)
		for (unsigned int j=0; j<rows(); ++j)
			std::copy(rhs[i][j].begin(), rhs[i][j].end(), m_data[i][j].begin());

}

template <typename T>
void matrix_3d<T>::setZero() {

	for (unsigned int i=0; i<m_stacks; ++i)
		for (unsigned int j=0; j<m_rows; ++j)
			std::memset(&m_data[i][j][0], 0, m_cols*sizeof(T));

}

} // namespace IBK

/*! \file IBK_matrix_3d.h
	\brief Contains the class template matrix for 3D data storage matrices.
*/

#endif // IBK_matrix_3dH
