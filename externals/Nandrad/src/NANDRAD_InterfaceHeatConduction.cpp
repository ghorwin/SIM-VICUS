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

#include "NANDRAD_InterfaceHeatConduction.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>
#include <IBK_assert.h>

#include <tinyxml.h>

#include "NANDRAD_Constants.h"

namespace NANDRAD {

InterfaceHeatConduction::InterfaceHeatConduction() :
	m_modelType(NUM_MT)
{
	std::set<int> parametersModelConstant;
	parametersModelConstant.insert( (int) P_HeatTransferCoefficient);
	m_modelTypeToParameterMapping[MT_CONSTANT] = parametersModelConstant;
}


void InterfaceHeatConduction::readXML(const TiXmlElement * element) {
#if 0
	const char * const FUNC_ID = "[InterfaceHeatConduction::readXML]";
	// read attributes
	try {
		const TiXmlAttribute * model = TiXmlAttribute::attributeByName(element, "model");
		// error undefined model
		if( !model)
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
							IBK::FormatString("Missing 'model' attribute.")
							), FUNC_ID);

		if (! KeywordList::KeywordExists("InterfaceHeatConduction::modelType_t", model->Value() ) ) {
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Invalid model id '%1'.").arg(model->Value())
				), FUNC_ID);
		}

		m_modelType = (modelType_t)KeywordList::Enumeration("InterfaceHeatConduction::modelType_t", model->Value());
		// read parameters
		// read sub-elements
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "IBK:Parameter") {
				// use utility function to read parameter
				std::string namestr, unitstr;
				double value;
				TiXmlElement::readIBKParameterElement(c, namestr, unitstr, value);
				// determine type of parameter
				para_t t = (para_t)KeywordList::Enumeration("InterfaceHeatConduction::para_t", namestr);

				// check validity of parameter
				IBK_ASSERT( IBK::map_contains( m_modelTypeToParameterMapping, m_modelType ) );

				// check parameter contained in set
				if ( ! IBK::map_contains ( m_modelTypeToParameterMapping[ m_modelType ], t) ){
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Parameter %1 is not supportet by selected model type %2.")
						.arg(namestr)
						.arg(KeywordList::Keyword("InterfaceHeatConduction::modelType_t", m_modelType))
						), FUNC_ID);
				}

				m_para[t].set(namestr, value, unitstr);

				// check parameter unit
				std::string paraUnit = KeywordList::Unit("InterfaceHeatConduction::para_t", t);
				if (unitstr != paraUnit) {
					try {
						m_para[t].get_value(paraUnit);
					}
					catch (IBK::Exception &ex) {
						throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Invalid unit '#%1' of parameter #%2!")
							.arg(paraUnit)
							.arg(namestr)
							), FUNC_ID);
					}
				}
			}
			// try to read a generic parametrization
			else {
				readGenericParameterElement(c);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex,
			IBK::FormatString("Error reading 'HeatConduction' element."),
			FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'HeatConduction' element.").arg(ex2.what()), FUNC_ID);
	}
#endif
}


void InterfaceHeatConduction::writeXML(TiXmlElement * parent, bool detailedOutput) const {
#if 0
	TiXmlElement * e = new TiXmlElement("HeatConduction");
	parent->LinkEndChild(e);

	e->SetAttribute( "model", KeywordList::Keyword("InterfaceHeatConduction::modelType_t", m_modelType) );

	// write InterfaceHeatConduction parameters
	for (unsigned int i=0; i<NUM_P; ++i) {
		if(m_para[i].name.empty()) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("InterfaceHeatConduction::para_t",i));
		TiXmlElement::appendIBKParameterElement(e,
			m_para[i].name,
			m_para[i].IO_unit.name(),
			m_para[i].get_value());
	}

	// write all generic parameters
	writeGenericParameters(e, detailedOutput);
#endif
}


bool InterfaceHeatConduction::operator!=(const InterfaceHeatConduction & other) const {
#if 0
	// base class comparison
	if (GenericParametrizationObject::operator!=(other))
		return true;

	// model comparison
	if (m_modelType != other.m_modelType) return true;
	if (m_modelTypeToParameterMapping != other.m_modelTypeToParameterMapping) return true;
	for (int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
#endif
	return false; // not different
}

} // namespace NANDRAD

