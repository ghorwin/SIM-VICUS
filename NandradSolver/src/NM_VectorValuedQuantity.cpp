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
#include <algorithm>

#include <IBK_assert.h>

namespace NANDRAD_MODEL {

VectorValuedQuantity::VectorValuedQuantity(unsigned int n, double value) :
	m_data(n, value),
	m_keyType(VectorValuedQuantityIndex::IK_Index)
{
	m_indexKeys.resize(n);

	for (unsigned int i = 0; i < n; ++i)
		m_indexKeys[i] = i;
}


const double & VectorValuedQuantity::operator[](unsigned int i) const {
	FUNCID(VectorValuedQuantity::operator[]);
	// debug check: have data and key vectors the same size
	IBK_ASSERT(m_data.size() == m_indexKeys.size() );
	// find index/modelID
	std::vector<unsigned int>::const_iterator it = std::find(m_indexKeys.begin(), m_indexKeys.end(), i);
	if (it == m_indexKeys.end())
		throw IBK::Exception(IBK::FormatString("Index/modelID %1 not in list of available indexes/model IDs.").arg(i), FUNC_ID);

	// calculate vector position
	size_t index = (size_t)std::distance(m_indexKeys.begin(), it);
	return m_data[index];
}


const double & VectorValuedQuantity::insert(unsigned int i) {
	// debug check: have data and key vectors the same size
	IBK_ASSERT(m_data.size() == m_indexKeys.size() );
	// find index/modelID
	std::vector<unsigned int>::iterator it = std::find(m_indexKeys.begin(), m_indexKeys.end(), i);
	if (it == m_indexKeys.end()) {
		m_data.push_back(0);
		m_indexKeys.push_back(i);
		return m_data.back();
	}

	// calculate vector position
	size_t index = (size_t)std::distance(m_indexKeys.begin(), it);
	return m_data[index];
}

} // namespace NANDRAD_MODEL

