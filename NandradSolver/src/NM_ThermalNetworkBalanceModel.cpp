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
#include "NM_HydraulicNetworkModel_p.h"
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

	// heat flux vector is a result
	QuantityDescription desc("FluidHeatFluxes", "W", "Heat flux from all flow elements into environment", false);
//	desc.resize(m_statesModel->m_elementIds, VectorValuedQuantityIndex::IK_ModelID);
//	resDesc.push_back(desc);

	// set a description for each flow element
	desc.m_name = "FluidHeatFlux";
	desc.m_displayName = m_displayName;
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

	if(!m_statesModel->m_zoneIds.empty()) {
		// set a description for each zone
		desc = QuantityDescription("ZoneHeatFluxes", "W", "Heat flux into all zones from flow elements", false);
		// add current index to description
		desc.resize(m_statesModel->m_zoneIds, VectorValuedQuantityIndex::IK_ModelID);
		resDesc.push_back(desc);
	}

	// inlet node temperature is a result
	desc = QuantityDescription("InletNodeTemperature", "C", "Inlet node temperatures of a flow elements", false);
	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

	// outlet node temperature is a result
	desc = QuantityDescription("OutletNodeTemperature", "C", "Outlet node temperatures of a flow elements", false);
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
		res.push_back(m_statesModel->m_p->m_fluidHeatFluxRefs[i]);
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_zoneHeatFluxes.size(); ++i)
		res.push_back(&m_zoneHeatFluxes[i]);
	// inlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		res.push_back(m_statesModel->m_p->m_inletNodeTemperatureRefs[i]);
	// outlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		res.push_back(m_statesModel->m_p->m_outletNodeTemperatureRefs[i]);
}


const double * ThermalNetworkBalanceModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// return vector of heat fluxes

//	if(quantityName == std::string("FluidHeatFluxes")) {
//		if(quantity.m_id == id() && quantity.m_referenceType ==
//		   NANDRAD::ModelInputReference::MRT_NETWORK) {
//				return m_statesModel->m_p->m_fluidHeatFluxRefs[0];
//		}
//		return nullptr;
//	}
	// return ydot
	if(quantityName == std::string("ydot")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return &m_ydot[0];
		return nullptr;
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


	// everything below will be reftype NETWORKELEMENT, so ignore everything else
	if (quantity.m_referenceType != NANDRAD::ModelInputReference::MRT_NETWORKELEMENT)
		return nullptr;

	// lookup element index based on given ID
	std::vector<unsigned int>::iterator fIt =
			std::find(m_statesModel->m_elementIds.begin(), m_statesModel->m_elementIds.end(), (unsigned int) quantity.m_id);
	// invalid ID?
	if (fIt == m_statesModel->m_elementIds.end())
		return nullptr;
	unsigned int pos = (unsigned int) std::distance(m_statesModel->m_elementIds.begin(), fIt);

	if (quantityName == std::string("InletNodeTemperature"))
		return m_statesModel->m_p->m_inletNodeTemperatureRefs[pos];
	else if (quantityName == std::string("OutletNodeTemperature"))
		return m_statesModel->m_p->m_outletNodeTemperatureRefs[pos];
	else if (quantityName == std::string("FluidHeatFlux"))
		return m_statesModel->m_p->m_fluidHeatFluxRefs[pos];
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
	inputRef.m_name = std::string("FluidMassFluxes");
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
	for(unsigned int i = 0; i < m_statesModel->m_zoneIds.size(); ++i) {
		// set reference to zone temperature
		m_statesModel->m_zoneTemperatureRefs[i] = resultValueRefs[1 + i];
	}
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {
		// set dependencies between fluid temperatures and mean temperature references
		resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_fluidTemperatures[i], m_statesModel->m_meanTemperatureRefs[i]));
		// set dependencies between heat exchange values and zone inputs
		if(!m_statesModel->m_zoneIdxs.empty() && m_statesModel->m_zoneIdxs[i] != (unsigned int) (-1)) {
			// zone temperature is requested
			unsigned int refIdx = m_statesModel->m_zoneIdxs[i];
			IBK_ASSERT(m_statesModel->m_zoneTemperatureRefs[refIdx] != nullptr);
			resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_heatExchangeValues[i], m_statesModel->m_zoneTemperatureRefs[refIdx]));
		}
	}

	unsigned int offset = 0;
	// we at first try use dense pattern between all element results and internal states
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];


		const Element &elem = m_statesModel->m_p->m_network->m_elements[i];
		// try to retrieve individual dependencies from flow model itself
		std::vector<std::pair<const double *, const double *> > deps;
		fe->dependencies(&m_ydot[offset], &m_statesModel->m_y[offset],
						 m_statesModel->m_p->m_fluidMassFluxes + i,
						 &m_statesModel->m_p->m_nodalSpecificEnthalpies[elem.m_nodeIndexInlet],
						 &m_statesModel->m_p->m_nodalSpecificEnthalpies[elem.m_nodeIndexOutlet],
						 deps);

		// copy dependencies
		if(!deps.empty()) {
			resultInputValueReferences.insert(resultInputValueReferences.end(), deps.begin(), deps.end());
		}
		// not implemented: assume all results to depend on all inputs
		else {
			for(unsigned int n = 0; n < fe->nInternalStates(); ++n) {
				// dependencyies to ydot
				// all elements depend on mass flux
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], m_statesModel->m_p->m_fluidMassFluxes + i) );

				if(m_statesModel->m_p->m_fluidHeatFluxRefs[i] != nullptr)
					// all elements depend on heat flux
					resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], m_statesModel->m_p->m_fluidHeatFluxRefs[i]) );
				// all elements depend on external conditions
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_heatExchangeValues[i]) );

				// assume a dense matrix between ydot and y
				for(unsigned int l = 0; l < fe->nInternalStates(); ++l) {
					resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_y[offset + l]) );
				}

				// heat balance per default sums up heat fluxes and entahpy flux differences through the pipe
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet]) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet]) );

				// dependency to Qdot
				if(m_statesModel->m_p->m_fluidHeatFluxRefs[i] != nullptr)
					resultInputValueReferences.push_back(std::make_pair(m_statesModel->m_p->m_fluidHeatFluxRefs[i],
																		&m_statesModel->m_y[offset + n] ) );
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

		IBK_ASSERT(m_statesModel->m_zoneIdxs.size() == m_statesModel->m_p->m_fluidHeatFluxRefs.size());

		for(unsigned int i = 0; i < m_statesModel->m_zoneIdxs.size(); ++i) {
			unsigned int index = m_statesModel->m_zoneIdxs[i];
			// invaid index
			if(index == (unsigned int)(-1))
				continue;
			IBK_ASSERT(m_statesModel->m_p->m_fluidHeatFluxRefs[i] != nullptr);
			// we sum up heat flux of all zones
			IBK_ASSERT(index < m_zoneHeatFluxes.size());
			m_zoneHeatFluxes[index] += *m_statesModel->m_p->m_fluidHeatFluxRefs[i];
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
	for (unsigned int i=0; i<m_statesModel->m_p->m_fluidHeatFluxRefs.size(); ++i) {
		// skip adiabatic models
		if(m_statesModel->m_p->m_fluidHeatFluxRefs[i] == nullptr)
			continue;
		std::cout << "  " << i << "   " << m_statesModel->m_p->m_fluidHeatFluxRefs[i]  << std::endl;
	}

	std::cout << "Fluid tempertaures [C]" << std::endl;
	for (unsigned int i=0; i<m_statesModel->m_fluidTemperatures.size(); ++i)
		std::cout << "  " << i << "   " << m_statesModel->m_fluidTemperatures[i] - 273.15 << std::endl;
}




} // namespace NANDRAD_MODEL
