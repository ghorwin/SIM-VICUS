/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#include "NM_ConstantZoneModel.h"

#include <NANDRAD_Zone.h>

namespace NANDRAD_MODEL {


void ConstantZoneModel::setup(const NANDRAD::Zone & zone) {
	// copy zone type
	m_zoneType = zone.m_type;
	// Initialize pointer to temperature with constant parameter value for a zone of type 'Constant'.
	// This is a required parameter and was checked for in Zone::checkParameters().
	if(m_zoneType == NANDRAD::Zone::ZT_Constant)
		m_temperature = &zone.m_para[NANDRAD::Zone::P_Temperature].value;
}


void ConstantZoneModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	QuantityDescription result;

	// temperature value
	result.m_constant = true;
	result.m_description = "Predefined zone (air) temperature";
	result.m_name = "AirTemperature";
	result.m_unit = "C";

	resDesc.push_back(result);
}


const double * ConstantZoneModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;

	// 'setInputValueRefs' is always called before 'resultValueRef'
	// so we can transport input references from schedules toward the dependend model

	if (quantityName.m_name == "AirTemperature")
		return m_temperature;

	return nullptr;
}


void ConstantZoneModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// set reference to temperature schedule for zone type 'Schedule'
	if(m_zoneType == NANDRAD::Zone::ZT_Scheduled) {
		InputReference inputRef;
		inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		inputRef.m_name = std::string("TemperatureSchedule");
		inputRef.m_required = true;
		inputRef.m_id = m_id;
		inputRefs.push_back(inputRef);
	}
}


void ConstantZoneModel::setInputValueRefs(const std::vector<QuantityDescription> &, const std::vector<const double *> & resultValueRefs) {
	// if schedule is provided, overwrite constant definition
	if(m_zoneType == NANDRAD::Zone::ZT_Scheduled)
		m_temperature = resultValueRefs[0];
}

} // namespace NANDRAD_MODEL
