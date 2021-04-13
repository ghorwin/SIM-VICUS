#include "NM_ShadingControlModel.h"


#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_ShadingControlModel.h>
#include <NANDRAD_Sensor.h>


namespace NANDRAD_MODEL {


void ShadingControlModel::setup(const NANDRAD::ShadingControlModel & controller)
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

void ShadingControlModel::inputReferences(std::vector<InputReference> & inputRefs) const
{
	// a sensor is referenced
	if(m_controller->m_sensor != nullptr) {
		InputReference ref;
		ref.m_id = 0;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
		ref.m_name.m_name = "GlobalSWRadOnPlane";
		ref.m_name.m_index = (int) m_controller->m_sensorID;
		inputRefs.push_back(ref);
	}
	// a construction/embedded object is referenced
	else { //(m_controller->m_constructionInstance != nullptr)
		InputReference ref;
		ref.m_id = m_controller->m_constructionInstance->m_id;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		// link to correct side of construction
		if(m_controller->m_constructionInstance->interfaceAZoneID() == 0) {
			ref.m_name.m_name = "FluxShortWaveRadiationA";
		}
		else {
			ref.m_name.m_name = "FluxShortWaveRadiationB";
		}
		inputRefs.push_back(ref);
	}
}

void ShadingControlModel::setInputValueRefs(const std::vector<QuantityDescription> &, const std::vector<const double *> & resultValueRefs)
{
	if(resultValueRefs.size() == 1)
		m_SWRadiationRef = resultValueRefs[0];
}

void ShadingControlModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const
{
	resultInputValueReferences.push_back(std::make_pair(&m_controllerOutput, m_SWRadiationRef));
}

int ShadingControlModel::update()
{
	// set current state value from sensor
	m_currentState = *m_SWRadiationRef;
	// calculate controller output
	updateControllerOutput();
	// signal success
	return 0;
}



} // namespace NANDRAD_MODEL
