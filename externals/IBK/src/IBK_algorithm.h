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

#ifndef IBK_algorithmH
#define IBK_algorithmH

#include <algorithm>
#include <string>
#include <sstream>
#include <map>

#include "IBK_configuration.h"
#include "IBK_messages.h"
#include "IBK_Exception.h"

namespace IBK {

/*! This algorithm copies only those elements of the range 'first','last' to the range
	starting with 'result' if the unary predicate pred(element) yields true.
*/
template <typename inIt, typename outIt, typename predicate>
outIt copy_if(inIt first, inIt last, outIt result, predicate pred) {
	while (first!=last) {
		if (pred(*first))  *result++ = *first;
		++first;
	}
	return result;
}


/*! This algorithm copies only those elements of the range 'first','last' to the range
	starting with 'result' if they are equal to 'value'.
*/
template <typename inIt, typename outIt, typename T>
outIt copy_if_equal(inIt first, inIt last, outIt result, const T& value) {
	while (first!=last) {
		if (*first == value)  *result++ = *first;
		++first;
	}
	return result;
}


/*! This algorithm extracts the iterators to those elements of the range
	'first', 'last', who are fulfilling the unary predicate pred.
	Remember that depending on the container type you are using the copied
	iterators are invalidated when the number of elements in the container
	changes. Therefore this function should not be used to create permanent
	index tables.
*/
template <typename inIt, typename outIt, typename predicate>
outIt iterator_copy_if(inIt first, inIt last, outIt result, predicate pred) {
	while (first != last) {
		if (pred(*first))  *result++ = first;
		++first;
	}
	return result;
}


/*! Function object that cuts a slice of arrays/vectors within an array/container.
	When used in a transform algorithm, you can copy every stride value
	from the input vectors to the target vector, as in the following example.
	\code
	std::vector< std::vector<double> > srcvec;
	// ... populate srcvec with values ...

	// Target vector, must have the same size as srcvec
	std::vector<double> targetvec(srcvec.size());

	// we now copy the 5th element in each of the vectors in srcvec
	// to the target vector
	std::transform(srcvec.begin(), srcvec.end(),
				   targetvec.begin(),
				   IBK::strided_copy_iterator<std::vector<double> >(5) );
	\endcode

	The container must support the [] operator.
*/
template<typename container, typename reftype = typename container::const_reference>
class strided_copy_iterator {
public:
	/*! Constructor, takes the index of the element to copy from the inner vectors/arrays.
		\param stride The stride/offset to use for copying.
	*/
	explicit strided_copy_iterator(unsigned int stride) :
		m_stride(stride)
	{
	}

	/*! The actual operator, returns a const reference to the m_stride element in
		the vector passed to the function object.
		\param vec The inner container within the outer container that this function object operates on.
	*/
	reftype operator()(const container & vec) { return vec[m_stride]; }

	/*! The stride index. */
	unsigned int m_stride;
};


/*! Utility function for checking whether a map contains a certain key.
	\param mapType Map-like container that implements a fast find() member function.
	\param key The key to search for.
	\return Returns true if the container holds at least one element with the given key, otherwise false.

	\code
	std::map<std::string, A> mapOfA;

	// search for a given key in the map
	std::string key = "...";
	if (map_contains(mapOfA, key)) {

	}

	std::map<unsigned int, B> mapOfB;

	// call is just the same even though key is now an integer
	unsigned int id = 15;
	if (map_contains(mapOfB, id)) {

	}
	\endcode
*/
template <typename T>
bool map_contains(const T & mapType, const typename T::key_type & key) {
	return (mapType.find(key) != mapType.end());
}


/*! Utility function to retrieve an element from a constant std::map object.
	This function throws an IBK::Exception if an invalid key is being used.
	\param mapType Map-like container that implements a fast find() member function.
	\param key The key to search for.
	\return Returns the value associated with the key.

	\code
	std::map<std::string, A> mapOfA;
	std::string key = "...";

	const A & a = map_element(mapOfA, key);
	\endcode
	\note With C++11 you can use std::map::at() to do the same, however, with an std::out_of_range exception.
*/
template <typename T>
const typename T::value_type & map_element(const T & mapType, const typename T::key_type & key) {
	typename T::const_iterator it = mapType.find(key);
	if (it == mapType.end()) {
		throw IBK::Exception(IBK::FormatString("Cannot retrieve value for key '%1', "
											   "map doesn't contain such a key!").arg(key), "[IBK::map_element]");
	}
	return it->second;
}


/*! Utility function for checking removes content of b from a and stores it in a again.
	\param A Container (a set) holding values to be filtered.
	\param B Container (a set) holding values that should be removed from container A.

	\code
	int a[] = {0,1,2,3};
	int b[] = {1,3};
	std::set< int > setA.insert( a, a+4 );
	std::set< int > setB.insert( b, b+2 );

	remove_B_from_A( setA, setB );

	// setA -> {0,2}
	\endcode

	\warning Only works on containers that implement the erase(*iter) function.
*/
template <typename T>
void remove_B_from_A( T & A, const T & B) {
	for (typename T::const_iterator iter = B.begin(), end = B.end(); iter != end; ++iter)
		A.erase( *iter );
}


/*! This algorithm advances an iterator using the std::advance algorithm
	and returns the advanced iterator.
	This is essentially a convenience function that allow usage like:
	\code
	// iter is an iterator

	// Access value at position 5 after the current iterator position
	// but do not modify iter!
	value = *IBK::advance_iterator(iter, 5);
	\endcode

	\param it Iterator pointing to starting location.
	\param distance Steps to advance iterator.
	\return Returns iterator to advanced position.
*/
template <typename inIt>
inIt advance_iterator(inIt it, std::size_t distance) {
	std::advance(it, distance);
	return it;
}



// The algorithms below have all special requirements on the container elements, mostly
// in form of necessary member functions.


/*! This algorithm calls the clear() member function in all elements of the
	given range.
	Requires member function clear() to be implemented in container element type.

	\warning Include declaration of container element type before IBK_algorithm.h
*/
template <typename Iterator>
void clear(Iterator first, const Iterator & last) {
	while (first != last)
		(first++)->clear();
}


/*! Algorithm that checks if all elements in a container are empty.
	Calls empty() in all elements of the vector returns false as soon as one
	function evalualtes to false

	Requires member function empty() to be implemented in container element type.

	\warning Include declaration of container element type before IBK_algorithm.h
*/
template <typename Iterator>
bool all_empty(Iterator first, const Iterator & last) {
	bool empty = true;
	while (first != last)
		empty &= (first++)->empty();
	return empty;
}


/*! For each element in the range [first,last] a new element is appened to the
	container 'cont' and the member function read() is called with the element
	as parameter.
	Remember to include the type used in the container before including IBK_algorithm.h.
*/
template <typename const_iterator, typename container>
void read_range(const_iterator first, const_iterator last, container& cont) {
	while (first!=last) {
		typename container::value_type t;  // create new element
		try {
			t.read(*first++);         // read data from string, might throw
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading entry %1!")
				.arg(static_cast<unsigned int>(cont.size() + 1)), "[read_range]" );
		}
		cont.push_back(t);        // if successful add to container
	}
}


/*! For each element in the range [first,last] a new element is appended to the
	container 'cont' and the member function read() is called with the element
	as parameter.
	Remember to include the type used in the container before including IBK_algorithm.h.
*/
template <typename XMLElementType, typename container>
void read_range_XML(const XMLElementType * element, container& cont) {

	for (const XMLElementType * e = element; e; e = e->NextSiblingElement()) {
		typename container::value_type t;  // create new element
		try {
			t.readXML(e);         // read data from XML element, might throw
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading entry %1! Please check line: %2.")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(e->Row()), "[read_range_XML]" );
		}
		catch (std::exception & ex2) {
			throw IBK::Exception(IBK::FormatString("Error reading entry %1: %2 Please check line: %3.")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(ex2.what()).arg(e->Row()), "[read_range_XML]" );
		}
		cont.push_back(t);        // if successful add to container
	}
}


/*! For each child element of 'element' a new container element is put into the
	map by its id and the member function readXML() is called with the element
	as parameter.
	Requires that the map value type has a public unsigned int member m_id and a public
	member function readXML().
*/
template <typename XMLElementType, typename valueType>
void read_range_XML(const XMLElementType * element, std::map<unsigned int, valueType>& cont) {

	for (const XMLElementType * e = element; e; e = e->NextSiblingElement()) {
		valueType t;  // create new element
		try {
			t.readXML(e);         // read data from XML element, might throw
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading entry %1: Please check line: %2!")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(e->Row()), "[read_range_XML]" );
		}
		catch (std::exception & ex2) {
			throw IBK::Exception(IBK::FormatString("Error reading entry %1: %2 Please check line: %3!")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(ex2.what()).arg(e->Row()), "[read_range_XML]" );
		}
		// check if an element with this ID exists already
		if (map_contains(cont, t.m_id))
			throw IBK::Exception(IBK::FormatString("Error reading entry %1, an element with ID %2 "
												   "was already previously defined. Please check line: %3!")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(t.m_id).arg(e->Row()), "[read_range_XML]" );

		cont[ t.m_id ] = t;        // if successful add to container
	}
}


/*! For each child element of 'element' a new container element is put into the
	map by its id name and the member function readXML() is called with the element
	as parameter.
	Requires that the map value type has a public std::string member m_name and a public
	member function readXML().
*/
template <typename XMLElementType, typename valueType>
void read_range_XML(const XMLElementType * element, std::map<std::string, valueType>& cont) {

	for (const XMLElementType * e = element; e; e = e->NextSiblingElement()) {
		valueType t;  // create new element
		try {
			t.readXML(e);         // read data from XML element, might throw
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading entry %1! Please check line: %2.")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(e->Row()), "[read_range_XML]" );
		}
		catch (std::exception & ex2) {
			throw IBK::Exception(IBK::FormatString("Error reading entry %1: %2.  Please check line: %3.")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(ex2.what()).arg(e->Row()), "[read_range_XML]" );
		}
		// check if an element with this ID exists already
		if (map_contains(cont, t.m_name))
			throw IBK::Exception(IBK::FormatString("Error reading entry %1, an element with "
												   "Id name '%2' was already previously defined. Please check line: %3.")
								 .arg(static_cast<unsigned int>(cont.size() + 1)).arg(t.m_name).arg(e->Row()), "[read_range_XML]" );

		cont[ t.m_name ] = t;        // if successful add to container
	}
}


/*! Calls the member function write(ostream, int indent) in all elements in the
	range [first,last].
	Remember to include the declarations for iterator_traits<const_it>::value_type (or
	in other words the declaration of the type first and last are pointing to) before
	IBK_algorithm.h.
*/
template <class const_it>
void write_range(const_it first, const_it last, std::ostream& out, std::size_t indent) {
	while (first!=last)
		(first++)->write(out, static_cast<unsigned int>(indent));
}


/*! Calls the member function writeXML(element) in all elements in the range [first,last].
	Remember to include the declarations for iterator_traits<const_it>::value_type (or
	in other words the declaration of the type first and last are pointing to) before
	IBK_algorithm.h.
*/
template <class const_it, typename XMLElementType>
void write_range_XML(const_it first, const_it last, XMLElementType * element) {
	while (first!=last)
		(first++)->writeXML(element);
}



/*! \brief Appends indexes like '[1]' with increasing numbers to the basename until no
			matching names are no longer found in the given range.

	Example:
	\code
	string base_name = "temperature output";  // the base name
	vector<string>  existing_names;           // a vector with existing names
	// .. assume the vector is filled with values,
	// like "temperature output", "temperature output [1]", "relative humidity output"
	string new_name = pick_name(base_name, existing_names.begin(), existing_names.end());
	// new_name will be "temperature output [2]"
	\endcode
*/
template <typename iterator, typename const_iterator>
std::string pick_name(const std::string& basename, iterator first, const_iterator last) {
	if (first==last) return basename;
	std::string basename_mod = basename;
	std::string name;
	// remove [] if present
	size_t pos = basename_mod.rfind('[');
	while (pos != std::string::npos && pos > 0 && basename_mod[pos-1] == ' ') --pos;
	basename_mod = basename_mod.substr(0, pos);
	int i=0;
	do {
		std::stringstream namestrm;
		if (++i==1)  namestrm << basename_mod;
		else         namestrm << basename_mod << " [" << i << ']';
		name = namestrm.str();
	} while (std::find(first, last, name)!=last);
	return name;
}

} // namespace IBK

/*! \file IBK_algorithm.h
	\brief Contains different independent algorithms that enlarge the set of
		   algorithms that come with the Standard Template Library.

	\example algorithm.cpp
	This is an example of how to use the IBK algorithms.
*/

#endif // IBK_algorithmH
