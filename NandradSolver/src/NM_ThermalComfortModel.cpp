#include "NM_ThermalComfortModel.h"

#include <IBK_Exception.h>
#include <IBK_physics.h>

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>
#include "NM_ConstructionStatesModel.h"
#include "NM_RoomStatesModel.h"
#include "NM_WindowModel.h"

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {



void ThermalComfortModel::setup() {
	FUNCID(ThermalComfortModel::setup);

}


void ThermalComfortModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	QuantityDescription result;
	result.m_constant = true;
	result.m_description = NANDRAD_MODEL::KeywordList::Description("ThermalComfortModel::Results", R_OperativeTemperature);
	result.m_name = NANDRAD_MODEL::KeywordList::Keyword("ThermalComfortModel::Results", R_OperativeTemperature);
	result.m_displayName = m_displayName;
	result.m_unit = NANDRAD_MODEL::KeywordList::Unit("ThermalComfortModel::Results", R_OperativeTemperature);

	resDesc.push_back(result);
}


const double * ThermalComfortModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	IBK_ASSERT(quantityName.m_name == NANDRAD_MODEL::KeywordList::Description("ThermalComfortModel::Results", R_OperativeTemperature));
	return &m_operativeTemperature;
}


void ThermalComfortModel::initInputReferences(const std::vector<AbstractModel *> & models) {
#if 0
	std::vector<InputReference> heatCondIR;
	std::vector<InputReference> windowHeatCondIR;

	// search all models for construction models that have an interface to this zone
	for (AbstractModel * model : models) {

		// *** surface temperature from walls ***
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE) {
			// we need a construction states model here
			ConstructionStatesModel* conMod = dynamic_cast<ConstructionStatesModel*>(model);
			if (conMod == nullptr) continue;

			// check if either interface references us

			// side A
			if (conMod->interfaceAZoneID() == m_id) {
				// create input reference for surface temperature
				InputReference r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name.m_name = "SurfaceTemperatureA";
				heatCondIR.push_back(r);
			}

			// check if either interface references us
			if (conMod->interfaceBZoneID() == m_id) {
				// create input reference for heat conduction fluxes into this zone
				InputReference r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name.m_name = "SurfaceTemperatureB";
				heatCondIR.push_back(r);
			}
		}

		// *** heat conduction and solar radiation loads through windows ***
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT) {
			// we need a window model here
			WindowModel* mod = dynamic_cast<WindowModel*>(model);
			if (mod == nullptr) continue;

			// check if either interface references us

			// side A
			if (mod->interfaceAZoneID() == m_id) {
				// create input reference for heat conduction fluxes into this zone
				InputReference r;
				r.m_id = mod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
				r.m_name.m_name = "FluxHeatConductionA";
				m_windowHeatCondValueRefs.push_back(nullptr);
				windowHeatCondIR.push_back(r);
			}

			// check if either interface references us
			if (mod->interfaceBZoneID() == m_id) {
				// create input reference for heat conduction fluxes into this zone
				InputReference r;
				r.m_id = mod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
				r.m_name.m_name = "FluxHeatConductionB";
				m_windowHeatCondValueRefs.push_back(nullptr);
				windowHeatCondIR.push_back(r);
			}
		}

		}
	} // model object loop


	// now combine the input references into the global vector
	m_inputRefs = heatCondIR;
	m_inputRefs.insert(m_inputRefs.end(), windowHeatCondIR.begin(), windowHeatCondIR.end());
	if (m_haveSolarRadiationModel)
		m_inputRefs.push_back(windowSolarRadIR);
	m_inputRefs.insert(m_inputRefs.end(), ventilationRH.begin(), ventilationRH.end());
	m_inputRefs.insert(m_inputRefs.end(), internalLoadsRH.begin(), internalLoadsRH.end());
	m_inputRefs.insert(m_inputRefs.end(), networkLoadsRH.begin(), networkLoadsRH.end());
#endif
}


void ThermalComfortModel::inputReferences(std::vector<InputReference> & inputRefs) const {



}


void ThermalComfortModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	// simply store and check value references
	m_valueRefs = resultValueRefs;
}


void ThermalComfortModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// operative temperature depends on all input variables
	for (const double * valRef : m_valueRefs) {
		// dependency on room air temperature of corresponding zone
		resultInputValueReferences.push_back(std::make_pair(&m_operativeTemperature, valRef) );
	}
}


int ThermalComfortModel::update() {

	// compute radiant temperature as area-weighted mean temperature
	// add room air temperature



	return 0; // signal success
}


} // namespace NANDRAD_MODEL
