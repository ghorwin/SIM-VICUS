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

#ifndef IBK_matrix_viewH
#define IBK_matrix_viewH

#include "IBK_configuration.h"

#include "IBK_FormatString.h"
#include "IBK_assert.h"

namespace IBK {

/*! The class matrix_view is a container adapter providing a matrix view for
	linear array data.
	A matrix_view is not a container itself, it merely allows you to treat
	a linear data vector, such as the plain C array or the std::vector as a
	matrix. When creating a matrix view you simply have to specify the
	dimensions and a pointer to the start of the linear data storage.
	Note that creating and destructing a matrix view does not change the
	data in the storage at all.
*/
template <class T>
class matrix_view {
  public:
	typedef T           value_type;         ///< The data type of the values.
	typedef T&          reference;          ///< Reference to one element.
	typedef const T&    const_reference;    ///< Constant reference to one element.
	typedef T*          iterator;           ///< Iterator to one element.
	typedef const T*    const_iterator;     ///< Constant iterator to one element.

	/*! Constructor, creates a matrix view for a linear storage container.
		\param cols Number of colums of the matrix.
		\param rows Number of rows of the matrix.
		\param storage Pointer to the start of the data.
	*/
	matrix_view(std::size_t cols, std::size_t rows, T* storage)
		: cols_(cols), rows_(rows), storage_(storage) {}

	/*! Returns an iterator to the first element in the storage. */
	iterator        begin()         { return storage_; }
	/*! Returns a constant iterator to the first element in the storage. */
	const_iterator  begin() const   { return storage_; }
	/*! Returns an iterator to one past the last element in the storage. */
	iterator        end()           { return storage_+cols_*rows_; }
	/*! Returns a constant iterator to one past the last element in the storage. */
	const_iterator  end() const     { return storage_+cols_*rows_; }

#ifndef IBK_DEBUG

	/*! Returns a reference to the element at (col,row). */
	reference       operator() (std::size_t col, std::size_t row) { return storage_[row*cols_+col]; }
	/*! Returns a constant reference to the element at (col,row). */
	const_reference operator() (std::size_t col, std::size_t row) const { return storage_[row*cols_+col]; }

#else // IBK_DEBUG

	reference       operator() (std::size_t col, std::size_t row) {
		IBK_ASSERT(col<cols_);
		IBK_ASSERT(row<rows_);
		return storage_[row*cols_+col];
	}

	const_reference operator() (std::size_t col, std::size_t row) const {
		IBK_ASSERT(col<cols_);
		IBK_ASSERT(row<rows_);
		return storage_[row*cols_+col];
	}

#endif // IBK_DEBUG

	/*! Changes the dimensions of the matrix view and reassigns a new data storage. */
	void resize(std::size_t cols, std::size_t rows, T* storage);

	/*! Returns the number of columns in the matrix. */
	std::size_t     cols() const    { return cols_; }
	/*! Returns the number of rows in the matrix. */
	std::size_t     rows() const    { return rows_; }

  private:
	std::size_t cols_, rows_;
	T *storage_;
};
// ---------------------------------------------------------------------------

template <class T>
void matrix_view<T>::resize(std::size_t cols, std::size_t rows, T* storage) {
	cols_=cols;
	rows_=rows;
	IBK_ASSERT(storage != NULL);
	storage_=storage;
}

} // namespace IBK

/*! \file IBK_matrix_view.h
	\brief Contains the class template matrix_view for treating linear data arrays as 2D matrices.
*/

#endif // IBK_matrix_viewH
