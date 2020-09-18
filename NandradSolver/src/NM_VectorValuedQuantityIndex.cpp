/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NM_VectorValuedQuantityIndex.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

namespace NANDRAD_MODEL {

void VectorValuedQuantityIndex::set(const std::string &keyString) {
	FUNCID(VectorValuedQuantityIndex::set);

	std::vector<std::string> tokens;
	std::string value;
	// we have a direct type definition
	if (IBK::explode_in2(keyString,tokens,'=') == (size_t) 2) {
		// find type
		size_t indexPos = tokens[0].find("index");
		size_t modelIDPos = tokens[0].find("id");
		// we reference via model id
		if(modelIDPos != std::string::npos)
			m_keyType = IK_ModelID;
		else if(indexPos != std::string::npos)
			m_keyType = IK_Index;
		// return invalid value
		else {
			throw IBK::Exception( IBK::FormatString("Malformed index reference '%1' (expected 'index=<value>' or 'id=<value>' .").arg(keyString), FUNC_ID);
		}
		value = tokens[1];
	}
	// only the key value is given
	else {
		m_keyType = IK_Index;
		value = keyString;
	}
	// fill key value
	try {
		m_keyValue = IBK::string2val<unsigned int>(value);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception( ex, IBK::FormatString("Malformed index reference '%1'.").arg(keyString), FUNC_ID);
	}
}


void VectorValuedQuantityIndex::set(const IndexKeyType keyType, const unsigned int keyValue) {
	FUNCID(VectorValuedQuantityIndex::set);
	// check if keyType is correct
	if (keyType == NUM_IndexKeyType)
		throw IBK::Exception(IBK::FormatString("Invalid index type '%1' (expected 'IK_Index' or 'IK_ModelID').").arg(NUM_IndexKeyType), FUNC_ID);

	m_keyType = keyType;
	m_keyValue = keyValue;
}


std::string VectorValuedQuantityIndex::toString() const {
	if (m_keyType == IK_Index)
		return IBK::val2string<unsigned int>(m_keyValue);
	else if (m_keyType == IK_ModelID)
		return std::string("id=") + IBK::val2string<unsigned int>(m_keyValue);
	throw IBK::Exception("Cannot generate encoded index string from invalid vector index (type is invalid).", "[VectorValuedQuantityIndex::toString]");
}


} // namespace NANDRAD_MODEL
