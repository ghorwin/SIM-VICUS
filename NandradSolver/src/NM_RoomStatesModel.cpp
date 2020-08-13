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
#include <IBK_physics.h>

#include "NM_RoomStatesModel.h"

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Zone.h>
#include <NANDRAD_SimulationParameter.h>

//#include "NM_Constants.h"
//#include "NM_ConstructionStatesModel.h"
//#include "NM_EquipmentLoadModel.h"
//#include "NM_FluidModel.h"
//#include "NM_LightingLoadModel.h"

#include "NM_KeywordList.h"
#include "NM_InputReference.h"


namespace NANDRAD_MODEL {

RoomStatesModel::RoomStatesModel(unsigned int id, const std::string & displayName)
	: m_id(id), m_displayName(displayName)
{
}


void RoomStatesModel::setup(const NANDRAD::Zone & zone, const NANDRAD::SimulationParameter & simPara) {
	FUNCID(RoomStatesModel::setup);

	// Only initialization of zone with matching ID allowed
	IBK_ASSERT(zone.m_id != id());

	// store pointers to the NANDRAD parametrization objects
	m_zone		= &zone;
	m_simPara	= &simPara;

	// check for required parameters
	if (zone.m_para[NANDRAD::Zone::ZP_VOLUME].name.empty())
		throw IBK::Exception(IBK::FormatString("Missing parameter 'Volume' in zone #%1 '%2'")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);

	// check for valid parameters
	m_volume = zone.m_para[NANDRAD::Zone::ZP_VOLUME].value;
	if (m_volume <= 0)
		throw IBK::Exception(IBK::FormatString("'Volume' in zone #%1 '%2' must be > 0!")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);

	if (!zone.m_para[NANDRAD::Zone::ZP_HEATCAPACITY].name.empty() &&
		zone.m_para[NANDRAD::Zone::ZP_HEATCAPACITY].value <= 0)
	{
		throw IBK::Exception(IBK::FormatString("'HeatCapacity' in zone #%1 '%2' must be > 0!")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
	}
	m_additionalHeatCapacity = zone.m_para[NANDRAD::Zone::ZP_HEATCAPACITY].value;

	// resize memory cache for results
	// results depend on calculation mode
	if (simPara.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {
		/// \todo hygrothermal implementation
	}
	else {
		m_results.resize(1);

		// cache initial condition
		m_results[0] = simPara.m_para[NANDRAD::SimulationParameter::SP_INITIAL_TEMPERATURE].value;
	}
}


void RoomStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	int varCount = 1; // room air temperature is always the first result
	if (m_simPara->m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {
		varCount = NUM_R; // more variables for hygrothermal calculation
	}

	/// \todo what about CO2 ???

	for (int i=0; i<varCount; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("RoomStatesModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("RoomStatesModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("RoomStatesModel::Results", i);

		/// \todo constraints?
		resDesc.push_back(result);
	}
}


const double * RoomStatesModel::resultValueRef(const QuantityName & quantityName) const {
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "RoomStatesModel::Results";

	if (KeywordList::CategoryExists(category) && KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}
	else
		return nullptr;
}


void RoomStatesModel::yInitial(double * y) const {
	// different implementations based on thermal or hygrothermal calculation
	if (m_simPara->m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {
		/// \todo hygrothermal implementation
	}
	else {

		// initial energy density is calculated from initial temperature

		/// \todo if constants should be adjustable, create parameters in SimulationParameters data section and
		/// uses these here.

		const double rhoAir		= IBK::RHO_AIR;
		const double cAir		= IBK::C_AIR;
		const double TInitial	= m_results[0];

		// return initial value to global solver
		y[0] = TInitial * (rhoAir*cAir*m_volume + m_additionalHeatCapacity);
	}
}


int RoomStatesModel::update(const double * y) {
	// update procedure decomposes all solver quantities

	// different implementations based on thermal or hygrothermal calculation
	if (m_simPara->m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {
		/// \todo hygrothermal implementation
	}
	else {
		double uR = y[0];

		/// \todo if constants should be adjustable, create parameters in SimulationParameters data section and
		/// uses these here.

		const double rhoAir		= IBK::RHO_AIR;
		const double cAir		= IBK::C_AIR;

		// calculate room air temperature
		double TRoom = uR/(rhoAir*cAir*m_volume + m_additionalHeatCapacity);

		m_results[R_AirTemperature] = TRoom;
	}

	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL

