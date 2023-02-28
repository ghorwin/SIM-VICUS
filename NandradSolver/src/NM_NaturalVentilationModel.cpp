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

	if(m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_Constant) {
		// no need to check for parameters here, the NaturalVentilationModel parametrization was already checked
		// schedule parameters are requested below
		m_ventilationRate = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_VentilationRate].value;
	}

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

	unsigned int varCount = 2; // air change rate and heat flux are the first quantities
	if (m_moistureBalanceEnabled) {
		varCount = NUM_VVR; // more variables for hygrothermal calculation
	}
	// reserve storage memory for results
	m_vectorValuedResults.resize(varCount);

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


void NaturalVentilationModel::initResults(const std::vector<AbstractModel *> &) {
	FUNCID(NaturalVentilationModel::initResults);

	// no model IDs, nothing to do (see explanation in resultDescriptions())
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of ventilated zones
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	unsigned int varCount = 2; // air change rate and heat flux are the first quantities
	if (m_moistureBalanceEnabled) {
		varCount = NUM_VVR; // more variables for hygrothermal calculation
	}

	// resize result vectors accordingly
	unsigned int dummyVal=900;
	for (unsigned int varIndex=0; varIndex<varCount; ++varIndex) {
		m_vectorValuedResults[varIndex] = VectorValuedQuantity(indexKeys);
		// initialize with dummy values for debugging
		for (unsigned int i=0; i< m_vectorValuedResults[varIndex].size(); ++i)
			 m_vectorValuedResults[varIndex].dataPtr()[i] = ++dummyVal;
	}

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

	int varCount = 2; // air change rate and heat flux are the first quantities
	if (m_moistureBalanceEnabled) {
		varCount = NUM_VVR; // more variables for hygrothermal calculation
	}

	// For each of the zones in the object list we generate vector-valued results as defined
	// in the type VectorValuedResults.
	for (int varIndex=0; varIndex < varCount; ++varIndex) {
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

	int varCount = 2; // air change rate and heat flux are the first quantities
	if (m_moistureBalanceEnabled) {
		varCount = NUM_VVR; // more variables for hygrothermal calculation
	}

	// determine variable enum index
	int varIndex=0;
	for (; varIndex<varCount; ++varIndex) {
		if (KeywordList::Keyword("NaturalVentilationModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
			break;
	}
	if (varIndex == varCount)
		return nullptr;
	// now check the index
	if (quantityName.m_index == -1) // no index - return start of vector
		return m_vectorValuedResults[(unsigned int) varIndex].dataPtr();
	// search for index
	try {
		const double & valRef = m_vectorValuedResults[(unsigned int) varIndex][(unsigned int)quantityName.m_index];
		return &valRef;
	} catch (...) {
		// exception is thrown when index is not available - return nullptr
		return nullptr;
	}
}


void NaturalVentilationModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// We need temperature, wind velocity and relative humidity from loads and we need air temperatures from all ventilated zones.
	// Not all quantities are always needed.
	//
	// Order of input refs:
	// - Loads.Temperature
	// - (optional) Loads.WindVelocity     (only for MT_ScheduledWithBaseACRxxxx models)
	// - (optional) Loads.MoistureDensity  (only for moisture balance)
	//
	// then the zonal variables, a set of variables for each zone
	// - Zone[id1].AirTemperature
	// - (optional) Zone[id1].MoistureDensity    (only for moisture balance)
	// - (optional) Zone[id1].VentilationRateSchedule
	// - (optional) Zone[id1].VentilationRateIncreaseSchedule
	// - (optional) Zone[id1].VentilationMinAirTemperatureSchedule
	// - (optional) Zone[id1].VentilationMaxAirTemperatureSchedule
	//
	// - Zone[id2].AirTemperature
	// ...
	// where the zone IDs follow the order of the IDs in the object list

	m_zoneVariableOffset = 0;
	InputReference ref;
	ref.m_id = 0;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
	ref.m_name.m_name = "Temperature";
	ref.m_required = true;
	inputRefs.push_back(ref); ++m_zoneVariableOffset;

	if (m_simPara->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled()) {
		ref.m_name.m_name = "MoistureDensity";
		inputRefs.push_back(ref);  ++m_zoneVariableOffset;
	}

	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR||
		m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACRDynamicTLimit)
	{
		// also the wind speed from location
		ref.m_name.m_name = "WindVelocity";
		inputRefs.push_back(ref);  ++m_zoneVariableOffset;
	}

	// now the zone-specific variables

	for (unsigned int id : m_objectList->m_filterID.m_ids) {
		unsigned int zoneVarCount = 0;
		InputReference ref;
		ref.m_id = id;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ref.m_name.m_name = "AirTemperature";
		ref.m_required = true;
		inputRefs.push_back(ref); ++zoneVarCount;

		if (m_simPara->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled()) {
			ref.m_name.m_name = "MoistureDensity";
			inputRefs.push_back(ref); ++zoneVarCount;
		}

		if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_Scheduled
			|| m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR
			|| m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACRDynamicTLimit)
		{
			ref.m_name.m_name = "VentilationRateSchedule";
			inputRefs.push_back(ref); ++zoneVarCount;
		}

		if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR ||
			m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACRDynamicTLimit)
		{
			ref.m_name.m_name = "VentilationRateIncreaseSchedule";
			inputRefs.push_back(ref); ++zoneVarCount;
		}

		if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACRDynamicTLimit) {
			ref.m_name.m_name = "VentilationMinAirTemperatureSchedule";
			inputRefs.push_back(ref); ++zoneVarCount;
			ref.m_name.m_name = "VentilationMaxAirTemperatureSchedule";
			inputRefs.push_back(ref); ++zoneVarCount;
		}
		m_zoneVariableCount = zoneVarCount;
	} // for (unsigned int id : m_objectList->m_filterID.m_ids)

}


void NaturalVentilationModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	// simply store and check value references
	unsigned int zoneCount =  m_objectList->m_filterID.m_ids.size();
	unsigned int expectedSize = 1 + zoneCount; // ambient temperature and all zone's temperatures
	// moisture balance
	if (m_simPara->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled())
		expectedSize += 1 + zoneCount; // ambient and room air moisture density

	if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_Scheduled)
		expectedSize += zoneCount; // VentilationRateSchedule
	else if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR) {
		expectedSize += 2*zoneCount; // VentilationRateSchedule and VentilationRateIncreaseSchedule
		++expectedSize; // for wind velocity
	}
	else if (m_ventilationModel->m_modelType == NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACRDynamicTLimit) {
		expectedSize += 2*zoneCount; // VentilationRateSchedule and VentilationRateIncreaseSchedule
		expectedSize += 2*zoneCount; // MinimumAirTemperature and MaximumAirTemperature
		++expectedSize; // for wind velocity
	}
	IBK_ASSERT(resultValueRefs.size() == expectedSize);
	expectedSize = m_zoneVariableOffset + m_objectList->m_filterID.m_ids.size()*m_zoneVariableCount;
	IBK_ASSERT(resultValueRefs.size() == expectedSize);

	// copy result value references
	m_valueRefs = resultValueRefs;
}


void NaturalVentilationModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// We compute ventilation rates, heat fluxes and optional also moisture fluxes (production rates) per zone.
	// Ventilation rates may depend on zone temperature (in case of models MT_ScheduledWithBaseACRxxxx)
	// and for moisture balance also from zone moisture mass density. For simplicity, we add these dependencies always,
	// regardless of the model type.

	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		// pair: result - input

		// dependency on room air temperature of corresponding zone
		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_VentilationRate].dataPtr() + i, m_valueRefs[m_zoneVariableOffset+i*m_zoneVariableCount]) );
		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_VentilationHeatFlux].dataPtr() + i, m_valueRefs[m_zoneVariableOffset+i*m_zoneVariableCount]) );
	}

	if (m_moistureBalanceEnabled) {
		for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
			// pair: result - input
			// dependency on room moisture density of corresponding zone - this is the 2nd variable in the zone variable block
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_VentilationMoistureMassFlux].dataPtr() + i, m_valueRefs[m_zoneVariableOffset+i*m_zoneVariableCount+1]) );
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_VentilationHeatFlux].dataPtr() + i, m_valueRefs[m_zoneVariableOffset+i*m_zoneVariableCount+1]) );
		}
	}
}


int NaturalVentilationModel::update() {
	unsigned int zoneCount = m_zoneVolumes.size();

	// Note: order of value refs, see inputReferences()

	// get ambient temperature in  [K]
	double Tambient = *m_valueRefs[0];;

	// store pointer to result quantities
	double * resultVentRate = m_vectorValuedResults[VVR_VentilationRate].dataPtr();

	// if we have moisture balance enabled, we have one extra variable before scheduled quantities (see variable order)
	unsigned int moistureBalanceOffset = 0;
	if (m_moistureBalanceEnabled)
		moistureBalanceOffset = 1;

	// loop over all zones
	for (unsigned int i=0; i<zoneCount; ++i) {
		unsigned int varOffset = m_zoneVariableOffset+i*m_zoneVariableCount; // offset of first zone-specific variable
		// get room air temperature in [K]
		double Tzone = *m_valueRefs[varOffset];
		// ventilation rate in [1/s]
		double rate = 999; // initialized to silence compiler warnings
		switch (m_ventilationModel->m_modelType) {

			case NANDRAD::NaturalVentilationModel::MT_Constant : {
				rate = m_ventilationRate;
			} break;

			case NANDRAD::NaturalVentilationModel::MT_Scheduled : {
				// retrieve scheduled ventilation rate from schedules
				rate = *m_valueRefs[varOffset + moistureBalanceOffset + 1];
			} break;

			case NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR : {
				// initialize rate with base rate
				rate = *m_valueRefs[varOffset + moistureBalanceOffset + 1];
				// wind velocity in [m/s]
				double varWindVelocity = *m_valueRefs[moistureBalanceOffset + 1];
				double varWindSpeedACRLimit = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_MaxWindSpeed].value;
				if (varWindVelocity > varWindSpeedACRLimit)
					break; // wind speed too large, no increase of ventilation possible - keep already determined "rate"

				// get comfort range of temperatures
				double maxRoomTemp = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_VentilationMaxAirTemperature].value;
				double minRoomTemp = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_VentilationMinAirTemperature].value;

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
				double rateIncrease = *m_valueRefs[varOffset + moistureBalanceOffset + 2];

				// compute final rate
				rate += eps * rateIncrease;
			} break;

			case NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACRDynamicTLimit : {
				// initialize rate with base rate
				rate = *m_valueRefs[varOffset + moistureBalanceOffset + 1];
				// get the scheduled quantities
				double varWindVelocity = *m_valueRefs[moistureBalanceOffset + 1];
				double varWindSpeedACRLimit = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_MaxWindSpeed].value;
				if (varWindVelocity > varWindSpeedACRLimit)
					break; // wind speed too large, no increase of ventilation possible - keep already determined "rate"

				// get comfort range of temperatures
				double minRoomTemp = *m_valueRefs[varOffset + moistureBalanceOffset + 3];
				double maxRoomTemp = *m_valueRefs[varOffset + moistureBalanceOffset + 4];

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
				double rateIncrease = *m_valueRefs[varOffset + moistureBalanceOffset + 2];

				// compute final rate
				rate += eps * rateIncrease;
			} break;
			default: ;
		}
		// store ventilation rate result
		resultVentRate[i] = rate;
	}

	// Note: resultVentRate[i] now contains a ventilation rate in [1/s]

	// calculation of mass fluxes and enthalpies works as follows (as in DELPHIN):

	// - compute volumetric flow rate in m3/s
	// - assume constant dry air density and compute dry air mass flow rate in kg(dry_air)/s
	// - (hygrothermal) compute density of water vapour in supply/exhaust air
	// - compute vapour mass flow rate based on vapor pressure and absolute gas pressure (constant for now)
	//   and ratios of ideal gas constants (this assumes that gas pressure is dominated by air pressure and
	//   pg = pa holds for most situations

	// calculate heat and moisture mass fluxes

	// moist air
	if (m_moistureBalanceEnabled) {
		// constant parameters
		double cVapor = IBK::C_VAPOR;
		double cAir = IBK::C_AIR;
		double HEvap = IBK::H_EVAP;

		// store pointer to result quantities
		double * resultVentHeatFlux = m_vectorValuedResults[VVR_VentilationHeatFlux].dataPtr();
		double * resultVentMassFlux = m_vectorValuedResults[VVR_VentilationMoistureMassFlux].dataPtr();

		// get ambient vapor, gas and air density [kg/m3]
		double rhoVaporAmbient = *m_valueRefs[1];

		// loop over all zones
		for (unsigned int i=0; i<zoneCount; ++i) {
			unsigned int varOffset = m_zoneVariableOffset+i*m_zoneVariableCount; // offset of first zone-specific variable
			// get room air temperature in [K]
			double Tzone = *m_valueRefs[varOffset];

			// get ventilation rate in [1/s]
			double rate = resultVentRate[i];
			double mdot_air = rate*m_zoneVolumes[i]*IBK::RHO_AIR; // dry air mass flux

			// get zone vapor density in [kg/m3]
			double rhoVaporZone = *m_valueRefs[varOffset + 1];

			// [cv] = rho(Vapor) / rho(gas)
			double cvVaporZone = rhoVaporZone / IBK::RHO_AIR; // for now a constant
			double cvVaporAmbient = rhoVaporAmbient / IBK::RHO_AIR; // for now a constant

			// mass flux vapor
			// mdot_v = vapor_density/air_density * mdot_air
			// - mdot_air is an undirected air change rate
			// - mdot_v is directed: positive = moisture mass gain in zone, negative = moisture mass loss in zone
			double mdot_v = (cvVaporAmbient - cvVaporZone) * mdot_air;
			resultVentMassFlux[i] = mdot_v;

			// compute enthalpy flux differences between incoming and outgoing air in [W]
			// but use upwinding for moisture enthalpie
			resultVentHeatFlux[i] = cAir * (Tambient - Tzone) * mdot_air;
			if (mdot_v > 0) {
				// vapor flux into the zone, use ambient conditions
				// vapor leaving the zone
				resultVentHeatFlux[i] += (cVapor  * Tambient + HEvap)*mdot_v;
			}
			else {
				// vapour out of zone, use zone conditions
				resultVentHeatFlux[i] += (cVapor  * Tzone + HEvap) * mdot_v;
			}
		}
	}
	// dry air
	else {
		// constant parameters
		double cAir = IBK::C_AIR;
		double rhoAirAmbient = IBK::RHO_AIR;

		// store pointer to result quantities
		double * resultVentHeatFlux = m_vectorValuedResults[VVR_VentilationHeatFlux].dataPtr();

		// loop over all zones
		for (unsigned int i=0; i<zoneCount; ++i) {
			unsigned int varOffset = m_zoneVariableOffset+i*m_zoneVariableCount; // offset of first zone-specific variable
			// get room air temperature in [K]
			double Tzone = *m_valueRefs[varOffset];

			// get ventilation rate in [1/s]
			double rate = resultVentRate[i];
			resultVentHeatFlux[i] = rhoAirAmbient * cAir * m_zoneVolumes[i]*(Tambient - Tzone)*rate;
		}
	}

	return 0; // signal success
}


} // namespace NANDRAD_MODEL
