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

#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>

#include <IBKMK_SparseMatrixPattern.h>

namespace NANDRAD_MODEL {

class ThermalNetworkStatesModelImpl {
public:
	ThermalNetworkStatesModelImpl() { }
	~ThermalNetworkStatesModelImpl() { }

	/*! Initialized solver based on current content of m_flowElements.
		Setup needs to be called whenever m_flowElements vector changes
		(but not, when parameters inside flow elements change!).
	*/
	void setup(const NANDRAD::HydraulicNetwork & nw);

	/*! Container for flow element implementation objects.
		Size must equal the number of edges.
	*/
	std::vector<ThermalNetworkAbstractFlowElement*>	m_flowElements;
	/*! Vector of nodes.
	*/
	std::vector<Node>					m_nodes;
};


// *** ThermalNetworkStatesModel members ***

ThermalNetworkStatesModel::~ThermalNetworkStatesModel() {
	delete m_p; // delete pimpl object
}


void ThermalNetworkStatesModel::setup(const NANDRAD::HydraulicNetwork & nw, const std::vector<NANDRAD::HydraulicNetworkComponent> & components) {
	FUNCID(ThermalNetworkStatesModel::setup);
	// create implementation instance
	m_p = new ThermalNetworkStatesModelImpl; // we take ownership

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

		std::vector<NANDRAD::HydraulicNetworkComponent>::const_iterator it =
				std::find(components.begin(), components.end(), e.m_componentId);
		IBK_ASSERT(it != components.end());

		switch (it->m_modelType) {
			case NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe :
			case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
			case NANDRAD::HydraulicNetworkComponent::MT_DynamicAdiabaticPipe :
			{
				// create hydraulic pipe model
				TNPipeElement * pipeElement = new TNPipeElement;
				// set node index
				pipeElement->m_nInlet = std::distance(nodeIds.begin(), nodeIds.find(e.m_inletNodeId));
				pipeElement->m_nOutlet = std::distance(nodeIds.begin(), nodeIds.find(e.m_outletNodeId));
				// add to flow elements
				m_p->m_flowElements.push_back(pipeElement); // transfer ownership
			} break;
			default:
			break;
		}
	}
	// setup the enetwork
	try {
		m_p->setup(nw);
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
}

void ThermalNetworkStatesModelImpl::setup(const NANDRAD::HydraulicNetwork & nw) {
	FUNCID(HydraulicNetworkModelImpl::setup);


	// count number of nodes
	unsigned int nodeCount = 0;
	for (ThermalNetworkAbstractFlowElement * fe : m_flowElements) {
		nodeCount = std::max(nodeCount, fe->m_nInlet);
		nodeCount = std::max(nodeCount, fe->m_nOutlet);
	}

	// create fast access connections between nodes and flow elements
	m_nodes.resize(nodeCount+1);
	for (unsigned int i=0; i<m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement * fe = m_flowElements[i];
		// TODO : check inlet must be different from outlet
		m_nodes[fe->m_nInlet].m_flowElementIndexesInlet.push_back(i);
		m_nodes[fe->m_nOutlet].m_flowElementIndexesOutlet.push_back(i);
	}


	// error checks:
	// 1.) no open ends
	// -> all m_nodes[i] must have at least 2 m_flowElementIndexes
	// 2.) no single cycles:
	// -> inlet must be different from outlet
	for (unsigned int i=0; i<m_nodes.size(); ++i) {
		const Node &node = m_nodes[i];
		// error check 1
		if(node.m_flowElementIndexesInlet.size() +
		   node.m_flowElementIndexesOutlet.size() == 1){
			if(!node.m_flowElementIndexesInlet.empty())
				throw IBK::Exception(IBK::FormatString(
									"FlowElement with id %1 is an open end of hydraulic network!")
									 .arg(node.m_flowElementIndexesInlet[0]),
									FUNC_ID);
			else
				throw IBK::Exception(IBK::FormatString(
									"FlowElement with id %1 is an open end of hydraulic network!")
									 .arg(node.m_flowElementIndexesOutlet[0]),
									FUNC_ID);
		}
		// error check 2
		std::set<unsigned int> indexes;
		for(unsigned int j = 0; j < node.m_flowElementIndexesOutlet.size(); ++j) {
			indexes.insert(node.m_flowElementIndexesOutlet[j]);
		}

		for(unsigned int j = 0; j < node.m_flowElementIndexesInlet.size(); ++j) {
			unsigned int elementIdx = node.m_flowElementIndexesInlet[j];
			if(indexes.find(elementIdx) != indexes.end()){
				throw IBK::Exception(IBK::FormatString(
									"FlowElement with id %1 is an invalid cyclic connection!")
									 .arg(elementIdx),
									FUNC_ID);
			}
		}
	}


	// 3.) no distinct networks
	// -> each node must connect to any other
	// -> transitive closure of connectivity must form a dense matrix
	IBKMK::SparseMatrixPattern connectivity(m_nodes.size());
#ifdef BIDIRECTIONAL

	for (unsigned int k=0; k<m_flowElements.size(); ++k) {
		ThermalNetworkAbstractFlowElement * fe = m_flowElements[k];
		// TODO : check inlet must be different from outlet
		unsigned int i = fe->m_nInlet;
		unsigned int j = fe->m_nOutlet;
		// set a pattern entry for connected nodes
		if(!connectivity.test(i,j))
			connectivity.set(i,j);
		// as well as for the transposed
		if(!connectivity.test(j,i))
			connectivity.set(j,i);
	}

	// calculate transitive closure
	for(unsigned int k = 0; k < m_nodes.size(); ++k) {
		// set a connection (i,j) for each entry (i,k), (k,j)
		std::vector<unsigned int> rows;
		std::vector<unsigned int> cols;
		connectivity.indexesPerRow(k,cols);
		connectivity.indexesPerRow(k,rows);

		// set all entries (rows[iIdx], cols[jIdx])
		for(unsigned int iIdx = 0; iIdx < rows.size(); ++iIdx) {
			unsigned int i = rows[iIdx];
			for(unsigned int jIdx = 0; jIdx < cols.size(); ++jIdx) {
				unsigned int j = cols[jIdx];
				// set entry
				if(!connectivity.test(i,j))
					connectivity.set(i,j);
				// set symmetric entry
				if(!connectivity.test(j,i))
					connectivity.set(j,i);
			}
		}
	}

#else
	IBKMK::SparseMatrixPattern connectivityTranspose(m_nodes.size());

	for (unsigned int k=0; k<m_flowElements.size(); ++k) {
		const ThermalNetworkAbstractFlowElement *fe = m_flowElements[k];
		// TODO : check inlet must be different from outlet
		unsigned int i = fe->m_nInlet;
		unsigned int j = fe->m_nOutlet;
		// set a pattern entry for connected nodes
		if(!connectivity.test(i,j))
			connectivity.set(i,j);
		// as well as for the transposed
		if(!connectivityTranspose.test(j,i))
			connectivityTranspose.set(j,i);
	}

	// calculate transitive closure
	for(unsigned int k = 0; k < m_nodes.size(); ++k) {
		// set a connection (i,j) for each entry (i,k), (k,j)
		std::vector<unsigned int> rows;
		std::vector<unsigned int> cols;
		connectivity.indexesPerRow(k,cols);
		connectivityTranspose.indexesPerRow(k,rows);

		// set all entries (rows[iIdx], cols[jIdx])
		for(unsigned int iIdx = 0; iIdx < rows.size(); ++iIdx) {
			unsigned int i = rows[iIdx];
			for(unsigned int jIdx = 0; jIdx < cols.size(); ++jIdx) {
				unsigned int j = cols[jIdx];
				// set entry
				if(!connectivity.test(i,j))
					connectivity.set(i,j);
				// set symmetric entry
				if(!connectivityTranspose.test(j,i))
					connectivityTranspose.set(j,i);
			}
		}
	}

#endif
	// now assume, that we have a dense matrix pattern for a connected graph
	for(unsigned int i = 0; i < m_nodes.size(); ++i) {
		// count column entries for each row
		std::vector<unsigned int> cols;
		connectivity.indexesPerRow(i,cols);

		// error: missing connections
		if(cols.size() != m_nodes.size()) {
			// isolated nodes are not allowed
			IBK_ASSERT(!cols.empty());

			// find out disjunct network elements
			std::vector<unsigned int> disjunctElements;
			for(unsigned int j = 0; j < cols.size(); ++j) {
				const Node &node = m_nodes[cols[j]];

				for(unsigned int k =0; k < node.m_flowElementIndexesInlet.size(); ++k)
					disjunctElements.push_back(node.m_flowElementIndexesInlet[k]);
				for(unsigned int k =0; k < node.m_flowElementIndexesOutlet.size(); ++k)
					disjunctElements.push_back(node.m_flowElementIndexesOutlet[k]);
			}

			// create an error message string
			IBK_ASSERT(!disjunctElements.empty());
			std::string networkStr(IBK::val2string<unsigned int>(disjunctElements[0]));

			for(unsigned int k = 1; k < disjunctElements.size(); ++k)
				networkStr += std::string(",") + IBK::val2string<unsigned int>(disjunctElements[k]);

			throw IBK::Exception(IBK::FormatString(
								"Network is not completely connected! Distinct network formed by flow elements (%1)!")
								 .arg(networkStr),
								FUNC_ID);
		}
	}
	IBK::IBK_Message(IBK::FormatString("Nodes:         %1\n").arg(m_nodes.size()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message(IBK::FormatString("Flow elements: %1\n").arg(m_flowElements.size()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}

} // namespace NANDRAD_MODEL
