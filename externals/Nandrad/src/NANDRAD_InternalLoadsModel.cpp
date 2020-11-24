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

#include "NANDRAD_InternalLoadsModel.h"

namespace NANDRAD {

void InternalLoadsModel::checkParameters() const {

	FUNCID(InternalLoadsModel::checkParameters);

	// Required attribute
	IBK_ASSERT(m_modelType != NUM_MT);

	// check fixed parameters
	m_para[P_EquipmentRadiationFraction].checkedValue("---", "---",
												   0, true,
												   1, true,
								   "Equipment radiant faction must between 0 and 1.");

	m_para[P_PersonRadiationFraction].checkedValue("---", "---",
												   0, true,
												   1, true,
								   "Person radiant faction must between 0 and 1.");

	m_para[P_LightingRadiationFraction].checkedValue("---", "---",
												   0, true,
												   1, true,
								   "Lighting radiant faction must between 0 and 1.");

	// check parameters for constant model
	if(m_modelType == MT_Constant) {
		if(m_para[P_EquipmentHeatLoadPerArea].name.empty()) {
			throw IBK::Exception("Missing parameter 'EquipmentHeatLoadPerArea' for "
								 "model of type 'Constant'", FUNC_ID);
		}
		if(m_para[P_PersonHeatLoadPerArea].name.empty()) {
			throw IBK::Exception("Missing parameter 'PersonHeatLoadPerArea' for "
								 "model of type 'Constant'", FUNC_ID);
		}
		if(m_para[P_LightingHeatLoadPerArea].name.empty()) {
			throw IBK::Exception("Missing parameter 'LightingHeatLoadPerArea' for "
								 "model of type 'Constant'", FUNC_ID);
		}
		// check fixed parameters
		m_para[P_EquipmentHeatLoadPerArea].checkedValue("W/m2", "W/m2",
													   0, true,
													   std::numeric_limits<double>::max(), true,
								   "Equipment heat load per area must be >= 0 W/m2.");

		m_para[P_PersonHeatLoadPerArea].checkedValue("W/m2", "W/m2",
													 0, true,
													 std::numeric_limits<double>::max(), true,
								 "Person heat load per area must be >= 0 W/m2.");

		m_para[P_LightingHeatLoadPerArea].checkedValue("W/m2", "W/m2",
													   0, true,
													   std::numeric_limits<double>::max(), true,
								   "Lighting heat load per area must be >= 0 W/m2.");


	}
	else { // modelType == MT_Scheduled
		// invalide constant definitions
		if(!m_para[P_EquipmentHeatLoadPerArea].name.empty()) {
			throw IBK::Exception(IBK::FormatString("Invalid parameter 'EqupimentLoadPerArea' for model type 'Scheduled'!"),
								 FUNC_ID);
		}
		if(!m_para[P_PersonHeatLoadPerArea].name.empty()) {
			throw IBK::Exception(IBK::FormatString("Invalid parameter 'PersonLoadPerArea' for model type 'Scheduled'!"),
								 FUNC_ID);
		}
		if(!m_para[P_LightingHeatLoadPerArea].name.empty()) {
			throw IBK::Exception(IBK::FormatString("Invalid parameter 'LightingLoadPerArea' for model type 'Scheduled'!"),
								 FUNC_ID);
		}
	}
}

} // namespace NANDRAD

