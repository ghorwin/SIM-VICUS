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

#include "NANDRAD_InternalMoistureLoadsModel.h"

namespace NANDRAD {

void InternalMoistureLoadsModel::checkParameters(const NANDRAD::SimulationParameter& simPara) const {

	FUNCID(InternalMoistureLoadsModel::checkParameters);

	// model is only allowed for hygrothermal simulation
	if(!simPara.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled())
		throw IBK::Exception(IBK::FormatString("Model is only allowed for hygrothermal simulation!"),
							 FUNC_ID);

	// Required attribute
	IBK_ASSERT(m_modelType != NUM_MT);

	// check parameters for constant model
	if (m_modelType == MT_Constant) {
		// check fixed parameters
		m_para[P_PersonMoistureLoadPerArea].checkedValue("PersonMoistureLoadPerArea", "kg/m2s", "kg/m2s",
													 0, true,
													 std::numeric_limits<double>::max(), true,
								 "Person moisture load per area must be >= 0 kg/m2s.");

	}
	else { // modelType == MT_Scheduled
		// invalid constant definitions
		if(!m_para[P_PersonMoistureLoadPerArea].name.empty()) {
			throw IBK::Exception(IBK::FormatString("Invalid parameter 'PersonMoistureLoadPerArea' for model type 'Scheduled'!"),
								 FUNC_ID);
		}
	}
}

bool InternalMoistureLoadsModel::equal(const InternalMoistureLoadsModel & other) const {
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

