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

#include <NANDRAD_SolverParameter.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

TiXmlElement * SolverParameter::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("SolverParameter");
	parent->LinkEndChild(e);


	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}

	for (int i=0; i<NUM_F; ++i) {
		if (!m_flag[i].name().empty())
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flag[i].name(), m_flag[i].isEnabled() ? "true" : "false");
	}
	return e;
}

} // namespace NANDRAD
