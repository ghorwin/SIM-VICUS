/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_ConstructionInstance.h"

#include <algorithm>

#include <IBK_Parameter.h>
#include <IBK_Exception.h>
#include <IBK_messages.h>

#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_Project.h"

#include <tinyxml.h>

namespace NANDRAD {


void ConstructionInstance::checkParameters(const Project & prj) {
	FUNCID(ConstructionInstance::checkParameters);

	const std::vector<ConstructionType> & conTypes = prj.m_constructionTypes;

	// check and resolve construction type reference
	std::vector<ConstructionType>::const_iterator it = std::find(conTypes.begin(), conTypes.end(), m_constructionTypeId);
	if (it == conTypes.end())
		throw IBK::Exception( IBK::FormatString("Invalid/unknown construction type ID %1.").arg(m_constructionTypeId), FUNC_ID);
	m_constructionType = &(*it); // store pointer

	// check parameters
	double area = m_para[P_Area].checkedValue("Area", "m2", "m2", 0, false, std::numeric_limits<double>::max(), true,
											  "Cross section area of construction instance must be > 0 m2.");

	// Note: parameters orientation and inclination are only needed when an outdoor interface with solar radiation
	//       model is defined, so first look for such an interface.
	//
	// Also, we currently rely on the following convention: if an interface has parameters, i.e. m_modelType != NUM_MT
	// in any submodel, then the interface exists (m_id != INVALID_ID), which we do not test for explicitely.

	bool haveRadiationBCA = false;
	bool haveRadiationBCB = false;
	if (m_interfaceA.m_solarAbsorption.m_modelType != InterfaceSolarAbsorption::NUM_MT)	{
		// We only test for radiation boundary conditions, when we have an outside interface, i.e. zoneID == 0
		if (m_interfaceA.m_zoneId == 0)
			haveRadiationBCA = true;
		else
			IBK::IBK_Message(IBK::FormatString("Interface A of construction instance '%1' (#%2) is an "
											   "inside surface (connected to a zone), yet has solar radiation "
											   "BC parameters defined. This is likely an error.")
							 .arg(m_displayName).arg(m_id),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
	}
	if (m_interfaceB.m_solarAbsorption.m_modelType != InterfaceSolarAbsorption::NUM_MT) {
		if (m_interfaceB.m_zoneId == 0)
			haveRadiationBCB = true;
		else
			IBK::IBK_Message(IBK::FormatString("Interface B of construction instance '%1' (#%2) is an "
											   "inside surface (connected to a zone), yet has solar radiation "
											   "BC parameters defined. This is likely an error.")
							 .arg(m_displayName).arg(m_id),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
	}

	// check that we do not have solar radiation on both sides of the construction and both are outside constructions
	if (haveRadiationBCA && haveRadiationBCB)
		throw IBK::Exception( "Defining a construction with ambient solar radiation boundary "
							  "conditions on both sides is not supported.", FUNC_ID);

	if (haveRadiationBCA || haveRadiationBCB) {
		// we have solar radiation to outside - and we need orientation and inclination for that
		m_para[P_Orientation].checkedValue("Orientation", "Deg", "Deg", 0, true, 360, true,
										   "Parameter 'Orientation' outside allowed value range [0,360] Deg.");

		m_para[P_Inclination].checkedValue("Inclination", "Deg", "Deg", 0, true, 180, true,
										   "Parameter 'Inclination' outside allowed value range [0,180] Deg.");
	}

	// check boundary condition models
	try {
		m_interfaceA.checkParameters();
		// tell the interface that it is "Side A"
		m_interfaceA.m_isSideA = true;
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error checking model parameters for InterfaceA.", FUNC_ID);
	}
	try {
		m_interfaceB.checkParameters();
		m_interfaceA.m_isSideA = false;
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error checking model parameters for InterfaceB.", FUNC_ID);
	}

	// NOTE: constructions may have two internal interfaces that reference the same zone ID
	//       In this case, the construction may be an inner-zone construction, that is only modeled as
	//       thermal storage element.

	// check embedded objects
	double totalEmbeddedObjectsArea = 0;
	for (unsigned int i=0; i<m_embeddedObjects.size(); ++i) {
		EmbeddedObject & eo = m_embeddedObjects[i];
		try {
			eo.checkParameters(prj);
			// we have a valid area in the embedded object - add it up
			totalEmbeddedObjectsArea += eo.m_para[EmbeddedObject::P_Area].value;
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error checking model parameters for EmbeddedObject #%1 '%2' (id=%3).")
								 .arg(i).arg(eo.m_displayName).arg(eo.m_id), FUNC_ID);
		}
	}
	// now check that sum of embedded object area is less or equal to the construction instance's area
	m_netHeatTransferArea = area - totalEmbeddedObjectsArea;
	if (m_netHeatTransferArea < 0)
		throw IBK::Exception(IBK::FormatString("Area used by all embedded objects (=%1 m2) exceeds gross area of construction instance (=%2 m2).")
							 .arg(totalEmbeddedObjectsArea).arg(area), FUNC_ID);


	// check and initialize inner long wave radiation heat exchange
	if (m_interfaceA.m_zoneId > 0 &&
		m_interfaceA.m_longWaveEmission.m_modelType != InterfaceLongWaveEmission::NUM_MT)
		checkAndPrepareLongWaveHeatExchange(prj, m_interfaceA);
	if (m_interfaceB.m_zoneId > 0 &&
		m_interfaceB.m_longWaveEmission.m_modelType != InterfaceLongWaveEmission::NUM_MT)
		checkAndPrepareLongWaveHeatExchange(prj, m_interfaceB);

}


bool ConstructionInstance::behavesLike(const ConstructionInstance & other) const {
	if (m_constructionTypeId != other.m_constructionTypeId)
		return false;

	// now compare interfaces
	if (!m_interfaceA.behavesLike( other.m_interfaceA) )
		return false;

	if (!m_interfaceB.behavesLike( other.m_interfaceB) )
		return false;

	// compare parameters
	if (m_para[P_Area] != other.m_para[P_Area])
		return false;
	if (m_para[P_Orientation] != other.m_para[P_Orientation])
		return false;
	if (m_para[P_Inclination] != other.m_para[P_Inclination])
		return false;

	return true; // both construction instances would calculate effectively the same
}


unsigned int ConstructionInstance::interfaceAZoneID() const {
	if (m_interfaceA.m_id != NANDRAD::INVALID_ID)
		return m_interfaceA.m_zoneId;
	return 0;
}


unsigned int ConstructionInstance::interfaceBZoneID() const {
	if (m_interfaceB.m_id != NANDRAD::INVALID_ID)
		return m_interfaceB.m_zoneId;
	return 0;
}


void ConstructionInstance::checkAndPrepareLongWaveHeatExchange(const Project & prj, Interface & ourInterface) {
	FUNCID(ConstructionInstance::checkAndPrepareLongWaveHeatExchange);

	// Collect all construction instances and interfaces that interact with this one over the same zone
	// connectedConstructionInstances holds pointers of all other constructions that share at least one interface
	// to the same inner zone.
	std::vector<const NANDRAD::ConstructionInstance*> connectedConstructionInstances;
	ourInterface.m_connectedInterfaces.clear();
	// loop over all constructions
	for (const ConstructionInstance &ci: prj.m_constructionInstances) {
		// skip ourselves
		if (ci.m_id == m_id)
			continue;
		// if the construction has an interface A that references the same zone as ourInterface references,
		// both constructions/interfaces may have long-wave radiation exchange; if the interface does not
		// have LW rad exchange, an error is issued below
		if (ci.m_interfaceA.m_zoneId == ourInterface.m_zoneId) {
			ourInterface.m_connectedInterfaces[ci.m_id] = &ci.m_interfaceA;
			connectedConstructionInstances.push_back(&ci);
		}
		// same for side B interface; NOTE: a construction may be an inner-zone element and reference the same zone
		// with BOTH interfaces.
		if (ci.m_interfaceB.m_zoneId == ourInterface.m_zoneId) {
			ourInterface.m_connectedInterfaces[ci.m_id] = &ci.m_interfaceB;
			connectedConstructionInstances.push_back(&ci);
		}
	}

	// check if all of them have also long wave BC activated
	unsigned int i=0;
	IBK_ASSERT(ourInterface.m_connectedInterfaces.size() == connectedConstructionInstances.size());
	for (auto it=ourInterface.m_connectedInterfaces.begin(); it!=ourInterface.m_connectedInterfaces.end(); ++it) {
		const NANDRAD::Interface *inter = it->second;
		IBK_ASSERT(inter != nullptr);
		if (inter->m_longWaveEmission.m_modelType == InterfaceLongWaveEmission::NUM_MT) {
			throw IBK::Exception( IBK::FormatString("Interface #%1 in construction instance #%2 (%3) exchanges long wave radiation with interface #%4 in construction instance #%5 (%6), but it has no long wave emission model defined.")
								 .arg(inter->m_id).arg(connectedConstructionInstances[i]->m_id).arg(connectedConstructionInstances[i]->m_displayName)
								 .arg(ourInterface.m_id).arg(m_id).arg(m_displayName), FUNC_ID );
		}
		++i;
	}

	// now collect view factors from the given zone and store them as "our own" view factors, i.e. view factors from this ci to the connected one
	std::vector<Zone>::const_iterator it = std::find(prj.m_zones.begin(), prj.m_zones.end(), ourInterface.m_zoneId);
	// ensure that the referenced zone exists
	// NOTE: for long-wave exchange we need the zone, yet for general simulations we may access zone properties (like AirTemperature) also via
	//       defined FMU interface. Hence, the mandatory check for zone existances is only done here and not for ALL constructions.
	if (it == prj.m_zones.end())
		throw IBK::Exception( IBK::FormatString("Interface #%1 of construction instance #%2 references zone #%3, but that doesn't exist.")
							  .arg(ourInterface.m_id).arg(m_id).arg(ourInterface.m_zoneId), FUNC_ID);

	ourInterface.m_zone = &(*it);
	if (ourInterface.m_zone->m_viewFactors.empty())
		throw IBK::Exception( IBK::FormatString("Construction instance #%1 has a long wave radiation model defined at an inner surface, "
												"but no view factors are given for according zone #%2.").arg(m_id).arg(ourInterface.m_zoneId), FUNC_ID);
	double viewFactorSum = 0;
	for (auto it = ourInterface.m_zone->m_viewFactors.begin(); it!=ourInterface.m_zone->m_viewFactors.end(); ++it) {
		const std::pair<unsigned int, unsigned int>  & viewFactorPair = it->first;
		unsigned int sourceId = viewFactorPair.first;
		unsigned int targetId = viewFactorPair.second;
		if (m_id == sourceId) { // view factor from ourself to other construction instance
			// check that target-ID is in set of connected constructions
			ourInterface.m_viewFactors[targetId] = it->second;
			viewFactorSum += it->second;
		}
	}
	if (std::fabs(viewFactorSum-1) > 0.01) {
		IBK::IBK_Message(IBK::FormatString("View factors defined in zone #%1 for construction instance #%2 sum up to %3, but should be 1 (maybe not closed space?).")
						 .arg(ourInterface.m_zoneId).arg(m_id).arg(viewFactorSum), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
	}

	// check if we have a view factor for each connected ci
	for (const ConstructionInstance *ci: connectedConstructionInstances) {
		if (ourInterface.m_viewFactors.find(ci->m_id) == ourInterface.m_viewFactors.end())
			throw IBK::Exception( IBK::FormatString("Construction instance #%1 exchanges long wave radiation with construction instance #%2, "
													"but no view factor is defined between both.").arg(m_id).arg(ci->m_id), FUNC_ID );
	}
	// check if we have a connected ci for each view factor
	for (auto it = ourInterface.m_viewFactors.begin(); it!=ourInterface.m_viewFactors.end(); ++it) {
		unsigned int targetId = it->first;
		if (ourInterface.m_connectedInterfaces.find(targetId) == ourInterface.m_connectedInterfaces.end())
			throw IBK::Exception( IBK::FormatString("View factor is given from construction instance #%1 to construction instance #%2, "
													"but construction instance #%2 does not seem to exist or is not connected to construction instance #%1").arg(m_id).arg(targetId), FUNC_ID );
	}
	// for each long wave exchange we have one target construction and a view factor
	IBK_ASSERT(ourInterface.m_connectedInterfaces.size() == ourInterface.m_viewFactors.size());
}


} // namespace NANDRAD

