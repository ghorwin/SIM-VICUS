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

#include "NM_ThermalComfortModel.h"

//#include "NM_ConstructionInsideBCModel.h"
//#include "NM_ConstructionStatesModel.h"
#include "NM_KeywordList.h"
#include "NM_RoomBalanceModel.h"
//#include "NM_ConstantWindowModel.h"

#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_Zone.h>
#include <NANDRAD_Interface.h>
//#include <NANDRAD_ParametrizationDefaults.h>
#include <NANDRAD_SimulationParameter.h>

using namespace std;

namespace NANDRAD_MODEL {

int ThermalComfortModel::priorityOfModelEvaluation() const {
	// The thermal comfort model may be evaluated as soon the wall
	// surface and window temperatures are available. This calculation is
	// performed by the ConstrcutionStatesModel at evaluation
	// level 0 and by the WindowModel  at evaluation
	// level 1 of the dependency graph.
	// return 1;
	return 2;
}

void ThermalComfortModel::inputReferenceDescriptions(std::vector<QuantityDescription> & refDesc) const {
#if 0
	// use default model implementation
	DefaultStateDependency::inputReferenceDescriptions(refDesc);
	// exclude inactive references to interfaces embedded objects
	if (m_surfaceIds.empty()) {
		// deactivate radiant temperature
		refDesc[InputRef_RadiantTemperature].clear();
		// deactivate surface area
		refDesc[InputRef_Area].clear();
	}
	// resize with correct number of surfaces
	else {
		// resize radiant temperature
		refDesc[InputRef_RadiantTemperature].resize(m_surfaceIds, VectorValuedQuantityIndex::IK_ModelID);
		// resize surface area
		refDesc[InputRef_Area].resize(m_surfaceIds, VectorValuedQuantityIndex::IK_ModelID);
	}
#endif
}

void ThermalComfortModel::initInputReferences(const std::vector<AbstractModel*> & models) {
#if 0
	const char * const FUNC_ID = "[ThermalComfortModel::initInputReferences]";

	/*! Pointer to all inside interfaces of the current room. */
	std::vector<const NANDRAD::Interface *>				ifaces;
	/*! Id numbers of all connected construction: same size as interfaces. */
	std::vector<unsigned int>							conInstanceIds;
	/*! Id numbers of all connected windows: same size as m_windows. */
	std::vector<unsigned int>							windowIds;

	// find all enclosing comnstructions and interfaces
	for(unsigned int i = 0; i < models.size(); ++i) {
		// skip all models that are no inside walls
		const ConstructionInsideBCModel* insideBCModel = dynamic_cast<const ConstructionInsideBCModel*>(models[i]);
		if(insideBCModel != NULL) {
			// The inside boundary condition model stores a reference to the described interface
			// and the connected construction instance. We only accept interfaces
			/// that connect to the current zone id number.
			if(insideBCModel->iface()->m_zoneId == id()) {
				ifaces.push_back(insideBCModel->iface());
				conInstanceIds.push_back(insideBCModel->constructionInstance()->m_id);
				m_surfaceIds.insert(insideBCModel->iface()->m_id);
			}
			continue;
		}
		const ConstantWindowModel* windowModel = dynamic_cast<const ConstantWindowModel*>(models[i]);
		if(windowModel != NULL) {
			// We only accept interfaces that connect to the current zone id number.
			for(unsigned int i = 0; i < windowModel->constructionInstance()->m_interfaces.size(); ++i) {
				if(windowModel->constructionInstance()->m_interfaces[i].m_zoneId == id()) {
					windowIds.push_back(windowModel->id() );
					m_surfaceIds.insert(windowModel->id());
					break;
				}
			}
		}
	}

	// retrieve names of the requested quantities using the keyword list
	// information
	std::string sourceCategory	= "ConstructionStatesModel::Results";
	std::string targetCategory	= ModelIDName() + std::string("::InputReferences");
	QuantityName targetName;

	// retrieve all model input references to wall surface temperatures
	for(unsigned int i = 0; i < ifaces.size(); ++i)
	{
		const NANDRAD::Interface &iface = *ifaces[i];
		// Compose target name from the quantity
		// including index information. Because we need access to more than one
		// wall surface temperatures we compose an input reference to a vector valued target
		// using the interface id-number as index identification
		targetName = QuantityName(
			KeywordList::Keyword(targetCategory.c_str(), InputRef_RadiantTemperature),
				iface.m_id);
		// construct a new input reference
		InputReference surfaceTempRef;
		surfaceTempRef.m_targetName = targetName;
		// Anyhow, we only get access to the wall surface temperatures from the construction instance/
		// the corresponding ConstructuionStatesModel
		surfaceTempRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		surfaceTempRef.m_id			   = conInstanceIds[i];
		// chose surface temperture from construction instance location
		// -> we ensure that values are updated at the beginning of complete update procedure
		// and can set reference to 'Constant'
		if(iface.m_location == NANDRAD::Interface::IT_A)
		{
			surfaceTempRef.m_sourceName = KeywordList::Keyword(sourceCategory.c_str(), ConstructionStatesModel::R_SurfaceTemperatureA);
		}
		else if(iface.m_location == NANDRAD::Interface::IT_B)
		{
			surfaceTempRef.m_sourceName = KeywordList::Keyword(sourceCategory.c_str(), ConstructionStatesModel::R_SurfaceTemperatureB);
		}
		else {
			throw IBK::Exception(IBK::FormatString("Error initializing ConstructionOutsideBCModel of interface with ID %1: "
				"No interface location side of the construction is specified.")
				.arg(id()), FUNC_ID);
		}
		// using the quantity type and index information sort reference into
		// m_inpoutReferences vector
		inputReference(InputRef_RadiantTemperature, iface.m_id) = surfaceTempRef;
	}

	// retrieve all model input references to window inside surface temperatures
	for(unsigned int i = 0; i < windowIds.size(); ++i)
	{
		// Compose target name from the quantity
		// including index information. Because we need access to more than one
		// wall surface temperatures we compose an input reference to a vector valued target
		// using the interface id-number as index identification
		targetName = QuantityName(
			KeywordList::Keyword(targetCategory.c_str(), InputRef_RadiantTemperature),
			windowIds[i]);
		// construct a new input reference
		InputReference windowTempRef;
		windowTempRef.m_targetName = targetName;
		// Anyhow, we only get access to the wall surface temperatures from the construction instance/
		// the corresponding ConstructuionStatesModel
		windowTempRef.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
		windowTempRef.m_id			  = windowIds[i];
		windowTempRef.m_sourceName	  = KeywordList::Keyword(targetCategory.c_str(), InputRef_RadiantTemperature);
		// using the quantity type and index information sort reference into
		// m_inpoutReferences vector
		inputReference(InputRef_RadiantTemperature, windowIds[i]) = windowTempRef;
	}

	targetName = KeywordList::Keyword(targetCategory.c_str(), InputRef_AirTemperature);
	// create input reference for room air temperature
	InputReference roomTempRef;
	roomTempRef.m_targetName    = targetName;
	// we retrieve this quantity from a zone calculatioon model
	roomTempRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	roomTempRef.m_id = id();
	roomTempRef.m_sourceName    = targetName;
	// using the quantity type sort reference into
	// m_inputReferences vector
	inputReference(InputRef_AirTemperature) = roomTempRef;

	// retrieve all model input references to wall areas
	for(unsigned int i = 0; i < ifaces.size(); ++i)
	{
		const NANDRAD::Interface &iface = *ifaces[i];
		// the interface (boundary condition) models offer the area as constant result quantity
		targetName = QuantityName(
			KeywordList::Keyword(targetCategory.c_str(), InputRef_Area), iface.m_id);

		InputReference wallAreaRef;
		wallAreaRef.m_sourceName = KeywordList::Keyword(targetCategory.c_str(), InputRef_Area);
		wallAreaRef.m_referenceType	   = NANDRAD::ModelInputReference::MRT_INTERFACE;
		// interface id number
		wallAreaRef.m_id			   = iface.m_id;
		wallAreaRef.m_targetName	   = targetName;
		// set reference to constant
		wallAreaRef.m_constant = true;
		// using the quantity type and index information sort reference into
		// m_inpoutReferences vector
		inputReference(InputRef_Area, iface.m_id) = wallAreaRef;
	}
	// retrieve all model input references to window areas
	for(unsigned int i = 0; i < windowIds.size(); ++i)
	{
		// Compose target name from the quantity
		// including index information. Because we need access to more than one
		// wall surface temperatures we compose an input reference to a vector valued target
		// using the interface id-number as index identification
		targetName = QuantityName(
			KeywordList::Keyword(targetCategory.c_str(), InputRef_Area), windowIds[i]);
		// construct a new input reference
		InputReference windowAreaRef;
		windowAreaRef.m_targetName = targetName;
		// Anyhow, we only get access to the wall surface temperatures from the construction instance/
		// the corresponding ConstructuionStatesModel
		windowAreaRef.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
		windowAreaRef.m_id			  = windowIds[i];
		windowAreaRef.m_sourceName	  = KeywordList::Keyword(targetCategory.c_str(), InputRef_Area);
		// using the quantity type and index information sort reference into
		// m_inpoutReferences vector
		inputReference(InputRef_Area, windowIds[i]) = windowAreaRef;
	}
#endif
}

int ThermalComfortModel::update() {

#if 0
	// we start with scalar references
	const double *roomTempRef = m_inputValueRefs()[InputRef_AirTemperature];

	IBK_ASSERT(roomTempRef != NULL);
	// retrieve room temperature
	const double RoomTemperature = *roomTempRef;

	// simplest case: we only have room temperature (no enclosing
	// walls
	if(m_surfaceIds.empty())
	{
		// than set radiant temparture to 0
		m_results[R_RadiantTemperature]   = 0.0;
		m_results[R_OperativeTemperature] = RoomTemperature;
		// signal success
		return 0;
	}

	// otherwise calculate mean surface temperature
	double ComleteWallArea = 0.0;
	double RadiantTemperature = 0.0;
	// for this purpose we set iterators to the starting position
	// of surface temperature and area references
	std::vector<const double *>::const_iterator surfTempIt = inputValueRefs(InputRef_RadiantTemperature);
	std::vector<const double *>::const_iterator areaIt	   = inputValueRefs(InputRef_Area);

	// loop over all enclosing surfaces
	for(std::set<unsigned int>::const_iterator
		surfaceIt = m_surfaceIds.begin();
		surfaceIt != m_surfaceIds.end();
		++surfaceIt, ++surfTempIt, ++areaIt)
	{
		// retrieve references to the depending quantities
		// for the current interface
		const double * surfTempRef = *surfTempIt;
		const double * areaRef	   = *areaIt;
		// ensure validity of teh implementation
		IBK_ASSERT(surfTempRef != NULL);
		IBK_ASSERT(areaRef != NULL);

		const double SurfaceTemperature = *surfTempRef;
		const double Area				= *areaRef;
		// calculate weighted values and sum up wall area
		RadiantTemperature += Area * SurfaceTemperature;
		ComleteWallArea    += Area;
	}
	// calculate mean value
	RadiantTemperature /= ComleteWallArea;
	// store wall temperature
	m_results[R_RadiantTemperature]   = RadiantTemperature;

	// calculate operative temperature as weighted average between room air and
	// radiant temperature
	double OperativeTemperature = 0.5 * (RadiantTemperature + RoomTemperature);
	// store the result
	m_results[R_OperativeTemperature] = OperativeTemperature;
#endif
	// signal success
	return 0;
}


void ThermalComfortModel::stateDependencies(std::vector< std::pair<const double *, const double *> >
							&resultInputValueReferences) const
{
#if 0
	// connect all convective and radiant sources
	const double *valueRef = &m_results[R_RadiantTemperature];
	// retreive all fluxes
	if(!m_surfaceIds.empty() ) {
		std::vector<const double *>::const_iterator it =
			inputValueRefs(InputRef_RadiantTemperature);
		// reference to surface temperature
		for (std::set<unsigned int>::const_iterator
			surfaceIt = m_surfaceIds.begin();
			surfaceIt != m_surfaceIds.end();
			++surfaceIt, ++it)
		{
			IBK_ASSERT(it != inputValueRefs().end());
			const double *inputRef = *it;

			resultInputValueReferences.push_back(std::make_pair(valueRef, inputRef)  );
		}
		it = inputValueRefs(InputRef_Area);
		// reference to surface temperature
		for (std::set<unsigned int>::const_iterator
			surfaceIt = m_surfaceIds.begin();
			surfaceIt != m_surfaceIds.end();
			++surfaceIt, ++it)
		{
			IBK_ASSERT(it != inputValueRefs().end());
			const double *inputRef = *it;

			resultInputValueReferences.push_back(std::make_pair(valueRef, inputRef)  );
		}
	}
	// connect operative temperature to all temepratures
	// connect all convective and radiant sources
	valueRef = &m_results[R_OperativeTemperature];
	// room air temperature
	const double *inputRef  = inputValueRef(InputRef_AirTemperature);
	resultInputValueReferences.push_back(std::make_pair(valueRef, inputRef) );

	// connect to surface temperatures
	if(!(m_surfaceIds.empty() ) ) {
		std::vector<const double *>::const_iterator it =
			inputValueRefs(InputRef_RadiantTemperature);
		// reference to surface temperature
		for (std::set<unsigned int>::const_iterator
			surfaceIt = m_surfaceIds.begin();
			surfaceIt != m_surfaceIds.end();
			++surfaceIt, ++it)
		{
			IBK_ASSERT(it != inputValueRefs().end());
			const double *inputRef = *it;

			resultInputValueReferences.push_back(std::make_pair(valueRef, inputRef)  );
		}
		it = inputValueRefs(InputRef_Area);
		// reference to surface temperature
		for (std::set<unsigned int>::const_iterator
			surfaceIt = m_surfaceIds.begin();
			surfaceIt != m_surfaceIds.end();
			++surfaceIt, ++it)
		{
			IBK_ASSERT(it != inputValueRefs().end());
			const double *inputRef = *it;

			resultInputValueReferences.push_back(std::make_pair(valueRef, inputRef)  );
		}
	}
#endif
}


} // namespace NANDRAD_MODEL

