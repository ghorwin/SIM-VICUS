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

#include "NM_ThermalNetworkBalanceModel.h"
#include "NM_ThermalNetworkStatesModel.h"
#include "NM_ThermalNetworkStatesModel.cpp"

#include <NANDRAD_ModelInputReference.h>

#include "NM_KeywordList.h"


namespace NANDRAD_MODEL {

class ThermalNetworkBalanceModelImpl {
public:
	ThermalNetworkBalanceModelImpl() { }
	~ThermalNetworkBalanceModelImpl() { }

	/*! Updates all heat fluxes through the pipes.
	*/
	int updateFluxes();

	/*! Container for flow element implementation objects.
		Size must equal the number of edges.
	*/
	std::vector<ThermalNetworkAbstractFlowElement*>	m_flowElements;
	/*! Container with mass flux references for each flow element.
	*/
	std::vector<const double*>						m_massFluxReferences;
	/*! Vector of nodes.
	*/
	std::vector<Node>								m_nodes;
	/*! Container with specific enthalpy for each node.
	*/
	std::vector<double>								m_specificEnthalpy;
};


void ThermalNetworkBalanceModel::setup(ThermalNetworkStatesModel *statesModel) {
	// copy network information and flow models from states model
	m_p->m_flowElements = statesModel->m_p->m_flowElements;
	m_p->m_nodes = statesModel->m_p->m_nodes;
	// resize results
	m_p->m_specificEnthalpy.resize(m_p->m_nodes.size(),0);
	m_heatFluxes.resize(m_p->m_flowElements.size());
}


void ThermalNetworkBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// TODO: implement
}


void ThermalNetworkBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	// first seach in m_results vector
	res.clear();
	// TODO: implement
}


const double * ThermalNetworkBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// TODO: implement
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
	inputRef.m_required = true;

	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		// generate name of the quantity
		inputRef.m_name = QuantityName("MassFlux",i);
		inputRefs.push_back(inputRef);
	}
}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	IBK_ASSERT(resultValueRefs.size() == m_p->m_flowElements.size());
	// copy references into mass flux vector
	m_p->m_massFluxReferences = resultValueRefs;
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// TODO: implement
}


int ThermalNetworkBalanceModel::update() {

	return m_p->updateFluxes();
	// sum up heat fluxes
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement *fe : m_p->m_flowElements) {
		fe->internalHeatLosses(&m_ydot[offset]);
		offset += fe->nInternalStates();
	}

	offset = 0;
	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		const ThermalNetworkAbstractFlowElement *fe = m_p->m_flowElements[i];
		// sum up
		double heatFlux = 0.0;
		for(unsigned int j = offset; j < offset + fe->nInternalStates(); ++j)
			heatFlux += m_ydot[j];
		// copy heat flux
		m_heatFluxes[i] = heatFlux;
		offset += fe->nInternalStates();
	}
}


int ThermalNetworkBalanceModel::ydot(double* ydot) {
	// get inlet heat losses from all flow elements
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement *fe : m_p->m_flowElements) {
		fe->internalHeatLosses(&m_ydot[offset]);
		offset += fe->nInternalStates();
	}
	// copy values to ydot
	std::memcpy(ydot, &m_ydot[0], m_ydot.size() * sizeof (double));
	// signal success
	return 0;
}

int ThermalNetworkBalanceModelImpl::updateFluxes() {

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
			IBK_ASSERT(m_massFluxReferences[idx] != nullptr);
			const double massFlux = *m_massFluxReferences[idx];
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
			IBK_ASSERT(m_massFluxReferences[idx] != nullptr);
			const double massFlux = *m_massFluxReferences[idx];
			if(massFlux < 0) {
				massFluxInlet -= massFlux;
				// and retrieve specfic enthalpy
				double specEnthalpy;
				m_flowElements[idx]->outletSpecificEnthalpy(specEnthalpy);
				// sum up
				specEnthalp -= massFlux * specEnthalpy;
			}
		}
		IBK_ASSERT(massFluxInlet != 0);
		specEnthalp/=massFluxInlet;

		m_specificEnthalpy[i] = specEnthalp;
	}

	// transport enthalpy flux into all flow elements
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *flowElem = m_flowElements[i];
		// get inlet node
		IBK_ASSERT(m_massFluxReferences[i] != nullptr);
		const double massFlux = *m_massFluxReferences[i];
		// positive mass flux
		if(massFlux >= 0) {
			const double specEnthalp = m_specificEnthalpy[flowElem->m_nInlet];
			flowElem->setInletFluxes(massFlux, specEnthalp * massFlux);
		}
		// negative mass flux
		else {
			const double specEnthalp = m_specificEnthalpy[flowElem->m_nOutlet];
			flowElem->setInletFluxes(massFlux, specEnthalp * massFlux);
		}
	}

}



} // namespace NANDRAD_MODEL

