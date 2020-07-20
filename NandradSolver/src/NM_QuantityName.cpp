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

#include "NM_QuantityName.h"

#include <IBK_Exception.h>

// include individual model implementation headers

namespace NANDRAD_MODEL {

void QuantityName::fromEncodedString(const std::string &quantity) {

	const char * const FUNC_ID = "[QuantityName::fromEncodedString]";
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
			m_index = indexKey.m_keyValue;
		}
		catch (IBK::Exception &ex)
		{
			throw IBK::Exception(ex, IBK::FormatString("Cannot split quantity name %1 "
				"into name and index!")
				.arg(quantity),
				FUNC_ID);
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

	if (m_name != other.m_name)
		return true;
	if (m_index != other.m_index)
		return true;
	return false;
}

} // namespace NANDRAD_MODEL
