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

#include "NANDRAD_HydraulicNetwork.h"

#include <algorithm>

#include "NANDRAD_KeywordList.h"

#include "NANDRAD_Project.h"

namespace NANDRAD {


void HydraulicNetwork::checkParameters(const Project & prj, std::set<unsigned int> &otherNodeIds)  {
	FUNCID(HydraulicNetwork::checkParameters);

	// register all zone nodes
	std::set<unsigned int> zoneNodeIds;
	// register all nodes
	std::set<unsigned int> nodeIds;

	// check our own properties, first
	switch (m_modelType) {
		case MT_HydraulicNetwork :
			m_para[P_DefaultFluidTemperature].checkedValue("DefaultFluidTemperature", "K",
														   "C", -50, true, 500, true,
														   "Fluid temperature should be in the range of -50..500 C.");
		break;

		case NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork:
		case NANDRAD::HydraulicNetwork::MT_AirNetwork:
			m_para[P_InitialFluidTemperature].checkedValue("InitialFluidTemperature", "K",
														   "C", -50, true, 500, true,
														   "Fluid temperature should be in the range of -50..500 C.");
		break;

		case NANDRAD::HydraulicNetwork::NUM_MT: break; // just to make compiler warnings disappear
	}

	// check reference pressure and set default if missing
	if (m_para[P_ReferencePressure].empty())
		NANDRAD::KeywordList::setParameter(m_para, "HydraulicNetwork::para_t", P_ReferencePressure, 0);

	// do not allow an empty network
	if (m_elements.empty())
		throw IBK::Exception(IBK::FormatString("Network has no elements"), FUNC_ID);
	if (m_components.empty())
		throw IBK::Exception(IBK::FormatString("Network has no components"), FUNC_ID);

	// check reference element id
	if (std::find(m_elements.begin(), m_elements.end(), m_referenceElementId) == m_elements.end())
		throw IBK::Exception(IBK::FormatString("Invalid reference #%1 in referenceElementId, must be the id of an existing flow element!")
							 .arg(m_referenceElementId), FUNC_ID);

	// check parameters of fluid
	try {
		m_fluid.checkParameters(m_modelType);
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing fluid properties."), FUNC_ID);
	}

	std::set<unsigned int> conInstanceIds;
	// check all elements and fill references to components and pipe properties
	for (HydraulicNetworkElement &e : m_elements) {
		try {
			// select all node and zone node ids
			if(e.m_inletZoneId != NANDRAD::INVALID_ID && e.m_inletZoneId != 0)
				zoneNodeIds.insert(e.m_inletZoneId);
			else if(e.m_inletNodeId != NANDRAD::INVALID_ID)
				nodeIds.insert(e.m_inletNodeId);
			if(e.m_outletZoneId != NANDRAD::INVALID_ID && e.m_outletZoneId != 0)
				zoneNodeIds.insert(e.m_outletZoneId);
			else if(e.m_outletNodeId != NANDRAD::INVALID_ID)
				nodeIds.insert(e.m_outletNodeId);
			// the checkParameters of HydraulicNetworkHeatExchange will be executed within this function
			e.checkParameters(*this, prj);
			// select construction ids
			unsigned int conInstanceId =
					e.m_heatExchange.m_idReferences[HydraulicNetworkHeatExchange::ID_ConstructionInstanceId];
			if (conInstanceId != INVALID_ID) {
				if (!conInstanceIds.empty() && conInstanceIds.find(conInstanceId) != conInstanceIds.end())
					// error: construction is already covered by a hydraulic element
					throw IBK::Exception(IBK::FormatString("Construction instance with id #%1 is referenced twice! There must not be two flow elements "
														   "exchanging heat with the same construction instance.")
										 .arg(conInstanceId), FUNC_ID);
				// register construction instance id
				conInstanceIds.insert(conInstanceId);
			}

		}
		catch (IBK::Exception &ex) {
			if (e.m_component != nullptr)
				throw IBK::Exception(ex, IBK::FormatString("Error initializing network element with id #%1 and type %2.")
									 .arg(e.m_id).arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::ModelType",
																					e.m_component->m_modelType)), FUNC_ID);
			else
				throw IBK::Exception(ex, IBK::FormatString("Error initializing network element with id #%1.")
									 .arg(e.m_id), FUNC_ID);
		}
	}

	// only allow zone nodes in an air network
	if(!zoneNodeIds.empty() && m_modelType != MT_AirNetwork) {
		throw IBK::Exception(IBK::FormatString("Use of 'inletZoneId' and 'outletZoneId' is only allowed for type "
							 "'AirNetwork'!")
							 , FUNC_ID);
	}

	if(m_modelType == MT_AirNetwork) {
		// ensure that zone nodes are only used in current network
		// and zone ids exist in project
		for(std::set<unsigned int>::const_iterator
			idIt = zoneNodeIds.begin(); idIt != zoneNodeIds.end(); ++idIt) {
			// check if zone exists in project
			const std::vector<NANDRAD::Zone>::const_iterator it = std::find(prj.m_zones.begin(), prj.m_zones.end(), *idIt);
			if (it == prj.m_zones.end())
				throw IBK::Exception(IBK::FormatString("Zone with id '%1' requested from inletZoneId' or 'outletZoneId' attribute "
													   "does not exist!")
									 .arg(*idIt), FUNC_ID);

			// ensure, that node and zoneids are unique
			if(nodeIds.find(*idIt) != nodeIds.end())
				throw IBK::Exception(IBK::FormatString("Node with id '%1' requested from inletNoneId' or 'outletNoneId' attribute "
													   "is not unique! The id also exists as 'inletZoneId' and 'outletZoneId'! "
													   "Mind, that we use the same id space for nodes and zones.")
									 .arg(*idIt), FUNC_ID);

			// check against other node ids
			if(otherNodeIds.find(*idIt) != otherNodeIds.end()) {
				throw IBK::Exception(IBK::FormatString("Invalid use of 'inletZoneId' and 'outletZoneId' #%1! "
									 "This zone is part of another network.").arg(*idIt)
									 , FUNC_ID);
			}
			// add to container
			otherNodeIds.insert(*idIt);
		}
	}

	// check parameters of all components
	for (HydraulicNetworkComponent &c : m_components) {
		try {
			c.checkParameters(m_modelType);
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing network component with id #%1.")
								 .arg(c.m_id), FUNC_ID);
		}
	}
	// check parameters of all pipe properties
	for (HydraulicNetworkPipeProperties &p : m_pipeProperties) {
		try {
			p.checkParameters(m_modelType);
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing pipe property with id #%1.")
								 .arg(p.m_id), FUNC_ID);
		}
	}
	// check control elements
	for (HydraulicNetworkControlElement &e : m_controlElements) {
		try {
			e.checkParameters(prj.m_zones);
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing control element with id #%1.")
								 .arg(e.m_id), FUNC_ID);
		}
	}

	// if nodes are given: check if all used nodes are given
	if (!m_nodes.empty()) {
		for (const HydraulicNetworkElement &e: m_elements) {
			if (std::find(m_nodes.begin(), m_nodes.end(), e.m_inletNodeId) == m_nodes.end())
				throw IBK::Exception(IBK::FormatString("Inlet node id #%1 used in element with id #%2 is not listed in hydraulic nodes.")
									 .arg(e.m_inletNodeId).arg(e.m_id), FUNC_ID);
			if (std::find(m_nodes.begin(), m_nodes.end(), e.m_outletNodeId) == m_nodes.end())
				throw IBK::Exception(IBK::FormatString("Outlet node id #%1 used in element with id #%2 is not listed in hydraulic nodes.")
									 .arg(e.m_outletNodeId).arg(e.m_id), FUNC_ID);
		}
	}

}


} // namespace NANDRAD
