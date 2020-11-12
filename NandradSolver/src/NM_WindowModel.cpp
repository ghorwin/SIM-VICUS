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

        // compute heat conduction flux through the glazing system
        double fluxHeatCondLeft = 0;
        double fluxHeatCondRight = 0;

        // distinguish between standard/detailed model for glazing
        computeHeatConductionThroughGlazing(fluxHeatCondLeft, fluxHeatCondRight);

         // check if we have a frame
        if(m_windowModel->m_frame.m_materialID != NANDRAD::INVALID_ID) {
            // parameters were checked for validity already
            IBK_ASSERT(m_windowModel->m_frame.m_lambda > 0);
            IBK_ASSERT(m_windowModel->m_frame.m_thickness.value > 0);
            // calculate heat transfer coefficient
            double alphaFrame = m_windowModel->m_frame.m_lambda/m_windowModel->m_frame.m_thickness.value;
            double fluxHeatCondFrameLeft = alphaFrame * m_windowModel->m_frame.m_area.value * deltaT;
            // add flux to heat conduction flux
            fluxHeatCondLeft += fluxHeatCondFrameLeft;
            fluxHeatCondRight -= fluxHeatCondFrameLeft;
        }
        // check if we have a divider
        if(m_windowModel->m_divider.m_materialID != NANDRAD::INVALID_ID) {
            // parameters were checked for validity already
            IBK_ASSERT(m_windowModel->m_divider.m_lambda > 0);
            IBK_ASSERT(m_windowModel->m_divider.m_thickness.value > 0);
            // calculate heat transfer coefficient
            double alphaDivider = m_windowModel->m_divider.m_lambda/m_windowModel->m_divider.m_thickness.value;
            double fluxHeatCondDividerLeft = alphaDivider * m_windowModel->m_divider.m_area.value * deltaT;
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
		qRadGlobal = m_loads->qSWRad(m_con->m_id, qRadDir, qRadDiff, incidenceAngle);

        // reduce by shading factor
        if(m_windowModel->m_shading.m_modelType != NANDRAD::WindowShading::NUM_MT) {
            // parameters were checked already
            IBK_ASSERT(! m_windowModel->m_shading.m_para[NANDRAD::WindowShading::P_ReductionFactor].name.empty());
            double shadingFac = m_windowModel->m_shading.m_para[NANDRAD::WindowShading::P_ReductionFactor].value;
            qRadGlobal *= shadingFac;
            qRadDir *= shadingFac;
            qRadDiff *= shadingFac;
        }

        // compute solar flux through the glazing system
        double fluxSolarLeft = 0;
        double fluxSolarRight = 0;
        // compute radiation flux in direction of sun light
        computeSolarFluxThroughGlazing(qRadDir, qRadDiff, incidenceAngle, fluxSolarLeft, fluxSolarRight);

        // now reduce by frame and divider area and shading
        double areaComplete = m_windowModel->m_glasArea;
        // check if we have a frame
       if(m_windowModel->m_frame.m_materialID != NANDRAD::INVALID_ID) {
           areaComplete += m_windowModel->m_frame.m_area.value;
       }
       // check if we have a divider
      if(m_windowModel->m_divider.m_materialID != NANDRAD::INVALID_ID) {
          areaComplete += m_windowModel->m_divider.m_area.value;
      }

      // frame and divider correction
      if(areaComplete > m_windowModel->m_glasArea)  {
          double glassFraction = m_windowModel->m_glasArea/areaComplete;
          fluxSolarLeft *= glassFraction;
          fluxSolarRight *= glassFraction;
      }

       // store results
      m_results[R_FluxShortWaveRadiationA] = fluxSolarLeft;
      m_results[R_FluxShortWaveRadiationB] = fluxSolarRight;
    }
	else {
        m_results[R_FluxShortWaveRadiationA] = 0;
		m_results[R_FluxShortWaveRadiationB] = 0;
	}

    // signal success
    return 0;
}


void WindowModel::computeSolarFluxThroughGlazing(double qDir, double qDiff, double incidenceAngle,
                                                 double &fluxSolarLeft, double &fluxSolarRight) {
    fluxSolarLeft = 0;
    fluxSolarRight= 0;

    // for simple model we calculate solar flux
    if(m_windowModel->m_glazingSystem->m_modelType == NANDRAD::WindowGlazingSystem::MT_Simple) {
        // we have an SHGC value
        if(!m_windowModel->m_glazingSystem->m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_name.empty()) {
           // values were checked already
            double SHGC = m_windowModel->m_glazingSystem->m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_values.value(incidenceAngle);
            double SHGCHemis = m_windowModel->m_glazingSystem->m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_values.value(0.0);
            // calculate flux density from left to right
            double solarDensity = 0.0;
            if(m_haveSolarLoadsOnA) {
                solarDensity = SHGC * qDir + SHGCHemis * qDiff;
            }
            else {
                IBK_ASSERT(m_haveSolarLoadsOnB);
                solarDensity = -(SHGC * qDir + SHGCHemis * qDiff);
            }
            // flux towards window surface
            fluxSolarLeft = solarDensity * m_windowModel->m_glasArea;
            // we assume thermal equilibrium
            fluxSolarRight = -fluxSolarLeft;
        }
    }
    // otherwise we ignore solar flux
}

void WindowModel::computeHeatConductionThroughGlazing(double &fluxHeatCondLeft, double &fluxHeatCondRight) {

    fluxHeatCondLeft = 0;
    fluxHeatCondRight = 0;

    // simple model:
    if(m_windowModel->m_glazingSystem->m_modelType == NANDRAD::WindowGlazingSystem::MT_Simple) {
        // values were checked already
        double alpha = m_windowModel->m_glazingSystem->m_para[NANDRAD::WindowGlazingSystem::P_ThermalTransmittance].value;
        // non-implemented model or inactive heat conduction
        if(alpha > 0.0) {
             // use temperature diffenece through single glazing as potential for heat flux
            double deltaT = *m_valueRefs[InputRef_SideATemperature] - *m_valueRefs[InputRef_SideBTemperature];

            // compute mean resistance
            double thermalResistance = 1/alpha;

            // add surface resistances
            if(!m_con->m_interfaceA.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].name.empty()
                    && m_con->m_interfaceA.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value > 0) {
                double alphaLeft = m_con->m_interfaceA.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
                thermalResistance += 1.0/alphaLeft;
            }
            if(!m_con->m_interfaceB.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].name.empty()
                    && m_con->m_interfaceB.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value > 0) {
                double alphaRight = m_con->m_interfaceB.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
                thermalResistance += 1.0/alphaRight;
            }

            // compute flux
            IBK_ASSERT(thermalResistance > 0);

            // heat conduction density from left to right (A to B)
            double heatCondDensity = 1/thermalResistance * deltaT;

            // flux towards window surface
            fluxHeatCondLeft = heatCondDensity * m_windowModel->m_glasArea;
            // we assume thermal equilibrium
            fluxHeatCondRight = -fluxHeatCondLeft;
        }
    }
    // otherwise ignore heat transfer
}

} // namespace NANDRAD_MODEL
