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

#include "NM_RoomBalanceModel.h"

#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"


using namespace std;

namespace NANDRAD_MODEL {

void RoomBalanceModel::setup( const NANDRAD::SimulationParameter &simPara) {
	// copy all object pointers
	m_simPara     = &simPara;

	// results depend on calculation mode
	m_moistureBalanceEnabled = simPara.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled();
	if (m_moistureBalanceEnabled) {
		m_results.resize(NUM_R);
		// resize ydot vector - two balance equations
		m_ydot.resize(2);
	}
	else {
		// resize results vector
		m_results.resize(1);

		// resize ydot vector - one balance equation
		m_ydot.resize(1);
	}
}


void RoomBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	int varCount = 1; // room air temperature is always the first result
	if (m_simPara->m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {
		varCount = NUM_R; // more variables for hygrothermal calculation
	}

	/// \todo what about CO2 ???

	for (int i=0; i<varCount; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("RoomBalanceModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("RoomBalanceModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("RoomBalanceModel::Results", i);

		resDesc.push_back(result);
	}
}


void RoomBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	// first seach in m_results vector
	res.clear();
	// fill with all results and vector valued results

	for (unsigned int i = 0; i < m_results.size(); ++i) {
		res.push_back(&m_results[i]);
	}
}


const double * RoomBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "RoomBalanceModel::Results";

	if (KeywordList::CategoryExists(category) && KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}
	else
		return nullptr;
}


int RoomBalanceModel::priorityOfModelEvaluation() const {
	// room balance model is evaluated one step before outputs
	return AbstractStateDependency::priorityOffsetTail+5;
}


void RoomBalanceModel::initInputReferences(const std::vector<AbstractModel *> & /*models*/) {
	/// \todo
}


void RoomBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {

}

void RoomBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	m_valueRefs = resultValueRefs;



}




int RoomBalanceModel::update() {

	double SumQdot = 0.0;
#if 0

	// retrieve heat conduction fluxes from all walls (and revert the direction of loss)
	const double QdotHeatCondWall = *inputValueRefs()[InputRef_WallsHeatConductionLoad];
	// add heat conduction flux to room balance
	SumQdot += QdotHeatCondWall;
	// retrieve radiation fluxes from windows
	const double gammaRad = *inputValueRefs()[InputRef_RadiationLoadFraction];
	const double QdotSWRad = gammaRad * (*inputValueRefs()[InputRef_WindowsSWRadLoad]);
	// add radiation loads
	SumQdot += QdotSWRad;
	// retrieve heat transfer fluxes from all embedded objects (and revert the flux direction)
	const double QdotHeatTransmission = *inputValueRefs()[InputRef_WindowsHeatTransmissionLoad];
	// add heat transfer losses
	SumQdot += QdotHeatTransmission;
	// retrieve long wave radiation balance fluxes at all windows (ignored by the windows surface equation)
	const double QdotLWRadBalance = *inputValueRefs()[InputRef_LWRadBalanceLoad];
	// add long wave radiation balance fluxes
	SumQdot += QdotLWRadBalance;
	// retrieve short wave radiation balance fluxes at all windows (ignored by the windows surface equation)
	const double QdotSWRadBalance = *inputValueRefs()[InputRef_SWRadBalanceLoad];
	// add short wave radiation balance fluxes
	SumQdot += QdotSWRadBalance;
	// retrieve convective heat fluxes from all heatings
	const double QdotConvHeating = *inputValueRefs()[InputRef_ConvectiveHeatingsLoad];
	// add heating loads
	SumQdot += QdotConvHeating;
	// retrieve losses from cooling (and revert the flux direction)
	const double QdotConvCooling = *inputValueRefs()[InputRef_ConvectiveCoolingsLoad];
	// add cooling losses
	SumQdot += QdotConvCooling;

	double OccupancyLoads = 0.0;
	// retrieve heat gains from users
	OccupancyLoads += *inputValueRefs()[InputRef_ConvectiveUsersLoad];
	// retrieve heat gains from electric equipment
	OccupancyLoads += *inputValueRefs()[InputRef_ConvectiveEquipmentLoad];
	// retrieve heat gains from light
	OccupancyLoads += *inputValueRefs()[InputRef_ConvectiveLightingLoad];

	// add heat gains
	SumQdot += OccupancyLoads;
	// retrieve heat load from natural ventilation (and revert the flux direction)
	const double QdotNaturalVentilation = *inputValueRefs()[InputRef_UserVentilationThermalLoad];
	// add heat gains
	SumQdot += QdotNaturalVentilation;
	// retrieve heat load from infiltration (and revert the flux direction)
	const double QdotInfiltration = *inputValueRefs()[InputRef_InfiltrationThermalLoad];
	// add heat gains
	SumQdot += QdotInfiltration;
	// retrieve heat load from air condictioning (and revert the flux direction)
	const double QdotAirCondition = *inputValueRefs()[InputRef_AirConditionThermalLoad];
	// add heat gains
	SumQdot += QdotAirCondition;

	// retrieve heat gains from domestic water consumption
	const double QdotDomesticWaterSensitiveGain = *inputValueRefs()[InputRef_DomesticWaterConsumptionSensitiveHeatGain];
	// add heat gains
	SumQdot += QdotDomesticWaterSensitiveGain;
#endif

	// store the sum of all loads
	m_results[R_CompleteThermalLoad] = SumQdot;
	// solve the balance: ydot = sum loads
	m_ydot[0] = m_results[R_CompleteThermalLoad];
	// signal success
	return 0;
}


int RoomBalanceModel::ydot(double* ydot) {
	// copy values to ydot
	ydot[0] = m_ydot[0];
	if (m_ydot.size() > 1) {
		ydot[1] = m_ydot[1];
	}
	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL

