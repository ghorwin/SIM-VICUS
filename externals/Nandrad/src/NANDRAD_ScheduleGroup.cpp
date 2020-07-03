/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
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

#include "NANDRAD_ScheduleGroup.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Constants.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>

#include <tinyxml.h>

namespace NANDRAD {


void ScheduleGroup::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[ScheduleGroup::readXML]";

#if 0

	try {
		// read schedules
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			std::string cname = c->Value();
			if (cname == "SpaceTypeGroup") {
				readSpaceTypeGroupXML(c);
			}
			else if (cname == "StartDate") {
				m_startDate.readXML(c);
				if (m_startDate.m_name.empty())
					m_startDate.m_name = cname;
			}
			else if (cname == "EndDate") {
				m_endDate.readXML(c);
				if (m_endDate.m_name.empty())
					m_endDate.m_name = cname;
			}
			else {
				throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag with name '%1' in ScheduleGroup section.").arg(cname)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading ScheduleGroup data."), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading ScheduleGroup data.").arg(ex.what()), FUNC_ID);
	}

#endif
}


void ScheduleGroup::writeXML(TiXmlElement * parent, bool detailedOutput) const {
#if 0
	// skip empty schedulegroups
	if (m_spaceTypeGroups.empty() &&
		m_startDate.empty() &&
		m_endDate.empty()) return;

	TiXmlElement * e = new TiXmlElement("ScheduleGroup");
	parent->LinkEndChild(e);

	if (!m_startDate.empty())
		m_startDate.writeXML(e, false);
	if (!m_endDate.empty())
		m_endDate.writeXML(e, false);

	writeSpaceTypeGroupXML(e, detailedOutput);
#endif
}


bool ScheduleGroup::operator!=(const ScheduleGroup & other) const {
#if 0
	if (m_startDate != other.m_startDate) return true;
	if (m_endDate != other.m_endDate) return true;
	if (m_spaceTypeGroups != other.m_spaceTypeGroups) return true;
#endif
	return false;
}


void ScheduleGroup::readSpaceTypeGroupXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[ScheduleGroup::readSpaceTypeGroupXML]";

	// read attributes
	const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "spaceTypeName");
	if (!attrib)
		throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(element->Row()).arg(
			IBK::FormatString("Expected 'spaceTypeName' attribute in SpaceTypeGroup element.")
			), FUNC_ID);
	std::string spaceTypeName = attrib->Value();
	// check if we have already such a SpaceType in the map
	if (IBK::map_contains(m_spaceTypeGroups, spaceTypeName)) {
		throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(element->Row()).arg(
			IBK::FormatString("Duplicate SpaceTypeGroup with space type name '%1'. ").arg( spaceTypeName)
			), FUNC_ID);
	}

	ScheduleMap scheduleMap;
	try {
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			std::string cname = c->Value();
			if (cname == "Schedule") {
				Schedule s;
				s.readXML(c);
				// check if schedule type is already present in map
				if (IBK::map_contains(scheduleMap, s.m_type))
					throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
						IBK::FormatString("Duplicate Schedule with type '%1' defined within SpaceTypeGroup for space type '%2'. "
										  ).arg( KeywordList::Keyword("Schedule::type_t", s.m_type)).arg(spaceTypeName)
						), FUNC_ID);
				scheduleMap[s.m_type] = s;
			}
			else {
				throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag with name '%1' in SpaceTypeGroup section.").arg(cname)
					), FUNC_ID);
			}
		}

		// insert scheduleMap into SpaceTypeGroup container
		m_spaceTypeGroups[spaceTypeName] = scheduleMap;
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading SpaceTypeGroup data."), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading SpaceTypeGroup data.").arg(ex.what()), FUNC_ID);
	}
}


void ScheduleGroup::writeSpaceTypeGroupXML(TiXmlElement * parent, bool detailedOutput) const {
	for (std::map<std::string, ScheduleMap>::const_iterator it = m_spaceTypeGroups.begin();
		 it != m_spaceTypeGroups.end(); ++it)
	{
		// write SpaceTypeGroup entries
		TiXmlElement * e = new TiXmlElement("SpaceTypeGroup");
		parent->LinkEndChild(e);

		e->SetAttribute("spaceTypeName", it->first);

		for (ScheduleMap::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
			sit->second.writeXML(e, detailedOutput);
		}
	}
}

} // namespace NANDRAD

