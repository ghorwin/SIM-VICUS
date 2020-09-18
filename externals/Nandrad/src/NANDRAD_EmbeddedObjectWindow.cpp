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

#include "NANDRAD_EmbeddedObjectWindow.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>
#include <IBK_algorithm.h>

#include <tinyxml.h>

#include "NANDRAD_Constants.h"

namespace NANDRAD {

EmbeddedObjectWindow::EmbeddedObjectWindow() :
	m_modelType(NUM_MT)
{
	// model patrameters for constant model
	std::set<int> parametersModelConstant;
	parametersModelConstant.insert( (int) P_GlassFraction);
	parametersModelConstant.insert( (int) P_SolarHeatGainCoefficient);
	parametersModelConstant.insert( (int) P_ThermalTransmittance);
	parametersModelConstant.insert( (int) P_ShadingFactor);
	parametersModelConstant.insert((int)P_LeakageCoefficient);
	m_modelTypeToParameterMapping[MT_Constant] = parametersModelConstant;

	// model patrameters for detailed model
	std::set<int> parametersModelDetailed;
	parametersModelDetailed.insert((int)P_ShadingFactor);
	parametersModelDetailed.insert((int)P_LeakageCoefficient);
	m_modelTypeToParameterMapping[MT_Detailed] = parametersModelDetailed;

	// model patrameters for detailed ODE model
	m_modelTypeToParameterMapping[MT_DetailedWithStorage] = parametersModelDetailed;
}


#if 0
void EmbeddedObjectWindow::readXML(const TiXmlElement * element) {
	FUNCID(EmbeddedObjectWindow::readXML);

	try {
		// read attributes
		const TiXmlAttribute * model = TiXmlAttribute::attributeByName(element, "model");
		// error undefined model
		if( !model)
		{
		   throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				"Missing 'model' attribute."
				), FUNC_ID);
		}

		if(! KeywordList::KeywordExists("EmbeddedObjectWindow::modelType_t", model->Value()) )
		{
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Invalid model id '%1'.").arg(model->Value()).arg(element->Row())
				), FUNC_ID);
		}

		m_modelType = (modelType_t)KeywordList::Enumeration("EmbeddedObjectWindow::modelType_t", model->Value());

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
				if (KeywordList::KeywordExists("EmbeddedObjectWindow::para_t", namestr)) {
					para_t t = (para_t)KeywordList::Enumeration("EmbeddedObjectWindow::para_t", namestr);

					// check validity of parameter
					IBK_ASSERT(IBK::map_contains(m_modelTypeToParameterMapping, m_modelType));

					// check parameter contained in set
					if (!IBK::map_contains(m_modelTypeToParameterMapping[m_modelType], t)) {
						IBK::IBK_Message(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Parameter %1 is not supportet by selected model type %2.")
							.arg(namestr)
							.arg(KeywordList::Keyword("InterfaceLongWaveEmission::modelType_t", m_modelType))),
							IBK::MSG_WARNING,
							FUNC_ID,
							IBK::VL_STANDARD);
					}

					m_para[t].set(namestr, value, unitstr);
				}
				// we have a generic parameter
				else
				{
					readGenericParameterElement(c);
				}
			}
			// window type reference
			else if (cname == "WindowTypeReference") {
				// not supported for constant window models
				if (m_modelType == MT_CONSTANT) {
					IBK::IBK_Message(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("WindowTypeReference is not supportet by selected model type 'Constant'.")
						), IBK::MSG_WARNING,
						FUNC_ID,
						IBK::VL_STANDARD);
				}
				// store file reference
				m_windowTypeReference = c->GetText();
			}
			// try to read a generic parametrization
			else {
				readGenericParameterElement(c);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading constant 'Window' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading constant 'Window' element.").arg(ex2.what()), FUNC_ID);
	}
}


void EmbeddedObjectWindow::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Window");
	parent->LinkEndChild(e);

	e->SetAttribute( "model", KeywordList::Keyword("EmbeddedObjectWindow::modelType_t", m_modelType) );

	// write EmbeddedObjectWindow parameters
	for (unsigned int i=0; i<NUM_P; ++i) {
		if(m_para[i].name.empty()) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("EmbeddedObjectWindow::para_t",i));
		TiXmlElement::appendIBKParameterElement(e,
			m_para[i].name,
			m_para[i].IO_unit.name(),
			m_para[i].get_value());
	}

	// write all generic parameters
	writeGenericParameters(e);
}
#endif


} // namespace NANDRAD

