#include "NM_ShadingControlModel.h"

#include "NM_Loads.h"


#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_ShadingControlModel.h>
#include <NANDRAD_Sensor.h>


namespace NANDRAD_MODEL {


void ShadingControlModel::setup(const NANDRAD::ShadingControlModel & controller,
								const Loads &loads)
{
	// overwrite tolerance band
	double minValue = controller.m_para[NANDRAD::ShadingControlModel::P_MaxIntensity].value;
	double maxValue = controller.m_para[NANDRAD::ShadingControlModel::P_MinIntensity].value;
	// target value is average
	m_targetValue = 0.5 * (minValue + maxValue);
	// tolerance band is mean diffence to target
	m_hysteresisBand = 0.5 * (maxValue - minValue);
	// copy controller
	m_controller = &controller;
	// store loads
	m_loads = &loads;
}

void ShadingControlModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const
{
	QuantityDescription result;
	result.m_constant = true;
	result.m_description = "Controlled shading factor [0,...,1].";
	result.m_name = "ShadingFactor";
	result.m_displayName = m_displayName;
	result.m_unit = "---";

	resDesc.push_back(result);
}

void ShadingControlModel::resultValueRefs(std::vector<const double *> & res) const
{
	res.push_back(&m_controllerOutput);
}

const double *ShadingControlModel::resultValueRef(const InputReference & quantity) const
{
	const QuantityName & quantityName = quantity.m_name;
	if(quantityName.m_name == "ShadingFactor")
		return &m_controllerOutput;

	return nullptr;
}

int ShadingControlModel::update()
{
	// set current state value from sensor
	double qSWRadDir, qSWRadDiff, incidenceAngle;

	m_currentState = m_loads->qSWRad(m_controller->m_sensorID, qSWRadDir, qSWRadDiff, incidenceAngle);
	// calculate controller output
	updateControllerOutput();
	// signal success
	return 0;
}



} // namespace NANDRAD_MODEL
