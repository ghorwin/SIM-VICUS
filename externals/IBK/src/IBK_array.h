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

#ifndef IBK_arrayH
#define IBK_arrayH

#include "IBK_configuration.h"

#include <stdexcept>
#include <algorithm>
#include "IBK_FormatString.h"             // for FormatString()
#include "IBK_Exception.h"

namespace IBK {

/*! The class template IBK::array is a replacement for the C-Array and provides
	a certain level of STL support.
	The array can be used with standard C++ algorithms and should be used
	whenever the size of the array is known at compile time already. Since
	the array is created on the stack it will be faster than using the
	std::vector. In debug mode the operator[] will
	perform a range check and throw an exception if the index is out of range.
	Most of the IBK::array's is very similar to the BOOST:Array.
*/
template <typename T, std::size_t N>
class array {
  public:
	typedef T               value_type;         ///< The data type of the values in the array.
	typedef T*              iterator;           ///< Iterator type to elements.
	typedef const T*        const_iterator;     ///< Constant iterator type to elements.
	typedef T&              reference;          ///< Reference type to elements.
	typedef const T&        const_reference;    ///< Constant reference type to elements.
	typedef std::size_t     size_type;          ///< Size type used in the array.
	typedef std::ptrdiff_t  difference_type;    ///< Difference type used in the array.

	/*! Default constructor, creates an array whose elements are not initialised. */
	array() {}
	/*! Constructor which initialises all elements in the vector with the initial value 'value'. */
	explicit array(const T& value) { std::fill(begin(), end(), value); }
	/*! Constructor initializing the array with data from a range 'first' to 'last'. */
	array(const_iterator first, const_iterator last) { std::copy(first, last, data_); }
	/*! Returns a random access iterator to the first element. */
	iterator        begin()         { return data_; }
	/*! Returns a random access iterator to the first element (read only). */
	const_iterator  begin() const   { return data_; }
	/*! Returns a random access iterator to the position after the last element. */
	iterator        end()           { return data_+N; }
	/*! Returns a random access iterator to the position after the last element (read only). */
	const_iterator  end() const     { return data_+N; }

#ifndef IBK_DEBUG
	/*! Returns the element with the index 'i'
		In debug mode throws an IBK_exception exception if the index is out of range.
	*/
	reference       operator[](size_type i)         { return data_[i]; }
	/*! Returns the element with the index 'i' (read only)
		In debug mode throws an IBK_exception exception if the index is out of range.
	*/
	const_reference operator[](size_type i) const   { return data_[i]; }
#else
	reference       operator[](size_type i);
	const_reference operator[](size_type i) const;
#endif // IBK_DEBUG

	/*! Return the reference to the first element in the array. */
	reference       front()                         { return data_[0]; }
	/*! Return a const reference to the first element in the array. */
	const_reference front() const                   { return data_[0]; }
	/*! Return the reference to the last element in the array. */
	reference       back()                          { return data_[N-1]; }
	/*! Return a const reference to the last element in the array. */
	const_reference back() const                    { return data_[N-1]; }

	/*! Returns the size of the array. */
	static size_type size()                         { return N; }

	/*! Swaps the content of the array with the other array 'arr'. */
	void swap (array<T,N>& arr) {
		std::swap_ranges(begin(), end(), arr.begin());
	}

	/*! Returns a pointer to the storage. */
	const T* data() const                           { return data_; }

	/*! Assignment operator= with type conversion.
		The data type T2 must be convertible to T1 and the size of both arrays
		has to be equal. The function returns a reference to the array.
	*/
	template <typename T2>
	array<T,N>& operator=(const array<T2,N>& rhs) {
		std::copy(rhs.begin(),rhs.end(), begin());
		return *this;
	}

  private:
	T data_[N]; // C-Array as storage member
};

#ifdef IBK_DEBUG
template <typename T, std::size_t N>
typename array<T,N>::reference array<T,N>::operator[](size_type i) {
	if (i>=N)
		throw IBK::Exception(FormatString("Index %1 out of range [%2 .. %3].").arg(i).arg(0).arg(N-1), "[IBK::array::operator[] ]");
	return data_[i];
}

template <typename T, std::size_t N>
typename array<T,N>::const_reference array<T,N>::operator[](size_type i) const {
	if (i>=N)
		throw IBK::Exception(FormatString("Index %1 out of range [% .. %].").arg(i).arg(0).arg(N-1), "[IBK::array::operator[] const]");
	return data_[i];
}
#endif

}  // namespace IBK

/*! \file IBK_array.h
	\brief Contains the declaration of the template class array.

	\example array.cpp
	This small example demonstrates the use of the IBK::array in comparison to a normal C-Array.
*/

#endif // IBK_arrayH
