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

#include "NANDRAD_Location.h"

#include "NANDRAD_KeywordList.h"
#include <IBK_messages.h>

namespace NANDRAD {

void Location::checkParameters() const {
	FUNCID(Location::checkParameters);

	// check albedo
	m_para[NANDRAD::Location::P_Albedo].checkedValue("Albedo","---","---", 0, true, 1, true,
													 "Location parameter 'Albedo' is expected between 0 and 1.");

	// for now we require a climate data file
	// if dummy values are needed, it is possible to create a simple dummy climate data file
	// with constant values throughout the year
	if (m_climateFilePath.str().empty())
		throw IBK::Exception("Climate data file path required in location data.", FUNC_ID);

	const IBK::Parameter & latitude = m_para[NANDRAD::Location::P_Latitude];
	const IBK::Parameter & longitude = m_para[NANDRAD::Location::P_Longitude];
	// ensure that either both latitude and longitude are given, or none
	if (!((latitude.name.empty() && longitude.name.empty()) || (!latitude.name.empty() && !longitude.name.empty())))
		throw IBK::Exception("If 'Latitude' or 'Longitude' are given in the location data, you need to specify always both.", FUNC_ID);

	// if either is given, check for valid range
	if (!latitude.name.empty()) {
		latitude.checkedValue("Latitude", "Deg", "Deg", -90, true, 90, true,
							  "Location parameter 'Latitude' is expected to be between -90 and 90 degrees.");
	}
	if (!longitude.name.empty()) {
		longitude.checkedValue("Longitude", "Deg", "Deg", -180, true, 180, true,
							  "Location parameter 'Longitude' is expected to be between -180 and 180 degrees.");
	}
}

void Location::initDefaults() {
	m_flags[Location::F_PerezDiffuseRadiationModel].set("PerezDiffuseRadiationModel", true);
	NANDRAD::KeywordList::setParameter(m_para, "Location::para_t", NANDRAD::Location::P_Albedo, 0.2);
	NANDRAD::KeywordList::setParameter(m_para, "Location::para_t", NANDRAD::Location::P_Longitude, 13.1);
	NANDRAD::KeywordList::setParameter(m_para, "Location::para_t", NANDRAD::Location::P_Longitude, 13.1);
}

} // namespace NANDRAD
