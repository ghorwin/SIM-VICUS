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

#include "NM_NaturalVentilationModel.h"

#include <IBK_Exception.h>
#include <IBK_physics.h>

#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_NaturalVentilationModel.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {



void NaturalVentilationModel::setup(const NANDRAD::NaturalVentilationModel & ventilationModel,
									const NANDRAD::SimulationParameter & simPara,
									const std::vector<NANDRAD::ObjectList> & objLists,
									const std::vector<NANDRAD::Zone> & zones)
{
	FUNCID(NaturalVentilationModel::setup);

	m_ventilationModel = &ventilationModel;
	m_moistureBalanceEnabled = simPara.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled();
	m_zones = &zones;
	m_simPara = &simPara;

	// no need to check for parameters here, the NaturalVentilationModel parametrization was already checked
	// schedule parameters are requested below
	m_ventilationRate = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_VentilationRate].value;

	// all models require an object list with indication of ventilated zones
	if (m_ventilationModel->m_zoneObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ZoneObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  m_ventilationModel->m_zoneObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined ZoneObjectList '%1'.")
							 .arg(m_ventilationModel->m_zoneObjectList), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_ZONE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'Zone'.")
							 .arg(m_objectList->m_name), FUNC_ID);

	// reserve storage memory for results
	m_vectorValuedResults.resize(NUM_VVR);

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


void NaturalVentilationModel::initResults(const std::vector<AbstractModel *> &) {
	FUNCID(NaturalVentilationModel::initResults);

	// no model IDs, nothing to do (see explanation in resultDescriptions())
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of ventilated zones
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
		m_zoneVolumes.push_back(it->m_para[NANDRAD::Zone::P_Volume].value);
	}
}


void NaturalVentilationModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
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
		const std::string &quantityName = KeywordList::Keyword("NaturalVentilationModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("NaturalVentilationModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("NaturalVentilationModel::VectorValuedResults", varIndex );
		// vector-valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription,
			false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
	}
}


const double * NaturalVentilationModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_VVR; ++varIndex) {
		if (KeywordList::Keyword("NaturalVentilationModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
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


void NaturalVentilationModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// We need ambient temperature from loads and we need air temperatures from all ventilated zones:
	// 1. Loads.Temperature
	// 2. Zone[id1].AirTemperature
	// 3. Zone[id2].AirTemperature
	// 4. ...
	// where the zone IDs follow the order of the IDs in the object list

	InputReference ref;
	ref.m_id = 0;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
	ref.m_name.m_name = "Temperature";
	ref.m_required = true;
	inputRefs.push_back(ref);
	for (unsigned int id : m_objectList->m_filterID.m_ids) {
		InputReference ref;
		ref.m_id = id;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ref.m_name.m_name = "AirTemperature";
		ref.m_required = true;
		inputRefs.push_back(ref);
	}

	// offset 1 + nZones
	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_Scheduled
		|| m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR)
	{
		for (unsigned int id : m_objectList->m_filterID.m_ids) {
			InputReference ref;
			ref.m_id = id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ref.m_name.m_name = "VentilationRateSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
		}
	}

	// offset 1 + 2*nZones
	// for MT_ScheduledWithBaseACR we also need VentilationRateIncreaseSchedule
	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR) {
		for (unsigned int id : m_objectList->m_filterID.m_ids) {
			InputReference ref;
			ref.m_id = id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ref.m_name.m_name = "VentilationRateIncreaseSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
		}
	}

	// offset 1 + 2*nZones
	// for MT_ScheduledWithBaseACR we also need MinimumAirTemperature
	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR) {
		for (unsigned int id : m_objectList->m_filterID.m_ids) {
			InputReference ref;
			ref.m_id = id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ref.m_name.m_name = "MinimumAirTemperatureSchedule";
			ref.m_required = false;
			inputRefs.push_back(ref);
		}
	}

	// offset 1 + 2*nZones
	// for MT_ScheduledWithBaseACR we also need MaximumAirTemperature
	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR) {
		for (unsigned int id : m_objectList->m_filterID.m_ids) {
			InputReference ref;
			ref.m_id = id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ref.m_name.m_name = "MaximumAirTemperatureSchedule";
			ref.m_required = false;
			inputRefs.push_back(ref);
		}
	}

	// offset 1 + 2*nZones for MT_Scheduled  or
	// offset 1 + 3*nZones for MT_ScheduledWithBaseACR
	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR) {
		// also the wind speed from location
		InputReference ref;
		ref.m_id = 0;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
		ref.m_name.m_name = "WindVelocity";
		ref.m_required = true;
		inputRefs.push_back(ref);
	}

}


void NaturalVentilationModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	unsigned int startIndexMinAirTemperature = 0;
	unsigned int startIndexMaxAirTemperature = 0;
	// simply store and check value references
	unsigned int expectedSize = 1 + m_objectList->m_filterID.m_ids.size(); // ambient temperature and all zone's temperatures
	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_Scheduled)
		expectedSize += m_objectList->m_filterID.m_ids.size(); // VentilationRateSchedule
	else if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR) {
		expectedSize += 2*m_objectList->m_filterID.m_ids.size(); // VentilationRateSchedule and VentilationRateIncreaseSchedule
		// mark index ranges for optional references
		startIndexMinAirTemperature = expectedSize;
		startIndexMaxAirTemperature = expectedSize + m_objectList->m_filterID.m_ids.size();
		expectedSize += 2*m_objectList->m_filterID.m_ids.size(); // MinimumAirTemperature and MaximumAirTemperature
		++expectedSize; // for wind velocity
	}
	IBK_ASSERT(resultValueRefs.size() == expectedSize);

	for(unsigned int i = 0; i < expectedSize; ++i) {
		if(resultValueRefs[i] == nullptr) {
			// minimum air temperature
			if(i >= startIndexMinAirTemperature && i < startIndexMaxAirTemperature) {
				m_valueRefs.push_back(&m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_MinAirTemperature].value);
				continue;
			}
			// maximum air temperature
			if(i >= startIndexMaxAirTemperature && i < startIndexMaxAirTemperature + m_objectList->m_filterID.m_ids.size()) {
				m_valueRefs.push_back(&m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_MaxAirTemperature].value);
				continue;
			}
		}
		m_valueRefs.push_back(resultValueRefs[i]);
	}
}


void NaturalVentilationModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// we compute ventilation rates per zone and heat fluxes per zone, ventilation rates (currently) have
	// no dependencies (only from schedules), but heat fluxes depend on ambient temperatures and on zone temperatures
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		// pair: result - input

		// dependency on room air temperature of corresponding zone
		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_VentilationRate].dataPtr() + i, m_valueRefs[1+i]) );
		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_VentilationHeatFlux].dataPtr() + i, m_valueRefs[1+i]) );
	}
}


int NaturalVentilationModel::update() {
	unsigned int zoneCount = m_zoneVolumes.size();
	// Note: order of value refs
	//  - ambient temperature from loads
	//  - zonal air temperatures (size = zoneCount)
	//  - zonal VentilationRateSchedule (size = zoneCount)
	//  - (only for MT_ScheduledWithBaseACR) zonal VentilationRateIncreaseSchedule (size = zoneCount)
	//  - (only for MT_ScheduledWithBaseACR) WindVelocity


	// get ambient temperature in  [K]
	double Tambient = *m_valueRefs[0];
	// loop over all zones
	double * resultVentRate = m_vectorValuedResults[VVR_VentilationRate].dataPtr();
	double * resultVentHeatFlux = m_vectorValuedResults[VVR_VentilationHeatFlux].dataPtr();
	for (unsigned int i=0; i<zoneCount; ++i) {
		// get room air temperature in [K]
		double Tzone = *m_valueRefs[i+1];
		// get ventilation rate in [1/s]
		double rate = m_ventilationRate;
		switch (m_ventilationModel->m_modelType) {

			case NANDRAD::NaturalVentilationModel::MT_Scheduled : {
				// retrieve scheduled ventilation rate from schedules
				rate = *m_valueRefs[1+zoneCount+i];
			} break;

			case NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR : {
				// initialize rate with base rate
				rate = *m_valueRefs[1+zoneCount+i];
				// get the scheduled quantities
				double varWindVelocity = *m_valueRefs[1+5*zoneCount];
				double varWindSpeedACRLimit = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_MaxWindSpeed].value;
				if (varWindVelocity > varWindSpeedACRLimit)
					break; // wind speed to large, no increase of ventilation possible - keep already determined "rate"

				// get comfort range of temperatures
				double maxRoomTemp = *m_valueRefs[1+4*zoneCount + i];
				double minRoomTemp = *m_valueRefs[1+3*zoneCount + i];

				// we only increase ventilation when outside of the comfort zone _and_ if increasing the ventilation rate helps
				const double RAMPING_DELTA_T = 0.2; // ramping range

				double eps = 0; // assume no increase in ventilation
				if (Tzone < minRoomTemp) {
					double epsRamp = IBK::scale( std::min(1., (minRoomTemp - Tzone)/RAMPING_DELTA_T ) );
					// ventilation increases room temperature?
					if (Tambient > Tzone) {
						double epsRamp2 = IBK::scale( std::min(1., (Tambient - Tzone)/RAMPING_DELTA_T) );
						eps = epsRamp*epsRamp2;
					}
				}

				// above comfort range?
				else if (Tzone > maxRoomTemp) {
					double epsRamp = IBK::scale( std::min(1., (Tzone - maxRoomTemp)/RAMPING_DELTA_T ) );
					// ventilation decreases room temperature?
					if (Tambient < Tzone) {
						double epsRamp2 = IBK::scale( std::min(1., (Tzone - Tambient)/RAMPING_DELTA_T) );
						eps = epsRamp*epsRamp2;
					}
				}

				// get _additional_ ventilation rate
				double rateIncrease = *m_valueRefs[1+2*zoneCount+i];

				// compute final rate
				rate += eps * rateIncrease;
			} break;

			default: ;
		}
		// store ventilation rate result
		resultVentRate[i] = rate;
		// compute ventilation heat flux in [W]
		resultVentHeatFlux[i] = IBK::RHO_AIR*IBK::C_AIR*m_zoneVolumes[i]*(Tambient - Tzone)*rate;
	}

	return 0; // signal success
}


} // namespace NANDRAD_MODEL
