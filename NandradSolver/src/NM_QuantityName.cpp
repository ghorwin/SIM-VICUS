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

#include "NM_QuantityName.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

// include individual model implementation headers

namespace NANDRAD_MODEL {


std::string QuantityName::encodedString() const {
	if (m_index == -1)
		return m_name;
	else
		return m_name + "[" + IBK::val2string(m_index) + "]";
}


void QuantityName::fromEncodedString(const std::string &quantity) {
	FUNCID(QuantityName::fromEncodedString);

	std::string::size_type firstPos = quantity.rfind("[");
	std::string::size_type lastPos = quantity.rfind("]");

	m_name = quantity;
	m_index = -1;
	// an element of a vector valued target
	if (firstPos != std::string::npos && lastPos != std::string::npos) {
		std::string indexKeyStr;
		// cut index information out of target name
		indexKeyStr = quantity.substr(firstPos + 1, lastPos - firstPos - 1);
		m_name = quantity.substr(0, firstPos);
		try {
			VectorValuedQuantityIndex indexKey;
			indexKey.set(indexKeyStr);
			// now retrieve key value from key string
			m_index = (int)indexKey.m_keyValue;
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Cannot split quantity name %1 "
				"into name and index!").arg(quantity), FUNC_ID);
		}
	}
}


void QuantityName::clear() {
	m_name.clear();
	m_index = -1;

}


bool QuantityName::empty() {
	return m_name.empty();
}


bool QuantityName::operator!=(const QuantityName & other) const {
	if (m_index != other.m_index)
		return true;
	if (m_name != other.m_name)
		return true;
	return false;
}

} // namespace NANDRAD_MODEL
