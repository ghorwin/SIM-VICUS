#include "NM_WindowModel.h"

#include <NANDRAD_ConstructionInstance.h>

#include "NM_KeywordList.h"
#include "NM_Loads.h"

#include <CCM_Defines.h> // must be included last

namespace NANDRAD_MODEL {

void WindowModel::setup(const NANDRAD::EmbeddedObjectWindow & windowModelPara,
						const NANDRAD::SimulationParameter & simPara,
						const NANDRAD::ConstructionInstance & conInst,
						Loads & loads)
{

	m_con = &conInst;
	m_loads = &loads;

	// if either side of window is facing the ambient, register construction surface plane with
	// loads model for radiation load calculation
	m_haveSolarLoadsOnA = (m_con->m_interfaceA.m_id != NANDRAD::INVALID_ID && m_con->m_interfaceA.m_zoneId == 0);
	m_haveSolarLoadsOnB = (m_con->m_interfaceB.m_id != NANDRAD::INVALID_ID && m_con->m_interfaceB.m_zoneId == 0);
	if (m_haveSolarLoadsOnA || m_haveSolarLoadsOnB)
		loads.addSurface(m_con->m_id,
				m_con->m_para[NANDRAD::ConstructionInstance::P_Orientation].value/DEG2RAD,
				m_con->m_para[NANDRAD::ConstructionInstance::P_Inclination].value/DEG2RAD);

	// TODO : warn if solar loads on either side --> should be signaled already on construction level

	m_results.resize(NUM_R);
}


void WindowModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	for (int i=0; i<NUM_R; ++i) {
		QuantityDescription result;
		result.m_constant = false;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("WindowModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("WindowModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("WindowModel::Results", i);

		resDesc.push_back(result);
	}
}


const double * WindowModel::resultValueRef(const QuantityName & quantityName) const {
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "WindowModel::Results";

	if (KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}

	return nullptr;
}


void WindowModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// input references
	inputRefs.resize(NUM_InputRef);

	// set input references for heat conduction

	// side A

	// TODO : clarify if we need the HeatConduction parameter block to take alpha value into account also for Windows

	if (m_con->m_interfaceA.haveBCParameters()) {
		// check if we have heat conduction
		if (m_con->m_interfaceA.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
			// if ambient zone, create an input reference for
			if (m_con->m_interfaceA.m_zoneId == 0) {
				InputReference ref;
				ref.m_id = 0;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
				ref.m_name.m_name = "Temperature";
				inputRefs[InputRef_AmbientTemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceA.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				inputRefs[InputRef_RoomATemperature] = ref;
			}
		}
	}

	// side B

	if (m_con->m_interfaceB.haveBCParameters()) {
		// check if we have heat conduction
		if (m_con->m_interfaceB.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
			// if ambient zone, create an input reference for
			if (m_con->m_interfaceB.m_zoneId == 0) {
				InputReference ref;
				ref.m_id = 0;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
				ref.m_name.m_name = "Temperature";
				inputRefs[InputRef_AmbientTemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceB.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				inputRefs[InputRef_RoomBTemperature] = ref;
			}
		}
	}

}


void WindowModel::setInputValueRefs(const std::vector<QuantityDescription> &, const std::vector<const double *> & resultValueRefs) {
	m_valueRefs = resultValueRefs;
}


void WindowModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

}

int WindowModel::update() {

	// *** heat conduction flux ***


	// *** solar radiation flux

	// if we have solar loads on either side, compute radiation loads and incidence angle

	if (m_haveSolarLoadsOnA || m_haveSolarLoadsOnB) {
		double qRadDir, qRadDiff, incidenceAngle, qRadGlobal;

		// get nominal radiation fluxes across surface of this construction
		qRadGlobal = m_loads->qSWRad(m_con->m_id, qRadDir, qRadDiff, incidenceAngle);

		// compute radiation flux in direction of sun light
		if (m_haveSolarLoadsOnA)
			computeSolarFlux(qRadGlobal, incidenceAngle, true);
		else
			computeSolarFlux(qRadGlobal, incidenceAngle, false);
	}

}


void WindowModel::computeSolarFlux(double qGlobal, double incidenceAngle, bool fromSideA) {

}



} // namespace NANDRAD_MODEL
