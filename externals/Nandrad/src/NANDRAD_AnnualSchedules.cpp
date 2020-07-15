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

#include "NANDRAD_AnnualSchedules.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Constants.h"

#include <IBK_Exception.h>
#include <IBK_Parameter.h>
#include <IBK_algorithm.h>

#include <tinyxml.h>

namespace NANDRAD {


// *** AnnualSchedules ***

void AnnualSchedules::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[AnnualSchedules::readXML]";

	const TiXmlElement * c;
	try {
		// read sub-elements
		for (c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {

			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "SpaceTypeGroup") {
				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c, "spaceTypeName");
				if (attrib == nullptr)
					throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
						"Missing 'spaceTypeName' attribute in SpaceTypeGroup tag."
						), FUNC_ID);
				std::string spaceTypeName = attrib->Value();
				if (IBK::map_contains(m_parameters, spaceTypeName)) {
					throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
						IBK::FormatString("Duplicate SpaceTypeGroup with SpaceType ID-name '%1' in "
						"AnnualSchedules section.").arg(spaceTypeName)
						), FUNC_ID);
				}

				LinearSplineParameterMap	parameterMap;
				// now read all parameters within SpaceTypeGroup
				for (const TiXmlElement * e = c->FirstChildElement(); e; e = e->NextSiblingElement()) {
					std::string ename = e->Value();
					if (ename == "IBK:LinearSpline") {
						LinearSplineParameter splineParameter;
						splineParameter.readXML(e);
						// check that parameter is not yet in the parameterMap
						if (IBK::map_contains(parameterMap, splineParameter.m_name)) {
							throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(e->Row()).arg(
								IBK::FormatString("Duplicate IBK:LinearSpline element with parameter name '%1' in "
								"SpaceTypeGroup section.").arg(splineParameter.m_name)
								), FUNC_ID);
						}
						parameterMap[splineParameter.m_name] = splineParameter;
					}
					else {
						throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(e->Row()).arg(
							IBK::FormatString("Unknown XML tag with name '%1' in SpaceTypeGroup section.").arg(ename)
							), FUNC_ID);
					}
				} // for e

				// add parameters to map
				m_parameters[spaceTypeName] = parameterMap;
			}
			else {
				throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag with name '%1' in AnnualSchedules section.").arg(cname)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading AnnualSchedules data."), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading AnnualSchedules data.").arg(ex.what()), FUNC_ID);
	}
}
// ----------------------------------------------------------------------------

void AnnualSchedules::writeXML(TiXmlElement * parent) const {
	if (m_parameters.empty())
		return;

	TiXmlElement * e = new TiXmlElement("AnnualSchedules");
	parent->LinkEndChild(e);
	// write all SpaceTypeGroups

	for (std::map<std::string, LinearSplineParameterMap>::const_iterator it = m_parameters.begin();
		it != m_parameters.end(); ++it)
	{
		TiXmlElement * c = new TiXmlElement("SpaceTypeGroup");
		e->LinkEndChild(c);
		c->SetAttribute("spaceTypeName", it->first);

		// loop over all parameters within spaceTypeGroup
		for (LinearSplineParameterMap::const_iterator pit = it->second.begin(); pit != it->second.end(); ++pit) {
			pit->second.writeXML(c);
		}
	}
}
// ----------------------------------------------------------------------------

//void AnnualSchedules::createLinearSpline(const std::string &quantityName, IBK::LinearSpline &spline) const {
//	const char * const FUNC_ID = "[DailyCycle::linearSpline]";

//	spline.clear();
//	// check if interval section contains quantity name
////	std::string quantityUnit;
//	unsigned int j = 0;
//	// check if hourly values section contains quantity name
//	for ( ; j < m_parameters.size(); ++j)
//	{
//		if(m_parameters[j].m_name != quantityName)
//			continue;

//		break;
//	}
//	// no schedule definition for the requested quantity, return an empty spline parameter
//	if(j == m_parameters.size())
//		return;

//	const NANDRAD::LinearSplineParameter &parameter = m_parameters[j];

//	// convert x-values
//	std::vector<double> xValues(parameter.m_values.x().size());
//	try {
//		for(unsigned int i = 0; i < parameter.m_values.x().size(); ++i)
//		{
//			IBK::Parameter timeVal(parameter.m_name,parameter.m_values.x()[i],parameter.m_xUnit);
//			xValues[i] = timeVal.get_value("s");
//		}
//	}
//	catch(IBK::Exception &ex)
//	{
//		throw IBK::Exception(IBK::FormatString("Error converting annual schedule of quantity %1 to linear spline. "
//								"The error message was: %2")
//								.arg(quantityName)
//								.arg(ex.what()),
//								FUNC_ID);
//	}
//	// constrcut the spline
//	spline.setValues(xValues, parameter.m_values.y());
//	std::string errmsg;
//	// error generating the spline
//	if(!spline.makeSpline(errmsg))
//			throw IBK::Exception(IBK::FormatString("Error converting annual schedule to a linear spline parameter for quantity with name %1: "
//								"The error message was: %2.")
//								.arg(quantityName)
//								.arg(errmsg),
//								FUNC_ID);
//}

bool AnnualSchedules::operator!=(const AnnualSchedules & other) const {
	if (m_parameters != other.m_parameters) return true;
	return false;
}

} // namespace NANDRAD

