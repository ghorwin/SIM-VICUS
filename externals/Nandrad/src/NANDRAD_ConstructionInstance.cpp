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

#include "NANDRAD_ConstructionInstance.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Parameter.h>
#include <IBK_Exception.h>
//#include <IBK_StringUtils.h>
//#include <IBK_algorithm.h>

#include "NANDRAD_Constants.h"

namespace NANDRAD {


#if 0
const EmbeddedObject & ConstructionInstance::embeddedObjectById( const unsigned int id) const {
	const char * const FUNC_ID = "[ConstructionInstance::embeddedObjectById]";

	// preparation for std::map
	std::vector<NANDRAD::EmbeddedObject>::const_iterator embeddedObjectIt =
		m_embeddedObjects.begin();

	for ( ; embeddedObjectIt != m_embeddedObjects.end(); ++embeddedObjectIt)
	{
		if (embeddedObjectIt->m_id == id)
			return *embeddedObjectIt;
	}
	throw IBK::Exception( IBK::FormatString("Requested invalid Embedded object id '%1' inside ConstructionInstance with id %2.")
											.arg(id).arg(m_id),
											FUNC_ID);
}
#endif

bool ConstructionInstance::behavesLike(const ConstructionInstance & other) const {
	if (m_constructionTypeId != other.m_constructionTypeId)
		return false;

	// now compare interface at location A with interface A of other object
	if (m_interfaces.size() != other.m_interfaces.size())
		return false;

	int AIndex = -1;
	int BIndex = -1;
	for (unsigned int i=0; i<m_interfaces.size(); ++i) {
		switch (m_interfaces[i].m_location) {
			case Interface::IT_A : AIndex = i; break;
			case Interface::IT_B : BIndex = i; break;
			default:; // error not necessary, will bail out later
		}
	}
	int AIndexOther = -1;
	int BIndexOther = -1;
	for (unsigned int i=0; i<other.m_interfaces.size(); ++i) {
		switch (other.m_interfaces[i].m_location) {
			case Interface::IT_A : AIndexOther = i; break;
			case Interface::IT_B : BIndexOther = i; break;
			default:; // error not necessary, will bail out later
		}
	}

	bool isOutside = false;
	if (AIndex != -1) {
		if (AIndexOther == -1) return false;
		if (!m_interfaces[AIndex].behavesLike(other.m_interfaces[AIndexOther])) return false;

		if (m_interfaces[AIndex].m_zoneId == 0)
			isOutside = true;
	}
	if (BIndex != -1) {
		if (BIndexOther == -1) return false;
		if (!m_interfaces[BIndex].behavesLike(other.m_interfaces[BIndexOther])) return false;

		if (m_interfaces[BIndex].m_zoneId == 0)
			isOutside = true;
	}

	if (isOutside) {
		if (m_para[CP_ORIENTATION] != other.m_para[CP_ORIENTATION])
			return false;
		if (m_para[CP_INCLINATION] != other.m_para[CP_INCLINATION])
			return false;
	}

	return true; // both construction instances would calculate effectively the same
}

} // namespace NANDRAD

