/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "Vic3DPickObject.h"


namespace Vic3D {

const PickObject::PickResult & PickObject::pickResultNotOf(const std::set<PickObject::ResultType> & maskedTypes) const {
	// search through pick candidates until a suitable candidate was found
	unsigned int i=0;
	for (; i<m_candidates.size(); ++i) {
		// pick candidate filtered out?
		if (maskedTypes.find(m_candidates[i].m_resultType) != maskedTypes.end())
			continue;
		break; // found a candidate
	}
	// if we did not hit anything, fall back to the last point, which is the farpoint
	if (i == m_candidates.size())
		--i;
	IBK_ASSERT(i>=0);

	return m_candidates[i];
}


} // namespace Vic3D
