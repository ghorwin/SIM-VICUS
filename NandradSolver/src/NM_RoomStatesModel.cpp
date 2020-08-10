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

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Zone.h>

//#include "NM_Constants.h"
//#include "NM_ConstructionStatesModel.h"
//#include "NM_EquipmentLoadModel.h"
//#include "NM_FluidModel.h"
//#include "NM_LightingLoadModel.h"

#include "NM_KeywordList.h"
#include "NM_Schedules.h"
#include "NM_ThermalComfortModel.h"
#include "NM_InputReference.h"


namespace NANDRAD_MODEL {


void RoomStatesModel::setup(const NANDRAD::Zone & zone) {
	FUNCID(RoomStatesModel::setup);

	// Only initialization of zone with matching ID allowed
	IBK_ASSERT(zone.m_id != id());

	// check for required parameters
	if (zone.m_para[NANDRAD::Zone::ZP_VOLUME].name.empty())
		throw IBK::Exception(IBK::FormatString("Missing parameter 'Volume' in zone #%1 '%2'")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);

	// check for valid parameters
	if (zone.m_para[NANDRAD::Zone::ZP_VOLUME].value <= 0)
		throw IBK::Exception(IBK::FormatString("'Volume' in zone #%1 '%2' must be > 0!")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
	if (!zone.m_para[NANDRAD::Zone::ZP_HEATCAPACITY].name.empty() &&
		zone.m_para[NANDRAD::Zone::ZP_HEATCAPACITY].value <= 0)
	{
		throw IBK::Exception(IBK::FormatString("'HeatCapacity' in zone #%1 '%2' must be > 0!")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
	}

	// set a pointer to the NANDRAD parametrization objects
	m_zone					  = &zone;
}


void RoomStatesModel::initResults(const std::vector<AbstractModel*> & models) {
//	FUNCID(RoomStatesModel::initResults);

	// resize m_results from keyword list
	DefaultModel::initResults(models);

#if 0
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
#endif
}


void RoomStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// fill result definitions from keyword list
	DefaultModel::resultDescriptions(resDesc);

#if 0
	// set constraint for room  air temperature
	std::vector<QuantityDescription>::iterator tempIt = std::find_if(resDesc.begin(), resDesc.end(),
		FindQuantityDescriptionByName(KeywordList::Keyword("RoomStatesModel::Results", R_AirTemperature)));
	// room air temperature >= -100°C
	tempIt->m_minMaxValue = std::make_pair(233.15, std::numeric_limits<double>::max());

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
#endif
}

const double * RoomStatesModel::resultValueRef(const QuantityName & quantityName) const {
#if 0
	// search inside keyword list result quantities
	const double *refValue = DefaultModel::resultValueRef(quantityName);
	if( refValue != NULL)
		return refValue;
	// only scalar quantities are allowed any longer
	if (quantityName.index() != -1)
		return NULL;

	// check if we request a prededined zone parameter
	if(NANDRAD::KeywordList::KeywordExists("Zone::para_t",quantityName.name())
		&& !m_zone->m_para[NANDRAD::KeywordList::Enumeration("Zone::para_t",quantityName.name())].name.empty())
	{
		return &m_zone->m_para[NANDRAD::KeywordList::Enumeration("Zone::para_t",quantityName.name())].value;
	}
#endif
	return NULL;
}


void RoomStatesModel::yInitial(double * y) const {
#if 0
	// initial energy density is calculated from initial temperature
	const double rhoAir		= m_fluid->density(293.15, 0.0);
	const double cAir		= m_fluid->specificHeatCapacity(293.15, 0.0);
	const double CMass		= *inputValueRefs()[InputRef_HeatCapacityAdditionalMass];
	const double Volume	    = m_results[R_Volume].value;
	const double TInitial	= m_zone->m_para[];

	// return initial value to global solver
	y[0] = TInitial * (rhoAir*cAir*Volume + CMass);
#endif
}

int RoomStatesModel::update() {
#if 0
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
#endif
	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL

