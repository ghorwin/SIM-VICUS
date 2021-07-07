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

#include "NANDRAD_IdealHeatingCoolingModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void IdealHeatingCoolingModel::checkParameters() const {
	m_para[P_MaxHeatingPowerPerArea].checkedValue("MaxHeatingPowerPerArea", "W/m2", "W/m2",
											   0, true,
											   std::numeric_limits<double>::max(), true,
											   "Maximum for heating power must be >= 0 W/m2.");
	m_para[P_MaxCoolingPowerPerArea].checkedValue("MaxCoolingPowerPerArea", "W/m2", "W/m2",
											   0, true,
											   std::numeric_limits<double>::max(), true,
											   "Maximum for cooling power must be >= 0 W/m2.");
	if (!m_para[P_Kp].name.empty())
		m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true,
											   "Kp value for PI controller.");
	if (!m_para[P_Ki].name.empty())
		m_para[P_Ki].checkedValue("Ki", "---", "---", 0, false, std::numeric_limits<double>::max(), true,
								  "Ki value for PI controller.");
}

bool IdealHeatingCoolingModel::equal(const IdealHeatingCoolingModel & other) const {
	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != other.m_para[i])
			return false;
	}
	return true;
}


} // namespace NANDRAD

