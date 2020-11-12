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
				inputRefs[InputRef_SideATemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceA.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				inputRefs[InputRef_SideATemperature] = ref;
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
				inputRefs[InputRef_SideBTemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceB.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				inputRefs[InputRef_SideBTemperature] = ref;
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

	// if either interface is INVALID/unavailable than window is adiabatic surface and heat conduction flux is zero
	if (m_valueRefs[InputRef_SideATemperature] == nullptr || m_valueRefs[InputRef_SideBTemperature] == nullptr) {
		m_results[R_FluxHeatConductionA] = 0;
		m_results[R_FluxHeatConductionB] = 0;
	}
	else {

		// compute heat flux from side A to B

		double deltaT = *m_valueRefs[InputRef_SideATemperature] - *m_valueRefs[InputRef_SideBTemperature];

		// TODO : distinguish between standard/detailed model


		// compute mean alpha value for glass, frame and divider

		//    alpha_mean = (alpha_glass * area_glass + alpha_frame * area_frame + ...)/(area_glass + area_frame + ...)

		// compute mean resistance

		//   res_mean = 1/alpha_mean

		// add surface resistances

		//    res_total = res_left + res_mean + res_right

		// compute flux

		//    heatCondFlux = 1/res_total * deltaT



		// heat flux = (sum_i  A_i * U_i / sum_i A_i) * deltaT
//		double meanU = m_windowModel->m_glasArea * m_windowModel->m_glazingSystem->m_para[NANDRAD::WindowGlazingSystem::P_ThermalTransmittance].value;

//		// add frame if existing
//		if (m_windowModel->m_frame.m_materialID != NANDRAD::INVALID_ID)
//			meanU += m_windowModel->m_frame.m_area.value * m_windowModel->m_frame.m_lambda

	}




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
	else {
		m_results[R_FluxShortWaveRadiationA] = 0;
		m_results[R_FluxShortWaveRadiationB] = 0;
	}

}


void WindowModel::computeSolarFlux(double qGlobal, double incidenceAngle, bool fromSideA) {
	// TODO :
	m_results[R_FluxShortWaveRadiationA] = 0;
	m_results[R_FluxShortWaveRadiationB] = 0;
}



} // namespace NANDRAD_MODEL
