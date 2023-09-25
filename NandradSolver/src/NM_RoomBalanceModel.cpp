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

#include "NM_RoomBalanceModel.h"

#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"
#include "NM_ConstructionBalanceModel.h"
#include "NM_InternalLoadsModel.h"
#include "NM_InternalMoistureLoadsModel.h"
#include "NM_NaturalVentilationModel.h"
#include "NM_WindowModel.h"
#include "NM_RoomRadiationLoadsModel.h"
#include "NM_ThermalNetworkBalanceModel.h"
#include "NM_IdealHeatingCoolingModel.h"


namespace NANDRAD_MODEL {

void RoomBalanceModel::setup( const NANDRAD::SimulationParameter &simPara) {
	// copy all object pointers
	m_simPara     = &simPara;

	// parameter had already been checked
	m_solarRadiationLoadFraction = m_simPara->m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone].value;

	// results depend on calculation mode
	m_moistureBalanceEnabled = simPara.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled();
	if (m_moistureBalanceEnabled) {
		m_results.resize(NUM_R);
		// resize ydot vector - two balance equations
		m_ydot.resize(2);
	}
	else {
		// resize results vector
		m_results.resize(R_CompleteMoistureLoad); // R_CompleteMoistureLoad = first moisture-related result

		// resize ydot vector - one balance equation
		m_ydot.resize(1);
	}
}


void RoomBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	int varCount = R_CompleteMoistureLoad; // R_CompleteMoistureLoad = first moisture-related result
	if (m_simPara->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled()) {
		varCount = NUM_R; // more variables for hygrothermal calculation
	}

	/// \todo what about CO2 ???

	for (int i=0; i<varCount; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("RoomBalanceModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("RoomBalanceModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("RoomBalanceModel::Results", i);

		resDesc.push_back(result);
	}
}


const double * RoomBalanceModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "RoomBalanceModel::Results";

	if (quantityName.m_name == "ydot") {
		return &m_ydot[0];
	}
	else if (KeywordList::CategoryExists(category) && KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}
	else
		return nullptr;
}


int RoomBalanceModel::priorityOfModelEvaluation() const {
	// room balance model is evaluated one step before outputs
	return AbstractStateDependency::priorityOffsetTail+5;
}


void RoomBalanceModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// we create batches of input references for all input quantities that we require in the room model

	// WARNING: The order of objects in the models vector may vary from project to project. Hence,
	//          the order of the input references in the m_inputReferences vector also varies after this loop.
	//          In order to get a decent order, we first create groups of input refs, which are joined into
	//          a single vector afterwards.

	std::vector<InputReference> heatCondIR;
	std::vector<InputReference> windowHeatCondIR;
	InputReference	windowSolarRadIR;
	std::vector<InputReference> ventilationRH;
	std::vector<InputReference> internalLoadsRH;
	std::vector<InputReference> internalMoistLoadsRH;
	std::vector<InputReference> idealHeatingsRH;
	std::vector<InputReference> networkLoadsRH;

	// search all models for construction models that have an interface to this zone
	for (AbstractModel * model : models) {

		// *** heat conduction from walls ***
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE) {
			// we need a construction balance model here
			ConstructionBalanceModel* conMod = dynamic_cast<ConstructionBalanceModel*>(model);
			if (conMod == nullptr) continue;

			// check if either interface references us

			// side A
			if (conMod->interfaceAZoneID() == m_id) {
				// create input reference for heat conduction fluxes into this zone
				InputReference r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name.m_name = "FluxHeatConductionA";
				m_heatCondValueRefs.push_back(nullptr);
				heatCondIR.push_back(r);
			}

			// check if either interface references us
			if (conMod->interfaceBZoneID() == m_id) {
				// create input reference for heat conduction fluxes into this zone
				InputReference r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name.m_name = "FluxHeatConductionB";
				m_heatCondValueRefs.push_back(nullptr);
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

		else if (model->referenceType() == NANDRAD::ModelInputReference::MRT_ZONE) {
			// *** solar radiation summation model ***

			RoomRadiationLoadsModel * mod = dynamic_cast<RoomRadiationLoadsModel *>(model);
			if (mod != nullptr && mod->id() == m_id) {
				IBK_ASSERT(!m_haveSolarRadiationModel); // must have only one model providing this summation flux for us!
				m_haveSolarRadiationModel = true;
				InputReference r;
				r.m_id = mod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				r.m_name.m_name = "WindowSolarRadiationFluxSum";
				windowSolarRadIR = r;
			}

		}
		// create input references for model that generate zone-specific inputs (optional)
		else if (model->referenceType() == NANDRAD::ModelInputReference::MRT_MODEL) {

			// *** natural ventilation model ***

			NaturalVentilationModel * natVentModel = dynamic_cast<NaturalVentilationModel *>(model);
			if (natVentModel != nullptr) {
				++m_infiltrationModelCount;
				InputReference r;
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
				r.m_name.m_name = "VentilationHeatFlux";
				r.m_name.m_index = (int)m_id; // select result for us (our zone id)
				// this reference is only provided if the corresponding object list
				// contains current zone
				r.m_required = false;
				ventilationRH.push_back(r);

				// add moisture load
				if (m_moistureBalanceEnabled) {
					r.m_name.m_name = "VentilationMoistureMassFlux";
					ventilationRH.push_back(r);
				}
				continue;
			}

			// *** internal loads model ***
			InternalLoadsModel * intLoadsModel = dynamic_cast<InternalLoadsModel *>(model);
			if (intLoadsModel != nullptr) {
				++m_internalLoadsModelCount;
				InputReference r;
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
				r.m_name.m_index = (int)m_id; // select result for us (our zone id)
				// this reference is only provided if the corresponding object list
				// contains current zone
				r.m_required = false;
				// equipment electrical power (needed as output only)
				r.m_name.m_name = "EquipmentElectricalPower";
				internalLoadsRH.push_back(r);
				// light electrical power  (needed as output only)
				r.m_name.m_name = "LightingElectricalPower";
				internalLoadsRH.push_back(r);
				// equipment load
				r.m_name.m_name = "ConvectiveEquipmentHeatLoad";
				internalLoadsRH.push_back(r);
				// person load
				r.m_name.m_name = "ConvectivePersonHeatLoad";
				internalLoadsRH.push_back(r);
				// lighting load
				r.m_name.m_name = "ConvectiveLightingHeatLoad";
				internalLoadsRH.push_back(r);
			}

			if (m_moistureBalanceEnabled) {
				// *** internal moisture loads model ***
				InternalMoistureLoadsModel * intMoistLoadsModel = dynamic_cast<InternalMoistureLoadsModel *>(model);
				if (intMoistLoadsModel != nullptr) {
					++m_internalMoistureLoadsModelCount;
					InputReference r;
					r.m_id = model->id();
					r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
					r.m_name.m_index = (int)m_id; // select result for us (our zone id)
					// this reference is only provided if the corresponding object list
					// contains current zone
					r.m_required = false;
					// moisture production rate
					r.m_name.m_name = "MoistureLoad";
					internalMoistLoadsRH.push_back(r);
					// moisture production enthalpy flux
					r.m_name.m_name = "MoistureEnthalpyFlux";
					internalMoistLoadsRH.push_back(r);
				}
			}

			// *** internal loads model ***
			IdealHeatingCoolingModel * idealHeatingCoolingModel = dynamic_cast<IdealHeatingCoolingModel *>(model);
			if (idealHeatingCoolingModel != nullptr) {
				++m_idealHeatingCoolingModelCount;
				InputReference r;
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
				r.m_name.m_index = (int)m_id; // select result for us (our zone id)
				// this reference is only provided if the corresponding object list
				// contains current zone
				r.m_required = false;
				// heating load
				r.m_name.m_name = "IdealHeatingLoad";
				idealHeatingsRH.push_back(r);
				// cooling load
				r.m_name.m_name = "IdealCoolingLoad";
				idealHeatingsRH.push_back(r);
			}

		}
		// create input references for heat fluxes out of hydraulic networks
		else if (model->referenceType() == NANDRAD::ModelInputReference::MRT_NETWORK) {
			ThermalNetworkBalanceModel * thermNetworkModel = dynamic_cast<ThermalNetworkBalanceModel *>(model);
			if (thermNetworkModel != nullptr) {
				++m_networkHeatLoadsModelCount;
				InputReference r;
				r.m_name.m_name = "NetworkZoneThermalLoad";
				// add current id as index so that we can sum uphat fluxes from all networks
				r.m_name.m_index = (int) id();
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORK;
				// this reference is only provided if the corresponding network with element
				// heat flux into current zone
				r.m_required = false;
				m_networkHeatLoadValueRefs.push_back(nullptr);
				networkLoadsRH.push_back(r);
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
	m_inputRefs.insert(m_inputRefs.end(), internalMoistLoadsRH.begin(), internalMoistLoadsRH.end());
	m_inputRefs.insert(m_inputRefs.end(), idealHeatingsRH.begin(), idealHeatingsRH.end());
	m_inputRefs.insert(m_inputRefs.end(), networkLoadsRH.begin(), networkLoadsRH.end());

}


void RoomBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void RoomBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	FUNCID(RoomBalanceModel::setInputValueRefs);
	// copy value references as ordered by input references

	// Important: the order in which pointers are copied from resultValueRefs must match the
	//            order the InputReference objects were added to the vector in initInputReferences().

	std::vector<const double *>::const_iterator it = resultValueRefs.begin();

	// mandatory pointers
	for (unsigned int i=0; i<m_heatCondValueRefs.size(); ++i)
		m_heatCondValueRefs[i] = *it++;

	// mandatory pointers
	for (unsigned int i=0; i<m_windowHeatCondValueRefs.size(); ++i) {
		m_windowHeatCondValueRefs[i] = *it++;
	}

	// solar radiation load (summation flux)
	if (m_haveSolarRadiationModel)
		m_windowSolarRadiationLoadsRef = *it++;

	if (m_moistureBalanceEnabled) {
		// infiltration heat and mositure mass flux
		for (unsigned int i=0; i<m_infiltrationModelCount; ++i) {
			// if we do not have an infiltration heat flux, we skip the next two references, but first check for
			// invalid/missing moisture flux
			if (*it == nullptr)	{
				if(*(it+1) != nullptr)
					throw IBK::Exception(IBK::FormatString("Infiltration moisture mass flux without heat flux generated "
														   "for zone id=%1.").arg(m_id), FUNC_ID);
				it += 2; // advance both pointers
				continue; // nullptr = no flux = skip
			}
			// check against duplicate definition
			if (m_infiltrationHeatfluxValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate infiltration heat flux result generated by different models "
													   "for zone id=%1.").arg(m_id), FUNC_ID);
			if (m_infiltrationMoistMassfluxValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate infiltration moisture mass flux result generated by different models "
													   "for zone id=%1.").arg(m_id), FUNC_ID);
			m_infiltrationHeatfluxValueRef = *it++;
			m_infiltrationMoistMassfluxValueRef = *it++;
		}
	}
	else {
		// infiltration heat flux
		for (unsigned int i=0; i<m_infiltrationModelCount; ++i) {
			if (*it == nullptr)	{
				++it; // advance pointer
				continue; // nullptr = no flux = skip
			}
			// check against duplicate definition
			if (m_infiltrationHeatfluxValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate infiltration heat flux result generated by different models "
													   "for zone id=%1.").arg(m_id), FUNC_ID);
			m_infiltrationHeatfluxValueRef = *it++;
		}
	}


	// internal loads
	for (unsigned int i=0; i<m_internalLoadsModelCount; ++i) {
		if (*it == nullptr) {
			it += 5; // advance pointer
			continue; // nullptr = no flux = skip
		}
		// check against duplicate definition
		if (m_equipmentLoadValueRef != nullptr)
			throw IBK::Exception(IBK::FormatString("Duplicate internal loads generated by different models "
												   "for zone id=%1.").arg(m_id), FUNC_ID);
		// copy all other definitions
		m_equipmentElPowerValueRef = *it++;
		m_lightingElPowerValueRef = *it++;
		m_equipmentLoadValueRef = *it++;
		m_personLoadValueRef = *it++;
		m_lightingLoadValueRef = *it++;

		// now check existence of all definitions
		if (m_equipmentLoadValueRef == nullptr &&
			(m_personLoadValueRef !=  nullptr || m_lightingLoadValueRef != nullptr) )	 {
			throw IBK::Exception(IBK::FormatString("Missing equipment load result "
												   "for zone id=%1.").arg(m_id), FUNC_ID);
		}
		if (m_personLoadValueRef == nullptr &&
			(m_equipmentLoadValueRef !=  nullptr || m_lightingLoadValueRef != nullptr))	 {
			throw IBK::Exception(IBK::FormatString("Missing person load result "
												   "for zone id=%1.").arg(m_id), FUNC_ID);
		}
		if (m_lightingLoadValueRef == nullptr &&
			(m_equipmentLoadValueRef !=  nullptr || m_personLoadValueRef != nullptr))	 {
			throw IBK::Exception(IBK::FormatString("Missing lighting load result "
												   "for zone id=%1.").arg(m_id), FUNC_ID);
		}
	}

	if (m_moistureBalanceEnabled) {
		// internal moisture loads
		for (unsigned int i=0; i<m_internalMoistureLoadsModelCount; ++i) {
			if (*it == nullptr)	{
				it += 2; // advance pointer
				continue; // nullptr = no flux = skip
			}
			// check against duplicate definition
			if (m_moistureLoadValueRef != nullptr ||
				m_moistureEnthalpyFluxValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate internal moisture loads generated by different models "
													   "for zone id=%1.").arg(m_id), FUNC_ID);
			m_moistureLoadValueRef = *it++;
			// copy all other definitions
			m_moistureEnthalpyFluxValueRef = *it++;

			// now check existence of all definitions
			if (m_moistureLoadValueRef == nullptr &&
				m_moistureEnthalpyFluxValueRef !=  nullptr )	 {
				throw IBK::Exception(IBK::FormatString("Missing moisture load result "
													   "for zone id=%1.").arg(m_id), FUNC_ID);
			}
			if (m_moistureEnthalpyFluxValueRef == nullptr &&
				m_moistureLoadValueRef !=  nullptr)	 {
				throw IBK::Exception(IBK::FormatString("Missing moisture enthalpy flux result "
													   "for zone id=%1.").arg(m_id), FUNC_ID);
			}
		}
	}

	// ideal heating/cooling loads
	for (unsigned int i=0; i<m_idealHeatingCoolingModelCount; ++i) {
		if (*it == nullptr)	{
			it += 2; // advance pointer
			continue; // nullptr = no flux = skip
		}
		// check against duplicate definition
		if (m_idealHeatingLoadValueRef != nullptr)
			throw IBK::Exception(IBK::FormatString("Duplicate ideal heating/cooling loads generated by different models "
												   "for zone id=%1.").arg(m_id), FUNC_ID);
		m_idealHeatingLoadValueRef = *it++;
		m_idealCoolingLoadValueRef = *it++;
	}

	// network loads
	for (unsigned int i=0; i<m_networkHeatLoadsModelCount; ++i) {
		m_networkHeatLoadValueRefs[i] = *it++;
	}
	/// \todo other input fluxes that we sum up

	// check that we have all
	IBK_ASSERT(it == resultValueRefs.end());
}


void RoomBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	// for each computed quantity indicate which variables are needed for computation

	// add all heat conduction flux vars as input references for the summation heat flux
	for (const double * heatCondVars : m_heatCondValueRefs)
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_ConstructionHeatConductionLoad], heatCondVars));
	for (const double * heatCondVars : m_windowHeatCondValueRefs)
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_WindowHeatConductionLoad], heatCondVars));
	// Sum up all network loads
	for (const double * networkVars : m_networkHeatLoadValueRefs) {
		if(networkVars == nullptr)
			continue;
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_NetworkHeatLoad], networkVars));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], networkVars));
	}

	if (m_haveSolarRadiationModel)
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_WindowSolarRadiationLoad], m_windowSolarRadiationLoadsRef));
	// total flux depends on all computed fluxes
	resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], &m_results[R_ConstructionHeatConductionLoad]));
	resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], &m_results[R_WindowHeatConductionLoad]));
	resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], &m_results[R_WindowSolarRadiationLoad]));
	if (m_infiltrationHeatfluxValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], m_infiltrationHeatfluxValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_VentilationHeatLoad], m_infiltrationHeatfluxValueRef));
	}
	if (m_equipmentLoadValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], m_equipmentLoadValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_ConvectiveEquipmentHeatLoad], m_equipmentLoadValueRef));
	}
	if (m_equipmentElPowerValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_EquipmentElectricalPower], m_equipmentElPowerValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_TotalElectricalPower], m_equipmentElPowerValueRef));
	}
	if (m_personLoadValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], m_personLoadValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_ConvectivePersonHeatLoad], m_personLoadValueRef));
	}
	if (m_lightingLoadValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], m_lightingLoadValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_ConvectiveLightingHeatLoad], m_lightingLoadValueRef));
	}
	if (m_lightingElPowerValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_LightingElectricalPower], m_lightingElPowerValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_TotalElectricalPower], m_lightingElPowerValueRef));
	}
	if (m_idealHeatingLoadValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], m_idealHeatingLoadValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_IdealHeatingLoad], m_idealHeatingLoadValueRef));
	}
	if (m_idealCoolingLoadValueRef != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], m_idealCoolingLoadValueRef));
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_IdealCoolingLoad], m_idealCoolingLoadValueRef));
	}

	// the room energy balance now depends on the sum of all heat fluxes
	resultInputValueReferences.push_back(std::make_pair(&m_ydot[0], &m_results[R_CompleteThermalLoad]));

	if (m_moistureBalanceEnabled) {
		// the moisture balance now depends on the sum of all moisture mass fluxes
		resultInputValueReferences.push_back(std::make_pair(&m_ydot[1], &m_results[R_CompleteMoistureLoad]));

		if (m_infiltrationMoistMassfluxValueRef != nullptr)
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteMoistureLoad], m_infiltrationMoistMassfluxValueRef));

		// The room energy balance depends on the moisture production model results
		if (m_moistureEnthalpyFluxValueRef != nullptr) {
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteThermalLoad], m_moistureEnthalpyFluxValueRef));
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_CompleteMoistureLoad], m_moistureLoadValueRef));
		}
	}

}


int RoomBalanceModel::update() {

	double sumQHeatCondToWalls = 0.0; // sum of heat fluxes in [W] positive from wall surfaces to room
	for (const double ** flux = m_heatCondValueRefs.data(), **fluxEnd = flux + m_heatCondValueRefs.size(); flux != fluxEnd; ++flux)
		sumQHeatCondToWalls -= **flux;

	double sumQHeatCondThroughWindows = 0.0; // sum of heat fluxes in [W] positive from windows to room
	for (const double ** flux = m_windowHeatCondValueRefs.data(), **fluxEnd = flux + m_windowHeatCondValueRefs.size(); flux != fluxEnd; ++flux)
		sumQHeatCondThroughWindows -= **flux;


	// store results
	m_results[R_ConstructionHeatConductionLoad] = sumQHeatCondToWalls;
	m_results[R_WindowHeatConductionLoad] = sumQHeatCondThroughWindows;

	double SumQdot =	sumQHeatCondToWalls +
						sumQHeatCondThroughWindows;

	double sumQHydraHeating = 0.0; // sum up all heat fluxes from hydraulic networks

	for (const double * flux : m_networkHeatLoadValueRefs) {
		if(flux == nullptr)
			continue;
		sumQHydraHeating += *flux;
	}

	// add network loads
	m_results[R_NetworkHeatLoad] = sumQHydraHeating;
	SumQdot += sumQHydraHeating;

	// add solar radiation flux load
	if (m_haveSolarRadiationModel) {
		m_results[R_WindowSolarRadiationLoad] = *m_windowSolarRadiationLoadsRef * m_solarRadiationLoadFraction;
		SumQdot += m_results[R_WindowSolarRadiationLoad];
	}

	// add ventilation rate flux
	if (m_infiltrationHeatfluxValueRef != nullptr) {
		SumQdot += *m_infiltrationHeatfluxValueRef;
		m_results[R_VentilationHeatLoad] = *m_infiltrationHeatfluxValueRef;
	}

	// add equipment loads
	if (m_equipmentLoadValueRef != nullptr) {
		SumQdot += *m_equipmentLoadValueRef;
		m_results[R_ConvectiveEquipmentHeatLoad] = *m_equipmentLoadValueRef;
	}

	// add person loads
	if (m_personLoadValueRef != nullptr) {
		SumQdot += *m_personLoadValueRef;
		m_results[R_ConvectivePersonHeatLoad] = *m_personLoadValueRef;
	}

	// add lighting loads
	if (m_lightingLoadValueRef != nullptr) {
		SumQdot += *m_lightingLoadValueRef;
		m_results[R_ConvectiveLightingHeatLoad] = *m_lightingLoadValueRef;
	}

	// add heating loads
	if (m_idealHeatingLoadValueRef != nullptr) {
		SumQdot += *m_idealHeatingLoadValueRef;
		m_results[R_IdealHeatingLoad] = *m_idealHeatingLoadValueRef;
	}

	// add cooling loads
	if (m_idealCoolingLoadValueRef != nullptr) {
		SumQdot -= *m_idealCoolingLoadValueRef; // Mind the negative sign here!
		m_results[R_IdealCoolingLoad] = *m_idealCoolingLoadValueRef;
	}

	// electrical power: these are only outputs, not needed for calculation
	if (m_equipmentElPowerValueRef != nullptr)
		m_results[R_EquipmentElectricalPower] = *m_equipmentElPowerValueRef;
	if (m_lightingElPowerValueRef != nullptr)
		m_results[R_LightingElectricalPower] = *m_lightingElPowerValueRef;
	if (m_equipmentElPowerValueRef != nullptr && m_lightingElPowerValueRef != nullptr)
		m_results[R_TotalElectricalPower] = *m_equipmentElPowerValueRef + *m_lightingElPowerValueRef;


	// moisture balance
	if(m_moistureBalanceEnabled)
	{
		double sumMdotMoist = 0.0;

		// add ventilation moisture mass flux
		if (m_infiltrationMoistMassfluxValueRef != nullptr) {
			sumMdotMoist += *m_infiltrationMoistMassfluxValueRef;
		}

		// add person moisture mass and enthalpyflux
		if (m_moistureLoadValueRef != nullptr) {
			sumMdotMoist += *m_moistureLoadValueRef;
		}
		if (m_moistureEnthalpyFluxValueRef != nullptr) {
			SumQdot += *m_moistureEnthalpyFluxValueRef;
		}

		// store the sum of all moisture loads
		m_results[R_CompleteMoistureLoad] = sumMdotMoist;
		// solve the balance: ydot = sum loads in [kg]
		m_ydot[1] = sumMdotMoist;

	}

	// store the sum of all loads
	m_results[R_CompleteThermalLoad] = SumQdot;
	// solve the balance: ydot = sum loads in [W] (no need to devide by volume since conserved quantity is energy of room air in Joule)
	m_ydot[0] = SumQdot;

	// signal success
	return 0;
}


int RoomBalanceModel::ydot(double* ydot) {
	// copy values to ydot
	ydot[0] = m_ydot[0];
	if (m_ydot.size() > 1) {
		ydot[1] = m_ydot[1];
	}
	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL

