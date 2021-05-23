/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#include "NANDRAD_InterfaceHeatConduction.h"

namespace NANDRAD {

bool InterfaceHeatConduction::operator!=(const InterfaceHeatConduction & other) const {
	// model comparison
	if (m_modelType != other.m_modelType) return true;
	for (int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	return false; // not different
}


void InterfaceHeatConduction::checkParameters() const {
	// only check parameters if model is enable

	switch (m_modelType) {
		case MT_Constant:
			m_para[P_HeatTransferCoefficient].checkedValue("HeatTransferCoefficient", "W/m2K", "W/m2K",
													   0, true,
													   std::numeric_limits<double>::max(), true,
													   "Heat transfer coefficient must be >= 0 W/m2K.");
		break;
		case NUM_MT:
		return;
	}

}

} // namespace NANDRAD

