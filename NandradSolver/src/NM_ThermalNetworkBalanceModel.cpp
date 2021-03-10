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

	// add individual model result value references

	for(unsigned int elemIdx = 0; elemIdx < m_statesModel->m_elementIds.size(); ++elemIdx) {
		// add offset
		m_modelQuantityOffset.push_back(m_modelQuantities.size());
		// retrieve current flow element
		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[elemIdx];
		fe->modelQuantities(m_modelQuantities);
		fe->modelQuantityValueRefs(m_modelQuantityRefs);
		// correct type and id of quantity description
		for(unsigned int i = m_modelQuantityOffset.back(); i < m_modelQuantities.size(); ++i) {
			m_modelQuantities[i].m_id = m_statesModel->m_elementIds[elemIdx];
			m_modelQuantities[i].m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		}
		// implementation check
		IBK_ASSERT(m_modelQuantities.size() == m_modelQuantityRefs.size());
	}
	// mark end of vector
	m_modelQuantityOffset.push_back(m_modelQuantities.size());

	// resize heat fluxes to zones
	if(!statesModel->m_zoneIds.empty())
		m_networkZoneHeatLoad.resize(statesModel->m_zoneIds.size());
	// resize heat fluxes to construction layers
	if(!statesModel->m_constructionInstanceIds.empty())
		m_networkActiveLayerHeatLoad.resize(statesModel->m_constructionInstanceIds.size());

	// resize vectors
	m_ydot.resize(m_statesModel->nPrimaryStateResults());
}


void ThermalNetworkBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	// publish heat loss from flow element towards environment
	QuantityDescription desc("FlowElementHeatLoss", "W", "Heat flux from flow element into environment", false);

	// TODO : Anne, update descriptions below

	// set a description for each flow element
	desc.m_displayName = m_displayName;
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

	// add output
	if (!m_statesModel->m_zoneIds.empty()) {
		// set a description for each zone

		// TODO : rename?
		desc = QuantityDescription("NetworkZoneHeatLoad", "W", "Complete Heat load to zones from all hydraulic network elements", false);
		// add current index to description
		desc.resize(m_statesModel->m_zoneIds, VectorValuedQuantityIndex::IK_ModelID);
		resDesc.push_back(desc);
	}
	if (!m_statesModel->m_constructionInstanceIds.empty()) {
		// set a description for each construction

		// TODO : rename?
		desc = QuantityDescription("NetworkActiveLayerHeatLoad", "W", "Heat load to the construction layers from all hydraulic network elements", false);
		// add current index to description
		desc.resize(m_statesModel->m_constructionInstanceIds, VectorValuedQuantityIndex::IK_ModelID);
		resDesc.push_back(desc);
	}

	// inlet node temperature is a result
	desc = QuantityDescription("InletNodeTemperature", "C", "Inlet node temperature of a flow element", false);
	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

	// outlet node temperature is a result
	desc = QuantityDescription("OutletNodeTemperature", "C", "Outlet node temperature of a flow element", false);
	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_statesModel->m_elementIds.size(); ++i) {
		desc.m_id = m_statesModel->m_elementIds[i];
		resDesc.push_back(desc);
	}

	// add individual model results
	if(!m_modelQuantities.empty())
		resDesc.insert(resDesc.end(), m_modelQuantities.begin(), m_modelQuantities.end());
}


void ThermalNetworkBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	if(!res.empty())
		res.clear();
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_elementValueRefs.m_nValues; ++i)
		res.push_back(m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i]);
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_networkZoneHeatLoad.size(); ++i)
		res.push_back(&m_networkZoneHeatLoad[i]);
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_networkActiveLayerHeatLoad.size(); ++i)
		res.push_back(&m_networkActiveLayerHeatLoad[i]);
	// inlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_elementValueRefs.m_nValues; ++i)
		res.push_back(m_statesModel->m_elementValueRefs.m_inletNodeTemperatureRefs[i]);
	// outlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_statesModel->m_elementValueRefs.m_nValues; ++i)
		res.push_back(m_statesModel->m_elementValueRefs.m_outletNodeTemperatureRefs[i]);
	// add individual model result value references
	if(!m_modelQuantityRefs.empty())
		res.insert(res.end(), m_modelQuantityRefs.begin(), m_modelQuantityRefs.end());
}


const double * ThermalNetworkBalanceModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;

	// return ydot
	if(quantityName == std::string("ydot")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return &m_ydot[0];
		return nullptr;
	}
	if(quantityName.m_name == std::string("NetworkZoneHeatLoad")) {
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
		return &m_networkZoneHeatLoad[index];
	}
	if(quantityName.m_name == std::string("NetworkActiveLayerHeatLoad")) {
		// no zones are given
		if(m_statesModel->m_constructionInstanceIds.empty())
			return nullptr;
		// find zone id
		std::vector<unsigned int>::iterator fIt =
			std::find(m_statesModel->m_constructionInstanceIds.begin(),
					  m_statesModel->m_constructionInstanceIds.end(),
				  (unsigned int) quantityName.m_index);
		// invalid index access
		if(fIt == m_statesModel->m_constructionInstanceIds.end())
			return nullptr;

		unsigned int index = (unsigned int) std::distance(m_statesModel->m_constructionInstanceIds.begin(), fIt);
		// found a valid entry
		return &m_networkActiveLayerHeatLoad[index];
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
		return m_statesModel->m_elementValueRefs.m_inletNodeTemperatureRefs[pos];
	else if (quantityName == std::string("OutletNodeTemperature"))
		return m_statesModel->m_elementValueRefs.m_outletNodeTemperatureRefs[pos];
	else if (quantityName == std::string("FlowElementHeatLoss"))
		return m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[pos];

	// search for quantity inside individual element results
	IBK_ASSERT(pos < m_modelQuantityOffset.size() - 1);
	unsigned int startIdx = m_modelQuantityOffset[pos];
	unsigned int endIdx = m_modelQuantityOffset[pos + 1];

	// check if element contains requested quantity
	for(unsigned int resIdx = startIdx; resIdx < endIdx; ++resIdx) {
		const QuantityDescription &modelDesc = m_modelQuantities[resIdx];
		if(modelDesc.m_name == quantityName.m_name) {
			// index is not allowed for network element output
			if(quantityName.m_index != -1)
				return nullptr;
			return m_modelQuantityRefs[resIdx];
		}
	}

	return nullptr;
}


int ThermalNetworkBalanceModel::priorityOfModelEvaluation() const {
	// network balance model is evaluated one step before construction balance model
	return AbstractStateDependency::priorityOffsetTail+3;
}

void ThermalNetworkBalanceModel::initInputReferences(const std::vector<AbstractModel *> & /*models*/) {
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
	// set references to construction layer temperatures
	if(!m_statesModel->m_constructionInstanceIds.empty()) {
		InputReference inputRef;
		inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		inputRef.m_name = std::string("ActiveLayerTemperature");
		inputRef.m_required = true;
		for(unsigned int conInstanceId : m_statesModel->m_constructionInstanceIds) {
			inputRef.m_id = conInstanceId;
			inputRefs.push_back(inputRef);
		}
	}
}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	// copy references into mass flux vector
	m_statesModel->m_p->m_fluidMassFluxes = resultValueRefs[0];
	//set zone temparture references inside network
	for(unsigned int i = 0; i < m_statesModel->m_zoneIds.size(); ++i) {
		// set reference to zone temperature
		m_statesModel->m_zoneTemperatureRefs[i] = resultValueRefs[1 + i];
	}
	//set active layer references inside network
	for(unsigned int i = 0; i < m_statesModel->m_constructionInstanceIds.size(); ++i) {
		// set reference to zone temperature
		m_statesModel->m_activeLayerTemperatureRefs[i] = resultValueRefs[1 + i];
	}
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	for(unsigned int i = 0; i < m_statesModel->m_elementValueRefs.m_nValues; ++i) {
		// set dependencies between heat exchange values and zone inputs
		if(!m_statesModel->m_elementValueRefs.m_zoneIdxs.empty() &&
		   m_statesModel->m_elementValueRefs.m_zoneIdxs[i] != NANDRAD::INVALID_ID) {
			// zone temperature is requested
			unsigned int refIdx = m_statesModel->m_elementValueRefs.m_zoneIdxs[i];
			IBK_ASSERT(m_statesModel->m_zoneTemperatureRefs[refIdx] != nullptr);
			resultInputValueReferences.push_back(std::make_pair( &m_statesModel->m_elementValueRefs.m_heatExchangeRefValues[i],
																 m_statesModel->m_zoneTemperatureRefs[refIdx]));
			resultInputValueReferences.push_back(std::make_pair( &m_networkZoneHeatLoad[refIdx],
																 m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i]));
		}
		// set dependencies between heat exchange values and active layer inputs
		if(!m_statesModel->m_elementValueRefs.m_constructionInstanceIdxs.empty() &&
		   m_statesModel->m_elementValueRefs.m_constructionInstanceIdxs[i] != NANDRAD::INVALID_ID) {
			// zone temperature is requested
			unsigned int refIdx = m_statesModel->m_elementValueRefs.m_constructionInstanceIdxs[i];
			IBK_ASSERT(m_statesModel->m_activeLayerTemperatureRefs[refIdx] != nullptr);
			resultInputValueReferences.push_back(std::make_pair( &m_statesModel->m_elementValueRefs.m_heatExchangeRefValues[i],
																 m_statesModel->m_activeLayerTemperatureRefs[refIdx]));
			resultInputValueReferences.push_back(std::make_pair( &m_networkActiveLayerHeatLoad[refIdx],
																 m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i]));
		}
	}

	unsigned int offset = 0;
	// we at first try use dense pattern between all internal element divergences and internal states
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];


		const Element &elem = m_statesModel->m_p->m_network->m_elements[i];

		// try to retrieve individual dependencies of ydot and y
		std::vector<std::pair<const double *, const double *> > deps;
		fe->dependencies(&m_ydot[offset], &m_statesModel->m_y[offset],
						 m_statesModel->m_p->m_fluidMassFluxes + i,
						 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet],
						 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet],
						 deps);

		// copy dependencies
		if(!deps.empty()) {
			resultInputValueReferences.insert(resultInputValueReferences.end(), deps.begin(), deps.end());
		}
		// not implemented: assume all ydot and y values to depend on all inputs and result quantities
		else {
			for(unsigned int n = 0; n < fe->nInternalStates(); ++n) {
				// dependencyies to ydot: y
				// assume a dense matrix between ydot and y
				for(unsigned int l = 0; l < fe->nInternalStates(); ++l) {
					resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_y[offset + l]) );
				}

				// dependencyies to ydot: mass flux
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], m_statesModel->m_p->m_fluidMassFluxes + i) );

				// dependencyies to ydot: nodal temperatures at inlet and outlet
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet]) );
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet]) );

				// dependencyies to ydot: heat loss
				if(m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i] != nullptr)
					resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i]) );

				// dependencyies to ydot: heat exchange values (either externbal temperature or heat flux)
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_elementValueRefs.m_heatExchangeRefValues[i]) );

				// dependencies of y to result quantities: mean air temperature
				resultInputValueReferences.push_back(std::make_pair(m_statesModel->m_elementValueRefs.m_meanTemperatureRefs[i],
																	&m_statesModel->m_y[offset + n] ) );

				// dependencyies of y to result quantities:nodal temperatures
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet],
													 &m_statesModel->m_y[offset + n] ) );
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet],
													 &m_statesModel->m_y[offset + n] ) );

				// dependencies of y to result quantities: heat loss
				if(m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i] != nullptr)
					resultInputValueReferences.push_back(std::make_pair(m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i],
																		&m_statesModel->m_y[offset + n] ) );
			}
		}
		offset += fe->nInternalStates();
	}
	// retrieve dependencies of network connections
	m_statesModel->m_p->dependencies(resultInputValueReferences);
}


int ThermalNetworkBalanceModel::update() {

	//update all network internal calulation quantities
	int res = m_statesModel->m_p->update();
	if (res != 0)
		return res;

	// update zone specific fluxes
	if(!m_statesModel->m_zoneIds.empty()) {
		// set zone heat fluxes to 0
		std::fill(m_networkZoneHeatLoad.begin(), m_networkZoneHeatLoad.end(), 0);

		for(unsigned int i = 0; i < m_statesModel->m_elementValueRefs.m_nValues; ++i) {
			unsigned int index = m_statesModel->m_elementValueRefs.m_zoneIdxs[i];
			// invaid index
			if(index == NANDRAD::INVALID_ID)
				continue;
			IBK_ASSERT(m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i] != nullptr);
			// we sum up heat flux of all zones
			IBK_ASSERT(index < m_networkZoneHeatLoad.size());
			m_networkZoneHeatLoad[index] += *m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i];
		}
	}

	// update construction layer specific fluxes
	if(!m_statesModel->m_constructionInstanceIds.empty()) {
		// set zone heat fluxes to 0
		std::fill(m_networkActiveLayerHeatLoad.begin(), m_networkActiveLayerHeatLoad.end(), 0);

		for(unsigned int i = 0; i < m_statesModel->m_elementValueRefs.m_nValues; ++i) {
			unsigned int index = m_statesModel->m_elementValueRefs.m_constructionInstanceIdxs[i];
			// invaid index
			if(index == NANDRAD::INVALID_ID)
				continue;
			IBK_ASSERT(m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i] != nullptr);
			// we sum up heat flux of all zones
			IBK_ASSERT(index < m_networkActiveLayerHeatLoad.size());
			m_networkActiveLayerHeatLoad[index] += *m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i];
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
	for (unsigned int i=0; i<m_statesModel->m_elementValueRefs.m_nValues; ++i) {
		// skip adiabatic models
		if(m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i] == nullptr)
			continue;
		std::cout << "  " << i << "   " << m_statesModel->m_elementValueRefs.m_flowElementHeatLossRefs[i]  << std::endl;
	}

	std::cout << "Fluid tempertaures [C]" << std::endl;
	for (unsigned int i=0; i<m_statesModel->m_elementValueRefs.m_nValues; ++i)
		std::cout << "  " << i << "   " << *m_statesModel->m_elementValueRefs.m_meanTemperatureRefs[i] - 273.15 << std::endl;
}


} // namespace NANDRAD_MODEL
