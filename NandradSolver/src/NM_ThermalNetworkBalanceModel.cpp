/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include <IBK_messages.h>

#include "NM_ThermalNetworkPrivate.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_HydraulicNetworkModelPrivate.h"
#include "NM_ThermalNetworkBalanceModel.h"
#include "NM_ThermalNetworkFlowElements.h"
#include "NM_ThermalNetworkStatesModel.h"

#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_SimulationParameter.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>
#include <iostream>

namespace NANDRAD_MODEL {


void ThermalNetworkBalanceModel::setup(ThermalNetworkStatesModel *statesModel) {
	// copy states model pointer
	m_statesModel = statesModel;

	// sanity checks
	IBK_ASSERT(m_statesModel->m_network != nullptr);
	IBK_ASSERT(!m_statesModel->m_network->m_elements.empty());
	IBK_ASSERT(m_statesModel->m_network->m_elements.size() == m_statesModel->m_p->m_flowElements.size());

	// here we initialize all data structures needed to communicate our results to other NANDRAD models

	// For each flow element we create an object of type FlowElementProperties to store information needed
	// to publish results (inlet/outlet temperatures, mean temperatures, heat exchange fluxes, ...)
	for (unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i)
		m_flowElementProperties.push_back(FlowElementProperties(m_statesModel->m_network->m_elements[i].m_id));

	// Note: the vector m_flowElementProperties will not be changed in size lateron, so we can store persistent
	//       pointers to vector elements and their data members.

	// process all flow elements
	for (unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {
		FlowElementProperties &elemProp = m_flowElementProperties[i]; // readability improvement

		// *** Store Value References ***

		// Store value references to inlet and outlet temperatures
		const Element &elem = m_statesModel->m_p->m_network->m_elements[i];
		elemProp.m_inletNodeTemperatureRef = &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet];
		elemProp.m_outletNodeTemperatureRef = &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet];

		// Store value reference to computed heatLoss
		// Note: vector m_heatLossElements has same length as m_network->m_elements. For all elements without
		//       heat loss calculation, this vector contains a nullptr
		const ThermalNetworkAbstractFlowElementWithHeatLoss *heatLossElem = m_statesModel->m_p->m_heatLossElements[i];
		if (heatLossElem != nullptr)
			elemProp.m_heatLossRef = &heatLossElem->m_heatLoss;


		// *** Zone/Active Layer Interface Objects ***

		// Create storage locations of zone/active layer exchange and populate the FlowElementProperties objects.
		const NANDRAD::HydraulicNetworkHeatExchange &heatExchange = m_statesModel->m_network->m_elements[i].m_heatExchange;
		switch (heatExchange.m_modelType) {
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant: break;

			// zone heat exchange
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone: {
				unsigned int zoneId = heatExchange.m_idReferences[NANDRAD::HydraulicNetworkHeatExchange::ID_ZoneId];
				IBK_ASSERT(zoneId != NANDRAD::INVALID_ID);

				// check whether zone is registered
				std::list<ZoneProperties>::iterator fIt = std::find(m_zoneProperties.begin(), m_zoneProperties.end(), zoneId);
				if (fIt == m_zoneProperties.end()) {
					// not yet in list, add a new entry
					m_zoneProperties.push_back(ZoneProperties(zoneId)); // Note: does not invalidate pointers already in list!
					// store pointer to newly added object (last object = first from end)
					elemProp.m_zoneProperties = &(*m_zoneProperties.rbegin());
				}
				else {
					// store pointer to object
					elemProp.m_zoneProperties = &(*fIt);
				}
			} break;

			// construction heat exchange
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer: {
				// check for zone id
				unsigned int conInstanceId = heatExchange.m_idReferences[NANDRAD::HydraulicNetworkHeatExchange::ID_ConstructionInstanceId];
				IBK_ASSERT(conInstanceId != NANDRAD::INVALID_ID);
				// double entry is not allowed - this has been checked already in HydraulicNetwork::checkParameters()
				IBK_ASSERT(std::find(m_activeProperties.begin(), m_activeProperties.end(), conInstanceId) == m_activeProperties.end());

				m_activeProperties.push_back(ActiveLayerProperties(conInstanceId)); // Note: does not invalidate pointers already in list!
				// store pointer to newly added object (last object = first from end)
				elemProp.m_activeLayerProperties = &(*m_activeProperties.rbegin());
			} break;

			// exchange with purely time-dependent temperature spline data
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
			break;

			case NANDRAD::HydraulicNetworkHeatExchange::NUM_T: ;
		}


		// *** Collect Model Quantities ***

		m_modelQuantityOffset.push_back(m_modelQuantities.size());
		// retrieve current flow element (m_flowElements vector has same size as
		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];
		fe->modelQuantities(m_modelQuantities);
		fe->modelQuantityValueRefs(m_modelQuantityRefs);
		// correct type and id of quantity description
		for(unsigned int k = m_modelQuantityOffset.back(); k < m_modelQuantities.size(); ++k) {
			m_modelQuantities[k].m_id = m_flowElementProperties[i].m_elementId;
			m_modelQuantities[k].m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		}
		// implementation check
		IBK_ASSERT(m_modelQuantities.size() == m_modelQuantityRefs.size());
	}
	// mark end of vector
	m_modelQuantityOffset.push_back(m_modelQuantities.size());

	// resize vectors (guaranteed to be not empty)
	m_ydot.resize(m_statesModel->nPrimaryStateResults());
}


void ThermalNetworkBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	// publish heat loss from flow element towards environment
	QuantityDescription desc("FlowElementHeatLoss", "W", "Heat flux from flow element into environment", false);

	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for (unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		// skip elements without heat loss
		if (m_statesModel->m_p->m_heatLossElements[i] == nullptr)
			continue;
		desc.m_id = m_flowElementProperties[i].m_elementId;
		resDesc.push_back(desc);
	}

	// heat load to zones with heat exchange
	if (!m_zoneProperties.empty()) {
		// select all zone ids
		std::vector<unsigned int> zoneIds;
		for (const ZoneProperties &zoneProp : m_zoneProperties)
			zoneIds.push_back(zoneProp.m_zoneId);

		// set a description for each zone
		desc = QuantityDescription("NetworkZoneThermalLoad", "W", "Complete Heat load to zones from all hydraulic network elements", false);
		// add current index to description
		desc.resize(zoneIds, VectorValuedQuantityIndex::IK_ModelID);
		resDesc.push_back(desc);
	}

	// heat load to heated contruction layers
	if (!m_activeProperties.empty()) {
		// select all constrcution instance ids
		std::vector<unsigned int> conInstanceIds;
		for (const ActiveLayerProperties &layerProp : m_activeProperties)
			conInstanceIds.push_back(layerProp.m_constructionInstanceId);

		// set a description for each construction
		desc = QuantityDescription("ActiveLayerThermalLoad", "W", "Heat load to the construction layers from all hydraulic network elements", false);
		// add current index to description
		desc.resize(conInstanceIds, VectorValuedQuantityIndex::IK_ModelID);
		resDesc.push_back(desc);
	}

	// inlet node temperature is a result
	desc = QuantityDescription("InletNodeTemperature", "C", "Inlet node temperature of a flow element", false);
	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for (unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		desc.m_id = m_flowElementProperties[i].m_elementId;
		resDesc.push_back(desc);
	}

	// outlet node temperature is a result
	desc = QuantityDescription("OutletNodeTemperature", "C", "Outlet node temperature of a flow element", false);
	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for (unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		desc.m_id = m_flowElementProperties[i].m_elementId;
		resDesc.push_back(desc);
	}

	// add individual model results
	if (!m_modelQuantities.empty())
		resDesc.insert(resDesc.end(), m_modelQuantities.begin(), m_modelQuantities.end());
}


const double * ThermalNetworkBalanceModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;

	// return ydot
	if (quantityName == std::string("ydot")) {
		// whole vector access
		if (quantityName.m_index == -1)
			return &m_ydot[0];
		return nullptr;
	}
	if (quantityName.m_name == std::string("NetworkZoneThermalLoad")) {
		// no zones are given?
		if (m_zoneProperties.empty())
			return nullptr;
		// find zone id
		std::list<ZoneProperties>::const_iterator fIt = std::find(m_zoneProperties.begin(), m_zoneProperties.end(),
				  (unsigned int) quantityName.m_index);
		// invalid index access
		if (fIt == m_zoneProperties.end())
			return nullptr;

		// found a valid entry
		return &fIt->m_zoneHeatLoad;
	}
	if (quantityName.m_name == std::string("ActiveLayerThermalLoad")) {
		// no active layers are given?
		if(m_activeProperties.empty())
			return nullptr;
		// find id
		std::list<ActiveLayerProperties>::const_iterator fIt = std::find(m_activeProperties.begin(), m_activeProperties.end(),
				  (unsigned int) quantityName.m_index);
		// invalid index access
		if (fIt == m_activeProperties.end())
			return nullptr;

		// found a valid entry
		return &fIt->m_activeLayerHeatLoad;
	}


	// everything else must be of reftype NETWORKELEMENT, so ignore everything else
	if (quantity.m_referenceType != NANDRAD::ModelInputReference::MRT_NETWORKELEMENT)
		return nullptr;

	// lookup element index based on given ID
	std::vector<FlowElementProperties>::const_iterator fIt =
			std::find(m_flowElementProperties.begin(), m_flowElementProperties.end(), (unsigned int) quantity.m_id);
	// invalid ID?
	if (fIt == m_flowElementProperties.end())
		return nullptr;

	if (quantityName == std::string("InletNodeTemperature"))
		return fIt->m_inletNodeTemperatureRef;
	else if (quantityName == std::string("OutletNodeTemperature"))
		return fIt->m_outletNodeTemperatureRef;
	else if (quantityName == std::string("FlowElementHeatLoss"))
		return fIt->m_heatLossRef;

	unsigned int pos = (unsigned int) std::distance(m_flowElementProperties.begin(), fIt);
	// search for quantity inside individual element results
	IBK_ASSERT(pos < m_modelQuantityOffset.size() - 1);
	unsigned int startIdx = m_modelQuantityOffset[pos];
	unsigned int endIdx = m_modelQuantityOffset[pos + 1];

	// check if element contains requested quantity
	for (unsigned int resIdx = startIdx; resIdx < endIdx; ++resIdx) {
		const QuantityDescription &modelDesc = m_modelQuantities[resIdx];
		if (modelDesc.m_name == quantityName.m_name) {
			// index is not allowed for network element output
			if (quantityName.m_index != -1)
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


void ThermalNetworkBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// request mass fluxes from hydraulic network model with same ID
	InputReference inputRef;
	inputRef.m_id = id();
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORK;
	inputRef.m_name = std::string("FluidMassFluxes");
	inputRef.m_required = true;
	// register reference
	inputRefs.push_back(inputRef);


	std::vector<InputReference> modelInputRefs;
	// loop over all elements and ask them to request individual inputs, for example scheduled quantities
	for (unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		m_statesModel->m_p->m_flowElements[i]->inputReferences(modelInputRefs);

	// Note: for constant and spline requests, we provide respective pointers by ThermalNetworkStatesModel. For
	//       requests of results from other models, we redirect references accordingly, for example to zone air and
	//       active layer temperature (by renaming requested quantities and source objects)
	if (!m_zoneProperties.empty() || !m_activeProperties.empty()) {

		// filter input references to heat exchange temperatur
		for (unsigned int i = 0; i < modelInputRefs.size(); ++i) {
			InputReference &inputRef = modelInputRefs[i];
			// skip uninteresting references
			if (inputRef.m_name.m_name != "HeatExchangeTemperature")
				continue;
			// find flow element properties
			std::vector<FlowElementProperties>::const_iterator fIt =
					std::find(m_flowElementProperties.begin(), m_flowElementProperties.end(), inputRef.m_id);

			IBK_ASSERT(fIt != m_flowElementProperties.end());

			// we assign a zone
			if(fIt->m_zoneProperties != nullptr) {
				// redirect to zone air temperature
				inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				inputRef.m_id = fIt->m_zoneProperties->m_zoneId;
				inputRef.m_name = QuantityName("AirTemperature");
			}
			// we assign an active construction layer
			else if(fIt->m_activeLayerProperties != nullptr) {
				// redirect to zone air temperature
				inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				inputRef.m_id = fIt->m_activeLayerProperties->m_constructionInstanceId;
				inputRef.m_name = QuantityName("ActiveLayerTemperature");
			}
		}
	}

	// add to global list
	inputRefs.insert(inputRefs.end(), modelInputRefs.begin(), modelInputRefs.end());

}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	// layout of resultValueRefs vector:
	// 0                                 - FluidMassFluxes
	// ...elements                       - ..

	// copy references into mass flux vector
	m_statesModel->m_p->m_fluidMassFluxes = resultValueRefs[0];

	//
	unsigned int resultValIdx = 1;

	// resultValIdx now points to the first input reference past the active layer/zone temperatures

	// now provide elements with their specific input quantities
	std::vector<const double *>::const_iterator valRefIt = resultValueRefs.begin() + resultValIdx; // Mind the index increase here

	for (unsigned int i = 0; i < m_statesModel->m_p->m_flowElements.size(); ++i)
		m_statesModel->m_p->m_flowElements[i]->setInputValueRefs(valRefIt);

	IBK_ASSERT(valRefIt == resultValueRefs.end());
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	for (unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {

		const FlowElementProperties &elemProp = m_flowElementProperties[i];
		const ZoneProperties *zoneProp = elemProp.m_zoneProperties;
		const ActiveLayerProperties *layerProp = elemProp.m_activeLayerProperties;

		// set dependencies between heat exchange values and zone inputs
		if (zoneProp != nullptr) {
			// heat loss computed in element depends also on requested zone temperature
			resultInputValueReferences.push_back(std::make_pair( &zoneProp->m_zoneHeatLoad, elemProp.m_heatLossRef) );
		}
		// set dependencies between heat exchange values and active layer inputs
		if (layerProp != nullptr) {
			// heat loss computed in element depends also on requested layer temperature
			resultInputValueReferences.push_back(std::make_pair( &layerProp->m_activeLayerHeatLoad, elemProp.m_heatLossRef) );
		}
	}

	unsigned int offset = 0;
	// we at first try use dense pattern between all internal element divergences and internal states
	for (unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];
		const Element &elem = m_statesModel->m_p->m_network->m_elements[i];

		// try to retrieve individual dependencies of ydot and y
		std::vector<std::pair<const double *, const double *> > deps;
		// special handling for elements without states
		if (fe->nInternalStates() == 0) {
			fe->dependencies(nullptr, nullptr,
							 m_statesModel->m_p->m_fluidMassFluxes + i,
							 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet],
							 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet],
							 deps);
		}
		else {
			fe->dependencies(&m_ydot[offset], &m_statesModel->m_y[offset],
							 m_statesModel->m_p->m_fluidMassFluxes + i,
							 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet],
							 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet],
							 deps);
		}

		// copy dependencies
		if (!deps.empty()) {
			resultInputValueReferences.insert(resultInputValueReferences.end(), deps.begin(), deps.end());
		}

		offset += fe->nInternalStates();
	}
	// retrieve dependencies of network connections
	m_statesModel->m_p->dependencies(resultInputValueReferences);
}


int ThermalNetworkBalanceModel::update() {

	// Update all network internal calculation quantities,
	// basically transfer mass fluxes (already computed) and inflow temperatures into all flow elements.
	// Setting the inflow temperatures in flow elements triggers their calculation.
	// Afterwards, outflow temperatures and heat exchange fluxes are available.
	int res = m_statesModel->m_p->update();
	if (res != 0)
		return res;

	// sum up zonal and construction layer heat loads from all elements
	if (!m_zoneProperties.empty() || !m_activeProperties.empty()) {
		// set zone heat fluxes to 0
		for (ZoneProperties &zoneProp : m_zoneProperties)
			zoneProp.m_zoneHeatLoad = 0.0;

		for (unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {

			const FlowElementProperties &elemProp = m_flowElementProperties[i];
			// skip invalid elements without access to zone temperature
			ZoneProperties *zoneProp = m_flowElementProperties[i].m_zoneProperties;
			ActiveLayerProperties *layerProp = m_flowElementProperties[i].m_activeLayerProperties;

			// exchange heat with a zone?
			if (zoneProp != nullptr) {
				zoneProp->m_zoneHeatLoad += *elemProp.m_heatLossRef;
			}

			// or exchange heat with construction layer?
			else if (layerProp != nullptr) {
				layerProp->m_activeLayerHeatLoad = *elemProp.m_heatLossRef;
			}
		}
	}

	// Now the sums of heat loads in all ZoneProperties and ActiveLayerProperties are up-to-date and can be
	// accessed as model results by other NANDRAD models.

	// update derivatives
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement *fe : m_statesModel->m_p->m_flowElements) {
		// skip elements without states
		if (fe->nInternalStates() != 0)
			fe->internalDerivatives(&m_ydot[offset]);
		offset += fe->nInternalStates();
	}

#ifdef NANDRAD_NETWORK_DEBUG_OUTPUTS
	printVars();
#endif // NANDRAD_NETWORK_DEBUG_OUTPUTS

	return 0;
}


int ThermalNetworkBalanceModel::ydot(double* ydot) {
	// copy values to ydot
	std::memcpy(ydot, &m_ydot[0], m_ydot.size() * sizeof (double));
	// signal success
	return 0;
}


void ThermalNetworkBalanceModel::printVars() const {
	std::cout << "Heat fluxes [W]" << std::endl;
	for (unsigned int i=0; i<m_flowElementProperties.size(); ++i) {
		// skip adiabatic models
		if(m_flowElementProperties[i].m_heatLossRef == nullptr)
			continue;
		std::cout << "  " << i << "   " << m_flowElementProperties[i].m_heatLossRef  << std::endl;
	}

	std::cout << "Fluid tempertaures [C]" << std::endl;
	for (unsigned int i=0; i<m_flowElementProperties.size(); ++i)
		std::cout << "  " << i << "   " << *m_statesModel->m_meanTemperatureRefs[i] - 273.15 << std::endl;
}


} // namespace NANDRAD_MODEL
