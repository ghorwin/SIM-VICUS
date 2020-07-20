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

#ifndef NM_QuantityDescriptionH
#define NM_QuantityDescriptionH

#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include <set>

#include <IBK_Unit.h>

#include <NANDRAD_CodeGenMacros.h>

#include "NM_VectorValuedQuantityIndex.h"

namespace NANDRAD_MODEL {

/*! Datatype returned by resultDescriptions() and inputReferenceDescriptions(). It contains
	definitions of result values and input references as name and physical unit. For
	vector valued results or input references to vector valued target quantities additional
	vector information (size, indices of occupied vector elements, description string) is provided.
*/
struct QuantityDescription {

	QuantityDescription() :
		m_minMaxValue(std::make_pair<double, double>
		(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max())),
		m_size(1)
	{}

	/*! Constructor for scalar valued quantities.*/
	QuantityDescription(const std::string & name, const std::string & unit, const std::string & description,
						const bool constant) :
		m_name(name),
		m_unit(unit),
		m_description(description),
		m_minMaxValue(std::make_pair<double, double>
		(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max())),
		m_constant(constant),
		m_size(1)
	{
	}

	/*! Constructor for vector valued quantities.*/
	QuantityDescription(const std::string & name, const std::string & unit, const std::string & description,
						const bool constant, const std::vector<VectorValuedQuantityIndex> &indexKeys,
						const std::vector<std::string> &indexKeyDescriptions) :
		m_name(name),
		m_unit(unit),
		m_description(description),
		m_minMaxValue(std::make_pair<double, double>
		(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max())),
		m_constant(constant),
		m_size((unsigned int) indexKeys.size()),
		m_indexKeys(indexKeys),
		m_indexKeyDescriptions(indexKeyDescriptions)
	{
	}

	std::vector<VectorValuedQuantityIndex>::iterator find(unsigned int index) {
		// resize index and index description vectors
		std::vector<unsigned int> idx;
		indexes(idx);

		std::vector<unsigned int>::const_iterator it =
			std::find(idx.begin(), idx.end(), index);
		// invalid element
		if (it == idx.end())
			return m_indexKeys.end();

		// extract iterator position
		int pos = (int) (it - idx.begin());
		return m_indexKeys.begin() + pos;
	}


	std::vector<VectorValuedQuantityIndex>::const_iterator find(unsigned int index) const {
		// resize index and index description vectors
		std::vector<unsigned int> idx;
		indexes(idx);

		std::vector<unsigned int>::const_iterator it =
			std::find(idx.begin(), idx.end(), index);
		// invalid element
		if (it == idx.end())
			return m_indexKeys.end();

		// extract iterator position
		int pos = (int) (it - idx.begin());
		return m_indexKeys.begin() + pos;
	}


	void resize(const std::set<unsigned int> &indexes, VectorValuedQuantityIndex::IndexKeyType keyType) {
		clear();
		// resize index and index description vectors
		for (std::set<unsigned int>::const_iterator
			it = indexes.begin();
			it != indexes.end(); ++it)
		{
			VectorValuedQuantityIndex indexKey(keyType, *it);
			m_indexKeys.push_back(indexKey);
			m_indexKeyDescriptions.push_back(indexKey.toString());
		}
		m_size = indexes.size();
	}

	void resize(unsigned int size) {
		clear();
		// resize index and index description vectors
		for (unsigned int i = 0;  i < size; ++i)
		{
			VectorValuedQuantityIndex indexKey(VectorValuedQuantityIndex::IK_Index, i);
			m_indexKeys.push_back(indexKey);
			m_indexKeyDescriptions.push_back(indexKey.toString());
		}
		m_size = size;
	}

	void clear() {
		m_size = 0;
		if (!m_indexKeys.empty())
			m_indexKeys.clear();
		if (!m_indexKeyDescriptions.empty())
			m_indexKeyDescriptions.clear();
	}

	void indexes(std::vector<unsigned int> &idx) const {
		idx.clear();
		for (unsigned int i = 0; i < m_indexKeys.size(); ++i)
			idx.push_back(m_indexKeys[i].m_keyValue);
	}

	/*! Declare comparison operator with string to find QuantityDescription by m_name. */
	NANDRAD_COMPARE_WITH_NAME

	// *** General attributes ***

	/*! Quantity name */
	std::string								m_name;
	/*! Quantity unit */
	std::string								m_unit;
	/*! Description string for the current quantity */
	std::string								m_description;
	/*! lower and upper value limit, (-\infty, +\infty) perdefault */
	std::pair<double, double>				m_minMaxValue;
	/*! Attribute determining whether the quantity is a constant or not. Parameters
		from project files will always be marked as constant while result quantities from
		models will be treated as non-constant.*/
	bool									m_constant;
	/*! Attributes for vector valued quantities: */
	/*! Vector size, = 1 for scalar quantities.*/
	unsigned int							m_size;
	/*! Vector of indices for each vector element.*/
	std::vector<VectorValuedQuantityIndex>	m_indexKeys;
	/*! Vector of strings that describe each vector element.*/
	std::vector<std::string>				m_indexKeyDescriptions;
};


/*! Utility functor for finding AbstractStateDependency::QuantityDescription in a vector
	by its quantity name.
	\code
	... = std::find_if(refDescs.begin(), refDescs.end(), FindQuantityDescriptionByName(quantityIDName));
	\endcode
*/
class FindQuantityDescriptionByName {
public:
	FindQuantityDescriptionByName(const std::string& name) :
	  m_name(name)
	{
	}

	bool operator()(const QuantityDescription& desc) const {
		return desc.m_name == m_name;
	}

	std::string		m_name;
};

} // namespace NANDRAD_MODEL

#endif // NM_QuantityDescriptionH
