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

#include <VICUS_ViewSettings.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <IBKMK_Vector3D.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void ViewSettings::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(ViewSettings::readXMLPrivate);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "GridSpacing")
				m_gridSpacing = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "GridWidth")
				m_gridWidth = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				NANDRAD::readFlagElement(c, f);
				bool success = false;
				try {
					Flags ftype = (Flags)KeywordList::Enumeration("ViewSettings::Flags", f.name());
					m_flags[ftype] = f; success=true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "CameraTranslation") {
				try {
					m_cameraTranslation = IBKMK::Vector3D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "FarDistance")
				m_farDistance = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "RotationMatrix")
				m_cameraRotation.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ViewSettings' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ViewSettings' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * ViewSettings::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("ViewSettings");
	parent->LinkEndChild(e);

	TiXmlElement::appendSingleAttributeElement(e, "GridSpacing", nullptr, std::string(), IBK::val2string<double>(m_gridSpacing));
	TiXmlElement::appendSingleAttributeElement(e, "GridWidth", nullptr, std::string(), IBK::val2string<double>(m_gridWidth));

	for (int i=0; i<NUM_F; ++i) {
		if (!m_flags[i].name().empty()) {
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
		}
	}
	TiXmlElement::appendSingleAttributeElement(e, "CameraTranslation", nullptr, std::string(), m_cameraTranslation.toString());

	m_cameraRotation.writeXML(e);
	TiXmlElement::appendSingleAttributeElement(e, "FarDistance", nullptr, std::string(), IBK::val2string<double>(m_farDistance));
	return e;
}

} // namespace VICUS
