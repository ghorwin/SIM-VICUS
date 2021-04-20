#include "NM_WindowModel.h"

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_WindowGlazingSystem.h>

#include "NM_KeywordList.h"
#include "NM_Loads.h"

#include <CCM_Defines.h> // must be included last

namespace NANDRAD_MODEL {

void WindowModel::setup(const NANDRAD::EmbeddedObjectWindow & windowModelPara,
						const NANDRAD::SimulationParameter & simPara,
						const NANDRAD::ConstructionInstance & conInst,
						Loads & loads)
{

	m_windowModel = &windowModelPara;
	m_simPara = &simPara;
	m_con = &conInst;
	m_loads = &loads;

	// if either side of window is facing the ambient, register construction surface plane with
	// loads model for radiation load calculation
	m_haveSolarLoadsOnA = (m_con->m_interfaceA.m_id != NANDRAD::INVALID_ID && m_con->m_interfaceA.m_zoneId == 0);
	m_haveSolarLoadsOnB = (m_con->m_interfaceB.m_id != NANDRAD::INVALID_ID && m_con->m_interfaceB.m_zoneId == 0);
	if (m_haveSolarLoadsOnA || m_haveSolarLoadsOnB) {
		// register our window with the embedded object's ID
		loads.addSurface(m_id,
				m_con->m_para[NANDRAD::ConstructionInstance::P_Orientation].value/DEG2RAD,
				m_con->m_para[NANDRAD::ConstructionInstance::P_Inclination].value/DEG2RAD);
	}

	// TODO : warn if solar loads on either side --> should be signaled already on construction level

	// overwrite constant shadig factor
	if(m_windowModel->m_shading.m_modelType == NANDRAD::WindowShading::MT_Constant ||
	   m_windowModel->m_shading.m_modelType == NANDRAD::WindowShading::MT_Controlled) {
		// parameters were checked already
		IBK_ASSERT(! m_windowModel->m_shading.m_para[NANDRAD::WindowShading::P_ReductionFactor].name.empty());
		m_shadingFactor = m_windowModel->m_shading.m_para[NANDRAD::WindowShading::P_ReductionFactor].value;
	}


	m_results.resize(NUM_R);
	 // these values should never appear anywhere in the simulation!
	m_results[R_FluxHeatConductionA] = 110001;
	m_results[R_FluxHeatConductionB] = 110002;
	m_results[R_FluxShortWaveRadiationA] = 110003;
	m_results[R_FluxShortWaveRadiationB] = 110004;
	m_results[R_SurfaceTemperatureA] = 110005;
	m_results[R_SurfaceTemperatureB] = 110006;
}


unsigned int WindowModel::interfaceAZoneID() const {
	if (m_con->m_interfaceA.m_id != NANDRAD::INVALID_ID)
		return m_con->m_interfaceA.m_zoneId;
	return 0;
}


unsigned int WindowModel::interfaceBZoneID() const {
	if (m_con->m_interfaceB.m_id != NANDRAD::INVALID_ID)
		return m_con->m_interfaceB.m_zoneId;
	return 0;
}


void WindowModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	for (int i=0; i<NUM_R; ++i) {
		QuantityDescription result;
		result.m_constant = false;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("WindowModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("WindowModel::Results", i);
		result.m_displayName = m_displayName;
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("WindowModel::Results", i);

		resDesc.push_back(result);
	}
}


const double * WindowModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
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

	// set shading factor as optional
	inputRefs[InputRef_ShadingControlValue].m_required = false;

	// shading control model: create reference to shading factor
	if(m_windowModel->m_shading.m_modelType == NANDRAD::WindowShading::MT_Controlled) {
		InputReference ref;
		ref.m_id = m_windowModel->m_shading.m_controlModelID;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
		ref.m_name.m_name = "ShadingControlValue";
		ref.m_required = true;
		inputRefs[InputRef_ShadingControlValue] = ref;
	}
}


void WindowModel::setInputValueRefs(const std::vector<QuantityDescription> &, const std::vector<const double *> & resultValueRefs) {
	// surface temperature and optional sahding factor
	m_valueRefs = resultValueRefs;
}


void WindowModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	// *** heat conduction flux ***

	// dependency on air temperature of both sides of the window
	if (m_valueRefs[InputRef_SideATemperature] == nullptr || m_valueRefs[InputRef_SideBTemperature] == nullptr) {
		resultInputValueReferences.push_back(
				std::make_pair(&m_results[R_FluxHeatConductionA], m_valueRefs[InputRef_SideATemperature]) );
		resultInputValueReferences.push_back(
				std::make_pair(&m_results[R_FluxHeatConductionB], m_valueRefs[InputRef_SideATemperature]) );
		resultInputValueReferences.push_back(
				std::make_pair(&m_results[R_FluxHeatConductionA], m_valueRefs[InputRef_SideBTemperature]) );
		resultInputValueReferences.push_back(
				std::make_pair(&m_results[R_FluxHeatConductionB], m_valueRefs[InputRef_SideBTemperature]) );
	}

	// *** solar radiation flux ***

	if (m_haveSolarLoadsOnA || m_haveSolarLoadsOnB) {
		// the shading factor and the radiation fluxes depend on control model, but only if a controlled shading is defined
		if (m_windowModel->m_shading.m_modelType == NANDRAD::WindowShading::MT_Controlled) {
			resultInputValueReferences.push_back(
					std::make_pair(&m_results[R_FluxShortWaveRadiationA], m_valueRefs[InputRef_ShadingControlValue]) );
			resultInputValueReferences.push_back(
					std::make_pair(&m_results[R_FluxShortWaveRadiationB], m_valueRefs[InputRef_ShadingControlValue]) );
			resultInputValueReferences.push_back(
					std::make_pair(&m_results[R_ShadingFactor], m_valueRefs[InputRef_ShadingControlValue]) );
		}
	}

}


int WindowModel::setTime(double t) {
	// update linear spline defined shading factor
	if(m_windowModel->m_shading.m_modelType == NANDRAD::WindowShading::MT_Precomputed) {
		// parameters were checked already
		IBK_ASSERT(! m_windowModel->m_shading.m_precomputedReductionFactor.m_name.empty());

		// TODO : apply time shift -> move simulation time t to "time of since beginning of start year"

		m_shadingFactor = m_windowModel->m_shading.m_precomputedReductionFactor.m_values.value(t);
	}
	return 0;
}


int WindowModel::update() {

	// *** heat conduction flux ***

	// if either interface is INVALID/unavailable than window is adiabatic surface and heat conduction flux is zero
	if (m_valueRefs[InputRef_SideATemperature] == nullptr || m_valueRefs[InputRef_SideBTemperature] == nullptr) {
		m_results[R_FluxHeatConductionA] = 0;
		m_results[R_FluxHeatConductionB] = 0;
		// surface temperatures of window depend on connected zone
		if (m_valueRefs[InputRef_SideATemperature] != nullptr)
			m_results[R_SurfaceTemperatureA] = *m_valueRefs[InputRef_SideATemperature];
		if (m_valueRefs[InputRef_SideBTemperature] != nullptr)
			m_results[R_SurfaceTemperatureB] = *m_valueRefs[InputRef_SideBTemperature];
	}
	else {
		// compute heat flux from side A to B
		double TSideA = *m_valueRefs[InputRef_SideATemperature];
		double TSideB = *m_valueRefs[InputRef_SideBTemperature];
		double deltaT = TSideA - TSideB;

		// compute thermal resistance of boundaries
		double thermalResistanceBC = 0.0;
		double alphaLeft = 0.0;
		double alphaRight = 0.0;

		// add surface resistances
		if (!m_con->m_interfaceA.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].name.empty()
				&& m_con->m_interfaceA.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value > 0)
		{
			alphaLeft = m_con->m_interfaceA.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
			thermalResistanceBC += 1.0/alphaLeft;
		}
		if (!m_con->m_interfaceB.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].name.empty()
				&& m_con->m_interfaceB.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value > 0)
		{
			alphaRight = m_con->m_interfaceB.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
			thermalResistanceBC += 1.0/alphaRight;
		}

		// compute heat conduction flux density [W/m2] through the glazing system
		double heatCondLeft = 0;
		double heatCondRight = 0;
		double surfaceTempA = 0;
		double surfaceTempB = 0;

		// we assume parallel heat flows through glazing system, frame and divider

		// calculate glazing system
		IBK_ASSERT(m_windowModel->m_glazingSystem != NULL);
		m_windowModel->m_glazingSystem->computeHeatConductionFluxDensity
				(TSideA, TSideB, alphaLeft, alphaRight, heatCondLeft, heatCondRight, surfaceTempA, surfaceTempB);

		// translate into heat flux [W]
		double fluxHeatCondLeft = heatCondLeft * m_windowModel->m_glasArea;
		double fluxHeatCondRight = heatCondRight * m_windowModel->m_glasArea;

		// we ignore the frame and divider and simply return the glazing surface temperature for now
		m_results[R_SurfaceTemperatureA] = surfaceTempA;
		m_results[R_SurfaceTemperatureB] = surfaceTempB;

		// check if we have a frame
		if (m_windowModel->m_frame.m_materialID != NANDRAD::INVALID_ID) {
			// parameters were checked for validity already
			IBK_ASSERT(m_windowModel->m_frame.m_lambda > 0);
			IBK_ASSERT(m_windowModel->m_frame.m_thickness.value > 0);
			// calculate heat transfer coefficient
			double alphaFrame = m_windowModel->m_frame.m_lambda/m_windowModel->m_frame.m_thickness.value;
			// correct by boundary resistances
			double alpha = 1.0/(1.0/alphaFrame + thermalResistanceBC);
			double fluxHeatCondFrameLeft = alpha * m_windowModel->m_frame.m_area.value * deltaT;
			// add flux to heat conduction flux
			fluxHeatCondLeft += fluxHeatCondFrameLeft;
			fluxHeatCondRight -= fluxHeatCondFrameLeft;
		}
		// check if we have a divider
		if (m_windowModel->m_divider.m_materialID != NANDRAD::INVALID_ID) {
			// parameters were checked for validity already
			IBK_ASSERT(m_windowModel->m_divider.m_lambda > 0);
			IBK_ASSERT(m_windowModel->m_divider.m_thickness.value > 0);
			// calculate heat transfer coefficient
			double alphaDivider = m_windowModel->m_divider.m_lambda/m_windowModel->m_divider.m_thickness.value;
			// correct by boundary resistances
			double alpha = 1.0/(1.0/alphaDivider + thermalResistanceBC);
			double fluxHeatCondDividerLeft = alpha * m_windowModel->m_divider.m_area.value * deltaT;
			// add flux to heat conduction flux
			fluxHeatCondLeft += fluxHeatCondDividerLeft;
			fluxHeatCondRight -= fluxHeatCondDividerLeft;
		}

		// store results
		m_results[R_FluxHeatConductionA] = fluxHeatCondLeft;
		m_results[R_FluxHeatConductionB] = fluxHeatCondRight;
	}


	// *** solar radiation flux

	// if we have solar loads on either side, compute radiation loads and incidence angle

	if (m_haveSolarLoadsOnA || m_haveSolarLoadsOnB) {
		double qRadDir, qRadDiff, incidenceAngle, qRadGlobal;

		// get nominal radiation fluxes across surface of this construction
		// Mind: we pass our embedded object's ID, so that we get the radiatiation onto the window including
		//       potential external shading
		qRadGlobal = m_loads->qSWRad(m_id, qRadDir, qRadDiff, incidenceAngle);

		// update controlled shading factor
		if (m_windowModel->m_shading.m_modelType == NANDRAD::WindowShading::MT_Controlled) {
			// retrieve shading factor from input references
			IBK_ASSERT(m_valueRefs[InputRef_ShadingControlValue] != nullptr);
			double controlledShadingFactor = *m_valueRefs[InputRef_ShadingControlValue]; // Fz value
			// effect
			double z = 1 - (1-m_shadingFactor)*controlledShadingFactor;
			m_results[R_ShadingFactor] = z;
			qRadGlobal *= z;
			qRadDir *= z;
			qRadDiff *= z;
		}
		// reduce by shading factor
		else {
			qRadGlobal *= m_shadingFactor;
			qRadDir *= m_shadingFactor;
			qRadDiff *= m_shadingFactor;
			m_results[R_ShadingFactor] = m_shadingFactor;
		}

		// compute solar flux density [W/m2] through the glazing system
		double solarLeft = 0;
		double solarRight = 0;
		// compute radiation flux in direction of sun light
		IBK_ASSERT(m_windowModel->m_glazingSystem != NULL);
		m_windowModel->m_glazingSystem->computeSolarFluxDensity(qRadDir, qRadDiff, incidenceAngle, solarLeft,
																solarRight, m_haveSolarLoadsOnA);

		// store results
		m_results[R_FluxShortWaveRadiationA] = solarLeft * m_windowModel->m_glasArea;
		m_results[R_FluxShortWaveRadiationB] = solarRight * m_windowModel->m_glasArea;
	}
	else {
		m_results[R_FluxShortWaveRadiationA] = 0;
		m_results[R_FluxShortWaveRadiationB] = 0;
	}

	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL
