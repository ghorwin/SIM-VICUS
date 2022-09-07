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
				m_id1 = NANDRAD::readPODAttributeValue<int>(element, attrib);
			else if (attribName == "id2")
				m_id2 = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "flag1")
				m_flag1 = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "val1")
				m_val1 = NANDRAD::readPODAttributeValue<double>(element, attrib);
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
			else if (attribName == "someStuffIDAsAttrib")
				m_someStuffIDAsAttrib = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
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
				m_id3 = NANDRAD::readPODElement<int>(c, cName);
			else if (cName == "Id4")
				m_id4 = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "Flag2")
				m_flag2 = NANDRAD::readPODElement<bool>(c, cName);
			else if (cName == "Val2")
				m_val2 = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Str2")
				m_str2 = c->GetText();
			else if (cName == "Path2")
				m_path2 = IBK::Path(c->GetText());
			else if (cName == "Path22")
				m_path22 = IBK::Path(c->GetText());
			else if (cName == "U2")
				m_u2 = NANDRAD::readUnitElement(c, cName);
			else if (cName == "X5")
				m_x5 = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				NANDRAD::readFlagElement(c, f);
				bool success = false;
				if (f.name() == "F") {
					m_f = f; success=true;
				}
				else if (f.name() == "F2") {
					m_f2 = f; success=true;
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
				m_time1 = NANDRAD::readTimeElement(c, cName);
			else if (cName == "Time2")
				m_time2 = NANDRAD::readTimeElement(c, cName);
			else if (cName == "Table")
				m_table.setEncodedString(c->GetText());
			else if (cName == "Table2")
				m_table2.setEncodedString(c->GetText());
			else if (cName == "DblVec")
				NANDRAD::readVector(c, "DblVec", m_dblVec);
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
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				if (p.name == "SinglePara") {
					m_singlePara = p; success = true;
				}
				if (!success) {
				test_t ptype;
				try {
					ptype = (test_t)KeywordList::Enumeration("SerializationTest::test_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "IBK:IntPara") {
				IBK::IntPara p;
				NANDRAD::readIntParaElement(c, p);
				bool success = false;
				if (p.name == "SingleIntegerPara") {
					m_singleIntegerPara = p; success = true;
				}
				try {
					intPara_t ptype = (intPara_t)KeywordList::Enumeration("SerializationTest::intPara_t", p.name);
					m_intPara[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "SomeStuffIDAsElement")
				m_someStuffIDAsElement = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IBK:LinearSpline") {
				IBK::LinearSpline p;
				std::string name;
				NANDRAD::readLinearSplineElement(c, p, name, nullptr, nullptr);
				bool success = false;
				if (name == "LinSpl") {
					m_linSpl = p; success = true;
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "LinearSplineParameter") {
				NANDRAD::LinearSplineParameter p;
				p.readXML(c);
				bool success = false;
				if (p.m_name == "SplineParameter") {
					m_splineParameter = p; success = true;
				}
				else if (p.m_name == "AnotherSplineParameter") {
					m_anotherSplineParameter = p; success = true;
				}
				try {
					splinePara_t ptype;
					ptype = (splinePara_t)KeywordList::Enumeration("SerializationTest::splinePara_t", p.m_name);
					m_splinePara[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.m_name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "Coordinate2D")
				NANDRAD::readPoint2D(c, "Coordinate2D", m_coordinate2D);
			else if (cName == "TestBlo") {
				try {
					m_testBlo = (test_t)KeywordList::Enumeration("SerializationTest::test_t", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else if (cName == "InterfaceA")
				m_interfaceA.readXML(c);
			else if (cName == "Schedule")
				m_sched.readXML(c);
			else if (cName == "OtherSchedule")
				m_sched2.readXML(c);
			else {
				bool found = false;
				for (int i=0; i<NUM_RefID; ++i) {
					if (cName == KeywordList::Keyword("SerializationTest::ReferencedIDTypes",i)) {
						m_idReferences[i] = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
						found = true;
						break;
					}
				}
				if (!found)
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
	if (m_id2 != NANDRAD::INVALID_ID)
		e->SetAttribute("id2", IBK::val2string<unsigned int>(m_id2));
	if (m_flag1 != SerializationTest().m_flag1)
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
	if (m_someStuffIDAsAttrib != NANDRAD::INVALID_ID)
		e->SetAttribute("someStuffIDAsAttrib", IBK::val2string<IDType>(m_someStuffIDAsAttrib));
	TiXmlElement::appendSingleAttributeElement(e, "Id3", nullptr, std::string(), IBK::val2string<int>(m_id3));
	if (m_id4 != NANDRAD::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "Id4", nullptr, std::string(), IBK::val2string<unsigned int>(m_id4));
	TiXmlElement::appendSingleAttributeElement(e, "Flag2", nullptr, std::string(), IBK::val2string<bool>(m_flag2));
	TiXmlElement::appendSingleAttributeElement(e, "Val2", nullptr, std::string(), IBK::val2string<double>(m_val2));

	if (m_testBlo != NUM_test)
		TiXmlElement::appendSingleAttributeElement(e, "TestBlo", nullptr, std::string(), KeywordList::Keyword("SerializationTest::test_t",  m_testBlo));
	if (!m_str2.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Str2", nullptr, std::string(), m_str2);
	if (m_path2.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "Path2", nullptr, std::string(), m_path2.str());
	if (m_path22.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "Path22", nullptr, std::string(), m_path22.str());
	if (m_u2.id() != 0)
		TiXmlElement::appendSingleAttributeElement(e, "U2", nullptr, std::string(), m_u2.name());
	TiXmlElement::appendSingleAttributeElement(e, "X5", nullptr, std::string(), IBK::val2string<double>(m_x5));
	if (!m_f.name().empty()) {
		IBK_ASSERT("F" == m_f.name());
		TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", "F", m_f.isEnabled() ? "true" : "false");
	}
	if (!m_f2.name().empty()) {
		IBK_ASSERT("F2" == m_f2.name());
		TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", "F2", m_f2.isEnabled() ? "true" : "false");
	}
	if (m_time1 != IBK::Time())
		TiXmlElement::appendSingleAttributeElement(e, "Time1", nullptr, std::string(), m_time1.toShortDateFormat());
	if (m_time2 != IBK::Time())
		TiXmlElement::appendSingleAttributeElement(e, "Time2", nullptr, std::string(), m_time2.toShortDateFormat());
	if (!m_table.m_values.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Table", nullptr, std::string(), m_table.encodedString());
	if (!m_table2.m_values.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Table2", nullptr, std::string(), m_table2.encodedString());
	NANDRAD::writeVector(e, "DblVec", m_dblVec);

	if (!m_interfaces.empty()) {
		TiXmlElement * child = new TiXmlElement("Interfaces");
		e->LinkEndChild(child);

		for (std::vector<Interface>::const_iterator it = m_interfaces.begin();
			it != m_interfaces.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	{
		TiXmlElement * customElement = m_interfaceA.writeXML(e);
		if (customElement != nullptr)
			customElement->ToElement()->SetValue("InterfaceA");
	}
	if (!m_singlePara.name.empty()) {
		IBK_ASSERT("SinglePara" == m_singlePara.name);
		TiXmlElement::appendIBKParameterElement(e, "SinglePara", m_singlePara.IO_unit.name(), m_singlePara.get_value(m_singlePara.IO_unit));
	}
	if (!m_singleIntegerPara.name.empty()) {
		IBK_ASSERT("SingleIntegerPara" == m_singleIntegerPara.name);
		TiXmlElement::appendSingleAttributeElement(e, "IBK:IntPara", "name", "SingleIntegerPara", IBK::val2string(m_singleIntegerPara.value));
	}

	for (unsigned int i=0; i<NUM_test; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}

	for (unsigned int i=0; i<NUM_IP; ++i) {
		if (!m_intPara[i].name.empty()) {
			TiXmlElement::appendSingleAttributeElement(e, "IBK:IntPara", "name", m_intPara[i].name, IBK::val2string(m_intPara[i].value));
		}
	}

	for (int i=0; i<NUM_test; ++i) {
		if (!m_flags[i].name().empty()) {
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
		}
	}
	if (m_someStuffIDAsElement != NANDRAD::INVALID_ID)
			TiXmlElement::appendSingleAttributeElement(e, "SomeStuffIDAsElement", nullptr, std::string(), IBK::val2string<unsigned int>(m_someStuffIDAsElement));

	for (int i=0; i<NUM_RefID; ++i) {
		if (m_idReferences[i] != NANDRAD::INVALID_ID)
				TiXmlElement::appendSingleAttributeElement(e, KeywordList::Keyword("SerializationTest::ReferencedIDTypes",  i), nullptr, std::string(), IBK::val2string<unsigned int>(m_idReferences[i]));
	}
	if (!m_linSpl.empty())
		NANDRAD::writeLinearSplineElement(e, "LinSpl", m_linSpl, "-", "-");
	if (!m_splineParameter.m_name.empty()) {
		IBK_ASSERT("SplineParameter" == m_splineParameter.m_name);
		m_splineParameter.writeXML(e);
	}
	if (!m_anotherSplineParameter.m_name.empty()) {
		IBK_ASSERT("AnotherSplineParameter" == m_anotherSplineParameter.m_name);
		m_anotherSplineParameter.writeXML(e);
	}
	for (int i=0; i<NUM_SP; ++i) {
		if (!m_splinePara[i].m_name.empty()) {
			m_splinePara[i].writeXML(e);
		}
	}

	m_sched.writeXML(e);

	{
		TiXmlElement * customElement = m_sched2.writeXML(e);
		if (customElement != nullptr)
			customElement->ToElement()->SetValue("OtherSchedule");
	}
	NANDRAD::writePoint2D(e, "Coordinate2D", m_coordinate2D);
	return e;
}

} // namespace NANDRAD
