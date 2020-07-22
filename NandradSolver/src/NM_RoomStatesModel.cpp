/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#include <IBK_messages.h>

#include "NM_RoomStatesModel.h"

#include "NM_Constants.h"
#include "NM_ConstructionStatesModel.h"
#include "NM_EquipmentLoadModel.h"
#include "NM_FluidModel.h"
#include "NM_FMU2ExportModel.h"
#include "NM_KeywordList.h"
#include "NM_LightingLoadModel.h"
#include "NM_Schedules.h"
#include "NM_ThermalComfortModel.h"

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_ParametrizationDefaults.h>
#include <NANDRAD_SpaceType.h>
#include <NANDRAD_Zone.h>

using namespace std;

namespace NANDRAD_MODEL {

RoomStatesModel::RoomStatesModel(unsigned int id, const std::string &displayName) :
	DefaultModel(id, displayName),
	DefaultStateDependency(SteadyState),
	m_fluid(NULL),
	m_zone(NULL),
	m_parametrizationDefaults(NULL),
	m_TInitial(0),
	m_defaultHeatCapacityAdditionalMass(0)
{
}

int RoomStatesModel::priorityOfModelEvaluation() const {
	// RoomStatesModel is evaluated first. It decomposes the quantities provided
	// from the solver and prepares result quantities for easy access.
	return 0;
}

void RoomStatesModel::setup(const NANDRAD::Zone & zone,
		const NANDRAD::ParametrizationDefaults &defaultPara)
{

	const char* const FUNC_ID = "[RoomStatesModel::setup]";
	// The id is copied from the generating zone
	if(zone.m_id != id())
		throw IBK::Exception(IBK::FormatString("Error initializing #%1 for zone with id #%2: "
			"Only initialization with zone of id #%3 is allowed." )
			.arg(ModelIDName()).arg(m_zone->m_id).arg(id()),FUNC_ID);
	// set a pointer to the NANDRAD parametrization objects
	m_zone					  = &zone;
	m_parametrizationDefaults = &defaultPara;
}

void RoomStatesModel::initResults(const std::vector<AbstractModel*> & models) {
	const char* const FUNC_ID = "[RoomStatesModel::initResults]";
	// resize m_results from keyword list
	DefaultModel::initResults(models);

	// The room volume may be given by zone parametrization or by floor area and height.
	// Thus, we better define a referencable result quantity than an input reference to
	// a project parameter.
	if (!m_zone->m_para[NANDRAD::Zone::ZP_VOLUME].name.empty()) {
		const double volume = m_zone->m_para[NANDRAD::Zone::ZP_VOLUME].value;
		if(volume < 0.0) {
			throw IBK::Exception(IBK::FormatString("Error initializing RoomStatesModel of zone with ID %1: "
			"Parameter 'Volume'is smaller than 0.")
			.arg(id()),
			FUNC_ID);
		}
		// provide room volume as a model result
		m_results[R_Volume].value = volume;
	}
	// otherwise area and height should be given
	else {
		if(m_zone->m_para[NANDRAD::Zone::ZP_AREA].name.empty() ||
			m_zone->m_para[NANDRAD::Zone::ZP_HEIGHT].name.empty()) {
				throw IBK::Exception(IBK::FormatString("Error initializing RoomStatesModel of zone with ID %1: "
				"Expected parameter 'Volume' or parameters 'Height' and 'Area' inside Zone element")
				.arg(id()),
				FUNC_ID);
		}
		const double area	= m_zone->m_para[NANDRAD::Zone::ZP_AREA].value;
		const double height = m_zone->m_para[NANDRAD::Zone::ZP_HEIGHT].value;
		if (area < 0.0) {
			throw IBK::Exception(IBK::FormatString("Error initializing RoomStatesModel of zone with ID %1: "
			"Parameter 'Area'is smaller than 0.")
			.arg(id()),
			FUNC_ID);
		}
		if (height < 0.0) {
			throw IBK::Exception(IBK::FormatString("Error initializing RoomStatesModel of zone with ID %1: "
			"Parameter 'Area' is smaller than 0.")
			.arg(id()),
			FUNC_ID);
		}
		// provide room volume as a model result
		m_results[R_Volume].value	= area * height;
	}
	// Check for additional heat capacity definition but do not store the value.
	// We will get access using a constant input reference to the project parameter later.
	double CMass = 0;
	if (!m_zone->m_para[NANDRAD::Zone::ZP_HEATCAPACITY].name.empty()) {
		CMass = m_zone->m_para[NANDRAD::Zone::ZP_HEATCAPACITY].value;
	}
	else if(!m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_HEATCAPACITY].name.empty() &&
			m_parametrizationDefaults->m_mode == NANDRAD::ParametrizationDefaults::SM_LAZY) {
				CMass = m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_HEATCAPACITY].value;
		IBK::IBK_Message( IBK::FormatString("No extra heat capacity is given for zone with id %1. "
											"Retreiving heat capacity of %2 J/K from default parameter settings")
											.arg(id())
											.arg(m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_HEATCAPACITY].get_value("J/K")),
											IBK::MSG_WARNING, FUNC_ID, 2);
	}
	if (CMass < 0.0) {
		throw IBK::Exception(IBK::FormatString("Error initializing RoomStatesModel of zone with ID %1: "
		"Parameter 'CMass'is smaller than 0.")
		.arg(id()),
		FUNC_ID);
	}

	for (unsigned int i = 0; i < models.size(); ++i)
	{
		const FluidModel* fluid = dynamic_cast<const FluidModel*>(models[i]);
		// skip models that are not fluids
		if (fluid == NULL)
			continue;

		// skip fluids with unsuitable id
		if (fluid->fluidType() != FluidModel::FT_AIR)
			continue;
		// basic fluid model (id = 0) was copied or substituted already
		if (m_fluid != NULL && fluid->id() == 0)
			continue;

		// duplicate definition
		if (m_fluid != NULL && m_fluid->id() != 0 /* && fluid->id() != 0*/) {
			throw IBK::Exception(IBK::FormatString("Duplicate definition of air model with id #%1 and #%2!")
				.arg(m_fluid->id()).arg(fluid->id()), FUNC_ID);
		}
		// store fluid model
		m_fluid = fluid;
	}
	// error: missing fluid model
	IBK_ASSERT(m_fluid != NULL);

	// in the case of explicitely defined fluid model check consistency
	if (!m_zone->m_intpara[NANDRAD::Zone::ZI_AIRMATERIALREFERENCE].name.empty()) {
		unsigned int fluidID = m_zone->m_intpara[NANDRAD::Zone::ZI_AIRMATERIALREFERENCE].value;
		if (fluidID != m_fluid->id()) {
			throw IBK::Exception(IBK::FormatString("Error initailizing RoomStatesModel with id %1: "
				"Inconsistant reference to FluidModel with id #%2 in 'FluidReference' tag! "
				"Expected id #%3!")
				.arg(id()).arg(fluidID).arg(m_fluid->id()), FUNC_ID);
		}
	}

	// Check for initial temperature definition and store the value.
	// As we could retrieve from different sources inside the project file an input 
	// reference is not practicable.
	m_TInitial = 0.0;
	// i.e. we could retrieve initial temperature from zone definition block...
	if (!m_zone->m_para[NANDRAD::Zone::ZP_INITIAL_TEMPERATURE].name.empty()) {
		m_TInitial = m_zone->m_para[NANDRAD::Zone::ZP_INITIAL_TEMPERATURE].value;
	}
	// ...or from simulation parameter block
	else if(!m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_TEMPERATURE].name.empty() &&
			m_parametrizationDefaults->m_mode == NANDRAD::ParametrizationDefaults::SM_LAZY) {
			m_TInitial = m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_TEMPERATURE].value;
		IBK::IBK_Message( IBK::FormatString("No initial temperature is given for zone with id %1. "
											"Retreiving initial temperature of %2 C from default parameter settings")
											.arg(id())
											.arg(m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_TEMPERATURE].get_value("C")),
											IBK::MSG_WARNING, FUNC_ID, 2);
	}
	// Initial temperature must be defined anyhow.
	else {
		throw IBK::Exception(IBK::FormatString("Error initializing RoomStatesModel of zone with ID %1: "
		"Parameter 'Temperature' is not defined.")
		.arg(id()), FUNC_ID);
	}
	if (m_TInitial < 0.0) {
		throw IBK::Exception(IBK::FormatString("Error initializing RoomStatesModel of zone with ID %1: "
		"Parameter 'Temperature'is smaller than 0.")
		.arg(id()),
		FUNC_ID);
	}
	// store initial temperature
	m_results[R_AirTemperature].value = m_TInitial;
}

void RoomStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// fill result definitions from keyword list
	DefaultModel::resultDescriptions(resDesc);

	// set constraint for room  air temperature
	std::vector<QuantityDescription>::iterator tempIt = std::find_if(resDesc.begin(), resDesc.end(),
		FindQuantityDescriptionByName(KeywordList::Keyword("RoomStatesModel::Results", R_AirTemperature)));
	// room air temperature >= -100°C
	tempIt->m_minMaxValue = std::make_pair(233.15, std::numeric_limits<double>::max());

	// mark result description to the zone volume as constant and set constarints
	std::vector<QuantityDescription>::iterator volumeIt = std::find_if(resDesc.begin(), resDesc.end(), 
		FindQuantityDescriptionByName(KeywordList::Keyword("RoomStatesModel::Results", R_Volume) ) );
	IBK_ASSERT(volumeIt != resDesc.end() );
	// volume > 0
	volumeIt->m_minMaxValue = std::make_pair(1e-07, std::numeric_limits<double>::max());

	// add zone specific parameters as results
	QuantityDescription heatCapResult;
	heatCapResult.m_name = KeywordList::Keyword("RoomStatesModel::InputReferences", InputRef_HeatCapacityAdditionalMass);
	heatCapResult.m_description = KeywordList::Description("RoomStatesModel::InputReferences", InputRef_HeatCapacityAdditionalMass);
	heatCapResult.m_unit = KeywordList::Unit("RoomStatesModel::InputReferences", InputRef_HeatCapacityAdditionalMass);
	// additional heat capacaity >= 0
	heatCapResult.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
	heatCapResult.m_constant = true;
	resDesc.push_back(heatCapResult);

	// add predefined zone parameters
	for(unsigned int i = 0; i < NANDRAD::Zone::NUM_ZP; ++i)
	{
		if(m_zone->m_para[i].name.empty())
			continue;
		// skip already prepared quantities
		if(m_zone->m_para[i].name ==  
			KeywordList::Keyword("RoomStatesModel::InputReferences", InputRef_HeatCapacityAdditionalMass))
			continue;
		if (m_zone->m_para[i].name ==
			KeywordList::Keyword("RoomStatesModel::Results", R_Volume))
			continue;
	
		std::vector<QuantityDescription>::iterator paraIt = std::find_if(resDesc.begin(), resDesc.end(),
			FindQuantityDescriptionByName(m_zone->m_para[i].name));
		// already defined
		if (paraIt != resDesc.end())
			continue;
		
		// predfined parameters are constants
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD::KeywordList::Description("Zone::para_t",i);
		result.m_name  = m_zone->m_para[i].name;
		result.m_unit = m_zone->m_para[i].unit().name();
		resDesc.push_back(result);
	}
	// add generic zone specific parameters from project file as a referencable result
	std::map<std::string, IBK::Parameter>::const_iterator genericParaIt
		= m_zone->m_genericParaConst.begin();
	for( ; genericParaIt != m_zone->m_genericParaConst.end(); ++genericParaIt)
	{
		// in gereral parameters are defined as constants
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = std::string();
		result.m_name  = genericParaIt->second.name;
		result.m_unit = genericParaIt->second.unit().name();
		resDesc.push_back(result);
	}
	// add space type specific parameters from project file as a referencable result
	genericParaIt = m_zone->m_spaceTypeRef->m_genericParaConst.begin();
	for( ; genericParaIt != m_zone->m_spaceTypeRef->m_genericParaConst.end(); ++genericParaIt)
	{
		// in gereral parameters are defined as constants
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = std::string();
		result.m_name  = genericParaIt->second.name;
		result.m_unit = genericParaIt->second.unit().name();
		resDesc.push_back(result);
	}

}

const double * RoomStatesModel::resultValueRef(const QuantityName & quantityName) const {
	// search inside keyword list result quantities
	const double *refValue = DefaultModel::resultValueRef(quantityName);
	if( refValue != NULL)
		return refValue;
	// only scalar quantities are allowed any longer
	if (quantityName.index() != -1)
		return NULL;

	// constant parameters are alo provided as constant references:
	// heat capacity of furniture
	if(quantityName == KeywordList::Keyword("RoomStatesModel::InputReferences",InputRef_HeatCapacityAdditionalMass) )
	{
		// parameter from zone
		if (!m_zone->m_para[NANDRAD::Zone::ZP_HEATCAPACITY].name.empty()) {
			return &m_zone->m_para[NANDRAD::Zone::ZP_HEATCAPACITY].value;
		}
		// parameter from parametrization defaults
		else if(!m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_HEATCAPACITY].name.empty() &&
				m_parametrizationDefaults->m_mode == NANDRAD::ParametrizationDefaults::SM_LAZY) {
			return &m_parametrizationDefaults->m_para[NANDRAD::ParametrizationDefaults::SP_HEATCAPACITY].value;
		}
		// local parameter
		else {
			return &m_defaultHeatCapacityAdditionalMass;
		}
	}
	// check if we request a prededined zone parameter
	if(NANDRAD::KeywordList::KeywordExists("Zone::para_t",quantityName.name())
		&& !m_zone->m_para[NANDRAD::KeywordList::Enumeration("Zone::para_t",quantityName.name())].name.empty())
	{
		return &m_zone->m_para[NANDRAD::KeywordList::Enumeration("Zone::para_t",quantityName.name())].value;
	}

	// the same with generic parameters of the current zone
	std::map<std::string, IBK::Parameter>::const_iterator genericParaIt
		= m_zone->m_genericParaConst.find(quantityName.name());
	// success: we found current parameter
	if(genericParaIt != m_zone->m_genericParaConst.end())
		return &genericParaIt->second.value;
	// the same with generic space type parameters of the current zone
	genericParaIt = m_zone->m_spaceTypeRef->m_genericParaConst.find(quantityName.name());
	// success: we found current parameter
	if(genericParaIt != m_zone->m_spaceTypeRef->m_genericParaConst.end())
		return &genericParaIt->second.value;

	return NULL;
}


void RoomStatesModel::initInputReferences(const std::vector<AbstractModel*> &  /*models*/) {

	std::string category	= "RoomStatesModel::InputReferences";

	// compose a reference for specific heat capacity that will be retrieved from the zone
	// itself
	std::string targetName = KeywordList::Keyword(category.c_str(), InputRef_HeatCapacityAdditionalMass);
	InputReference & inputRefCMass = inputReference(InputRef_HeatCapacityAdditionalMass);
	inputRefCMass.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	inputRefCMass.m_id = id();
	inputRefCMass.m_sourceName = targetName;
	inputRefCMass.m_targetName = targetName;
	inputRefCMass.m_constant = true;

	// compose a reference to the global solver solution quantity (internal energy)
	targetName						= KeywordList::Keyword(category.c_str(), InputRef_InternalEnergy);
	InputReference &yRef			= inputReference(InputRef_InternalEnergy);
	yRef.m_referenceType			= NANDRAD::ModelInputReference::MRT_ZONE;
	yRef.m_id						= id(); // solution variable is provided by the corresponding room balance model
	yRef.m_sourceName				= std::string("y");
	yRef.m_targetName				= targetName;
	yRef.m_constant					= true;
}

void RoomStatesModel::yInitial(double * y) const {

	// check if all references are filled
	IBK_ASSERT(inputValueRefs()[InputRef_HeatCapacityAdditionalMass] != NULL);

	// initial energy density is calculated from initial temperature
	const double rhoAir		= m_fluid->density(293.15, 0.0);
	const double cAir		= m_fluid->specificHeatCapacity(293.15, 0.0);
	const double CMass		= *inputValueRefs()[InputRef_HeatCapacityAdditionalMass];
	const double Volume	    = m_results[R_Volume].value;
	const double TInitial	= m_TInitial;

	// return initial value to global solver
	y[0] = TInitial * (rhoAir*cAir*Volume + CMass);

}

int RoomStatesModel::update() {

	// update procedure decomposes all solver quantities
	IBK_ASSERT(inputValueRefs()[InputRef_InternalEnergy] != NULL);
	// solver quantities are retrieved by a value reference to the
	// corresponding RoomBalanceModel
	double uR = *inputValueRefs()[InputRef_InternalEnergy];

	// check if all references are filled
	IBK_ASSERT(inputValueRefs()[InputRef_HeatCapacityAdditionalMass] != NULL);

	// initial energy density is calculated from initial temperature
	const double rhoAir		= m_fluid->density(293.15, 0.0);
	const double cAir		= m_fluid->specificHeatCapacity(293.15, 0.0);
	const double CMass		= *inputValueRefs()[InputRef_HeatCapacityAdditionalMass];
	const double Volume	    = m_results[R_Volume].value;

	// calculate room air temperature
	double TRoom = uR/(rhoAir*cAir*Volume + CMass);

	m_results[R_AirTemperature].value = TRoom;
	// signal success
	return 0;
}


unsigned int RoomStatesModel::FMU2Interfaces() const {

	const NANDRAD::SpaceType *spaceTypeRef = zone()->m_spaceTypeRef;

	unsigned int ifaces = 0;
	IBK_ASSERT(spaceTypeRef != NULL);
	std::string category = "FMU2QuantityDescription::FMUInterfaceDefinition";
	// default: no FMU interfaces
	std::set<FMU2QuantityDescription::FMUInterfaceDefinition> interfaceDefs;
	std::vector<std::string>								  interfaceTokens;
	// we provide an explicit defition
	std::map<std::string, std::string>::const_iterator fIt =
		spaceTypeRef->m_genericParaString.find("FMUInterfaceDefinition");

	// interface definition provides different values sepearted by ','
	if (fIt != spaceTypeRef->m_genericParaString.end()) {
		IBK::explode(fIt->second, interfaceTokens, ',');
		// now extract all definitions

		for (unsigned int j = 0; j < interfaceTokens.size(); ++j) {
			FMU2QuantityDescription::FMUInterfaceDefinition interfaceDef =
				FMU2QuantityDescription::ID_None;
			// only use trimmed string value
			IBK::trim(interfaceTokens[j]);
			// extract intergface definition
			interfaceDef = (FMU2QuantityDescription::FMUInterfaceDefinition)
				KeywordList::Enumeration(category.c_str(), interfaceTokens[j]);
			// skip interface none
			if (interfaceDef == FMU2QuantityDescription::ID_None ||
				interfaceDef == FMU2QuantityDescription::ID_ElectricUsageScenario)
				continue;
			// store definition
			ifaces |= interfaceDef;
		}
	}
	return ifaces;
}


void RoomStatesModel::FMU2ExportReference(const QuantityName &targetName,
	unsigned int &sourceID, int &modelType,
	QuantityName &quantity, bool &constant) const {

	std::string categoryTarget = "FMU2ExportModel::InputReferences";
	// extract quantity
	IBK_ASSERT(KeywordList::KeywordExists(categoryTarget.c_str(), targetName.name()));

	// translate target quantity name into a known enumeration value
	FMU2ExportModel::InputReferences targetQuantity = (FMU2ExportModel::InputReferences)
			KeywordList::Enumeration(categoryTarget.c_str(), targetName.name());

	quantity.clear();
	// decide which quantity to use
	switch (targetQuantity) {
	case FMU2ExportModel::InputRef_ZoneMeanAirTemperature:
		quantity = KeywordList::Keyword("RoomStatesModel::Results", 
			R_AirTemperature);
	break;
	// retrieve radiant temperature from thermal comfort model
	case FMU2ExportModel::InputRef_ZoneMeanRadiantTemperature:
		quantity = KeywordList::Keyword("ThermalComfortModel::Results", 
			ThermalComfortModel::R_RadiantTemperature);
	break;
	// retrieve heating setpoint temperature from schedules
	case FMU2ExportModel::InputRef_HeatingSetpointTemperature:
		quantity = KeywordList::Keyword("Schedules::Results",
			Schedules::R_HeatingSetPointTemperature);
	break;
	// retrieve cooling setpoint temperature from schedules
	case FMU2ExportModel::InputRef_CoolingSetpointTemperature:
		quantity = KeywordList::Keyword("Schedules::Results",
			Schedules::R_CoolingSetPointTemperature);
	break;
	default: break;
	}
	// no quantity: skip
	if (quantity.empty())
		return;

	sourceID = id();
	modelType = (int) referenceType();
	constant = false;
}


} // namespace NANDRAD_MODEL

