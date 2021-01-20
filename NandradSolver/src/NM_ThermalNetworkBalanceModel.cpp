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
#include <iostream>

namespace NANDRAD_MODEL {



void ThermalNetworkBalanceModel::setup(ThermalNetworkStatesModel *statesModel) {
	// copy states model pointer
	m_statesModel = statesModel;
	// create id vector for later access to heat fluxes
	IBK_ASSERT(m_statesModel->m_network != nullptr);
	IBK_ASSERT(!m_statesModel->m_network->m_elements.empty());
	IBK_ASSERT(m_statesModel->m_network->m_elements.size() ==
			   m_statesModel->m_p->m_flowElements.size());

	// resize heat fluxes to zones
	if(!statesModel->m_zoneIds.empty())
		m_zoneHeatFluxes.resize(statesModel->m_zoneIds.size());

	// resize vectors
	m_ydot.resize(m_statesModel->nPrimaryStateResults());
}


void ThermalNetworkBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	if(!resDesc.empty())
		resDesc.clear();
	// heat flux vector is a result
	QuantityDescription desc("FluidHeatFlux", "W", "Heat flux from all flow elements into environment", false);
	// deactivate description;
	if(m_statesModel->m_elementIds.empty())
		desc.m_size = 0;
	else
		desc.resize(m_statesModel->m_elementIds, VectorValuedQuantityIndex::IK_ModelID);
	resDesc.push_back(desc);

	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

	if(!m_statesModel->m_zoneIds.empty()) {
		// set a description for each zone
		desc = QuantityDescription("ZoneHeatFlux", "W", "Heat flux into all zones from flow elements", false);
		// add current index to description
		desc.resize(m_statesModel->m_zoneIds, VectorValuedQuantityIndex::IK_ModelID);
		resDesc.push_back(desc);
	}

	// inlet node temperature is a result
	desc = QuantityDescription("InletNodeTemperature", "C", "Inlet node temperatures of all flow elements", false);
	// deactivate description;
	if(m_statesModel->m_elementIds.empty())
		desc.m_size = 0;
	else
		desc.resize(m_statesModel->m_elementIds, VectorValuedQuantityIndex::IK_ModelID);
	resDesc.push_back(desc);

	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

	// outlet node temperature is a result
	desc = QuantityDescription("OutletNodeTemperature", "C", "Outlet node temperatures of all flow elements", false);
	// deactivate description;
	if(m_statesModel->m_elementIds.empty())
		desc.m_size = 0;
	else
		desc.resize(m_statesModel->m_elementIds, VectorValuedQuantityIndex::IK_ModelID);
	resDesc.push_back(desc);

	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

}


void ThermalNetworkBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	if(!res.empty())
		res.clear();
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		res.push_back(&m_statesModel->m_p->m_fluidHeatFluxes[i]);
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_zoneHeatFluxes.size(); ++i)
		res.push_back(&m_zoneHeatFluxes[i]);
	// inlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		res.push_back(&m_statesModel->m_p->m_inletNodeTemperatures[i]);
	// outlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		res.push_back(&m_statesModel->m_p->m_outletNodeTemperatures[i]);
}


const double * ThermalNetworkBalanceModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// return vector of heat fluxes
	if(quantityName == std::string("FluidHeatFlux")) {
		if(quantity.m_id == id() && quantity.m_referenceType ==
		   NANDRAD::ModelInputReference::MRT_NETWORK) {
			if(!m_statesModel->m_p->m_fluidHeatFluxes.empty())
				return &m_statesModel->m_p->m_fluidHeatFluxes[0];
			return nullptr;
		}
		IBK_ASSERT(quantity.m_referenceType == NANDRAD::ModelInputReference::MRT_NETWORKELEMENT);
		// access to an element heat flux
		std::vector<unsigned int>::iterator fIt =
				std::find(m_statesModel->m_elementIds.begin(),m_statesModel-> m_elementIds.end(),
						  (unsigned int) quantity.m_id);
		// invalid index access
		if(fIt == m_statesModel->m_elementIds.end())
			return nullptr;
		unsigned int index = (unsigned int) std::distance(m_statesModel->m_elementIds.begin(), fIt);
		IBK_ASSERT(index < m_statesModel->m_n);
		return &m_statesModel->m_p->m_fluidHeatFluxes[index];
	}
	if(quantityName.m_name == std::string("ZoneHeatFlux")) {
		// no zones are given
		if(m_statesModel->m_zoneIds.empty())
			return nullptr;
		// find zone id
		std::vector<unsigned int>::iterator fIt =
			std::find(m_statesModel->m_zoneIds.begin(), m_statesModel->m_zoneIds.end(),
				  (unsigned int) quantityName.m_index);
		// invalid index access
		if(fIt == m_statesModel->m_zoneIds.end())
			return nullptr;

		unsigned int index = (unsigned int) std::distance(m_statesModel->m_zoneIds.begin(), fIt);
		// found a valid entry
		return &m_zoneHeatFluxes[index];
	}
	// return vector of inlet node temperatures
	if(quantityName == std::string("InletNodeTemperature")) {
		if(quantity.m_id == id() && quantity.m_referenceType ==
		   NANDRAD::ModelInputReference::MRT_NETWORK) {
			if(!m_statesModel->m_p->m_inletNodeTemperatures.empty())
				return  &m_statesModel->m_p->m_inletNodeTemperatures[0];
			return nullptr;
		}
		IBK_ASSERT(quantity.m_referenceType == NANDRAD::ModelInputReference::MRT_NETWORKELEMENT);
		// find component id
		std::vector<unsigned int>::iterator fIt =
				std::find(m_statesModel->m_elementIds.begin(),m_statesModel-> m_elementIds.end(),
						  (unsigned int) quantity.m_id);
		// invalid index access
		if(fIt == m_statesModel->m_elementIds.end())
			return nullptr;

		unsigned int index = (unsigned int) std::distance(m_statesModel->m_elementIds.begin(), fIt);
		IBK_ASSERT(index < m_statesModel->m_n);
		// found a valid entry
		return &m_statesModel->m_p->m_inletNodeTemperatures[index];
	}
	// return vector of outlet node temperatures
	if(quantityName == std::string("OutletNodeTemperature")) {
		if(quantity.m_id == id() && quantity.m_referenceType ==
		   NANDRAD::ModelInputReference::MRT_NETWORK) {
			if(!m_statesModel->m_p->m_outletNodeTemperatures.empty())
				return  &m_statesModel->m_p->m_outletNodeTemperatures[0];
			return nullptr;
		}
		IBK_ASSERT(quantity.m_referenceType == NANDRAD::ModelInputReference::MRT_NETWORKELEMENT);
		// find component id
		std::vector<unsigned int>::iterator fIt =
			std::find(m_statesModel->m_elementIds.begin(),m_statesModel-> m_elementIds.end(),
				  (unsigned int) quantity.m_id);
		// invalid index access
		if(fIt == m_statesModel->m_elementIds.end())
			return nullptr;

		unsigned int index = (unsigned int) std::distance(m_statesModel->m_elementIds.begin(), fIt);
		IBK_ASSERT(index < m_statesModel->m_n);
		// found a valid entry
		return &m_statesModel->m_p->m_outletNodeTemperatures[index];
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

void ThermalNetworkBalanceModel::initInputReferences(const std::vector<AbstractModel *> & /*models*/) {
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
	inputRef.m_name = std::string("FluidMassFlux");
	inputRef.m_required = true;
	// register reference
	inputRefs.push_back(inputRef);

	// set references to zone air temperatures
	if(!m_statesModel->m_zoneIds.empty()) {
		InputReference inputRef;
		inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		inputRef.m_name = std::string("AirTemperature");
		inputRef.m_required = true;
		for(unsigned int zoneId : m_statesModel->m_zoneIds) {
			inputRef.m_id = zoneId;
			inputRefs.push_back(inputRef);
		}
	}
}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	// copy references into mass flux vector
	m_statesModel->m_p->m_fluidMassFluxes = resultValueRefs[0];
	//set ambient temparture references inside network
	for(unsigned int i = 0; i < m_statesModel->m_zoneIdxs.size(); ++i) {
		unsigned int zoneIdx = m_statesModel->m_zoneIdxs[i];
		// skip invalid zone ids
		if(zoneIdx == (unsigned int)(-1))
			continue;
		// set reference to zone temperature
		m_statesModel->m_p->m_ambientTemperatureRefs[i] = resultValueRefs[1 + zoneIdx];
	}
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	// set depndency between y and fluid temperatures
	const double *y = &m_statesModel->m_y[0];

	unsigned int offset = 0;
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {
		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];
		unsigned int nStates = fe->nInternalStates();
		for(unsigned int n = 0; n < nStates; ++n) {
			resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_fluidTemperatures[i], y + offset + n));
		}
		offset += nStates;
	}

	offset = 0;
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
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_nodalSpecificEnthalpies[elem.m_nInlet], &m_statesModel->m_y[offset + j]) );
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_nodalSpecificEnthalpies[elem.m_nOutlet], &m_statesModel->m_y[offset + j]) );
				// heat balance per default sums up heat fluxes and entahpy flux differences through the pipe
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], m_statesModel->m_p->m_fluidMassFluxes + i) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], &m_statesModel->m_p->m_fluidHeatFluxes[i] ) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], &m_statesModel->m_p->m_nodalSpecificEnthalpies[elem.m_nInlet]) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + j], &m_statesModel->m_p->m_nodalSpecificEnthalpies[elem.m_nOutlet]) );
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


	// update zone specific fluxes
	if(!m_statesModel->m_zoneIds.empty()) {
		// set zone heat fluxes to 0
		std::fill(m_zoneHeatFluxes.begin(), m_zoneHeatFluxes.end(), 0);

		IBK_ASSERT(m_statesModel->m_zoneIdxs.size() == m_statesModel->m_p->m_fluidHeatFluxes.size());

		for(unsigned int i = 0; i < m_statesModel->m_zoneIdxs.size(); ++i) {
			unsigned int index = m_statesModel->m_zoneIdxs[i];
			// invaid index
			if(index == (unsigned int)(-1))
				continue;
			// we sum up heat flux of all zones
			IBK_ASSERT(index < m_zoneHeatFluxes.size());
			m_zoneHeatFluxes[index] += m_statesModel->m_p->m_fluidHeatFluxes[i];
		}
	}

	// update derivatives
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement *fe : m_statesModel->m_p->m_flowElements) {
		fe->internalDerivatives(&m_ydot[offset]);
		offset += fe->nInternalStates();
	}

#ifdef NANDRAD_NETWORK_DEBUG_OUTPUTS
	printVars();
#endif // NANDRAD_NETWORK_DEBUG_OUTPUTS

	return 0;
}


int ThermalNetworkBalanceModel::ydot(double* ydot) {
	// get inlet heat losses from all flow elements
	// copy values to ydot
	std::memcpy(ydot, &m_ydot[0], m_ydot.size() * sizeof (double));
	// signal success
	return 0;
}

void ThermalNetworkBalanceModel::printVars() const {
	std::cout << "Heat fluxes [W]" << std::endl;
	for (unsigned int i=0; i<m_statesModel->m_p->m_fluidHeatFluxes.size(); ++i)
		std::cout << "  " << i << "   " << m_statesModel->m_p->m_fluidHeatFluxes[i]  << std::endl;

	std::cout << "Fluid tempertaures [C]" << std::endl;
	for (unsigned int i=0; i<m_statesModel->m_fluidTemperatures.size(); ++i)
		std::cout << "  " << i << "   " << m_statesModel->m_fluidTemperatures[i] - 273.15 << std::endl;
}




} // namespace NANDRAD_MODEL
