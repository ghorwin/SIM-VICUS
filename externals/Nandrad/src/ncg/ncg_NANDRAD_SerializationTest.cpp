/*	The NANDRAD data model library.
	Copyright (c) 2012-now, Institut fuer Bauklimatik, TU Dresden, Germany

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

#include <NANDRAD_SerializationTest.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void SerializationTest::readXML(const TiXmlElement * element) {
	FUNCID(SerializationTest::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id1"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id1' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id1")
				m_id1 = readPODAttributeValue<int>(element, attrib);
			else if (attribName == "id2")
				m_id2 = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "flag1")
				m_flag1 = readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "val1")
				m_val1 = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "testBla")
			try {
				m_testBla = (test_t)KeywordList::Enumeration("SerializationTest::test_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			else if (attribName == "str1")
				m_str1 = attrib->ValueStr();
			else if (attribName == "path1")
				m_path1 = IBK::Path(attrib->ValueStr());
			else if (attribName == "u1")
				try {
					m_u1 = IBK::Unit(attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Error converting '"+attrib->ValueStr()+"' attribute (unknown unit).") ), FUNC_ID);
				}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("Id3"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Id3' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Id3")
				m_id3 = readPODElement<int>(c, cName);
			else if (cName == "Id4")
				m_id4 = readPODElement<unsigned int>(c, cName);
			else if (cName == "Flag2")
				m_flag2 = readPODElement<bool>(c, cName);
			else if (cName == "Val2")
				m_val2 = readPODElement<double>(c, cName);
			else if (cName == "Str2")
				m_str2 = c->GetText();
			else if (cName == "Path2")
				m_path2 = IBK::Path(c->GetText());
			else if (cName == "U2")
				m_u2 = readUnitElement(c, cName);
			else if (cName == "X5")
				m_x5 = readPODElement<double>(c, cName);
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				readFlagElement(c, f);
				bool success = false;
				if (f.name() == "F") {
					m_f = f; success=true;
				}
				try {
					test_t ftype = (test_t)KeywordList::Enumeration("SerializationTest::test_t", f.name());
					m_flags[ftype] = f; success=true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "Time1")
				m_time1 = readTimeElement(c, cName);
			else if (cName == "Time2")
				m_time2 = readTimeElement(c, cName);
			else if (cName == "Table")
				m_table.setEncodedString(c->GetText());
			else if (cName == "DblVec")
				readVector(c, "DblVec", m_dblVec);
			else if (cName == "Interfaces") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Interface")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Interface obj;
					obj.readXML(c2);
					m_interfaces.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				readParameterElement(c, p);
				bool success = false;
				try {
					test_t ptype = (test_t)KeywordList::Enumeration("SerializationTest::test_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "IBK:IntPara") {
				IBK::IntPara p;
				readIntParaElement(c, p);
				bool success = false;
				try {
					intPara_t ptype = (intPara_t)KeywordList::Enumeration("SerializationTest::intPara_t", p.name);
					m_intPara[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "IBK:LinearSpline") {
				IBK::LinearSpline p;
				std::string name;
				readLinearSplineElement(c, p, name, nullptr, nullptr);
				bool success = false;
				if (name == "Spline") {
					m_spline = p; success = true;
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "TestBlo") {
				try {
					m_testBlo = (test_t)KeywordList::Enumeration("SerializationTest::test_t", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else if (cName == "Schedule")
				m_sched.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'SerializationTest' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'SerializationTest' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * SerializationTest::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("SerializationTest");
	parent->LinkEndChild(e);

	e->SetAttribute("id1", IBK::val2string<int>(m_id1));
	e->SetAttribute("id2", IBK::val2string<unsigned int>(m_id2));
	e->SetAttribute("flag1", IBK::val2string<bool>(m_flag1));
	e->SetAttribute("val1", IBK::val2string<double>(m_val1));
	if (m_testBla != NUM_test)
		e->SetAttribute("testBla", KeywordList::Keyword("SerializationTest::test_t",  m_testBla));
	if (!m_str1.empty())
		e->SetAttribute("str1", m_str1);
	if (m_path1.isValid())
		e->SetAttribute("path1", m_path1.str());
	if (m_u1.id() != 0)
		e->SetAttribute("u1", m_u1.name());
	TiXmlElement::appendSingleAttributeElement(e, "Id3", nullptr, std::string(), IBK::val2string<int>(m_id3));
	TiXmlElement::appendSingleAttributeElement(e, "Id4", nullptr, std::string(), IBK::val2string<unsigned int>(m_id4));
	TiXmlElement::appendSingleAttributeElement(e, "Flag2", nullptr, std::string(), IBK::val2string<bool>(m_flag2));
	TiXmlElement::appendSingleAttributeElement(e, "Val2", nullptr, std::string(), IBK::val2string<double>(m_val2));

	if (m_testBlo != NUM_test)
		TiXmlElement::appendSingleAttributeElement(e, "TestBlo", nullptr, std::string(), KeywordList::Keyword("SerializationTest::test_t",  m_testBlo));
	if (!m_str2.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Str2", nullptr, std::string(), m_str2);
	if (m_path2.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "Path2", nullptr, std::string(), m_path2.str());
	if (m_u2.id() != 0)
		TiXmlElement::appendSingleAttributeElement(e, "U2", nullptr, std::string(), m_u2.name());
	TiXmlElement::appendSingleAttributeElement(e, "X5", nullptr, std::string(), IBK::val2string<double>(m_x5));
	if (!m_f.name().empty())
		TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_f.name(), m_f.isEnabled() ? "true" : "false");
	if (m_time1 != IBK::Time())
		TiXmlElement::appendSingleAttributeElement(e, "Time1", nullptr, std::string(), m_time1.toShortDateFormat());
	if (m_time2 != IBK::Time())
		TiXmlElement::appendSingleAttributeElement(e, "Time2", nullptr, std::string(), m_time2.toShortDateFormat());

	m_sched.writeXML(e);
	if (!m_table.m_values.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Table", nullptr, std::string(), m_table.encodedString());
	writeVector(e, "DblVec", m_dblVec);

	if (!m_interfaces.empty()) {
		TiXmlElement * child = new TiXmlElement("Interfaces");
		e->LinkEndChild(child);

		for (std::vector<Interface>::const_iterator ifaceIt = m_interfaces.begin();
			ifaceIt != m_interfaces.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	for (unsigned int i=0; i<NUM_test; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}

	for (unsigned int i=0; i<NUM_IP; ++i) {
		if (!m_intPara[i].name.empty())
			TiXmlElement::appendSingleAttributeElement(e, "IBK:IntPara", "name", m_intPara[i].name, IBK::val2string(m_intPara[i].value));
	}

	for (int i=0; i<NUM_test; ++i) {
		if (!m_flags[i].name().empty())
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
	}
	if (!m_spline.empty())
		writeLinearSplineElement(e, "Spline", m_spline, "-", "-");
	return e;
}

} // namespace NANDRAD
