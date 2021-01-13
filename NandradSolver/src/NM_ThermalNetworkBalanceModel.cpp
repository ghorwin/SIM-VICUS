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

#include "NM_HydraulicNetworkModel.h"
#include "NM_ThermalNetworkBalanceModel.h"
#include "NM_ThermalNetworkStatesModel.h"

#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



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
	// resize vectors
	m_ydot.resize(m_statesModel->nPrimaryStateResults());
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
	for(unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		res.push_back(&m_statesModel->m_p->m_heatFluxes[i]);
}


const double * ThermalNetworkBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// return vector of heat fluxes
	if(quantityName == std::string("HeatFlux")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return  &m_statesModel->m_p->m_heatFluxes[0];
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
		return &m_statesModel->m_p->m_heatFluxes[index];
	}
	// return ydot
	if(quantityName == std::string("ydot")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return &m_ydot[0];
		return nullptr;
	}
	return nullptr;
}


int ThermalNetworkBalanceModel::priorityOfModelEvaluation() const {
	// network balance model is evaluated one step before construction balance model
	return AbstractStateDependency::priorityOffsetTail+3;
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
	// register reference
	inputRefs.push_back(inputRef);
}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	IBK_ASSERT(resultValueRefs.size() == 1);
	// copy references into mass flux vector
	m_statesModel->m_p->m_massFluxes = resultValueRefs[0];
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	unsigned int offset = 0;
	// we at first try use dense pattern between all element results and internal states
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];
		// try to retrieve individual dependencies from flow model itself
		std::vector<std::pair<const double *, const double *> > deps;
		fe->dependencies(&m_ydot[offset], &m_statesModel->m_y[offset], deps);

		// copy dependencies
		if(!deps.empty()) {
			resultInputValueReferences.insert(resultInputValueReferences.end(), deps.begin(), deps.end());
		}
		// not implemented: assume all results to depend on all inputs
		else {
			const Element &elem = m_statesModel->m_p->m_network->m_elements[i];
			for(unsigned int j = 0; j < fe->nInternalStates(); ++j) {
				// outlet specific enthalpy is a result quantity dependending on all internal states
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_specificEnthalpies[elem.m_nInlet], &m_statesModel->m_y[offset + j]) );
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_specificEnthalpies[elem.m_nOutlet], &m_statesModel->m_y[offset + j]) );
				// heat balance per default sums up heat fluxes and entahpy flux differences through the pipe
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], m_statesModel->m_p->m_massFluxes + i) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], &m_statesModel->m_p->m_heatFluxes[i] ) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], &m_statesModel->m_p->m_specificEnthalpies[elem.m_nInlet]) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], &m_statesModel->m_p->m_specificEnthalpies[elem.m_nOutlet]) );
			}
		}
		offset += fe->nInternalStates();
	}
	// ertrieve refereces for network model results
	m_statesModel->m_p->dependencies(resultInputValueReferences);
}


int ThermalNetworkBalanceModel::update() {

	//update all network internal calulation quantities
	int res = m_statesModel->m_p->updateStates();

	if(res != 0)
		return res;
	// TODO: call m_statesModel->m_p->
	res = m_statesModel->m_p->updateFluxes();

	if(res != 0)
		return res;

	// sum up heat fluxes
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement *fe : m_statesModel->m_p->m_flowElements) {
		fe->internalDerivatives(&m_ydot[offset]);
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



} // namespace NANDRAD_MODEL
