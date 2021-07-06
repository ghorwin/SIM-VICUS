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

#include "NANDRAD_InternalLoadsModel.h"

namespace NANDRAD {

void InternalLoadsModel::checkParameters() const {

	FUNCID(InternalLoadsModel::checkParameters);

	// Required attribute
	IBK_ASSERT(m_modelType != NUM_MT);

	// check fixed parameters
	m_para[P_EquipmentRadiationFraction].checkedValue("EquipmentRadiationFraction", "---", "---",
												   0, true,
												   1, true,
								   "Equipment radiant faction must between 0 and 1.");

	m_para[P_PersonRadiationFraction].checkedValue("PersonRadiationFraction", "---", "---",
												   0, true,
												   1, true,
								   "Person radiant faction must between 0 and 1.");

	m_para[P_LightingRadiationFraction].checkedValue("LightingRadiationFraction", "---", "---",
												   0, true,
												   1, true,
								   "Lighting radiant faction must between 0 and 1.");

	// check parameters for constant model
	if (m_modelType == MT_Constant) {
		// check fixed parameters
		m_para[P_EquipmentHeatLoadPerArea].checkedValue("EquipmentHeatLoadPerArea", "W/m2", "W/m2",
													   0, true,
													   std::numeric_limits<double>::max(), true,
								   "Equipment heat load per area must be >= 0 W/m2.");

		m_para[P_PersonHeatLoadPerArea].checkedValue("PersonHeatLoadPerArea", "W/m2", "W/m2",
													 0, true,
													 std::numeric_limits<double>::max(), true,
								 "Person heat load per area must be >= 0 W/m2.");

		m_para[P_LightingHeatLoadPerArea].checkedValue("LightingHeatLoadPerArea", "W/m2", "W/m2",
													   0, true,
													   std::numeric_limits<double>::max(), true,
								   "Lighting heat load per area must be >= 0 W/m2.");
	}
	else { // modelType == MT_Scheduled
		// invalid constant definitions
		if(!m_para[P_EquipmentHeatLoadPerArea].name.empty()) {
			throw IBK::Exception(IBK::FormatString("Invalid parameter 'EqupimentHeatLoadPerArea' for model type 'Scheduled'!"),
								 FUNC_ID);
		}
		if(!m_para[P_PersonHeatLoadPerArea].name.empty()) {
			throw IBK::Exception(IBK::FormatString("Invalid parameter 'PersonHeatLoadPerArea' for model type 'Scheduled'!"),
								 FUNC_ID);
		}
		if(!m_para[P_LightingHeatLoadPerArea].name.empty()) {
			throw IBK::Exception(IBK::FormatString("Invalid parameter 'LightingHeatLoadPerArea' for model type 'Scheduled'!"),
								 FUNC_ID);
		}
	}
}

bool InternalLoadsModel::equal(const InternalLoadsModel & other) const {
	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != other.m_para[i])
			return false;
	}

	if(m_modelType != other.m_modelType)
		return false;

	return true;
}

} // namespace NANDRAD

