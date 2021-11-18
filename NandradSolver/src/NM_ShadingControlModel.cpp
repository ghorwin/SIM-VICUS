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

#include "NM_ShadingControlModel.h"

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_ShadingControlModel.h>
#include <NANDRAD_Sensor.h>

#include "NM_Loads.h"
#include "NM_Controller.h"

namespace NANDRAD_MODEL {


void ShadingControlModel::setup(const NANDRAD::ShadingControlModel & controller, const Loads &loads) {
	// overwrite tolerance band
	double maxValue = controller.m_para[NANDRAD::ShadingControlModel::P_MaxIntensity].value;
	double minValue = controller.m_para[NANDRAD::ShadingControlModel::P_MinIntensity].value;
	// target value is average
	m_targetValue = 0.5 * (minValue + maxValue);
	// tolerance band is mean difference to target
	m_controller.m_hysteresisBand = 0.5 * (maxValue - minValue);
	// copy controller parameter block
	m_shadingControlModel = &controller;
	// store loads for direct evaluation of radiation sensor value
	m_loads = &loads;
}


void ShadingControlModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	QuantityDescription result;

	// shading control factor (do not mistake with shading factor)
	result.m_constant = false;
	result.m_description = "Shading control factor [0,...,1].";
	result.m_name = "ShadingControlValue";
	result.m_unit = "---";

	resDesc.push_back(result);

	result.m_constant = false;
	result.m_description = "Solar radiation flux intensity on shading sensor [W/m2].";
	result.m_name = "SolarIntensityOnShadingSensor";
	result.m_unit = "W/m2";

	resDesc.push_back(result);
}


const double *ShadingControlModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;

	// we directly return the cached state of the controller
	if (quantityName.m_name == "ShadingControlValue")
		return &m_controller.m_controlValue;
	if (quantityName.m_name == "SolarIntensityOnShadingSensor")
		return &m_currentIntensity;

	return nullptr;
}


std::size_t ShadingControlModel::serializationSize() const {
	return m_controller.serializationSize();
}


void ShadingControlModel::serialize(void *& dataPtr) const {
	// serialize controller and shift data pointer
	m_controller.serialize(dataPtr);
}


void ShadingControlModel::deserialize(void *& dataPtr) {
	// deserialize controller and shift data pointer
	m_controller.deserialize(dataPtr);
}


int ShadingControlModel::setTime(double /*t*/) {
	// set current state value from sensor
	double qSWRadDir, qSWRadDiff, incidenceAngle;

	// update the radiation sensor value (this includes precomputed shading in case of constructions or embedded objects)
	m_currentIntensity = m_loads->qSWRad(m_shadingControlModel->m_sensorId, qSWRadDir, qSWRadDiff, incidenceAngle);
	// update controller state: error value = currentIntensity - setpointIntensity
	m_controller.update(m_currentIntensity - m_targetValue);

	// signal success
	return 0;
}


void ShadingControlModel::stepCompleted(double t) {
	m_controller.stepCompleted(t);
}


} // namespace NANDRAD_MODEL
