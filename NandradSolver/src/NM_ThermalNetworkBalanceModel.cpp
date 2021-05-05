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
#include <NANDRAD_SimulationParameter.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>
#include <iostream>

namespace NANDRAD_MODEL {


void ThermalNetworkBalanceModel::setup(ThermalNetworkStatesModel *statesModel,
									   const NANDRAD::SimulationParameter & simPara)
{
	// copy states model pointer
	m_statesModel = statesModel;
	m_simPara = &simPara;

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
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline: {
				// store reference to spline
				elemProp.m_heatExchangeSplineRef = &heatExchange.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature];
				// store pointer to interpolated value into respective flow element
				ThermalNetworkAbstractFlowElementWithHeatLoss * heatLossElement =
						dynamic_cast<ThermalNetworkAbstractFlowElementWithHeatLoss*>(m_statesModel->m_p->m_flowElements[i]);
				IBK_ASSERT(heatLossElement != nullptr);
				heatLossElement->m_heatExchangeValueRef = &elemProp.m_heatExchangeSplineValue;
			}
			break;

				// exchange with purely time-dependent heat loss spline data
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser: {
				// store reference to spline
				elemProp.m_heatExchangeSplineRef = &heatExchange.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss];
				// store pointer to interpolated value into respective flow element
				ThermalNetworkAbstractFlowElementWithHeatLoss * heatLossElement =
						dynamic_cast<ThermalNetworkAbstractFlowElementWithHeatLoss*>(m_statesModel->m_p->m_flowElements[i]);
				IBK_ASSERT(heatLossElement != nullptr);
				heatLossElement->m_heatExchangeValueRef = &elemProp.m_heatExchangeSplineValue;
			}
			break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureFMUInterface:
				// TODO
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

	// resize vectors
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
		desc = QuantityDescription("NetworkZoneHeatLoad", "W", "Complete Heat load to zones from all hydraulic network elements", false);
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
		desc = QuantityDescription("NetworkActiveLayerHeatLoad", "W", "Heat load to the construction layers from all hydraulic network elements", false);
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


void ThermalNetworkBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
#if 0
	if(!res.empty())
		res.clear();
	// heat flux vector is a result quantity
	for (unsigned int i = 0; i < m_flowElementProperties.size(); ++i)
		res.push_back(m_flowElementProperties[i].m_heatLossRef);
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_zoneProperties.size(); ++i)
		res.push_back(&m_zoneProperties[i].m_zoneHeatLoad);
	// heat flux vector is a result quantity
	for(unsigned int i = 0; i < m_activeProperties.size(); ++i)
		res.push_back(&m_activeProperties[i].m_activeLayerHeatLoad);
	// inlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i)
		res.push_back(m_flowElementProperties[i].m_inletNodeTemperatureRef);
	// outlet node temperature vector is a result quantity
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i)
		res.push_back(m_flowElementProperties[i].m_outletNodeTemperatureRef);
	// add individual model result value references
	if(!m_modelQuantityRefs.empty())
		res.insert(res.end(), m_modelQuantityRefs.begin(), m_modelQuantityRefs.end());
#endif
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
	if (quantityName.m_name == std::string("NetworkZoneHeatLoad")) {
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
	if (quantityName.m_name == std::string("NetworkActiveLayerHeatLoad")) {
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


int ThermalNetworkBalanceModel::setTime(double t) {
	// update all spline values
	for (unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		FlowElementProperties &elemProp = m_flowElementProperties[i];
		if (elemProp.m_heatExchangeSplineRef != nullptr) {
			elemProp.m_heatExchangeSplineValue = m_simPara->evaluateTimeSeries(t, *elemProp.m_heatExchangeSplineRef);
		}
	}
	return 0;
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

	// request zone air temperatures
	if (!m_zoneProperties.empty()) {
		InputReference inputRef;
		inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		inputRef.m_name = std::string("AirTemperature");
		inputRef.m_required = true;
		for (const ZoneProperties &zoneProp : m_zoneProperties) {
			inputRef.m_id = zoneProp.m_zoneId;
			inputRefs.push_back(inputRef);
		}
	}
	// request construction layer temperatures
	if (!m_activeProperties.empty()) {
		InputReference inputRef;
		inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		inputRef.m_name = std::string("ActiveLayerTemperature");
		inputRef.m_required = true;
		for (const ActiveLayerProperties &actLayerProp : m_activeProperties) {
			inputRef.m_id = actLayerProp.m_constructionInstanceId;
			inputRefs.push_back(inputRef);
		}
	}
}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	// layout of resultValueRefs vector:
	// 0                                 - FluidMassFluxes
	// 1..m_zoneProperties.size()        - AirTemperature
	// ...m_activeProperties.size()      - ActiveLayerTemperature
	// ...m_scheduelParameters.size()    - ..

	// copy references into mass flux vector
	m_statesModel->m_p->m_fluidMassFluxes = resultValueRefs[0];

	// set zone temparture references inside network
	unsigned int resultValIdx = 0;
	for (std::list<ZoneProperties>::iterator it = m_zoneProperties.begin();
		 it != m_zoneProperties.end(); ++it)
	{
		// set reference to zone temperature
		it->m_zoneTemperatureRef = resultValueRefs[++resultValIdx];
	}

	// set active layer references inside network
	for (std::list<ActiveLayerProperties>::iterator it = m_activeProperties.begin();
		 it != m_activeProperties.end(); ++it)
	{
		// set reference to zone temperature
		it->m_activeLayerTemperatureRef = resultValueRefs[++resultValIdx];
	}

	// TODO : Schedule parameter

	// now transfer stored references to elements
	for (unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {
		const FlowElementProperties &elemProp = m_flowElementProperties[i];

		// do we have a zone temperature dependence?
		if (elemProp.m_zoneProperties != nullptr) {
			ThermalNetworkAbstractFlowElementWithHeatLoss *fe =
					dynamic_cast<ThermalNetworkAbstractFlowElementWithHeatLoss *>(m_statesModel->m_p->m_flowElements[i]);
			IBK_ASSERT(fe != nullptr);
			fe->m_heatExchangeValueRef = elemProp.m_zoneProperties->m_zoneTemperatureRef;
		}
		// or do we have an active layer temperature dependence?
		else if (elemProp.m_activeLayerProperties != nullptr) {
			ThermalNetworkAbstractFlowElementWithHeatLoss *fe =
					dynamic_cast<ThermalNetworkAbstractFlowElementWithHeatLoss *>(m_statesModel->m_p->m_flowElements[i]);
			IBK_ASSERT(fe != nullptr);
			fe->m_heatExchangeValueRef = elemProp.m_activeLayerProperties->m_activeLayerTemperatureRef;
		}

	}
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {

		const FlowElementProperties &elemProp = m_flowElementProperties[i];
		const ZoneProperties *zoneProp = elemProp.m_zoneProperties;
		const ActiveLayerProperties *layerProp = elemProp.m_activeLayerProperties;

		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];

		// set dependencies between heat exchange values and zone inputs
		if (zoneProp != nullptr) {
			// heat loss computed in element depends also on requested zone temperature
			resultInputValueReferences.push_back(std::make_pair( &zoneProp->m_zoneHeatLoad,
																 elemProp.m_heatLossRef));
		}
		// set dependencies between heat exchange values and active layer inputs
		if (layerProp != nullptr) {
			// heat loss computed in element depends also on requested layer temperature
			resultInputValueReferences.push_back(std::make_pair( &layerProp->m_activeLayerHeatLoad,
																 elemProp.m_heatLossRef));
		}
	}

	unsigned int offset = 0;
	// we at first try use dense pattern between all internal element divergences and internal states
	for (unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

		const ThermalNetworkAbstractFlowElement *fe = m_statesModel->m_p->m_flowElements[i];
		const Element &elem = m_statesModel->m_p->m_network->m_elements[i];
		const FlowElementProperties &elemProp = m_flowElementProperties[i];

		// try to retrieve individual dependencies of ydot and y
		std::vector<std::pair<const double *, const double *> > deps;
		fe->dependencies(&m_ydot[offset], &m_statesModel->m_y[offset],
						 m_statesModel->m_p->m_fluidMassFluxes + i,
						 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet],
						 &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet],
						 deps);

		// copy dependencies
		if (!deps.empty()) {
			resultInputValueReferences.insert(resultInputValueReferences.end(), deps.begin(), deps.end());
		}
#if 0
		// dependencies() may not be implemented: in this case assume all ydot and y values to depend on all
		// inputs and result quantities
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
				if(elemProp.m_heatLossRef != nullptr)
					resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], elemProp.m_heatLossRef) );

				// dependencyies to ydot: heat exchange values (either external temperature or heat flux)
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_heatExchangeRefValues[i]) );

				// TODO Anne, eigentlich könnten/müssten die rein states-model-spezifischen Abhängigkeiten auch
				//            im ThermalNetworkStatesModel gemacht werden - hier ist es aber auch ok, sofern man später bei Erweiterungen
				//            des ThermalNetworkStatesModel nicht irgendwas vergisst. Siehe auch Kommentar in ThermalNetworkStatesModel::dependencies()

				// dependencies of y to result quantities: mean air temperature
				resultInputValueReferences.push_back(std::make_pair(m_statesModel->m_meanTemperatureRefs[i],
																	&m_statesModel->m_y[offset + n] ) );

				// dependencyies of y to result quantities:nodal temperatures
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet],
													 &m_statesModel->m_y[offset + n] ) );
				resultInputValueReferences.push_back(std::make_pair(&m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet],
													 &m_statesModel->m_y[offset + n] ) );

				// dependencies of y to result quantities: heat loss
				if (elemProp.m_heatLossRef != nullptr)
					resultInputValueReferences.push_back(std::make_pair(elemProp.m_heatLossRef,
																		&m_statesModel->m_y[offset + n] ) );
			}
		}
#endif
		offset += fe->nInternalStates();
	}
	// retrieve dependencies of network connections
	m_statesModel->m_p->dependencies(resultInputValueReferences);
}


int ThermalNetworkBalanceModel::update() {
#if 0
	// update zone and construction layer temperatures
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		// skip invalid elements without access to zone temperature
		const ZoneProperties *zoneProp = m_flowElementProperties[i].m_zoneProperties;
		const ActiveLayerProperties *layerProp = m_flowElementProperties[i].m_activeLayerProperties;

		if(zoneProp != nullptr) {
			IBK_ASSERT(zoneProp->m_zoneTemperatureRef != nullptr);
			m_statesModel->m_heatExchangeRefValues[i] = *zoneProp->m_zoneTemperatureRef;
		}
		else if(layerProp != nullptr) {
			IBK_ASSERT(layerProp->m_activeLayerTemperatureRef != nullptr);
			m_statesModel->m_heatExchangeRefValues[i] = *layerProp->m_activeLayerTemperatureRef;
		}
	}
#endif

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
