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

#ifndef IBK_matrixH
#define IBK_matrixH

#include "IBK_configuration.h"

#include <vector>
#include <stdexcept>
#include <cmath>
#include <cstring>
#include "IBK_assert.h"
#include "IBK_FormatString.h"

namespace IBK {

/*! A matrix data container implementation.

	This is an implementation for a general matrix container which supports element access.
	Resizing is possible, see resize() for details.

	The internal data storage is implemented as vector of vectors, so the dimension of the matrix
	may be rather large.

	\note If you intend to implement/use math operation please check IBKMK::DenseMatrix.
*/
template <typename T>
class matrix {
  public:
	typedef T				value_type;         ///< The type of the values.
	typedef T&				reference;          ///< Reference to one element.
	typedef const T&		const_reference;    ///< Constant reference to one element.

	/*! Default construction.
		Creates an empty matrix.
	*/
	matrix() : rows_(0), cols_(0) {}
	/*! Initialisation constructor.
		Creates a matrix with the dimensions: cols x rows
	*/
	matrix(unsigned int cols, unsigned int rows);
	/*! Initialisation constructor with initial value.
		Creates a matrix with the dimensions: rows x cols and sets all elements to 'value'.
	*/
	matrix(unsigned int cols, unsigned int rows, const T& value);

	/*! Copy constructor for a matrix with an other type.
		\note Requires that a type conversion operation is defined from type R to type T.
	*/
	template <typename R> matrix(const matrix<R>& val);

#ifndef IBK_DEBUG
	/*! Returns a reference to the matrix element at [col][row].
		In debug mode it throws an out_of_range exception if the
		indexes are invalid.
	*/
	reference  operator() (unsigned int col, unsigned int row ) { return data_[row][col]; }
	/*! Returns a constant reference to the matrix element at [col][row].
		In debug mode it throws an out_of_range exception if the
		indexes are invalid.
	*/
	const_reference  operator() (unsigned int col, unsigned int row ) const { return data_[row][col]; }
#else
	reference  operator() (unsigned int col, unsigned int row ) { return at(col,row); }
	const_reference  operator() (unsigned int col, unsigned int row ) const { return at(col,row); }
#endif // IBK_DEBUG

	/*! Returns a reference to the matrix element at [col][row].
		It throws an IBK::Exception when the indexes are invalid.
	*/
	reference  at(unsigned int col, unsigned int row);

	/*! Returns a constant reference to the matrix element at [col][row].
		It throws an IBK::Exception exception when the indexes are invalid.
	*/
	const_reference  at(unsigned int col, unsigned int row) const;

	/*! Returns the number of rows in the matrix. */
	unsigned int rows() const { return rows_; }

	/*! Returns the number of columns in the matrix. */
	unsigned int cols() const { return cols_; }

	/*! Resizes the matrix (all data is lost). */
	void resize(unsigned int col, unsigned int row);

	/*! Swaps the content of the matrix with the content of another matrix 'mat'. */
	void swap(matrix<T>& mat);

	/*! Empties the matrix and sets its dimensions to 0. */
	void clear() { data_.clear(); rows_ = cols_ = 0; }

	/*! Fills the matrix with the given value. */
	void fill(const T& value);

	/*! Fills the matrix from another matrix.
		\todo Replace with assignment operator.
	*/
	template <typename R> void fill(const matrix<R>& rhs);

	/*! Sets the matrix values to zero. */
	void setZero();

	/*! Returns true is matrix is empty. */
	bool isEmpty() const { return data_.empty(); }

	/*! Reads a matrix from string in block format.
		\code
		string data =
			"1   4   5\n"
			"5   10  5\n"
			"3   5   3";
		m.read(data);
		\endcode
		Throws an IBK::Exception if reading fails.
	*/
	void read(const std::string& data);

	/*! Writes a matrix output.
		The matrix is dumped in the stream in columns separated with tabulators.
	*/
	std::ostream& write(std::ostream& out, unsigned int spacing=0) const;

private:
	std::vector<std::vector<T> >  data_;     // the main data member, stores data as [row][col]
	unsigned int     rows_;     // the rows of the matrix
	unsigned int     cols_;     // the columns of the matrix
};
// ---------------------------------------------------------------------------

// *** IMPLEMENTATIONS OF TEMPLATED MEMBER FUNCTIONS ***

template <typename T>
matrix<T>::matrix(unsigned int cols, unsigned int rows)
	: data_(rows), rows_(rows), cols_(cols)
{
	for (unsigned int i=0; i<rows_; ++i)
		data_[i].resize(cols_);
}

template <typename T>
matrix<T>::matrix(unsigned int cols, unsigned int rows, const T& value)
	: data_(rows), rows_(rows), cols_(cols)
{
	for (unsigned int i=0; i<rows_; ++i)
		data_[i].resize(cols_,value);
}

template <typename T>
template <typename R>
matrix<T>::matrix(const matrix<R>& val) {
	fill(val);
}

template <typename T>
inline typename matrix<T>::reference matrix<T>::at(unsigned int col, unsigned int row) {
	if (col>=cols_ || row>=rows_)
		throw IBK::Exception(
			FormatString("Matrix index [%1,%2] out of range! Matrix dimensions are [%3,%4].") .arg(col) .arg(row) .arg(cols_) .arg(rows_),
			"[IBK::matrix::at() ]");
	return data_[row][col];
}

template <typename T>
inline typename matrix<T>::const_reference  matrix<T>::at(unsigned int col, unsigned int row) const {
	if (col>=cols_ || row>=rows_)
		throw IBK::Exception(
			FormatString("Matrix index [%1,%2] out of range! Matrix dimensions are [%3,%4].") .arg(col) .arg(row) .arg(cols_) .arg(rows_),
			"[IBK::matrix::at() const]");
	return data_[row][col];
}

template <typename T>
void matrix<T>::resize(unsigned int cols, unsigned int rows) {
	cols_ = cols;
	rows_ = rows;
	data_.resize(rows_);
	for (unsigned int i=0; i<rows_; ++i)
		data_[i].resize(cols_);
}

template <typename T>
inline void matrix<T>::swap(matrix<T>& mat) {
	std::swap(cols_, mat.cols_);
	std::swap(rows_, mat.rows_);
	data_.swap(mat.data_);    // this is faster then normal swapping!
}

template <typename T>
void matrix<T>::fill(const T& value) {
	for (unsigned int i=0; i<rows_; ++i)
		std::fill(data_[i].begin(), data_[i].end(), value);
}

template <typename T>
template <typename R>
void matrix<T>::fill(const matrix<R>& rhs) {
	resize(rhs.cols(), rhs.rows());
	for (unsigned int i=0; i<rows(); ++i)
		std::copy(rhs[i].begin(), rhs[i].end(), data_[i].begin());
}

template <typename T>
void matrix<T>::setZero() {
	for (unsigned int i=0; i<rows_; ++i)
		std::memset(&data_[i][0], 0, cols_*sizeof(T));
}

template <typename T>
void matrix<T>::read(const std::string& data) {
	std::stringstream strm(data);
	std::string line;
	std::vector< std::vector<double> > vals;
	unsigned int count = 0;
	while (std::getline(strm, line)) {
		std::stringstream lstream(line);
		std::vector<double> linevals;
		double d;
		while (lstream >> d)
			linevals.push_back(d);
		if (linevals.empty()) break;
		vals.push_back(linevals);
		if (count == 0)
			count = linevals.size();
		else if (count != linevals.size()) {
			throw IBK::Exception("Different number of values in rows of data block.", "[matrix<T>::read]");
		}
	}
	// copy values into matrix
	resize(count, vals.size());
	for (unsigned int j=0; j<vals.size(); ++j)
		for (unsigned int i=0; i<count; ++i)
			at(i,j) = vals[j][i];
}

template <typename T>
std::ostream& matrix<T>::write(std::ostream& out, unsigned int spacing) const {
	for (unsigned int j=0; j<rows(); ++j) {
		for (unsigned int i=0; i<cols(); ++i)
			if (spacing == 0)   out << at(i,j) << '\t';
			else                out << std::setw(spacing) << at(i,j) << '\t';
		out << '\n';
	}
	return out;
}
// ----------------------------------------------------------------------------

} // namespace IBK

/*! \file IBK_matrix.h
	\brief Contains the class template matrix for general dense matrices of different data types.
*/

#endif // IBK_matrixH
