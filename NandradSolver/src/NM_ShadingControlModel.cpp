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


void ShadingControlModel::resultValueRefs(std::vector<const double *> & res) const {
	res.push_back(&m_controller.m_controlValue);
	// TODO : is this really needed? what about the second output?
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


int ShadingControlModel::setTime(double /*t*/) {
	// set current state value from sensor
	double qSWRadDir, qSWRadDiff, incidenceAngle;

	// update the radiation sensor value (this includes precomputed shading in case of constructions or embedded objects)
	m_currentIntensity = m_loads->qSWRad(m_shadingControlModel->m_sensorID, qSWRadDir, qSWRadDiff, incidenceAngle);
	// update controller state: error value = currentIntensity - setpointIntensity
	m_controller.update(m_currentIntensity - m_targetValue);

	// signal success
	return 0;
}


void ShadingControlModel::stepCompleted(double t) {
	m_controller.stepCompleted(t);
}


} // namespace NANDRAD_MODEL
