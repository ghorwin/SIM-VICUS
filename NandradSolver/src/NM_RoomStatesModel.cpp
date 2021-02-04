/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

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

#include <NANDRAD_Zone.h>
#include <NANDRAD_SimulationParameter.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {

void RoomStatesModel::setup(const NANDRAD::Zone & zone, const NANDRAD::SimulationParameter & simPara) {
	FUNCID(RoomStatesModel::setup);

	// Only initialization of zone with matching ID allowed
	IBK_ASSERT(zone.m_id == id());

	// store pointers to the NANDRAD parametrization objects
	m_zone		= &zone;
	m_simPara	= &simPara;

	// check for required parameters

	// m_volume = checkParameter(zone.m_para[NANDRAD::Zone::ZP_VOLUME], "m3", 0, false, std::numeric_limits<double>::max(), false);

	if (zone.m_para[NANDRAD::Zone::P_Volume].name.empty())
		throw IBK::Exception(IBK::FormatString("Missing parameter 'Volume' in zone #%1 '%2'")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);

	// check for valid parameters
	m_volume = zone.m_para[NANDRAD::Zone::P_Volume].get_value("m3");
	if (m_volume <= 0)
		throw IBK::Exception(IBK::FormatString("'Volume' in zone #%1 '%2' must be > 0!")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);

	if (!zone.m_para[NANDRAD::Zone::P_HeatCapacity].name.empty() &&
		zone.m_para[NANDRAD::Zone::P_HeatCapacity].value <= 0)
	{
		throw IBK::Exception(IBK::FormatString("'HeatCapacity' in zone #%1 '%2' must be > 0!")
							 .arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
	}
	m_additionalHeatCapacity = zone.m_para[NANDRAD::Zone::P_HeatCapacity].value;

	// resize memory cache for results
	// results depend on calculation mode
	m_moistureBalanceEnabled = simPara.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled();
	if (m_moistureBalanceEnabled) {
		m_y.resize(2); // two conserved states
		m_results.resize(NUM_R);
	}
	else {
		m_results.resize(1);

		// warn if temperature parameter is given in active zone
		if (!zone.m_para[NANDRAD::Zone::P_Temperature].name.empty())
			IBK::IBK_Message("Temperature parameter in active zone ignored. Using global default initial temperature "
							 "from simulation parameters.", IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);

		// cache initial condition
		m_results[0] = simPara.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].value;
		m_y.resize(1); // one state
	}
}


void RoomStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	int varCount = 1; // room air temperature is always the first result
	if (m_moistureBalanceEnabled) {
		varCount = NUM_R; // more variables for hygrothermal calculation
	}

	/// \todo what about CO2 ???

	for (int i=0; i<varCount; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("RoomStatesModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("RoomStatesModel::Results", i);
		result.m_displayName = m_displayName;
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("RoomStatesModel::Results", i);

		resDesc.push_back(result);
	}
}


const double * RoomStatesModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "RoomStatesModel::Results";

	if (quantityName.m_name == "y") {
		return &m_y[0];
	}
	else if (KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}
	else {
		return nullptr;
	}
}


unsigned int RoomStatesModel::nPrimaryStateResults() const {
	if (m_moistureBalanceEnabled)
		return 2;
	else
		return 1;
}


void RoomStatesModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal implementation
	}
	else {
		// for each computed quantity indicate which variables are needed for computation

		// temperature depends on energy density state
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_AirTemperature], &m_y[0]));
	}
}


void RoomStatesModel::yInitial(double * y) const {
	// different implementations based on thermal or hygrothermal calculation
	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal implementation
	}
	else {

		// initial energy density is calculated from initial temperature

		/// \todo if constants should be adjustable, create parameters in SimulationParameters data section and
		/// use these here.

		const double rhoAir		= IBK::RHO_AIR;
		const double cAir		= IBK::C_AIR;
		const double TInitial	= m_results[0];

		// store initial value of conserved quantity in [J]
		y[0] = TInitial * (rhoAir*cAir*m_volume + m_additionalHeatCapacity);
	}
}


int RoomStatesModel::update(const double * y) {
	// update procedure decomposes all solver quantities

	// different implementations based on thermal or hygrothermal calculation
	if (m_moistureBalanceEnabled) {
		// cache input quantities
		/// \todo hygrothermal implementation
	}
	else {
		// cache input quantities
		m_y[0] = y[0];

		double uR = y[0]; // conserved quantity is energy density

		/// \todo if constants should be adjustable, create parameters in SimulationParameters data section and
		/// use these here.

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
