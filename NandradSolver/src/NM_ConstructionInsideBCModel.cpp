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

#include "NM_ConstructionInsideBCModel.h"
#include "NM_ConstructionStatesModel.h"
#include "NM_RoomBalanceModel.h"
#include "NM_VectorValuedQuantityIndex.h"


#include "NM_KeywordList.h"

#include <IBK_assert.h>
#include <IBK_FormatString.h>
#include <WM_Types.h>

#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_ParametrizationDefaults.h>

using namespace std;

namespace NANDRAD_MODEL {

ConstructionInsideBCModel::ConstructionInsideBCModel(unsigned int id, const std::string &displayName) :
	DefaultModel(id, displayName),
	DefaultStateDependency(SteadyState),
	m_surfaceTemperature(0),
	m_interface(NULL),
	m_constructionInstance(NULL)
{
}


int ConstructionInsideBCModel::priorityOfModelEvaluation() const {
	// We have the following model dependency sequence at the end of evaluation stack:
	// ...
	// HeatingsLoadModel  +		WindowModel      +    LightingLoadModel (at priorityOffsetTail)
	//		^						^						^
	//		|						|						|
	//InsideBCHeatingsLoadModel + WindowsLoadModel + InsideBCLightingsLoadModel
	//		^				^		^						  ^
	//		|				|		|						  |
	//	InsideBCLWRadExchangeModel + InsideBCWindowsLoadModel |
	//	    |				^		^						  |
	//		|				|		|						  |
	//		|				|		InsideBCSWRadExchangeModel
	//	    |				|		^
	//		|				|		|
	//	    ConstructionInsideBCModel
	return AbstractStateDependency::priorityOffsetTail + 4;
}


void ConstructionInsideBCModel::setup(const NANDRAD::Interface & iface,
	const NANDRAD::ConstructionInstance & conInstance)
{
	// store interface
	m_interface					= &iface;
	// store construction instance
	m_constructionInstance		= &conInstance;
}


void ConstructionInsideBCModel::initResults(const std::vector<AbstractModel*> & models)
{
	const char * const FUNC_ID = "[ConstructionOutsideBCModel::initResults]";
	// ensure that all references are filled
	IBK_ASSERT(m_interface != NULL);
	IBK_ASSERT(m_constructionInstance != NULL);
	// resize m_results vector
	DefaultModel::initResults(models);

	// wall area is a constant result value that is related to the wall surface and can be referenced
	// by other models
	if(m_constructionInstance->m_para[NANDRAD::ConstructionInstance::CP_AREA].name.empty() )
		throw IBK::Exception(IBK::FormatString("Error initializing construction inside model for interface with id %1 "
		"of construction instance with id %2: "
		"Parameter 'Area' is undefined!").arg(m_interface->m_id).arg(m_constructionInstance->m_id),FUNC_ID);

	double area = m_constructionInstance->m_para[NANDRAD::ConstructionInstance::CP_AREA].value;
	// correct wall area by cutting out all windows
	std::vector<NANDRAD::EmbeddedObject>::const_iterator embeddedObjectIt
		= m_constructionInstance->m_embeddedObjects.begin();

	for( ; embeddedObjectIt != m_constructionInstance->m_embeddedObjects.end(); ++embeddedObjectIt)
	{
		// exclude objects that are no windows
		if (embeddedObjectIt->m_window.m_modelType == NANDRAD::EmbeddedObjectWindow::NUM_MT)
			continue;
		// enforce existence of window area
		if(embeddedObjectIt->m_para[NANDRAD::EmbeddedObject::P_Area].name.empty() )
			throw IBK::Exception(IBK::FormatString("Error initializing construction inside model for interface with id %1 "
			"of construction instance with id %2: "
			"Parameter 'Area' is undefined for embedded object with id %3!")
			.arg(m_interface->m_id)
			.arg(m_constructionInstance->m_id)
			.arg(embeddedObjectIt->m_id)
			,FUNC_ID);
		area -= embeddedObjectIt->m_para[NANDRAD::EmbeddedObject::P_Area].value;
	}
	m_results[R_Area].value = area;
}


void ConstructionInsideBCModel::initInputReferences(const std::vector<AbstractModel*> & /*models*/) {
	const char* const FUNC_ID = "[ConstructionInsideBCModel::modelInputReference]";

	std::string category = ModelIDName() + std::string("::InputReferences");
	std::string targetName;

	// create parameter input references: heat tranfer coefficient
	targetName = KeywordList::Keyword(category.c_str(), InputRef_HeatTransferCoefficient);
	// fill reference
	InputReference inputRef;
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_INTERFACE;
	inputRef.m_id			= id(); // current interface id
	inputRef.m_sourceName	= targetName;
	inputRef.m_targetName	= targetName;
	inputReference(InputRef_HeatTransferCoefficient) = inputRef;

	// zone air temperature
	targetName = KeywordList::Keyword(category.c_str(), InputRef_AirTemperature);
	// fill reference
	inputRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE; // dependency to outside zone temperature
	inputRef.m_id				= m_interface->m_zoneId; // zone id
	inputRef.m_sourceName		= targetName;
	inputRef.m_targetName		= targetName;
	inputReference(InputRef_AirTemperature) = inputRef;

	// flux density from short wave radiation on the inside wall. We retrieve this quantity from
	// a zone related splitting model.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_SWRadWindow);
	// The splitting model provides short wave radiation fluxes for all enclosing inside surfaces
	// and therefore we refence a vector valued quantity. We use our surface index for specifying the
	// corresponding vector element
	inputRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName = targetName;
	inputReference(InputRef_SWRadWindow) = inputRef;

	// flux density from visible light. We retrieve this quantity from
	// a zone related splitting model.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_SWRadLighting);
	// The splitting model provides short wave radiation fluxes for all enclosing inside surfaces
	// and therefore we refence a vector valued quantity. We use our surface index for specifying the
	// corresponding vector element
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName = targetName;
	inputReference(InputRef_SWRadLighting) = inputRef;

	// flux density from short wave radiation exchange. We retrieve this quantity from
	// a zone related splitting model.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_SWRadExchange);
	// The splitting model provides short wave radiation fluxes for all enclosing inside surfaces
	// and therefore we refence a vector valued quantity. We use our surface index for specifying the
	// corresponding vector element
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName = targetName;
	inputReference(InputRef_SWRadExchange) = inputRef;

	// flux density from long wave radiation exchange the inside wall.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_LWRadExchange);
	// The splitting model provides short wave radiation fluxes for all enclosing inside surfaces
	// and therefore we refence a vector valued quantity. We use our surface index for specifying the
	// corresponding vector element
	inputRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName		= targetName;
	inputReference(InputRef_LWRadExchange) = inputRef;

	// create input references for long wave radiation heating load from splitting model:
	// flux density from radiant heating gains on the inside wall.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_LWRadHeating);
	// we retrieve the reference again from a zone related splitting model
	// and access a vector valued result element using index notation
	inputRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName		= targetName;
	inputReference(InputRef_LWRadHeating) = inputRef;

	// create input references for long wave radiation cooling load from splitting model:
	// flux density from radiant heating gains on the inside wall.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_LWRadCooling);
	// we retrieve the reference again from a zone related splitting model
	// and access a vector valued result element using index notation
	inputRef.m_referenceType	= NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName		= targetName;
	inputReference(InputRef_LWRadCooling) = inputRef;

	// create input references for user loads from splitting model:
	// flux density from user radiant gains on the inside wall.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_LWRadUserLoad);
	// we retrieve the reference again from a zone related splitting model
	// and access a vector valued result element using index notation
	inputRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName			= targetName;
	inputReference(InputRef_LWRadUserLoad) = inputRef;

	// create input references for equipment loads from splitting model:
	// flux density from equipment radiant gains on the inside wall.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_LWRadEquipmentLoad);
	// we retrieve the reference again from a zone related splitting model
	// and access a vector valued result element using index notation
	inputRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id					= m_interface->m_zoneId;
	 // construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName			= targetName;
	inputReference(InputRef_LWRadEquipmentLoad) = inputRef;

	// create input references for light loads from splitting model:
	// flux density from light radiant gains on the inside wall.
	targetName = KeywordList::Keyword(category.c_str(), InputRef_LWRadLighting);
	// we retrieve the reference again from a zone related splitting model
	// and access a vector valued result element using index notation
	inputRef.m_referenceType		= NANDRAD::ModelInputReference::MRT_ZONE;
	// windows loads are selected from the inside zone
	inputRef.m_id = m_interface->m_zoneId;
	// construct a vector element name using id notation (quantity[id=....])
	inputRef.m_sourceName = QuantityName(targetName, id());
	inputRef.m_targetName	= targetName;
	inputReference(InputRef_LWRadLighting) = inputRef;
}


void ConstructionInsideBCModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	DefaultModel::resultDescriptions(resDesc);

	for (unsigned int i = 0; i < resDesc.size(); ++i) {
		// *** area >= 0 ***
		if (resDesc[i].m_name == KeywordList::Keyword("ConstructionInsideBCModel::Results", R_Area)) {
			resDesc[i].m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
		}
	}

	// create quantity description for surface temperature (set true in order to
	// reject construction of a new result quantity inside DefaultModel)
	QuantityDescription tempDesc("SurfaceTemperature", "C", "Wall surface temperature", true);
	resDesc.push_back(tempDesc);
}


void ConstructionInsideBCModel::resultValueRefs(std::vector<const double *> &res) const {
	// first seach in m_results vector
	DefaultModel::resultValueRefs(res);

	// Add all internal quantities to result value refs vector:
	res.push_back(m_surfaceTemperature);
}


const double * ConstructionInsideBCModel::resultValueRef(const QuantityName & quantityName) const
{
	// access the result quantities
	const double *valRef = DefaultModel::resultValueRef(quantityName);
	if(valRef != NULL)
		return valRef;

	// reference to surface temperature
	if (quantityName == "SurfaceTemperature")
		return m_surfaceTemperature;

	return NULL;
}


// this function is only called if a new time point has been set
int ConstructionInsideBCModel::update() {
	DEBUG_OBJECT_SIGNATURE("[ConstructionInsideBCModel::update()]");

	IBK_ASSERT(inputValueRefs()[InputRef_AirTemperature] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_HeatTransferCoefficient] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_SWRadWindow] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_SWRadLighting] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_SWRadExchange] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_LWRadExchange] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_LWRadHeating] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_LWRadCooling] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_LWRadUserLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_LWRadEquipmentLoad] != NULL);
	IBK_ASSERT(inputValueRefs()[InputRef_LWRadLighting] != NULL);
	IBK_ASSERT(m_surfaceTemperature != NULL);

	// directly retrieve wall surface and zone air temperature from input reference values
	const std::vector<const double *> & inputRef = inputValueRefs();
	const double Tzone		= *inputRef[InputRef_AirTemperature];
	const double Tsurface	 = *m_surfaceTemperature;
	// retrieve heat transfer coefficient, long wave and short wave radiation
	const double alpha		= *inputValueRefs()[InputRef_HeatTransferCoefficient];
	const double qSWRad     = *inputValueRefs()[InputRef_SWRadWindow]
							  + *inputValueRefs()[InputRef_SWRadLighting]
							  + *inputValueRefs()[InputRef_SWRadExchange];
	// radiant heating gains, user and equipment loads are treated as long wave radiation gains
	const double qLWRad     = *inputValueRefs()[InputRef_LWRadExchange]
							  + *inputValueRefs()[InputRef_LWRadHeating]
							  + *inputValueRefs()[InputRef_LWRadCooling]
							  + *inputValueRefs()[InputRef_LWRadUserLoad]
							  + *inputValueRefs()[InputRef_LWRadEquipmentLoad]
							  + *inputValueRefs()[InputRef_LWRadLighting];
	// retrieve wall area
	double area				= m_results[R_Area].value;

	// heat flux density by heat conduction is directed to the wall
	const double qHeatCond =  alpha * (Tzone - Tsurface);
	m_results[R_HeatConduction].value = qHeatCond;
	// heat flux by heat conduction
	m_results[R_HeatConductionFlux].value = area * qHeatCond;

	// heat flux by short wave radiation from the windows
	m_results[R_SWRadAbsorbed].value	   =  qSWRad;
	m_results[R_SWRadAbsorbedFlux].value =  area * qSWRad;

	// heat flux by long wave radiation
	m_results[R_LWRadBalance].value	  =  qLWRad;
	m_results[R_LWRadBalanceFlux].value =  area * qLWRad;

	// signal success
	return 0;
}


void ConstructionInsideBCModel::stateDependencies(std::vector< std::pair<const double *, const double *> >
							&resultInputValueReferences) const {

	// *** heat conduction flux ***

	const double *valueRef = &m_results[R_HeatConduction].value;

	// connect to heat transfer coefficient
	const double *inputRef = inputValueRefs()[InputRef_HeatTransferCoefficient];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to air temperature
	inputRef = inputValueRefs()[InputRef_AirTemperature];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to surface temperature
	inputRef = m_surfaceTemperature;
	resultInputValueReferences.push_back(std::make_pair(valueRef, inputRef));

	valueRef = &m_results[R_HeatConductionFlux].value;

	// connect to heat transfer coefficient
	inputRef = inputValueRefs()[InputRef_HeatTransferCoefficient];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to air temperature
	inputRef = inputValueRefs()[InputRef_AirTemperature];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to surface temperature
	inputRef = m_surfaceTemperature;
	resultInputValueReferences.push_back(std::make_pair(valueRef, inputRef));

	// *** long wave radiation flux ***

	valueRef = &m_results[R_LWRadBalance].value;

	// connect to heating long wave radiation exchange gains
	inputRef = inputValueRefs()[InputRef_LWRadExchange];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to heating long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadHeating];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to cooling long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadCooling];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to user long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadUserLoad];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to equipment long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadEquipmentLoad];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to lighting long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadLighting];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );

	valueRef = &m_results[R_LWRadBalanceFlux].value;

	// connect to heating long wave radiation exchange gains
	inputRef = inputValueRefs()[InputRef_LWRadExchange];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to heating long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadHeating];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to cooling long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadCooling];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to user long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadUserLoad];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to equipment long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadEquipmentLoad];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );
	// connect to lighting long wave radiation gains
	inputRef = inputValueRefs()[InputRef_LWRadLighting];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );

	// *** short wave radiation flux ***

	valueRef = &m_results[R_SWRadAbsorbed].value;

	// connect to distruibuted window radiation gains
	inputRef = inputValueRefs()[InputRef_SWRadWindow];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );

	// connect to distruibuted lighting radiation gains
	inputRef = inputValueRefs()[InputRef_SWRadLighting];
	resultInputValueReferences.push_back(std::make_pair
	(valueRef, inputRef));

	// connect to distruibuted exchange radiation gains
	inputRef = inputValueRefs()[InputRef_SWRadExchange];
	resultInputValueReferences.push_back(std::make_pair
	(valueRef, inputRef));


	valueRef = &m_results[R_SWRadAbsorbedFlux].value;

	// connect to distruibuted solar radiation gains
	inputRef = inputValueRefs()[InputRef_SWRadWindow];
	resultInputValueReferences.push_back(std::make_pair
			(valueRef, inputRef) );

	// connect to distruibuted lighting radiation gains
	inputRef = inputValueRefs()[InputRef_SWRadLighting];
	resultInputValueReferences.push_back(std::make_pair
	(valueRef, inputRef));

	// connect to distruibuted exchange radiation gains
	inputRef = inputValueRefs()[InputRef_SWRadExchange];
	resultInputValueReferences.push_back(std::make_pair
	(valueRef, inputRef));

}


const double *ConstructionInsideBCModel::heatConduction() const {
	if (m_results.empty())
		return NULL;

	return &m_results[R_HeatConduction].value;
}


const double *ConstructionInsideBCModel::lwRadBalance() const {
	if (m_results.empty())
		return NULL;

	return &m_results[R_LWRadBalance].value;
}


const double *ConstructionInsideBCModel::swRadAbsorbed() const {
	if (m_results.empty())
		return NULL;

	return &m_results[R_SWRadAbsorbed].value;
}


void ConstructionInsideBCModel::setSurfaceTemperature(const double *temperature) {
	m_surfaceTemperature = temperature;
}

} // namespace NANDRAD_MODEL

