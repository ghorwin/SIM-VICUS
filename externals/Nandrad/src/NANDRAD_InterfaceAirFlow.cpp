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

#include "NANDRAD_InterfaceAirFlow.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>
#include <IBK_assert.h>

#include <tinyxml.h>

#include "NANDRAD_Constants.h"

namespace NANDRAD {

InterfaceAirFlow::InterfaceAirFlow() :
	m_modelType(NUM_MT)
{
	std::set<int> splineParametersModelWindFlow;
	splineParametersModelWindFlow.insert( (int) SP_PressureCoefficient);
	m_modelTypeToSplineParameterMapping[MT_WINDFLOW] = splineParametersModelWindFlow;
}


void InterfaceAirFlow::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[InterfaceAirFlow::readXML]";

	try {
		// read attributes
		const TiXmlAttribute * model = TiXmlAttribute::attributeByName(element, "model");
		// error undefined model
		if( !model)
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
							IBK::FormatString("Missing 'type' attribute.")
							), FUNC_ID);

		if (! KeywordList::KeywordExists("InterfaceAirFlow::modelType_t", model->Value() ) ) {
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Invalid model id '%1'.").arg(model->Value())
				), FUNC_ID);
		}


		m_modelType = (modelType_t)KeywordList::Enumeration("InterfaceAirFlow::modelType_t", model->Value());

		// read parameters
		// read sub-elements
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "IBK:LinearSpline") {
				// use utility function to read parameter
				LinearSplineParameter splineParameter;
				splineParameter.readXML(c);

				// determine type of parameter
				splinePara_t t = (splinePara_t)KeywordList::Enumeration("InterfaceAirFlow::splinePara_t",
					splineParameter.m_name);

				// check validity of parameter
				IBK_ASSERT( IBK::map_contains( m_modelTypeToSplineParameterMapping, m_modelType ) );

				// check parameter contained in set
				if ( ! IBK::map_contains (m_modelTypeToSplineParameterMapping[ m_modelType ], t) ){
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Parameter %1 is not supportet by selected model type %2.")
						.arg(splineParameter.m_name)
						.arg(KeywordList::Keyword("InterfaceAirFlow::modelType_t", m_modelType))
						), FUNC_ID);
				}

				m_splinePara[t] = splineParameter;

				// check parameter unit: y
				std::string paraYUnit = KeywordList::Unit("InterfaceAirFlow::splinePara_t", t);
				if (splineParameter.m_yUnit.name() != paraYUnit) {
					try {
						IBK::Parameter testPara(splineParameter.m_name, 0, splineParameter.m_yUnit);
						testPara.get_value(paraYUnit);
					}
					catch (IBK::Exception &ex) {
						throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Invalid unit '#%1' of parameter #%2!")
							.arg(paraYUnit)
							.arg(splineParameter.m_name)
							), FUNC_ID);
					}
				}
			}
			else {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag '%1'.").arg(cname)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex,
			IBK::FormatString("Error reading 'AirFlow' element."),
			FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'AirFlow' element.").arg(ex2.what()), FUNC_ID);
	}
}


void InterfaceAirFlow::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("AirFlow");
	parent->LinkEndChild(e);

	e->SetAttribute( "model", KeywordList::Keyword("InterfaceAirFlow::modelType_t", m_modelType) );

	// write InterfaceAirFlow parameters
	for (unsigned int i=0; i<NUM_SP; ++i) {
		if(m_splinePara[i].m_name.empty()) continue;
		m_splinePara[i].writeXML(e);
	}
}


bool InterfaceAirFlow::operator!=(const InterfaceAirFlow & other) const {
	// model comparison
	if (m_modelType != other.m_modelType) return true;
	if (m_modelTypeToSplineParameterMapping != other.m_modelTypeToSplineParameterMapping)
		return true;
	for (int i=0; i<NUM_SP; ++i)
		if (m_splinePara[i] != other.m_splinePara[i]) return true;
	return false; // not different
}


} // namespace NANDRAD

