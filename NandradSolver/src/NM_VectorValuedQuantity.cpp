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

#include "NM_DefaultModel.h"

#include <NANDRAD_KeywordList.h>

#include <limits>

#include <IBK_assert.h>

namespace NANDRAD_MODEL {

VectorValuedQuantity::VectorValuedQuantity(unsigned int n, IBK::Unit IO_unit,
	VectorValuedQuantityIndex::IndexKeyType keyType) :
	IBK::UnitVector(n, IO_unit),
	m_keyType(keyType) {
	for(unsigned int i = 0; i < n; ++i)
	{
		m_indexKeys.insert(i);
	}
}

VectorValuedQuantity::VectorValuedQuantity(unsigned int n, double value, IBK::Unit IO_unit,
	VectorValuedQuantityIndex::IndexKeyType keyType) :
	IBK::UnitVector(n, value, IO_unit),
	m_keyType(keyType) {
	for(unsigned int i = 0; i < n; ++i)
	{
		m_indexKeys.insert(i);
	}
}

VectorValuedQuantity::VectorValuedQuantity(const std::string & name, IBK::Unit IO_unit) :
	IBK::UnitVector(name, IO_unit),
	m_keyType(VectorValuedQuantityIndex::IK_Index) {
}

VectorValuedQuantity::VectorValuedQuantity(const std::string & name, IBK::Unit IO_unit,
	VectorValuedQuantityIndex::IndexKeyType keyType, const std::set<unsigned int> &indexKeys) :
	IBK::UnitVector(name, IO_unit),
	m_keyType(keyType),
	m_indexKeys(indexKeys) {
	IBK::UnitVector::resize((unsigned int)indexKeys.size());
}

void VectorValuedQuantity::resize(unsigned int n,
	VectorValuedQuantityIndex::IndexKeyType keyType) {
	IBK::UnitVector::resize(n);
	m_keyType = keyType;
	for(unsigned int i = 0; i < n; ++i)
	{
		m_indexKeys.insert(i);
	}
}

void VectorValuedQuantity::resize(const std::set<unsigned int> &indexKeys,
	VectorValuedQuantityIndex::IndexKeyType keyType) {
	IBK::UnitVector::resize((unsigned int)indexKeys.size());
	m_keyType = keyType;
	m_indexKeys = indexKeys;
}

std::vector<double>::iterator VectorValuedQuantity::find(unsigned int i) {
	// debug check: have data and key vectors the same size
	IBK_ASSERT(m_data.size() == m_indexKeys.size() );
	// find index element
	std::set<unsigned int>::iterator it = m_indexKeys.find(i);
	// element undefined
	if(it == m_indexKeys.end() )
		return m_data.end();
	// calculate vector position
	int index = (int)std::distance(m_indexKeys.begin(), it);
	IBK_ASSERT(index < (int) m_data.size() );
	// construct a data iterator
	std::vector<double>::iterator dataIt = m_data.begin() + index;
	return dataIt;
}

std::vector<double>::const_iterator VectorValuedQuantity::find(unsigned int i) const{
	// debug check: have data and key vectors the same size
	IBK_ASSERT(m_data.size() == m_indexKeys.size() );
	// find index element
	std::set<unsigned int>::const_iterator it = m_indexKeys.find(i);
	// element undefined
	if(it == m_indexKeys.end() )
		return m_data.end();
	// calculate vector position
	int index = (int)std::distance(m_indexKeys.begin(), it);
	IBK_ASSERT(index < (int) m_data.size() );
	// construct a data iterator
	std::vector<double>::const_iterator dataIt = m_data.begin() + index;
	return dataIt;
}


double & VectorValuedQuantity::operator[] (unsigned int i) {
	// debug check: have data and key vectors the same size
	IBK_ASSERT(m_data.size() == m_indexKeys.size() );
	// find the corresponding index or insert a new one if the index does not exist already
	std::pair<std::set<unsigned int>::iterator,bool> it = m_indexKeys.insert(i);
	bool insertedNew = it.second;
	// calculate the vector position of the requested element
	int index = (int)std::distance(m_indexKeys.begin(), it.first);
	// the element exists already
	if(!insertedNew)
	{
		// choose the IBK::UnitVector function for vector access
		return IBK::UnitVector::operator[](index);
	}
	// a new element was inserted
	else {
		IBK_ASSERT(index < (int) m_data.size() );
		// add a new element to unit vector
		std::vector<double>::iterator dataIt = m_data.insert(m_data.begin() + index, 0.0);
		// and return its data
		return *dataIt;
	}
}

const double & VectorValuedQuantity::operator[] (unsigned int i) const {
	// debug check: have data and key vectors the same size
	IBK_ASSERT(m_data.size() == m_indexKeys.size() );
	// find index element
	std::set<unsigned int>::const_iterator it = m_indexKeys.find(i);
	// calculate vector position
	int index = (int)std::distance(m_indexKeys.begin(), it);
	// choose the IBK::UnitVector function for vector access
	return IBK::UnitVector::operator[](index);
}

} // namespace NANDRAD_MODEL

