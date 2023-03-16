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

#include "NM_IdealSurfaceHeatingCoolingModel.h"

#include <IBK_Exception.h>

#include <NANDRAD_IdealSurfaceHeatingCoolingModel.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Thermostat.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"
#include "NM_ThermostatModel.h"
#include "NM_ConstructionStatesModel.h"

namespace NANDRAD_MODEL {

void IdealSurfaceHeatingCoolingModel::setup(const NANDRAD::IdealSurfaceHeatingCoolingModel & model,
									 const std::vector<NANDRAD::ObjectList> & objLists)
{
	FUNCID(IdealSurfaceHeatingCoolingModel::setup);

	// check and resolve reference to object list (object list not empty has been checked already)
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  model.m_constructionObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined object list '%1'.")
							 .arg(model.m_constructionObjectList), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'ConstructionInstance'.")
							 .arg(m_objectList->m_name), FUNC_ID);

	// parameters have been checked already, power per surface/construction area
	m_maxHeatingPower = model.m_para[NANDRAD::IdealSurfaceHeatingCoolingModel::P_MaxHeatingPowerPerArea].value;
	m_maxCoolingPower = model.m_para[NANDRAD::IdealSurfaceHeatingCoolingModel::P_MaxCoolingPowerPerArea].value;


	// store zone
	m_thermostatZoneId = model.m_thermostatZoneId;

	// resize result vector
	m_vectorValuedResults.resize(NUM_VVR);
	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


void IdealSurfaceHeatingCoolingModel::initResults(const std::vector<AbstractModel *> & models) {
	// no model IDs, nothing to do (see explanation in resultDescriptions())
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of referenced constructions
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());
	// resize result vectors accordingly
	for (unsigned int varIndex=0; varIndex<NUM_VVR; ++varIndex)
		m_vectorValuedResults[varIndex] = VectorValuedQuantity(indexKeys);

	// reserve memory for construction areas
	unsigned int idSize = m_objectList->m_filterID.m_ids.size();
	m_constructionAreas.resize(idSize, -999);

	// now search through all models and lookup all construction areas
	for (const AbstractModel * mod : models) {
		// skip all that are not ConstructionStatesModels
		const ConstructionStatesModel * conMod = dynamic_cast<const ConstructionStatesModel *>(mod);
		if (conMod == nullptr)
			continue;
		// check if ID is in our ID vector
		unsigned int j=0;
		for (; j<idSize; ++j) {
			if (conMod->id() == indexKeys[j]) {
				// already set?
				IBK_ASSERT(m_constructionAreas[j] == -999);
				m_constructionAreas[j] = conMod->construction()->m_netHeatTransferArea;
				break;
			}
		}
	}
}


void IdealSurfaceHeatingCoolingModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// it may be possible that an object list does not contain a valid id, for example, when the
	// requested IDs did not exist - in this case a warning was already printed, so we can just bail out here
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// Retrieve index information from vector valued results.
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	// For each of the constructions in the object list we generate vector-valued results as defined
	// in the type Results.
	for (int varIndex=0; varIndex<NUM_VVR; ++varIndex) {
		// store name, unit and description of the quantity
		const std::string &quantityName = KeywordList::Keyword("IdealSurfaceHeatingCoolingModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("IdealSurfaceHeatingCoolingModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("IdealSurfaceHeatingCoolingModel::VectorValuedResults", varIndex );
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription, false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
	}
}


const double * IdealSurfaceHeatingCoolingModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_VVR; ++varIndex) {
		if (KeywordList::Keyword("IdealSurfaceHeatingCoolingModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
			break;
	}
	if (varIndex == NUM_VVR)
		return nullptr;
	// now check the index
	if (quantityName.m_index == -1) // no index - not allowed
		return &m_vectorValuedResults[varIndex][0];
	// search for index
	try {
		const double & valRef = m_vectorValuedResults[varIndex][(unsigned int)quantityName.m_index];
		return &valRef;
	} catch (...) {
		// exception is thrown when index is not available - return nullptr
		return nullptr;
	}
}


void IdealSurfaceHeatingCoolingModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	if (m_objectList->m_filterID.m_ids.empty())
		return;

	// we need control values for heating and cooling

	// For this purpose, we access a zone thermostat control value (identified via zone id).
	// Thermostat-models provide 'HeatingControlValue' and 'CoolingControlValue'.
	// Note, that for each zone only one thermostat is allowed. Once we loop
	// over all thermostat models, only one (and exactly) one request per zone must be fulfilled!
	// All references are set as optional and later (setInputValueRefs) proved, whether exactly one value is found.

	for (AbstractModel * model : models) {
		// ignore all models that are not thermostat models
		if (model->referenceType() != NANDRAD::ModelInputReference::MRT_MODEL)
			continue;
		ThermostatModel * mod = dynamic_cast<ThermostatModel *>(model);
		if (mod != nullptr) {
			// create an input reference to heating and cooling control values for all the zones that we heat
			InputReference r;
			r.m_id = model->id();
			r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
			r.m_name.m_name = "HeatingControlValue";
			r.m_name.m_index = (int) m_thermostatZoneId; // vector gets ID of requested zone
			r.m_required = false;
			m_inputRefs.push_back(r);
			r.m_name.m_name = "CoolingControlValue";
			m_inputRefs.push_back(r);
			++m_thermostatModelObjects;
		}
	}
}


void IdealSurfaceHeatingCoolingModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void IdealSurfaceHeatingCoolingModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	FUNCID(IdealSurfaceHeatingCoolingModel::setInputValueRefs);
	if (m_objectList->m_filterID.m_ids.empty())
		return;

	// We now must ensure, that for each zone there is exactly one matching control signal
	// In other means, exactly one result refernce value is filled with a valid adress
	// for each control value

	IBK_ASSERT(2 * m_thermostatModelObjects == resultValueRefs.size());

	for (unsigned int i=0; i<m_thermostatModelObjects; ++i) {
		// heating control value
		if (resultValueRefs[2 * i] != nullptr) {
			// we check only for heating control value if maximum power is > 0
			// perform check for each zone in order to avoid duplicate definition
			if (m_maxHeatingPower > 0) {
				if(m_heatingThermostatValueRef != nullptr)
					throw IBK::Exception(IBK::FormatString("Duplicate heating control value result generated by different thermostats "
														   "for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
				m_heatingThermostatValueRef = resultValueRefs[2 * i];
			}
		}
		if (resultValueRefs[2 * i + 1] != nullptr) {
			// we check only for cooling control value if maximum power is > 0
			// perform check for each zone in order to avoid duplicate definition
			if (m_maxCoolingPower > 0) {
				if( m_coolingThermostatValueRef != nullptr)
					throw IBK::Exception(IBK::FormatString("Duplicate cooling control value result generated by different thermostats "
														   "for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
				m_coolingThermostatValueRef = resultValueRefs[2 * i + 1];
			}
		}
	}

	// check that we have a heating control value for positive maximum heating power
	if (m_maxHeatingPower > 0 && m_heatingThermostatValueRef == nullptr)
		throw IBK::Exception(IBK::FormatString("Missing heating control value for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
	// check that we have indeed value refs for all zones
	if (m_maxCoolingPower > 0 && m_coolingThermostatValueRef == nullptr)
		throw IBK::Exception(IBK::FormatString("Missing cooling control value for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
}


void IdealSurfaceHeatingCoolingModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		// pair: result - input

		// we have heating defined
		if(m_heatingThermostatValueRef != nullptr)
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_ActiveLayerThermalLoad].dataPtr() + i, m_heatingThermostatValueRef) );
		// we operate in cooling mode
		if(m_coolingThermostatValueRef != nullptr)
			resultInputValueReferences.push_back(
							std::make_pair(m_vectorValuedResults[VVR_ActiveLayerThermalLoad].dataPtr() + i, m_coolingThermostatValueRef) );
	}
}


int IdealSurfaceHeatingCoolingModel::update() {
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		double & activeLayerLoad = *(m_vectorValuedResults[VVR_ActiveLayerThermalLoad].dataPtr() + i);
		activeLayerLoad = 0; // initialize with 0 W

		// get control value for heating
		if(m_heatingThermostatValueRef != nullptr) {
			double heatingControlValue = *m_heatingThermostatValueRef;
			// clip
			heatingControlValue = std::max(0.0, std::min(1.0, heatingControlValue));
			activeLayerLoad += heatingControlValue*m_maxHeatingPower*m_constructionAreas[i];
		}
		// get control value for cooling
		if(m_coolingThermostatValueRef != nullptr) {
			double coolingControlValue = *m_coolingThermostatValueRef;
			// clip
			coolingControlValue = std::max(0.0, std::min(1.0, coolingControlValue));
			activeLayerLoad -= coolingControlValue*m_maxCoolingPower*m_constructionAreas[i];
		}
	}

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
