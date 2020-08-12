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

#include "NM_RoomBalanceModel.h"
//#include "NM_Constants.h"
#include "NM_KeywordList.h"

#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_SimulationParameter.h>

using namespace std;

namespace NANDRAD_MODEL {

#if 0

int RoomBalanceModel::priorityOfModelEvaluation() const {
	// We have the following model dependency sequence at the end of evaluation stack:
	// ...
	// HeatingsLoadModel	  +		WindowModel	      +    LightingLoadModel 	  (at priorityOffsetTail)
	//	^	^						^						^
	//	|	|						|						|
	//InsideBCHeatingsLoadModel + WindowsLoadModel + InsideBCLightingsLoadModel
	//	|	^				^		^						  ^
	//	|	|				|		|						  |
	//	InsideBCLWRadExchangeModel + InsideBCWindowsLoadModel |
	//	|   |				^		 ^						  |
	//	|	|				|		 |						  |
	//	|   |  LWRadBalanceLoadModel + InsideBCSWRadExchangeModel
	//	|	|			    |	^	 ^	 					^
	//	|	|			    |	|	 |	  					|
	//	|    ConstructionInsideBCModel	  +			SWRadBalanceLoadModel
	//	|				^		|		  				    ^
	//	|				|		|		   					|
	//	|		ConstructionSolverModel 					|
	//	|				|		|		   					|
	//	|		WallsThermalLoadModel	  					|
	//	|				^		|		  					|
	//	|				|		|		  					|
	// ********** RoomBalanceModel **********************************************
	return AbstractStateDependency::priorityOffsetTail + 7;
}

void RoomBalanceModel::setup( const NANDRAD::SimulationParameter &simPara) {
	// copy all object pointers
	m_simulationParameter     = &simPara;

	// resize solution variable
	m_y.resize(nPrimaryStateResults());
	// resize ydot vector
	m_ydot.resize(nPrimaryStateResults());
}

void RoomBalanceModel::initResults(const std::vector<AbstractModel*> & models) {
	const char* const FUNC_ID = "[RoomBalanceModel::initResults]";
	// resize m_results from keyword list
	DefaultModel::initResults(models);

	const IBK::Parameter &RadiationLoadFraction = m_simulationParameter->m_para[NANDRAD::SimulationParameter::SP_RADIATION_LOAD_FRACTION];
	// retrieve radiation load fraction from simulation parameter
	if(RadiationLoadFraction.name.empty() )
	{
		throw IBK::Exception( IBK::FormatString(" Error initializing RoomBlanaceModel for Zone with id %1: "
			  "Simulation parameter 'RadiationLoadFraction' is undefined!")
			  .arg(id()) , FUNC_ID);
	}
	// check validity of parameter definition
	const double radLoadFrac = RadiationLoadFraction.value;
	if(radLoadFrac < 0.0 || radLoadFrac > 1.0)
		throw IBK::Exception( IBK::FormatString(" Error initializing RoomBlanaceModel for Zone with id %1: "
			  "Simulation parameter 'RadiationLoadFraction' is outside the interval [0,1]!")
			  .arg(id()) , FUNC_ID);
}

void RoomBalanceModel::initInputReferences(const std::vector<AbstractModel*> & /*models*/) {
	// prepare container seaching
	std::string category = ModelIDName() + std::string("::InputReferences");
	std::string sourceName;

	// find input reference for RadiationLoadFraction
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_RadiationLoadFraction);
	// create input references for RadiationLoadFraction
	InputReference & radLoadFracRef = inputReference(InputRef_RadiationLoadFraction);
	radLoadFracRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	radLoadFracRef.m_id				= id();
	radLoadFracRef.m_sourceName		= sourceName;
	radLoadFracRef.m_targetName		= sourceName;
	// we have a constant reference
	radLoadFracRef.m_constant		= true;

	// find input reference for heat condutcion loads model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_WallsHeatConductionLoad);
	// create input references for heat condutcion loads model
	InputReference & heatCondRef	= inputReference(InputRef_WallsHeatConductionLoad);
	heatCondRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	heatCondRef.m_id				= id(); // only choose loads with reference to current zone
	heatCondRef.m_sourceName		= sourceName;
	heatCondRef.m_targetName		= sourceName;

	// find input reference for long wave radiation load model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_LWRadBalanceLoad);
	// create input references for radiation loads model
	InputReference & lwradRef		= inputReference(InputRef_LWRadBalanceLoad);
	lwradRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	lwradRef.m_id					= id(); // only choose loads with reference to current zone
	lwradRef.m_sourceName			= sourceName;
	lwradRef.m_targetName			= sourceName;

	// find input reference for short wave radiation load model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_SWRadBalanceLoad);
	// create input references for radiation loads model
	InputReference & swradRef		= inputReference(InputRef_SWRadBalanceLoad);
	swradRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	swradRef.m_id					= id(); // only choose loads with reference to current zone
	swradRef.m_sourceName			= sourceName;
	swradRef.m_targetName			= sourceName;

	// find input reference for window short wave radiation load model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_WindowsSWRadLoad);
	// create input references for radiation loads model
	InputReference & swRadRef		= inputReference(InputRef_WindowsSWRadLoad);
	swRadRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	swRadRef.m_id					= id(); // only choose loads with reference to current zone
	swRadRef.m_sourceName			= sourceName;
	swRadRef.m_targetName			= sourceName;

	// find input reference for heat transmission loads model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_WindowsHeatTransmissionLoad);
	// create input references for heat transmission loads model
	InputReference & heatTransRef	= inputReference(InputRef_WindowsHeatTransmissionLoad);
	heatTransRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	heatTransRef.m_id				= id(); // only choose loads with reference to current zone
	heatTransRef.m_sourceName		= sourceName;
	heatTransRef.m_targetName		= sourceName;

	// find input reference for heatings load model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_ConvectiveHeatingsLoad);
	// create input references for heatings load model
	InputReference & heatingRef		= inputReference(InputRef_ConvectiveHeatingsLoad);
	heatingRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	heatingRef.m_id					= id(); // only choose loads with reference to current zone
	heatingRef.m_sourceName			= sourceName;
	heatingRef.m_targetName			= sourceName;

	// find input reference for coolings gains model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_ConvectiveCoolingsLoad);
	// create input references for coolings gains model
	InputReference & coolingRef		= inputReference(InputRef_ConvectiveCoolingsLoad);
	coolingRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	coolingRef.m_id					= id(); // only choose loads with reference to current zone
	coolingRef.m_sourceName			= sourceName;
	coolingRef.m_targetName			= sourceName;

	// find input reference for user loads model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_ConvectiveUsersLoad);
	// create input references for user gains model
	InputReference & userRef		= inputReference(InputRef_ConvectiveUsersLoad);
	userRef.m_referenceType			= NANDRAD::ModelInputReference::MRT_ZONE;
	userRef.m_id					= id(); // only choose loads with reference to current zone
	userRef.m_sourceName			= sourceName;
	userRef.m_targetName			= sourceName;

	// find input reference for equipment loads model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_ConvectiveEquipmentLoad);
	// create input references for equipment gains model
	InputReference & eqipmentRef	= inputReference(InputRef_ConvectiveEquipmentLoad);
	eqipmentRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	eqipmentRef.m_id				= id(); // only choose loads with reference to current zone
	eqipmentRef.m_sourceName		= sourceName;
	eqipmentRef.m_targetName		= sourceName;

	// find input reference for light loads model
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_ConvectiveLightingLoad);
	// create input references for light gains model
	InputReference & lightRef	= inputReference(InputRef_ConvectiveLightingLoad);
	lightRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	lightRef.m_id				= id(); // only choose loads with reference to current zone
	lightRef.m_sourceName		= sourceName;
	lightRef.m_targetName		= sourceName;

	// find input reference for natural ventilation losses
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_UserVentilationThermalLoad);
	// create input references for light gains model
	InputReference & natVentRef	= inputReference(InputRef_UserVentilationThermalLoad);
	natVentRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	natVentRef.m_id				= id(); // only choose loads with reference to current zone
	natVentRef.m_sourceName		= sourceName;
	natVentRef.m_targetName		= sourceName;

	// find input reference for infiltration losses
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_InfiltrationThermalLoad);
	// create input references for light gains model
	InputReference & infiltRef	= inputReference(InputRef_InfiltrationThermalLoad);
	infiltRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	infiltRef.m_id				= id(); // only choose loads with reference to current zone
	infiltRef.m_sourceName		= sourceName;
	infiltRef.m_targetName		= sourceName;

	// find input reference for air condition losses
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_AirConditionThermalLoad);
	// create input references for light gains model
	InputReference & airCondRef = inputReference(InputRef_AirConditionThermalLoad);
	airCondRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	airCondRef.m_id = id(); // only choose loads with reference to current zone
	airCondRef.m_sourceName = sourceName;
	airCondRef.m_targetName = sourceName;

	// find input reference for domestic water consumption heat gains
	sourceName = KeywordList::Keyword(category.c_str(), InputRef_DomesticWaterConsumptionSensitiveHeatGain);
	// create input references for light gains model
	InputReference & domWaterRef = inputReference(InputRef_DomesticWaterConsumptionSensitiveHeatGain);
	domWaterRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	domWaterRef.m_id = id(); // only choose loads with reference to current zone
	domWaterRef.m_sourceName = sourceName;
	domWaterRef.m_targetName = sourceName;
}


void RoomBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	// first seach in m_results vector
	DefaultModel::resultValueRefs(res);

	// Additionally we provide a reference to the solution quantity.
	// This reference will be accessed by the corresponding RoomStatesModel.
	res.push_back(&m_y[0]);

	// And we provide a reference to the divergences.
	res.push_back(&m_ydot[0]);
}


const double * RoomBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// first seach in m_results vector
	const double *refValue = DefaultModel::resultValueRef(quantityName);
	if( refValue != NULL)
		return refValue;
	// only scalar quantities are allowed any longer
	if (quantityName.index() != -1)
		return NULL;

	// now check constant parameters (we provide project parameters as input references
	// for the access by other models).
	// Here: radiation load fraction
	if(quantityName== KeywordList::Keyword("RoomBalanceModel::InputReferences",InputRef_RadiationLoadFraction) )
	{
		return &m_simulationParameter->m_para[NANDRAD::SimulationParameter::SP_RADIATION_LOAD_FRACTION].value;
	}
	// Additionally we provide a reference to the solution quantity.
	// This reference will be accessed by the corresponding RoomStatesModel.
	if(quantityName == std::string("y") )
	{
		return &m_y[0];
	}
	// And we provide a reference to the divergences.
	if(quantityName == std::string("ydot") )
	{
		return &m_ydot[0];
	}
	return NULL;
}


void RoomBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// fill definitions from keyword list
	DefaultModel::resultDescriptions(resDesc);
	// add descriptions for all project parameters
	QuantityDescription result;
	result.m_name = KeywordList::Keyword("RoomBalanceModel::InputReferences", InputRef_RadiationLoadFraction);
	result.m_description = KeywordList::Description("RoomBalanceModel::InputReferences", InputRef_RadiationLoadFraction);
	result.m_unit = KeywordList::Unit("RoomBalanceModel::InputReferences", InputRef_RadiationLoadFraction);
	result.m_constant = true;
	resDesc.push_back(result);
	// offer a reference to the solution quantity
	result.m_name = std::string("y");
	result.m_description = std::string("States");
	result.m_unit = std::string("---");
	result.m_constant = true;
	resDesc.push_back(result);
	// offer a reference to the divergences
	result.m_name = std::string("ydot");
	result.m_description = std::string("Divergences");
	result.m_unit = std::string("---");
	resDesc.push_back(result);
}

#if 0
void RoomBalanceModel::stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const
{
	// clear pattern
	if(!resultInputValueReferences.empty() )
		resultInputValueReferences.clear();
	// only connect to ydot and ignore complete loads
	const double *valueRef = &m_ydot[0];
	IBK_ASSERT(valueRef != NULL);
	// now we add a dependency between each result value and each input value
	for(unsigned int i = 0; i < inputValueRefs().size(); ++i)
	{
		// skip references to RadiationLoadFraction
		if( i == (unsigned int) InputRef_RadiationLoadFraction)
			continue;
		// retrieve value pointer
		const double *inputValue = inputValueRefs()[i];
		IBK_ASSERT(inputValue != NULL);
		// store the pair of adresses of result value and input
		// value inside pattern list (the pattern is dense)
		resultInputValueReferences.push_back(
			std::make_pair(valueRef, inputValue) );
	}
}
#endif

int RoomBalanceModel::update() {

	// check validity of the definition
	IBK_ASSERT(inputValueRefs()[InputRef_WallsHeatConductionLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_RadiationLoadFraction] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_WindowsSWRadLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_WindowsHeatTransmissionLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_LWRadBalanceLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_SWRadBalanceLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_ConvectiveHeatingsLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_ConvectiveCoolingsLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_ConvectiveUsersLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_ConvectiveEquipmentLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_ConvectiveLightingLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_UserVentilationThermalLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_InfiltrationThermalLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_AirConditionThermalLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_DomesticWaterConsumptionSensitiveHeatGain] != NULL);

	double SumQdot = 0.0;
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

	// store the sum of all loads
	m_results[R_CompleteThermalLoad].value = SumQdot;
	// signal success
	return 0;
}
#endif

void RoomBalanceModel::setY(const double * y) {
	m_y[0] = y[0];
}

int RoomBalanceModel::ydot(double* ydot) {
	// solve the balance: ydot = sum loads
	m_ydot[0] = m_results[R_CompleteThermalLoad];
	// and return ydot
	std::memcpy(ydot, &m_ydot[0], m_ydot.size()*sizeof(double) );
	// ydot is called at the end of an update cycle
	// an non-linear iteration calls:
	// - setTime()
	// - setY()
	// - update()
	// - ydot()
	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL

