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

#include "VICUS_Outputs.h"

namespace VICUS {

bool Outputs::operator!=(const Outputs & other) const {
	for (int i=0; i<NUM_F; ++i)
		if (m_flags[i] != other.m_flags[i])
			return true;
	if (m_timeUnit != other.m_timeUnit)
		return true;
	if (m_definitions != other.m_definitions)
		return true;
	if (m_grids != other.m_grids)
		return true;

	return false;
}


void Outputs::initDefaults() {
	m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].set("CreateDefaultZoneOutputs", true);
}


} // namespace VICUS
