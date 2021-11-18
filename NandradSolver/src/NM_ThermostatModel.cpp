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

#include "NM_ThermostatModel.h"

#include <IBK_Exception.h>
#include <IBK_physics.h>

#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Thermostat.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"
#include "NM_Controller.h"

namespace NANDRAD_MODEL {


ThermostatModel::~ThermostatModel() {
	// memory cleanup
	for (AbstractController * ptr : m_controllers)
		delete ptr;
	m_controllers.clear();
}


void ThermostatModel::setup(const NANDRAD::Thermostat & thermostat,
							const std::vector<NANDRAD::ObjectList> & objLists,
							const std::vector<NANDRAD::Zone> & zones)
{
	FUNCID(ThermostatModel::setup);

	m_thermostat = &thermostat;
	m_zones = &zones;

	// all models require an object list with indication of ventilated zones
	if (m_thermostat->m_zoneObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ZoneObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  m_thermostat->m_zoneObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined ZoneObjectList '%1'.")
							 .arg(m_thermostat->m_zoneObjectList), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_ZONE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'Zone'.")
							 .arg(m_objectList->m_name), FUNC_ID);

	// reserve storage memory for results
	m_vectorValuedResults.resize(NUM_VVR);

	// check if reference zone is given and check if such a zone exists
	if (m_thermostat->m_referenceZoneId != NANDRAD::INVALID_ID) {
		std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(), zones.end(),
			m_thermostat->m_referenceZoneId);
		if (zone_it == zones.end())
			throw IBK::Exception(IBK::FormatString("Invalid/undefined zone ID #%1.")
								 .arg(m_thermostat->m_referenceZoneId), FUNC_ID);
	}

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in initResults()
}


void ThermostatModel::initResults(const std::vector<AbstractModel *> &) {
//	FUNCID(ThermostatModel::initResults);

	// no model IDs, nothing to do (see explanation in resultDescriptions())
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of ventilated zones
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());
	// resize result vectors accordingly
	for (unsigned int varIndex=0; varIndex<NUM_VVR; ++varIndex)
		m_vectorValuedResults[varIndex] = VectorValuedQuantity(indexKeys);

	// initialize controller
	unsigned int zoneCount = 1; // assome reference zone
	if (m_thermostat->m_referenceZoneId == NANDRAD::INVALID_ID)
		zoneCount = indexKeys.size(); // one for each zone

	// actually, we create two controllers for each zone, one for heating, one for cooling
	for (unsigned int i = 0; i<zoneCount; ++i) {
		switch (m_thermostat->m_controllerType) {
			case NANDRAD::Thermostat::NUM_CT : // default to P-Controller
			case NANDRAD::Thermostat::CT_Analog : {
				PController * con = new PController;
				con->m_kP = 1/m_thermostat->m_para[NANDRAD::Thermostat::P_TemperatureTolerance].value;
				m_controllers.push_back(con);
				// now the cooling controller
				con = new PController;
				con->m_kP = 1/m_thermostat->m_para[NANDRAD::Thermostat::P_TemperatureTolerance].value;
				m_controllers.push_back(con);
			} break;

			case NANDRAD::Thermostat::CT_Digital : {
				DigitalHysteresisController * con = new DigitalHysteresisController;
				con->m_hysteresisBand = m_thermostat->m_para[NANDRAD::Thermostat::P_TemperatureBand].value;
				m_controllers.push_back(con);
				// now the cooling controller
				con = new DigitalHysteresisController;
				con->m_hysteresisBand = m_thermostat->m_para[NANDRAD::Thermostat::P_TemperatureBand].value;
				m_controllers.push_back(con);
			} break;
		}
	}
}


void ThermostatModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those zones were added, that are actually parameterized
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
		const std::string &quantityName = KeywordList::Keyword("ThermostatModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("ThermostatModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("ThermostatModel::VectorValuedResults", varIndex );
		// vector-valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription,
			false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
	}
}


const double * ThermostatModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_VVR; ++varIndex) {
		if (KeywordList::Keyword("ThermostatModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
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


std::size_t ThermostatModel::serializationSize() const {
	std::size_t size = 0;
	// sum up serialization size of all controllers
	for(const AbstractController* controller: m_controllers) {
		size += controller->serializationSize();
	}
	return size;
}


void ThermostatModel::serialize(void *& dataPtr) const {
	// serialize all controllers and shift data pointer
	for(const AbstractController* controller: m_controllers) {
		controller->serialize(dataPtr);
	}
}


void ThermostatModel::deserialize(void *& dataPtr) {
	// deserialize all controllers and shift data pointer
	for(AbstractController* controller: m_controllers) {
		controller->deserialize(dataPtr);
	}
}


void ThermostatModel::stepCompleted(double t) {
	for (NANDRAD_MODEL::AbstractController* c : m_controllers)
		c->stepCompleted(t);
}


void ThermostatModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// distinguish between single-zone sensor and per-zone-sensor
	if (m_thermostat->m_referenceZoneId != NANDRAD::INVALID_ID) {
		// only one input reference to the selected zone
		InputReference ref;
		ref.m_id = m_thermostat->m_referenceZoneId;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		switch (m_thermostat->m_temperatureType) {
			case NANDRAD::Thermostat::NUM_TT: // default to air temperature
			case NANDRAD::Thermostat::TT_AirTemperature:
				ref.m_name.m_name = "AirTemperature";
			break;
			case NANDRAD::Thermostat::TT_OperativeTemperature:
				ref.m_name.m_name = "OperativeTemperature";
			break;
		}
		ref.m_required = true;
		inputRefs.push_back(ref);
		// scheduled model variant?
		if (m_thermostat->m_modelType == NANDRAD::Thermostat::MT_Scheduled) {
			// add references to heating and cooling setpoints
			// Note: for now we require these... better to be very strict than to be lazy and cause hard-to-detect
			//       problems when using the model.
			InputReference ref;
			ref.m_id = m_thermostat->m_referenceZoneId;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ref.m_name.m_name = "HeatingSetpointSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);

			ref.m_name.m_name = "CoolingSetpointSchedule";
			inputRefs.push_back(ref);
		}
	}
	else {
		// each zone's thermstat uses the zone-specific inputs
		for (unsigned int id : m_objectList->m_filterID.m_ids) {
			InputReference ref;
			ref.m_id = id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			switch (m_thermostat->m_temperatureType) {
				case NANDRAD::Thermostat::NUM_TT: // default to air temperature
				case NANDRAD::Thermostat::TT_AirTemperature:
					ref.m_name.m_name = "AirTemperature";
				break;
				case NANDRAD::Thermostat::TT_OperativeTemperature:
					ref.m_name.m_name = "OperativeTemperature";
				break;
			}
			ref.m_required = true;
			inputRefs.push_back(ref);
			// scheduled model variant?
			if (m_thermostat->m_modelType == NANDRAD::Thermostat::MT_Scheduled) {
				// add references to heating and cooling setpoints
				// Note: for now we require these... better to be very strict than to be lazy and cause hard-to-detect
				//       problems when using the model.
				ref.m_name.m_name = "HeatingSetpointSchedule";
				inputRefs.push_back(ref);

				ref.m_name.m_name = "CoolingSetpointSchedule";
				inputRefs.push_back(ref);
			}
		}
	}

}


void ThermostatModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	// count number of inputs that we expect
	// zone temperatures
	unsigned int expectedSize =  m_objectList->m_filterID.m_ids.size(); // all zone's temperatures
	if (m_thermostat->m_referenceZoneId != NANDRAD::INVALID_ID)
		expectedSize = 1;
	// for thermostat with scheduled setpoints, we have a schedule input for each zone
	if (m_thermostat->m_modelType == NANDRAD::Thermostat::MT_Scheduled)
		expectedSize *= 3; // AirTemperature, HeatingSetpointSchedule and CoolingSetpointSchedule
	IBK_ASSERT(resultValueRefs.size() == expectedSize);
	m_valueRefs = resultValueRefs;
}


void ThermostatModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// resultInputValueReferences holds pairs: result - input

	// do we have a reference zone?
	if (m_thermostat->m_referenceZoneId != NANDRAD::INVALID_ID) {
		// all zone's heating and cooling control values depend on this zone's temperature
		for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
			// dependency on room air temperature of corresponding zone
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_HeatingControlValue].dataPtr() + i, m_valueRefs[0]) );
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_CoolingControlValue].dataPtr() + i, m_valueRefs[0]) );
		}
	}

	else {
		// each zone's heating and cooling control values depend on the zone's temperatures
		unsigned int inputVarsPerZone = 1;
		if (m_thermostat->m_modelType == NANDRAD::Thermostat::MT_Scheduled)
			inputVarsPerZone = 3;
		for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
			// dependency on room air temperature of corresponding zone
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_HeatingControlValue].dataPtr() + i, m_valueRefs[inputVarsPerZone*i]) );
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_CoolingControlValue].dataPtr() + i, m_valueRefs[inputVarsPerZone*i]) );
		}
	}

}


int ThermostatModel::update() {
	FUNCID(ThermostatModel::update);

	unsigned int inputVarsPerZone = 1;
	if (m_thermostat->m_modelType == NANDRAD::Thermostat::MT_Scheduled)
		inputVarsPerZone = 3;

	double * heatingControlSetpoints = m_vectorValuedResults[VVR_ThermostatHeatingSetpoint].dataPtr();
	double * coolingControlSetpoints = m_vectorValuedResults[VVR_ThermostatCoolingSetpoint].dataPtr();

	// loop over all thermostats
	unsigned int nZones = m_controllers.size()/2;
	for (unsigned int i=0; i<nZones; ++i) {
		double Troom = *m_valueRefs[i*inputVarsPerZone];
		double TSetpointHeating = m_thermostat->m_para[NANDRAD::Thermostat::P_HeatingSetpoint].value;
		double TSetpointCooling = m_thermostat->m_para[NANDRAD::Thermostat::P_CoolingSetpoint].value;
		if (m_thermostat->m_modelType == NANDRAD::Thermostat::MT_Scheduled) {
			TSetpointHeating = *m_valueRefs[i*inputVarsPerZone + 1];
			TSetpointCooling = *m_valueRefs[i*inputVarsPerZone + 2];
		}

		// if we have setpoint temperatures in each zone, we store the corresponding setpoint right here
		if (m_thermostat->m_referenceZoneId == NANDRAD::INVALID_ID) {
			heatingControlSetpoints[i] = TSetpointHeating;
			coolingControlSetpoints[i] = TSetpointCooling;
		}

		// update heating and cooling controllers
		m_controllers[i*2]->update(TSetpointHeating - Troom);
		m_controllers[i*2+1]->update(Troom - TSetpointCooling); // Mind the sign! Turn on cooling when room is _above_ setpoint
	}

	double * heatingControlValues = m_vectorValuedResults[VVR_HeatingControlValue].dataPtr();
	double * coolingControlValues = m_vectorValuedResults[VVR_CoolingControlValue].dataPtr();

	// transfer results
	if (m_thermostat->m_referenceZoneId != NANDRAD::INVALID_ID) {
		double TSetpointHeating = m_thermostat->m_para[NANDRAD::Thermostat::P_HeatingSetpoint].value;
		double TSetpointCooling = m_thermostat->m_para[NANDRAD::Thermostat::P_CoolingSetpoint].value;
		if (m_thermostat->m_modelType == NANDRAD::Thermostat::MT_Scheduled) {
			TSetpointHeating = *m_valueRefs[1];
			TSetpointCooling = *m_valueRefs[2];
		}

		// reference zone - all results get the same control values
		for (unsigned int i=0; i<nZones; ++i) {
			heatingControlValues[i] = m_controllers[0]->m_controlValue;
			coolingControlValues[i] = m_controllers[1]->m_controlValue;
			// setpoints are the same for all zones; override earlier stored zone-specific setpoints
			heatingControlSetpoints[i] = TSetpointHeating;
			coolingControlSetpoints[i] = TSetpointCooling;
		}
	}
	else {
		// each zone gets the results of its own controller
		for (unsigned int i=0; i<nZones; ++i) {
			heatingControlValues[i] = m_controllers[i*2]->m_controlValue;
			coolingControlValues[i] = m_controllers[i*2 + 1]->m_controlValue;
		}
	}

	// check that we never have heating and cooling at the same time
	for (unsigned int i=0; i<nZones; ++i) {
		if (*(m_vectorValuedResults[VVR_HeatingControlValue].dataPtr() + i) > 0 &&
			*(m_vectorValuedResults[VVR_CoolingControlValue].dataPtr() + i) > 0)
		{
			throw IBK::Exception(IBK::FormatString("Thermostat for zone #%1 turns on heating and cooling at the same time, using "
								 "heating/cooling setpoints %2 C and %3 C, respectively.").
				arg((*m_zones)[i].m_id).arg(heatingControlSetpoints[i]-273.15).arg(coolingControlSetpoints[i]-273.15), FUNC_ID);
		}
	}

	return 0; // signal success
}


} // namespace NANDRAD_MODEL
