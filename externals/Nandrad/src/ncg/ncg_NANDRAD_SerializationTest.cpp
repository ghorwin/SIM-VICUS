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

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_KeywordList.h>

#include <tinyxml.h>

namespace NANDRAD {

TiXmlElement * SerializationTest::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("SerializationTest");
	parent->LinkEndChild(e);

	e->SetAttribute("id1", IBK::val2string<int>(m_id1));
	e->SetAttribute("id2", IBK::val2string<unsigned int>(m_id2));
	e->SetAttribute("flag1", IBK::val2string<bool>(m_flag1));
	e->SetAttribute("val1", IBK::val2string<double>(m_val1));
	e->SetAttribute("testBla", KeywordList::Keyword("SerializationTest::test_t",  m_testBla));
	e->SetAttribute("str1", m_str1);
	e->SetAttribute("path1", m_path1.str());
	e->SetAttribute("u1", m_u1.name());

	TiXmlElement::appendSingleAttributeElement(e, "Id3", nullptr, std::string(), IBK::val2string<int>(m_id3));

	TiXmlElement::appendSingleAttributeElement(e, "Id4", nullptr, std::string(), IBK::val2string<unsigned int>(m_id4));

	TiXmlElement::appendSingleAttributeElement(e, "Flag2", nullptr, std::string(), IBK::val2string<bool>(m_flag2));

	TiXmlElement::appendSingleAttributeElement(e, "Val2", nullptr, std::string(), IBK::val2string<double>(m_val2));

	TiXmlElement::appendSingleAttributeElement(e, "TestBlo", nullptr, std::string(), KeywordList::Keyword("SerializationTest::test_t",  m_testBlo));

	TiXmlElement::appendSingleAttributeElement(e, "Str2", nullptr, std::string(), m_str2);

	TiXmlElement::appendSingleAttributeElement(e, "Path2", nullptr, std::string(), m_path2.str());

	TiXmlElement::appendSingleAttributeElement(e, "U2", nullptr, std::string(), m_u2.name());

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

	for (int i=0; i<NUM_test; ++i) {
		if (!m_flags[i].name().empty())
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
	}
	return e;
}

} // namespace NANDRAD
