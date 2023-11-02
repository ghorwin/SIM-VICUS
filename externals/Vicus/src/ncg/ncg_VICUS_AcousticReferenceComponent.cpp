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

#include <VICUS_AcousticReferenceComponent.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void AcousticReferenceComponent::readXML(const TiXmlElement * element) {
	FUNCID(AcousticReferenceComponent::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName.setEncodedString(attrib->ValueStr());
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "buildingType")
				m_buildingType = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "requirementType")
				try {
					m_requirementType = (RequirementType)KeywordList::Enumeration("AcousticReferenceComponent::RequirementType", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "ImpactSoundOneStructureUnit")
				m_impactSoundOneStructureUnit = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "ImpactSoundDifferentStructureUnit")
				m_impactSoundDifferentStructureUnit = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "AirborneSoundOneStructureUnit")
				m_airborneSoundOneStructureUnit = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "AirborneSoundDifferentStructureUnit")
				m_airborneSoundDifferentStructureUnit = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "IdAcousticTemplateA")
				m_idAcousticTemplateA = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdAcousticTemplateB")
				m_idAcousticTemplateB = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "Type") {
				try {
					m_type = (ComponentType)KeywordList::Enumeration("AcousticReferenceComponent::ComponentType", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'AcousticReferenceComponent' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'AcousticReferenceComponent' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * AcousticReferenceComponent::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("AcousticReferenceComponent");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_buildingType != VICUS::INVALID_ID)
		e->SetAttribute("buildingType", IBK::val2string<IDType>(m_buildingType));
	if (m_requirementType != NUM_RT)
		e->SetAttribute("requirementType", KeywordList::Keyword("AcousticReferenceComponent::RequirementType",  m_requirementType));

	if (m_type != NUM_CT)
		TiXmlElement::appendSingleAttributeElement(e, "Type", nullptr, std::string(), KeywordList::Keyword("AcousticReferenceComponent::ComponentType",  m_type));
	TiXmlElement::appendSingleAttributeElement(e, "ImpactSoundOneStructureUnit", nullptr, std::string(), IBK::val2string<double>(m_impactSoundOneStructureUnit));
	TiXmlElement::appendSingleAttributeElement(e, "ImpactSoundDifferentStructureUnit", nullptr, std::string(), IBK::val2string<double>(m_impactSoundDifferentStructureUnit));
	TiXmlElement::appendSingleAttributeElement(e, "AirborneSoundOneStructureUnit", nullptr, std::string(), IBK::val2string<double>(m_airborneSoundOneStructureUnit));
	TiXmlElement::appendSingleAttributeElement(e, "AirborneSoundDifferentStructureUnit", nullptr, std::string(), IBK::val2string<double>(m_airborneSoundDifferentStructureUnit));
	if (m_idAcousticTemplateA != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdAcousticTemplateA", nullptr, std::string(), IBK::val2string<unsigned int>(m_idAcousticTemplateA));
	if (m_idAcousticTemplateB != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdAcousticTemplateB", nullptr, std::string(), IBK::val2string<unsigned int>(m_idAcousticTemplateB));
	return e;
}

} // namespace VICUS
