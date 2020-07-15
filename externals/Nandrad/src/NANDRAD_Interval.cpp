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

#include "NANDRAD_Interval.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <IBK_FormatString.h>
#include <IBK_Exception.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace NANDRAD {

#if 0
void Interval::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[Interval::readXML]";
	try {
		// store duration and end time
		for (const TiXmlElement * e = element->FirstChildElement(); e; e = e->NextSiblingElement()) {

			// determine data based on element name
			std::string ename = e->Value();

			// right now, we only support IBK:Parameter child tags
			if (ename != "IBK:Parameter") {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown tag '%1'.").arg(ename)
						), FUNC_ID);
			}

			// read IBK parameter
			std::string namestr, unitstr;
			double value;
			TiXmlElement::readIBKParameterElement(e, namestr, unitstr, value);

			// check if we have a predefined parameter
			if (KeywordList::KeywordExists("Interval::para_t", namestr) ) {
				// determine type of parameter
				para_t t = (para_t)KeywordList::Enumeration("Interval::para_t", namestr);
				m_para[t].set(namestr, value, unitstr);
				if (m_para[t].IO_unit.base_id() != IBK::Unit("s").base_id())
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
							IBK::FormatString("Invalid/missing unit for Interval parameter '%1', should be a time unit.")
											  .arg(namestr)
							), FUNC_ID);
			}
			// we have a generic parameter
			else {
				readGenericParameterElement(e);
			}
		}
		// end of initialization: calculate missing parameters
		// we only can calculate duration if start and end time are given
		if(!m_para[IP_END].name.empty() && !m_para[IP_START].name.empty()) {
			// error: wrong interval definition
			if(m_para[IP_END].value < m_para[IP_START].value) {
				throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Error combining end time %1 s with start time %2 s!")
					.arg(m_para[IP_END].value).arg(m_para[IP_START].value)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'Interval' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'Interval' element.").arg(ex2.what()), FUNC_ID);
	}
}
#endif


bool Interval::operator!=(const Interval & other) const {
#if 0
	if(GenericParametrizationObject::operator !=(other))
		return true;
	for (int i=0; i<NUM_IP; ++i)
		if (m_para[i] != other.m_para[i]) return true;
#endif
	return false; // this and other hold the same data
}


void Interval::setStartEnd( double start, double endtime, IBK::Unit u ) {
	m_para[IP_START].set( KeywordList::Keyword("Interval::para_t", IP_START), start, u.name());
	if (endtime == std::numeric_limits<double>::max())
		m_para[IP_END].clear();
	else
		m_para[IP_END].set( KeywordList::Keyword("Interval::para_t", IP_END), endtime, u.name());
}


bool Interval::checkParameters(bool strict) const {

	const char * const FUNC_ID = "[Interval::checkParameters]";
	bool ok = true;

	// check start point
	if (!m_para[Interval::IP_START].name.empty()) {
		// start point is greater than zero
		if (m_para[Interval::IP_START].value < 0) {
			if (strict)
				throw IBK::Exception( IBK::FormatString("Interval has an invalid Start parameter."), FUNC_ID);
			else {
				IBK::IBK_Message( IBK::FormatString("Interval has an invalid Start parameter."), IBK::MSG_WARNING, FUNC_ID );
				ok = false;
			}
		}
	}

	// check end point
	if (!m_para[Interval::IP_END].name.empty()) {

		double endtime = m_para[Interval::IP_END].value;
		if (endtime < 0) {
			if (strict)
				throw IBK::Exception( IBK::FormatString("Invalid End time point in Interval."), FUNC_ID);
			else {
				IBK::IBK_Message( IBK::FormatString("Invalid End time point in Interval."), IBK::MSG_WARNING, FUNC_ID );
				ok = false;
			}
		}
		// check if end time point preceedes start time point
		if (!m_para[Interval::IP_START].name.empty()
			&& endtime <= m_para[Interval::IP_START].value)
		{
			if (strict) {
				throw IBK::Exception( IBK::FormatString( "End time point preceedes Start time point "
														 "in Interval, but End time point must be past the Start time."), FUNC_ID);
			}
			else {
				IBK::IBK_Message( IBK::FormatString( "End time point preceedes Start time point "
													 "in Interval, but End time point must be past the Start time."), IBK::MSG_WARNING, FUNC_ID );
				ok = false;
			}
		}
	}

	// check units for all provided parameters
	unsigned int time_base_id = IBK::Unit("s").base_id();
	for (unsigned int i=0; i<NUM_IP; ++i) {
		if (!m_para[i].name.empty()) {
			if (m_para[i].IO_unit.base_id() != time_base_id) {
				if (strict) {
					throw IBK::Exception( IBK::FormatString( "Parameter '%1' has an invalid unit, should be a time unit.").arg(m_para[i].name), FUNC_ID);
				}
				else {
					IBK::IBK_Message( IBK::FormatString( "Parameter '%1' has an invalid unit, should be a time unit.").arg(m_para[i].name),
									  IBK::MSG_WARNING, FUNC_ID );
					ok = false;
				}
			}
		}
	}
	return ok;
}


bool Interval::isInInterval(double t) const {
	return (IBK::f_fuzzyGTEQ(t, m_para[IP_START].value) && IBK::f_fuzzyLTEQ(t, endTime()) );
}


double Interval::endTime() const {
	if (!m_para[Interval::IP_END].name.empty())
		return m_para[Interval::IP_END].value;
	// interval lasts until the end of simulation
	return std::numeric_limits<double>::max();
}


} // namespace NANDRAD

