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

#include <VICUS_Drawing.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <IBKMK_Vector3D.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void Drawing::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(Drawing::readXMLPrivate);

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
			if (cName == "Radius")
				m_radius = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "StartAngle")
				m_startAngle = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "EndAngle")
				m_endAngle = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Origin") {
				try {
					m_origin = IBKMK::Vector3D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "ScalingFactor")
				m_scalingFactor = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Blocks") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Block")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Block obj;
					obj.readXML(c2);
					m_blocks.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Layers") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "DrawingLayer")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					DrawingLayer obj;
					obj.readXML(c2);
					m_layers.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Points") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Point")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Point obj;
					obj.readXML(c2);
					m_points.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Lines") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Line")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Line obj;
					obj.readXML(c2);
					m_lines.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Polylines") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "PolyLine")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					PolyLine obj;
					obj.readXML(c2);
					m_polylines.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Circles") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Circle")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Circle obj;
					obj.readXML(c2);
					m_circles.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Ellipses") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Ellipse")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Ellipse obj;
					obj.readXML(c2);
					m_ellipses.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Arcs") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Arc")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Arc obj;
					obj.readXML(c2);
					m_arcs.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Solids") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Solid")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Solid obj;
					obj.readXML(c2);
					m_solids.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Texts") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Text")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Text obj;
					obj.readXML(c2);
					m_texts.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ZCounter")
				m_zCounter = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "DefaultColor")
				m_defaultColor.setNamedColor(QString::fromStdString(c->GetText()));
			else if (cName == "IBKMK::Vector2D")
				m_center.readXML(c);
			else if (cName == "RotationMatrix")
				m_rotationMatrix.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Drawing::writeXMLPrivate(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("Drawing");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());

	m_center.writeXML(e);
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle));
	TiXmlElement::appendSingleAttributeElement(e, "Origin", nullptr, std::string(), m_origin.toString());

	m_rotationMatrix.writeXML(e);
	TiXmlElement::appendSingleAttributeElement(e, "ScalingFactor", nullptr, std::string(), IBK::val2string<double>(m_scalingFactor));

	if (!m_blocks.empty()) {
		TiXmlElement * child = new TiXmlElement("Blocks");
		e->LinkEndChild(child);

		for (std::vector<Block>::const_iterator it = m_blocks.begin();
			it != m_blocks.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_layers.empty()) {
		TiXmlElement * child = new TiXmlElement("Layers");
		e->LinkEndChild(child);

		for (std::vector<DrawingLayer>::const_iterator it = m_layers.begin();
			it != m_layers.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_points.empty()) {
		TiXmlElement * child = new TiXmlElement("Points");
		e->LinkEndChild(child);

		for (std::vector<Point>::const_iterator it = m_points.begin();
			it != m_points.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_lines.empty()) {
		TiXmlElement * child = new TiXmlElement("Lines");
		e->LinkEndChild(child);

		for (std::vector<Line>::const_iterator it = m_lines.begin();
			it != m_lines.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_polylines.empty()) {
		TiXmlElement * child = new TiXmlElement("Polylines");
		e->LinkEndChild(child);

		for (std::vector<PolyLine>::const_iterator it = m_polylines.begin();
			it != m_polylines.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_circles.empty()) {
		TiXmlElement * child = new TiXmlElement("Circles");
		e->LinkEndChild(child);

		for (std::vector<Circle>::const_iterator it = m_circles.begin();
			it != m_circles.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_ellipses.empty()) {
		TiXmlElement * child = new TiXmlElement("Ellipses");
		e->LinkEndChild(child);

		for (std::vector<Ellipse>::const_iterator it = m_ellipses.begin();
			it != m_ellipses.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_arcs.empty()) {
		TiXmlElement * child = new TiXmlElement("Arcs");
		e->LinkEndChild(child);

		for (std::vector<Arc>::const_iterator it = m_arcs.begin();
			it != m_arcs.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_solids.empty()) {
		TiXmlElement * child = new TiXmlElement("Solids");
		e->LinkEndChild(child);

		for (std::vector<Solid>::const_iterator it = m_solids.begin();
			it != m_solids.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_texts.empty()) {
		TiXmlElement * child = new TiXmlElement("Texts");
		e->LinkEndChild(child);

		for (std::vector<Text>::const_iterator it = m_texts.begin();
			it != m_texts.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (m_zCounter != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "ZCounter", nullptr, std::string(), IBK::val2string<unsigned int>(m_zCounter));
	if (m_defaultColor.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "DefaultColor", nullptr, std::string(), m_defaultColor.name().toStdString());
	return e;
}

} // namespace VICUS
