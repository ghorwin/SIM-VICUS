/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_VectorValuedQuantityH
#define NM_VectorValuedQuantityH

#include <vector>
#include <set>

#include "NM_VectorValuedQuantityIndex.h"

namespace NANDRAD_MODEL {

/*!	This class wraps a std::vector and provides index/id lookup information.
	To inialize a VectorValuedQuantity use one of the constructor variants.

	\warning Neither for index or modelID key types it is required that
		the indexes/modelIDs in m_indexKeys vector are monotonically increasing or
		that there are no gaps in indexes! Do not make assumptions about index numbering,
		but rather use operator[] or insert() to access elements.
*/
class VectorValuedQuantity {
public:
	/*! Default C'tor, only to allow this class to be used in std::vector. */
	VectorValuedQuantity() {}

	/*! Constructor, creates a vector with 'n' elements, optional default value and keyType = IK_Index.
		Index key vector is populated with index values increasing from 0 to n-1.
	*/
	VectorValuedQuantity(unsigned int n, double value = 0);

	/*! Constructor, initializes vector with modelIndexes.size() elements, keyType = IK_ModelID and
		optional default value.
	*/
	VectorValuedQuantity(const std::vector<unsigned int> & modelIndexes, double value = 0);

	/*! Returns type of index. */
	VectorValuedQuantityIndex::IndexKeyType keyType() const	{ return m_keyType; }

	/*! Returns registered indexes (not necessarily monotonically increasing). */
	const std::vector<unsigned int> &indexKeys() const {return m_indexKeys; }

	/*! Returns a constant reference to the vector element with index key i.
		\warning Throws IBK::Exception if index is not in indexKey vector.
		\param i one of the index values in m_indexKeys
	*/
	const double & operator[] (unsigned int i) const;

	/*! Inserts a new index into the vector.
		If index 'i' already exists, functions works essentially as operator[](i).
		Missing index is inserted (for indeyKey IK_Index this can lead to not monotonously increasing indexes)
		and reference to element is returned.
		\warning This operation invalidates all previously returned references to values.
	*/
	const double & insert(unsigned int i);

	/*! Returns number of elements/size of vector. */
	unsigned int size() const { return m_data.size(); }

	/*! Returns an iterator to the start of the storage vector. */
	std::vector<double>::iterator begin() { return m_data.begin(); }

	/*! Return address of first element of vector. */
	double * dataPtr() { return &m_data[0]; }

	/*! Return address of first element of vector (const-version). */
	const double * dataPtr() const { return &m_data[0]; }

	/*! Return read-only access to vector. */
	const std::vector<double> & data() const { return m_data; }

private:
	/*! Stores the actual data. */
	std::vector<double>						m_data;
	/*! Key type (for stringv output). */
	VectorValuedQuantityIndex::IndexKeyType m_keyType;
	/*! Set of key values, size matches m_data.size().
		Values in m_indexKeys are interpreted according to m_keyType.
	*/
	std::vector<unsigned int>				m_indexKeys;
};

} // namespace NANDRAD_MODEL

#endif // NM_VectorValuedQuantityH
