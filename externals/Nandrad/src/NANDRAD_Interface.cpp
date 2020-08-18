/*	The NANDRAD data model library.

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

#include "NANDRAD_Interface.h"

#include <algorithm>

#include "NANDRAD_Zone.h"

namespace NANDRAD {

void Interface::updateComment(const std::vector<Zone> & zones) {

	if (m_zoneId == 0)
		m_comment = "Interface to outside";
	else {
		// lookup zone and its display name\n"
		std::vector<Zone>::const_iterator it = std::find(zones.begin(), zones.end(), m_zoneId);
		if (it != zones.end()) {
			if (!it->m_displayName.empty())
				m_comment = IBK::FormatString("Interface to '%1'").arg(it->m_displayName).str();
			else
				m_comment = IBK::FormatString("Interface to anonymous zone with id #%1").arg(m_zoneId).str();
		}
		else {
			m_comment = IBK::FormatString("ERROR: Zone with id #%1 does not exist.").arg(m_zoneId).str();
		}
	}
}

} // namespace NANDRAD

