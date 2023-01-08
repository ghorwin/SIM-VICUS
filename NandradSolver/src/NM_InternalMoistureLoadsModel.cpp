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

#include "NM_InternalMoistureLoadsModel.h"

#include <IBK_Exception.h>
#include <IBK_physics.h>

#include <NANDRAD_InternalMoistureLoadsModel.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {


void InternalMoistureLoadsModel::setup(const NANDRAD::InternalMoistureLoadsModel & internalMoistLoadsModel,
									const std::vector<NANDRAD::ObjectList> & objLists,
									const std::vector<NANDRAD::Zone> & zones)
{
	FUNCID(InternalMoistureLoadsModel::setup);

	m_internalMoistureLoadsModel = &internalMoistLoadsModel;
	m_zones = &zones;

	// copy parameters for type 'Constant'
	switch (m_internalMoistureLoadsModel->m_modelType) {
		case NANDRAD::InternalMoistureLoadsModel::MT_Constant : {
			m_personMoistureLoadPerArea = m_internalMoistureLoadsModel->m_para[NANDRAD::InternalMoistureLoadsModel::P_MoistureLoadPerArea].value;
		}
		break;

		case NANDRAD::InternalMoistureLoadsModel::MT_Scheduled : {
		} break;

		default:
			throw IBK::Exception(IBK::FormatString("Unknown/undefined model type."), FUNC_ID);
	}

	// all models require an object list with indication of zones that this model applies to
	if (m_internalMoistureLoadsModel->m_zoneObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ZoneObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  m_internalMoistureLoadsModel->m_zoneObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined object list '%1'.")
							 .arg(m_internalMoistureLoadsModel->m_zoneObjectList), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_ZONE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'Zone'.")
							 .arg(m_objectList->m_name), FUNC_ID);

	// reserve storage memory for results
	m_vectorValuedResults.resize(NUM_VVR);

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


const NANDRAD::ObjectList & InternalMoistureLoadsModel::objectList() const{
	return *m_objectList;
}


void InternalMoistureLoadsModel::initResults(const std::vector<AbstractModel *> &) {
	FUNCID(InternalMoistureLoadsModel::initResults);

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

	// for constant model, precompute constant moisture production rates and store in result variable vector
	if (m_internalMoistureLoadsModel->m_modelType == NANDRAD::InternalMoistureLoadsModel::MT_Constant) {
		// pointer to result vector
		double * moistureLoadPtr = m_vectorValuedResults[VVR_MoistureLoad].dataPtr();

		unsigned int nZones = m_zoneAreas.size();
		// loop through all zone areas
		for (unsigned int i = 0; i < nZones; ++i) {
			// retrieve zone area
			double area = m_zoneAreas[i];

			// calculate moisture load in [kg/s]
			moistureLoadPtr[i] = area * m_personMoistureLoadPerArea;
		}
	}

}


void InternalMoistureLoadsModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
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
		const std::string &quantityName = KeywordList::Keyword("InternalMoistureLoadsModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("InternalMoistureLoadsModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("InternalMoistureLoadsModel::VectorValuedResults", varIndex );
		// vector-valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription,
			false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
	}
}


const double * InternalMoistureLoadsModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_VVR; ++varIndex) {
		if (KeywordList::Keyword("InternalMoistureLoadsModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
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


void InternalMoistureLoadsModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// set reference to all zone air temperatures which is required for all model types
	for (unsigned int id : m_objectList->m_filterID.m_ids) {
		InputReference ref;
		ref.m_id = id;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ref.m_name.m_name = "AirTemperature";
		ref.m_required = true;
		inputRefs.push_back(ref);
	}

	// for scheduled internalloads model request zone-specific loads from schedules
	if (m_internalMoistureLoadsModel->m_modelType == NANDRAD::InternalMoistureLoadsModel::MT_Scheduled) {
		for (unsigned int id : m_objectList->m_filterID.m_ids) {
			// person load
			InputReference ref;
			ref.m_id = id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ref.m_name.m_name = "MoistureLoadPerAreaSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
		}
	}
}


void InternalMoistureLoadsModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	IBK_ASSERT ((m_internalMoistureLoadsModel->m_modelType == NANDRAD::InternalMoistureLoadsModel::MT_Constant && resultValueRefs.size() == m_zoneAreas.size()) ||
				(m_internalMoistureLoadsModel->m_modelType == NANDRAD::InternalMoistureLoadsModel::MT_Scheduled && resultValueRefs.size() == 2 * m_zoneAreas.size()) );

	m_valueRefs = resultValueRefs; // Note: we set all our input refs as mandatory, so we can rely on getting valid pointers
}


void InternalMoistureLoadsModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// enthalpy fluxes depend on on zone temperatures
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		// pair: result - input

		// dependency on room air temperature of corresponding zone
		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_MoistureEnthalpyFlux].dataPtr() + i, m_valueRefs[i]) );
	}
}


int InternalMoistureLoadsModel::update() {
	unsigned int nZones = m_zoneAreas.size();

	// retrieve iterator to all result quantities
	double * moistureLoadPtr = m_vectorValuedResults[VVR_MoistureLoad].dataPtr();
	double * moistureEnthalpyFluxPtr = m_vectorValuedResults[VVR_MoistureEnthalpyFlux].dataPtr();

	// calculate moisture load for scheduled model
	if (m_internalMoistureLoadsModel->m_modelType != NANDRAD::InternalMoistureLoadsModel::MT_Constant) {

		// Note: for constant model, the moisture load [kg/s] was already precomputed in initResults() and
		//       stored in m_vectorValuedResults[VVR_MoistureLoad]

		// loop through all zone areas
		for (unsigned int i = 0; i < nZones; ++i) {
			// retrieve zone area
			double area = m_zoneAreas[i];
			double moistureLoadPerArea = *m_valueRefs[nZones + i]; // from schedule, in [kg/m2s]
			moistureLoadPtr[i] = area * moistureLoadPerArea; // in [kg/s]
		}
	}

	// enthalpy calculation

	// loop through all zone areas
	for (unsigned int i = 0; i < nZones; ++i) {
		// retrieve moisture production rate in [kg/s]
		double moistureMassProductionRate = moistureLoadPtr[i];
		// retrieve zone air temperature in [K]
		double zoneTemp = *m_valueRefs[i];

		// moisture production enthalpy in [W] (sensible and latent heat of evaporation)
		moistureEnthalpyFluxPtr[i] = moistureMassProductionRate * (IBK::C_VAPOR * zoneTemp + IBK::H_EVAP);
	}

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
