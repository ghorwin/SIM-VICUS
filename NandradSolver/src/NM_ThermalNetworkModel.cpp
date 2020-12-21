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
#include "NM_ThermalNetworkBalanceModel.h"

#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {

class ThermalNetworkModelImpl {
public:
	ThermalNetworkModelImpl() { }
	~ThermalNetworkModelImpl() { }

	/*! Constant access to heat flux vector*/
	const double *heatFluxes() const;

	/*! Initialized solver based on current content of m_flowElements.
		Setup needs to be called whenever m_flowElements vector changes
		(but not, when parameters inside flow elements change!).
	*/
	void setup();

	/*! Updates all states at network nodes.
	*/
	int updateStates();

	/*! Updates all heat fluxes through the pipes.
	*/
	int updateFluxes();

	/*! Container for flow element implementation objects.
		Size must equal the number of edges.
	*/
	std::vector<ThermalNetworkAbstractFlowElement*>	m_flowElements;

	/*! Container with global pointer to calculated mass fluxes.
	*/
	const double									*m_massFluxes;

private:
	/*! Stores connectivity information. */
	struct Node {
		Node() {}
		Node(unsigned int i1, unsigned int i2) {
			m_flowElementIndexesInlet.push_back(i1);
			m_flowElementIndexesOutlet.push_back(i2);
		}

		/*! Vector with indexes of inlet flow elements. */
		std::vector<unsigned int> m_flowElementIndexesInlet;
		std::vector<unsigned int> m_flowElementIndexesOutlet;
	};


	/*! Vector of nodes.
	*/
	std::vector<Node>								m_nodes;
	/*! Container with specific enthalpy for each node.
	*/
	std::vector<double>								m_specificEnthalpy;
	/*! Vector result heat fluxes. */
	std::vector<double>								m_heatFluxes;
};


// *** ThermalNetworkStatesModel members ***

ThermalNetworkStatesModel::~ThermalNetworkStatesModel() {
	delete m_p; // delete pimpl object
}


void ThermalNetworkStatesModel::setup(const NANDRAD::HydraulicNetwork & nw, const std::vector<NANDRAD::HydraulicNetworkComponent> & components) {
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
		m_p->setup();
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


void ThermalNetworkBalanceModel::setup(ThermalNetworkStatesModel *statesModel) {
	// copy states model pointer
	m_statesModel = statesModel;
	// create id vector for later access to heat fluxes
	IBK_ASSERT(m_statesModel->m_network != nullptr);
	IBK_ASSERT(!m_statesModel->m_network->m_elements.empty());
	IBK_ASSERT(m_statesModel->m_network->m_elements.size() ==
			   m_statesModel->m_p->m_flowElements.size());

	// fill ids
	for(const NANDRAD::HydraulicNetworkElement &elem : m_statesModel->m_network->m_elements) {
		m_componentIds.push_back(elem.m_componentId);
	}
}


void ThermalNetworkBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	if(!resDesc.empty())
		resDesc.clear();
	// mass flux vector is a result
	QuantityDescription desc("HeatFlux", "W", "Heat flux from all flow elements into environment", false);
	// deactivate description;
	if(m_componentIds.empty())
		desc.m_size = 0;
	else
		desc.resize(m_componentIds, VectorValuedQuantityIndex::IK_ModelID);
	resDesc.push_back(desc);
}


void ThermalNetworkBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	if(!res.empty())
		res.clear();
	// mass flux vector is a result quantity
	if(m_statesModel->m_p->heatFluxes() != nullptr) {
		for(unsigned int i = 0; i < m_statesModel->m_n; ++i)
		res.push_back(m_statesModel->m_p->heatFluxes() + i);
	}
}


const double * ThermalNetworkBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// return vector of heat fluxes
	if(quantityName == std::string("HeatFlux")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return m_statesModel->m_p->heatFluxes();
		// find component id
		std::vector<unsigned int>::const_iterator it =
				std::find(m_componentIds.begin(), m_componentIds.end(),
						  quantityName.m_index);
		// not found
		if(it == m_componentIds.end())
			return nullptr;

		unsigned int index = std::distance(m_componentIds.begin(), it);
		IBK_ASSERT(index < m_statesModel->m_n);
		// found a valid entry
		return m_statesModel->m_p->heatFluxes() + index;
	}
	return nullptr;
}


int ThermalNetworkBalanceModel::priorityOfModelEvaluation() const {
	// TODO: implement
	return -1;
}


void ThermalNetworkBalanceModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// TODO: implement
}


void ThermalNetworkBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// set input references to hydraulic network calculation
	if(!inputRefs.empty())
		inputRefs.clear();
	// use hydraulic network model to generate mass flux references
	InputReference inputRef;
	inputRef.m_id = id();
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORK;
	inputRef.m_name = std::string("MassFlux");
	inputRef.m_required = true;
}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	IBK_ASSERT(resultValueRefs.size() == 1);
	// copy references into mass flux vector
	m_statesModel->m_p->m_massFluxes = resultValueRefs[0];
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// TODO: implement
}


int ThermalNetworkBalanceModel::update() {

	//update all network internal calulation quantities
	int res = m_statesModel->m_p->updateStates();

	if(res != 0)
		return res;
	res = m_statesModel->m_p->updateFluxes();

	if(res != 0)
		return res;

	// sum up heat fluxes
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement *fe : m_statesModel->m_p->m_flowElements) {
		fe->internalHeatLosses(&m_ydot[offset]);
		offset += fe->nInternalStates();
	}

	return 0;
}


int ThermalNetworkBalanceModel::ydot(double* ydot) {
	// get inlet heat losses from all flow elements
	// copy values to ydot
	std::memcpy(ydot, &m_ydot[0], m_ydot.size() * sizeof (double));
	// signal success
	return 0;
}


const double *ThermalNetworkModelImpl::heatFluxes() const {
	if(!m_heatFluxes.empty())
		return &m_heatFluxes[0];
	return nullptr;
}

void ThermalNetworkModelImpl::setup() {
	FUNCID(ThermalNetworkModelImpl::setup);

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

	// resize specific enthalpy
	m_specificEnthalpy.resize(m_nodes.size());
	// resize heat fluxes
	m_heatFluxes.resize(m_flowElements.size());
}


int ThermalNetworkModelImpl::updateStates() {

	// calculate enthalpy fluxes for all nodes
	for(unsigned int i = 0; i < m_nodes.size(); ++i) {
		// set enthalpy flux to 0
		double specEnthalp = 0;

		std::vector<unsigned int> inletIdxs =
				m_nodes[i].m_flowElementIndexesInlet;
		std::vector<unsigned int> outletIdxs =
				m_nodes[i].m_flowElementIndexesOutlet;

		double massFluxInlet = 0.0;
		// select all pipes with positive flux into element
		for(unsigned int idx : inletIdxs) {
			const double massFlux = m_massFluxes[idx];
			if(massFlux > 0) {
				massFluxInlet += massFlux;
				// and retrieve specfic enthalpy
				double specEnthalpy;
				m_flowElements[idx]->outletSpecificEnthalpy(specEnthalpy);
				// sum up
				specEnthalp += massFlux * specEnthalpy;
			}
		}
		// select all pipes with negative flux into element
		for(unsigned int idx : outletIdxs) {
			const double massFlux = m_massFluxes[idx];
			if(massFlux < 0) {
				massFluxInlet -= massFlux;
				// and retrieve specfic enthalpy
				double specEnthalpy;
				m_flowElements[idx]->outletSpecificEnthalpy(specEnthalpy);
				// sum up
				specEnthalp -= massFlux * specEnthalpy;
			}
		}
		IBK_ASSERT(massFluxInlet != 0.0);
		specEnthalp/=massFluxInlet;

		m_specificEnthalpy[i] = specEnthalp;
	}
	return 0;
}

int ThermalNetworkModelImpl::updateFluxes() 	{
	// set enthalpy and mass fluxes for all flow elements
	// and update their simulation results
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *flowElem = m_flowElements[i];
		// get inlet node
		const double massFlux = m_massFluxes[i];
		const double specEnthalpInlet = m_specificEnthalpy[flowElem->m_nInlet];
		const double specEnthalpOutlet = m_specificEnthalpy[flowElem->m_nOutlet];
		// positive mass flux
		if(massFlux >= 0) {
			flowElem->setInletFluxes(massFlux, specEnthalpInlet * massFlux);
		}
		// negative mass flux
		else {
			flowElem->setInletFluxes(massFlux, specEnthalpOutlet * massFlux);
		}
		// heat los equals difference of enthalpy fluxes between inlet and outlet
		m_heatFluxes[i] = massFlux * (specEnthalpInlet - specEnthalpOutlet);
	}
	return 0;
}



} // namespace NANDRAD_MODEL
