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

#include "NM_ThermalNetworkPrivate.h"

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



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
