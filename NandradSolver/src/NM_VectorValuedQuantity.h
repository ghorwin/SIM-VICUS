/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef VectorValuedQuantityH
#define VectorValuedQuantityH

#include <set>
#include <IBK_UnitVector.h>
#include "NM_VectorValuedQuantityIndex.h"

namespace NANDRAD_MODEL {

/*!	\brief Declaration for class SortedUnitList.
	This class is an extension of the unit list containing index information.
*/
class VectorValuedQuantity: public IBK::UnitVector {
public:

	explicit VectorValuedQuantity(unsigned int n);

	/*! Constructor (creates a vector with 'size' elements and the IO-unit 'ioUnit') and keyType.*/
	VectorValuedQuantity(unsigned int n, IBK::Unit IO_unit, VectorValuedQuantityIndex::IndexKeyType keyType);

	/*! Constructor (creates a vector with 'size' elements, initial values 'value' and the IO-unit 'ioUnit') and keyType. */
	VectorValuedQuantity(unsigned int n, double value, IBK::Unit IO_unit, VectorValuedQuantityIndex::IndexKeyType keyType);

	/*! Constructor (creates a vector with 0 elements, a name 'name' and the IO-unit 'ioUnit'). For an empty vector it is always
		possible to chose an abitrary key type.*/
	VectorValuedQuantity(const std::string & name, IBK::Unit IO_unit);

	/*! Constructor (creates a vector with 0 elements, a name 'name' and the IO-unit 'ioUnit') for the case we do
		not want to chose classical index notation. Therefore, for a given vector size we need
		to specify all index keys.*/
	VectorValuedQuantity(const std::string & name, IBK::Unit IO_unit, VectorValuedQuantityIndex::IndexKeyType keyType,
		const std::set<unsigned int> &indexKeys);

	VectorValuedQuantityIndex::IndexKeyType keyType() const	{ return m_keyType; }

	const std::set<unsigned int> &indexKeys() const	{return m_indexKeys; }

	/*! Resizes content vector (provided for convenience) with
	sequentual index keys starting from 0,1,... and ending with n. */
	void resize(unsigned int n, VectorValuedQuantityIndex::IndexKeyType keyType);

	/*! Resizes content vector with given index keys and key type. */
	void resize(const std::set<unsigned int> &indexKeys, VectorValuedQuantityIndex::IndexKeyType keyType);

	/*! Request whether a special key index exists or not. */
	std::vector<double>::iterator find(unsigned int i);

	/*! Request whether a special key index exists or not. */
	std::vector<double>::const_iterator find(unsigned int i) const;

	/*! Begin iterator for loops. */
	std::vector<double>::iterator begin() { return m_data.begin(); }

	/*! Begin iterator for loops. */
	std::vector<double>::const_iterator begin() const { return m_data.begin(); }

	/*! End iterator to identify invalid results of the find operation. */
	std::vector<double>::iterator end() { return m_data.end(); }

	/*! End iterator to identify invalid results of the find operation. */
	std::vector<double>::const_iterator end() const { return m_data.end(); }

	/*! Reverse begin iterator for loops. */
	std::vector<double>::reverse_iterator rbegin() { return m_data.rbegin(); }

	/*! Reverse begin iterator for loops. */
	std::vector<double>::const_reverse_iterator rbegin() const { return m_data.rbegin(); }

	/*! Reverse end iterator to identify invalid results of the find operation. */
	std::vector<double>::reverse_iterator rend() { return m_data.rend(); }

	/*! Reverse end iterator to identify invalid results of the find operation. */
	std::vector<double>::const_reverse_iterator rend() const { return m_data.rend(); }
	/*! Returns a reference to the vector element with index key i.
		In debug mode it throws an out_of_range exception if the
		indices are invalid.
	*/
	double & operator[] (unsigned int i);

	/*! Returns a constant reference to the vector element with index key i.
		In debug mode it throws an out_of_range exception if the
		indices are invalid.
	*/
	const double & operator[] (unsigned int i) const;

private:
	/*! Key type (for stringv output). */
	VectorValuedQuantityIndex::IndexKeyType m_keyType;
	/*! Set of key values. */
	std::set<unsigned int>					m_indexKeys;
};

} // namespace NANDRAD_MODEL

#endif // VectorValuedQuantityH
