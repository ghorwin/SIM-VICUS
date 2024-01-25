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

#include "VICUS_OutputDefinition.h"

namespace VICUS {

bool OutputDefinition::operator!=(const OutputDefinition & other) const {

	if (m_quantity != other.m_quantity)
		return true;
	if (m_sourceObjectType != other.m_sourceObjectType)
		return true;
	if (m_timeType != other.m_timeType)
		return true;
	if (m_sourceObjectIds != other.m_sourceObjectIds)
		return true;
	if (m_vectorIdMap != other.m_vectorIdMap)
		return true;
	if (m_gridName != other.m_gridName)
		return true;

	return false;
}



} // namespace VICUS
