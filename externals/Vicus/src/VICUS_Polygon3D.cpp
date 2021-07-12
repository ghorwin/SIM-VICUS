/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "VICUS_Polygon3D.h"

#include <set>

#include <QLineF>
#include <QPolygonF>

#include <IBK_Line.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include <IBKMK_3DCalculations.h>

#include <NANDRAD_Utilities.h>


#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

// *** Polygon3D ***


void Polygon3D::readXML(const TiXmlElement * element) {
	FUNCID(Polygon3D::readXML);

	// read data (throws an exception in case of errors)
	NANDRAD::readVector3D(element, "Polygon3D", m_vertexes);

	unsigned int nVert = m_vertexes.size();
	checkPolygon();
	if (nVert != m_vertexes.size())
		IBK::IBK_Message(IBK::FormatString("Invalid polygon in project, removed invalid vertexes."), IBK::MSG_WARNING, FUNC_ID);
}


TiXmlElement * Polygon3D::writeXML(TiXmlElement * parent) const {
	if (*this == Polygon3D())
		return nullptr;

	return NANDRAD::writeVector3D(parent, "Polygon3D", m_vertexes);
}


// Comparison operator !=
bool Polygon3D::operator!=(const Polygon3D &other) const {
	if (m_type != other.m_type)
		return true;
	if (m_vertexes != other.m_vertexes)
		return true;
	return false;
}



} // namespace VICUS

