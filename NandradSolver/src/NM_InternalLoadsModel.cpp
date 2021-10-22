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

#include "NM_InternalLoadsModel.h"

#include <IBK_Exception.h>

#include <NANDRAD_InternalLoadsModel.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {



void InternalLoadsModel::setup(const NANDRAD::InternalLoadsModel & internalLoadsModel,
									const std::vector<NANDRAD::ObjectList> & objLists,
									const std::vector<NANDRAD::Zone> & zones)
{
	FUNCID(InternalLoadsModel::setup);

	m_internalLoadsModel = &internalLoadsModel;
	m_zones = &zones;

	// copy for mandatory parameters
	m_eqipmentRadiationFraction = m_internalLoadsModel->m_para[NANDRAD::InternalLoadsModel::P_EquipmentRadiationFraction].value;
	m_personRadiationFraction = m_internalLoadsModel->m_para[NANDRAD::InternalLoadsModel::P_PersonRadiationFraction].value;
	m_lightingRadiationFraction = m_internalLoadsModel->m_para[NANDRAD::InternalLoadsModel::P_LightingRadiationFraction].value;

	// copy parameters for type 'Constant'
	switch (m_internalLoadsModel->m_modelType) {
		case NANDRAD::InternalLoadsModel::MT_Constant : {
			m_equipmentLoadPerArea = m_internalLoadsModel->m_para[NANDRAD::InternalLoadsModel::P_EquipmentHeatLoadPerArea].value;
			m_personLoadPerArea = m_internalLoadsModel->m_para[NANDRAD::InternalLoadsModel::P_PersonHeatLoadPerArea].value;
			m_lightingLoadPerArea = m_internalLoadsModel->m_para[NANDRAD::InternalLoadsModel::P_LightingHeatLoadPerArea].value;
		}
		break;

		case NANDRAD::InternalLoadsModel::MT_Scheduled : {
		} break;

		default:
			throw IBK::Exception(IBK::FormatString("Unknown/undefined model type."), FUNC_ID);
	}

	// all models require an object list with indication of zones that this model applies to
	if (m_internalLoadsModel->m_zoneObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ZoneObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  m_internalLoadsModel->m_zoneObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined object list '%1'.")
							 .arg(m_internalLoadsModel->m_zoneObjectList), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_ZONE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'Zone'.")
							 .arg(m_objectList->m_name), FUNC_ID);

	// reserve storage memory for results
	m_vectorValuedResults.resize(NUM_VVR);

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


const NANDRAD::ObjectList & InternalLoadsModel::objectList() const{
	return *m_objectList;
}


void InternalLoadsModel::initResults(const std::vector<AbstractModel *> &) {
	FUNCID(InternalLoadsModel::initResults);

	// no model IDs, nothing to do (see explanation in resultDescriptions())
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of referenced zones
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());
	// resize result vectors accordingly
	for (unsigned int varIndex=0; varIndex<NUM_VVR; ++varIndex)
		m_vectorValuedResults[varIndex] = VectorValuedQuantity(indexKeys);

	// we also cache the zonal volumes for faster computation
	for (unsigned int id : indexKeys) {
		// find zone by ID
		const std::vector<NANDRAD::Zone>::const_iterator it = std::find(m_zones->begin(), m_zones->end(), id);
		if (it == m_zones->end())
			throw IBK::Exception(IBK::FormatString("Zone with id '%1' is referenced in object list '%2' but does not exist "
								 "(error in object list init code).").arg(id).arg(m_objectList->m_name), FUNC_ID);
		// check existence and validity of zone area

		m_zoneAreas.push_back(it->m_para[NANDRAD::Zone::P_Area].value ); // already checked in Zone::checkParameters()

	}
	// calculate initial solution for constant model
	if (m_internalLoadsModel->m_modelType == NANDRAD::InternalLoadsModel::MT_Constant) {
		// retrieve iterator to all result quantities
		double * convEquipLoadPtr = m_vectorValuedResults[VVR_ConvectiveEquipmentHeatLoad].dataPtr();
		double * radEquipLoadPtr = m_vectorValuedResults[VVR_RadiantEquipmentHeatLoad].dataPtr();
		double * convPersonLoadPtr = m_vectorValuedResults[VVR_ConvectivePersonHeatLoad].dataPtr();
		double * radPersonLoadPtr = m_vectorValuedResults[VVR_RadiantPersonHeatLoad].dataPtr();
		double * convLightLoadPtr = m_vectorValuedResults[VVR_ConvectiveLightingHeatLoad].dataPtr();
		double * radLightLoadPtr = m_vectorValuedResults[VVR_RadiantLightingHeatLoad].dataPtr();

		unsigned int nZones = m_zoneAreas.size();
		// loop through all zone areas
		for(unsigned int i = 0; i < nZones; ++i) {
			// retrieve zone area
			double area = m_zoneAreas[i];
			// calculate load in  [W]
			double equipmentLoad = area * m_equipmentLoadPerArea;
			double personLoad = area * m_personLoadPerArea;
			double lightingLoad = area * m_lightingLoadPerArea;

			// distribute
			convEquipLoadPtr[i]	= (1 - m_eqipmentRadiationFraction) * equipmentLoad;
			radEquipLoadPtr[i]	= m_eqipmentRadiationFraction * equipmentLoad;
			convPersonLoadPtr[i]= (1 - m_personRadiationFraction) * personLoad;
			radPersonLoadPtr[i]	= m_personRadiationFraction * personLoad;
			convLightLoadPtr[i]	= (1 - m_lightingRadiationFraction) * lightingLoad;
			radLightLoadPtr[i]	= m_lightingRadiationFraction * lightingLoad;
		}
	}

}


void InternalLoadsModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those zones were added, that are actual parameterized
	// so we can rely on the existence of zones whose IDs are in our object list and we do not need to search
	// through all the models

	// it may be possible, that an object list does not contain a valid id, for example, when the
	// requested IDs did not exist - in this case a warning was already printed, so we can just bail out here
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// Retrieve index information from vector valued results.
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	// For each of the zones in the object list we generate vector-valued results as defined
	// in the type VectorValuedResults.
	for (int varIndex=0; varIndex<NUM_VVR; ++varIndex) {
		// store name, unit and description of the vector quantity
		const std::string &quantityName = KeywordList::Keyword("InternalLoadsModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("InternalLoadsModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("InternalLoadsModel::VectorValuedResults", varIndex );
		// vector-valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription,
			false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
	}
}


const double * InternalLoadsModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_VVR; ++varIndex) {
		if (KeywordList::Keyword("InternalLoadsModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
			break;
	}
	if (varIndex == NUM_VVR)
		return nullptr;
	// now check the index
	if (quantityName.m_index == -1) // no index - return start of vector
		return m_vectorValuedResults[varIndex].dataPtr();
	// search for index
	try {
		const double & valRef = m_vectorValuedResults[varIndex][(unsigned int)quantityName.m_index];
		return &valRef;
	} catch (...) {
		// exception is thrown when index is not available - return nullptr
		return nullptr;
	}
}


void InternalLoadsModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// for scheduled internalloads model request zone-specific loads from schedules
	if (m_internalLoadsModel->m_modelType == NANDRAD::InternalLoadsModel::MT_Scheduled) {
		for (unsigned int id : m_objectList->m_filterID.m_ids) {
			// equipment load
			InputReference ref;
			ref.m_id = id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ref.m_name.m_name = "EquipmentHeatLoadPerAreaSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
			// person load
			ref.m_name.m_name = "PersonHeatLoadPerAreaSchedule";
			inputRefs.push_back(ref);
			// lighting load
			ref.m_name.m_name = "LightingHeatLoadPerAreaSchedule";
			inputRefs.push_back(ref);
		}
	}
}


void InternalLoadsModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	// simply store and check value references
	if (m_internalLoadsModel->m_modelType == NANDRAD::InternalLoadsModel::MT_Constant)
		return;
	IBK_ASSERT(resultValueRefs.size() ==  3 * m_zoneAreas.size());
	m_valueRefs = resultValueRefs; // Note: we set all our input refs as mandatory, so we can rely on getting valid pointers
}


void InternalLoadsModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & /*resultInputValueReferences*/) const {
		return; // nothing to compute, we only depend on time dependend quantities
}


int InternalLoadsModel::update() {
	// nothing to compute
	if (m_internalLoadsModel->m_modelType == NANDRAD::InternalLoadsModel::MT_Constant)
		return 0;

	unsigned int nZones = m_zoneAreas.size();

	// retrieve iterator to all result quantities
	double * convEquipLoadPtr = m_vectorValuedResults[VVR_ConvectiveEquipmentHeatLoad].dataPtr();
	double * radEquipLoadPtr = m_vectorValuedResults[VVR_RadiantEquipmentHeatLoad].dataPtr();
	double * convPersonLoadPtr = m_vectorValuedResults[VVR_ConvectivePersonHeatLoad].dataPtr();
	double * radPersonLoadPtr = m_vectorValuedResults[VVR_RadiantPersonHeatLoad].dataPtr();
	double * convLightLoadPtr = m_vectorValuedResults[VVR_ConvectiveLightingHeatLoad].dataPtr();
	double * radLightLoadPtr = m_vectorValuedResults[VVR_RadiantLightingHeatLoad].dataPtr();

	// loop through all zone areas
	for(unsigned int i = 0; i < nZones; ++i) {
		// retrieve zone area
		double area = m_zoneAreas[i];
		// calculate load in  [W]
		unsigned int numVVRHalf = NUM_VVR*0.5;
		// ATTENTION adjust numVVR !!!!
		double equipmentLoad = area * (*m_valueRefs[i * numVVRHalf]);
		double personLoad = area * (*m_valueRefs[i * numVVRHalf + 1]);
		double lightingLoad = area * (*m_valueRefs[i * numVVRHalf + 2]);

		// distribute
		convEquipLoadPtr[i]	= (1 - m_eqipmentRadiationFraction) * equipmentLoad;
		radEquipLoadPtr[i]	= m_eqipmentRadiationFraction * equipmentLoad;
		convPersonLoadPtr[i]= (1 - m_personRadiationFraction) * personLoad;
		radPersonLoadPtr[i]	= m_personRadiationFraction * personLoad;
		convLightLoadPtr[i]	= (1 - m_lightingRadiationFraction) * lightingLoad;
		radLightLoadPtr[i]	= m_lightingRadiationFraction * lightingLoad;
	}

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
