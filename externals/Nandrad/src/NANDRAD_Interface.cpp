#include "NANDRAD_Interface.h"

#include <algorithm>

#include "NANDRAD_Zone.h"

namespace NANDRAD {


void Interface::updateComment(const std::vector<Zone> & zoneList) {
	if (m_id == 0)
		m_comment = "Interface to outside";
	else {
		// lookup zone and its display name\n"
		std::vector<Zone>::const_iterator it = std::find(zoneList.begin(), zoneList.end(), m_id);
		if (it != zoneList.end()) {
			if (!it->m_displayName.empty())
				m_comment = IBK::FormatString("Interface to '%1'").arg(it->m_displayName).str();
		}
		else {
			m_comment = IBK::FormatString("ERROR: Zone with id '%1' does not exist.").arg(it->m_displayName).str();
		}
	}
}


} // namespace NANDRAD
