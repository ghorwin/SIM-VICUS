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

#include "NM_ThermalNetworkStatesModel.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_ThermalNetworkPrivate.h"
#include "NM_ThermalNetworkFlowElements.h"
#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



// *** ThermalNetworkStatesModel members ***

ThermalNetworkStatesModel::~ThermalNetworkStatesModel() {
	delete m_p; // delete pimpl object
}


void ThermalNetworkStatesModel::setup(const NANDRAD::HydraulicNetwork & nw,
									  const HydraulicNetworkModel &networkModel) {
	FUNCID(ThermalNetworkStatesModel::setup);

	// store network pointer
	m_network = &nw;
	// create implementation instance
	m_p = new ThermalNetworkModelImpl; // we take ownership

	// first register all nodes
	std::set<unsigned int> nodeIds;
	// for this purpose process all hydraulic network elements
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		nodeIds.insert(e.m_inletNodeId);
		nodeIds.insert(e.m_outletNodeId);
	}

	// now populate the m_edges vector of the network solver

	// process all hydraulic network elements and instatiate respective flow equation classes
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		// - instance-specific parameters from HydraulicNetworkElement e
		// - fluid property object from nw.m_fluid
		// - component definition (via reference from e.m_componentId) and component DB stored
		//   in network

		// retrieve component

		std::vector<NANDRAD::HydraulicNetworkComponent>::const_iterator itComp =
				std::find(nw.m_components.begin(), nw.m_components.end(), e.m_componentId);
		IBK_ASSERT(itComp != nw.m_components.end());

		switch (itComp->m_modelType) {
			case NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe :
			case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
			case NANDRAD::HydraulicNetworkComponent::MT_DynamicAdiabaticPipe :
			{
				// lookup pipe
				std::vector<NANDRAD::HydraulicNetworkPipeProperties>::const_iterator itPipe =
						std::find(m_network->m_pipeProperties.begin(), m_network->m_pipeProperties.end(), e.m_pipeId);
				if (itPipe == m_network->m_pipeProperties.end()) {
					throw IBK::Exception(IBK::FormatString("Missing pipe properties reference in hydraulic network element '%1' (id=%2).")
										 .arg(e.m_displayName).arg(e.m_id), FUNC_ID);
				}
				// create hydraulic pipe model
				TNPipeElement * pipeElement = new TNPipeElement(e, *itComp,  *itPipe, m_network->m_fluid);
				// add to flow elements
				m_p->m_flowElements.push_back(pipeElement); // transfer ownership
			} break;
			default:
			break;
		}
	}
	// setup the enetwork
	try {
		m_p->setup(*networkModel.network());
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error setting up flow network.", FUNC_ID);
	}

	// TODO
	// initialize all models

	// resize vectors
	m_n = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		m_n += fe->nInternalStates();
	}
	m_y.resize(m_n,0.0);
}


void ThermalNetworkStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	// TODO: implement
}


const double * ThermalNetworkStatesModel::resultValueRef(const QuantityName & quantityName) const {
	// TODO: implement
	return nullptr;
}


unsigned int ThermalNetworkStatesModel::nPrimaryStateResults() const {
	return m_n;
}


void ThermalNetworkStatesModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// TODO: implement
}


void ThermalNetworkStatesModel::yInitial(double * y) const {
	// TODO: implement
}


int ThermalNetworkStatesModel::update(const double * y) {
	// copy states vector
	std::memcpy(&m_y[0], y, m_n*sizeof(double));
	// set internal states
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->setInternalStates(y + offset);
		offset += fe->nInternalStates();
	}
	return 0;
}


} // namespace NANDRAD_MODEL
