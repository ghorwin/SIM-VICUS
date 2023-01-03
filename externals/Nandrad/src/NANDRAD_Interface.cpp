/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_Interface.h"

#include <algorithm>

#include "NANDRAD_Zone.h"

namespace NANDRAD {

void Interface::readXML(const TiXmlElement * element) {
	FUNCID(Interface::readXML);

	// sanity check against duplicate InterfaceA or InterfaceB definitions inside constructions
	if (m_id != NANDRAD::INVALID_ID)
		throw IBK::Exception("Duplicate interface definition in project file.", FUNC_ID);

	readXMLPrivate(element);
}


TiXmlElement * Interface::writeXML(TiXmlElement * parent) const {
	if (m_id != INVALID_ID)
		return writeXMLPrivate(parent); else return nullptr;
}


void Interface::checkParameters() const {
	m_heatConduction.checkParameters();
	m_solarAbsorption.checkParameters();
	m_longWaveEmission.checkParameters();
	m_vaporDiffusion.checkParameters();
	m_airFlow.checkParameters();
}


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
			// store pointer to zone
			m_zone = &(*it);
		}
		else {
			m_comment = IBK::FormatString("ERROR: Zone with id #%1 does not exist.").arg(m_zoneId).str();
		}
	}
}


bool Interface::haveBCParameters() const {
	if (m_id == INVALID_ID) return false;
	if (m_heatConduction.m_modelType != InterfaceHeatConduction::NUM_MT) return true;
	if (m_solarAbsorption.m_modelType != InterfaceSolarAbsorption::NUM_MT) return true;
	if (m_longWaveEmission.m_modelType != InterfaceLongWaveEmission::NUM_MT) return true;
	if (m_vaporDiffusion.m_modelType != InterfaceVaporDiffusion::NUM_MT) return true;
	if (m_airFlow.m_modelType != InterfaceAirFlow::NUM_MT) return true;
	return false;
}

} // namespace NANDRAD

