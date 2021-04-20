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

	// resize element properties and copy ids
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i)
		m_flowElementProperties.push_back(FlowElementProperties(m_statesModel->m_network->m_elements[i].m_id));

	// store index of zone ids for each exchanging flow element
	std::vector<unsigned int> zoneIdx(m_flowElementProperties.size(), NANDRAD::INVALID_ID);
	// store index of construction instance ids for each exchanging flow element
	std::vector<unsigned int> constructionInstanceIdx(m_flowElementProperties.size(), NANDRAD::INVALID_ID);

	// first set all zone and construction properties
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

		const NANDRAD::HydraulicNetworkHeatExchange &heatExchange = m_statesModel->m_network->m_elements[i].m_heatExchange;

		switch(heatExchange.m_modelType) {
			// zone heat exchange
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone: {
				unsigned int zoneId = heatExchange.m_idReferences[NANDRAD::HydraulicNetworkHeatExchange::ID_ZoneId];
				IBK_ASSERT(zoneId != NANDRAD::INVALID_ID);
				// parameters are checked, already
				// check whether zone is registered
				std::vector<ZoneProperties>::iterator fIt = std::find(m_zoneProperties.begin(), m_zoneProperties.end(), zoneId);
				// add a new entry
				if(fIt == m_zoneProperties.end()) {
					zoneIdx[i] = m_zoneProperties.size();
					m_zoneProperties.push_back(ZoneProperties(zoneId));
				}
				else {
					unsigned int index = std::distance(m_zoneProperties.begin(), fIt);
					zoneIdx[i] = index;
				}
			} break;

			// construction heat exchange
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer: {
				// check for zone id
				unsigned int conInstanceId = heatExchange.m_idReferences[NANDRAD::HydraulicNetworkHeatExchange::ID_ConstructionInstanceId];
				IBK_ASSERT(conInstanceId != NANDRAD::INVALID_ID);
				// double entry is not allowed - this has been checked already in HydraulicNetwork::checkParameters()
				IBK_ASSERT(std::find(m_activeProperties.begin(), m_activeProperties.end(), conInstanceId) == m_activeProperties.end());

				constructionInstanceIdx[i] = m_activeProperties.size();
				m_activeProperties.push_back(ActiveLayerProperties(conInstanceId));

			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
				// store reference to spline
				m_flowElementProperties[i].m_heatExchangeSplineRef = &heatExchange.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature].m_values;
			break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
				// store reference to spline
				m_flowElementProperties[i].m_heatExchangeSplineRef = &heatExchange.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss].m_values;
			break;

			default: break;
		}
	}

	// first set all zone and construction properties
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

		// set all value references
		const Element &elem = m_statesModel->m_p->m_network->m_elements[i];
		FlowElementProperties &elemProp = m_flowElementProperties[i];

		// copy nodal temperatures
		elemProp.m_inletNodeTemperatureRef = &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexInlet];
		elemProp.m_outletNodeTemperatureRef = &m_statesModel->m_p->m_nodalTemperatures[elem.m_nodeIndexOutlet];

		// set zone or conmstruction properties
		if(zoneIdx[i] != NANDRAD::INVALID_ID) {
			unsigned int idx = zoneIdx[i];
			IBK_ASSERT(idx < m_zoneProperties.size());
			elemProp.m_zoneProperties = &m_zoneProperties[idx];
		}
		else if(constructionInstanceIdx[i] != NANDRAD::INVALID_ID) {
			unsigned int idx = constructionInstanceIdx[i];
			IBK_ASSERT(idx < m_activeProperties.size());
			elemProp.m_activeLayerProperties = &m_activeProperties[idx];
		}
	}

	// set references to heat fluxes
	for(unsigned int i = 0; i < m_statesModel->m_p->m_heatLossElements.size(); ++i) {
		const ThermalNetworkAbstractFlowElementWithHeatLoss *heatLossElem = m_statesModel->m_p->m_heatLossElements[i];
		// skip empty elements
		if(heatLossElem == nullptr)
			continue;
		// copy heat fluxes
		m_flowElementProperties[i].m_heatLossRef = &heatLossElem->m_heatLoss;
	}

	// fill model model quanities vector
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {
		// add offset
		m_modelQuantityOffset.push_back(m_modelQuantities.size());
		// retrieve current flow element
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
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		desc.m_id = m_flowElementProperties[i].m_elementId;
		resDesc.push_back(desc);
	}

	// add output
	if (!m_zoneProperties.empty()) {
		// select all zone ids
		std::vector<unsigned int> zoneIds;
		for(const ZoneProperties &zoneProp : m_zoneProperties)
			zoneIds.push_back(zoneProp.m_zoneId);

		// set a description for each zone
		desc = QuantityDescription("NetworkZoneHeatLoad", "W", "Complete Heat load to zones from all hydraulic network elements", false);
		// add current index to description
		desc.resize(zoneIds, VectorValuedQuantityIndex::IK_ModelID);
		resDesc.push_back(desc);
	}
	if (!m_activeProperties.empty()) {
		// select all constrcution instance ids
		std::vector<unsigned int> conInstanceIds;
		for(const ActiveLayerProperties &layerProp : m_activeProperties)
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
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		desc.m_id = m_flowElementProperties[i].m_elementId;
		resDesc.push_back(desc);
	}

	// outlet node temperature is a result
	desc = QuantityDescription("OutletNodeTemperature", "C", "Outlet node temperature of a flow element", false);
	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		desc.m_id = m_flowElementProperties[i].m_elementId;
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
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i)
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
		if(m_zoneProperties.empty())
			return nullptr;
		// find zone id
		std::vector<ZoneProperties>::const_iterator fIt =
			std::find(m_zoneProperties.begin(), m_zoneProperties.end(),
				  (unsigned int) quantityName.m_index);
		// invalid index access
		if(fIt == m_zoneProperties.end())
			return nullptr;

		// found a valid entry
		return &fIt->m_zoneHeatLoad;
	}
	if(quantityName.m_name == std::string("NetworkActiveLayerHeatLoad")) {
		// no zones are given
		if(m_activeProperties.empty())
			return nullptr;
		// find zone id
		std::vector<ActiveLayerProperties>::const_iterator fIt =
			std::find(m_activeProperties.begin(),
					  m_activeProperties.end(),
				  (unsigned int) quantityName.m_index);
		// invalid index access
		if(fIt == m_activeProperties.end())
			return nullptr;

		// found a valid entry
		return &fIt->m_activeLayerHeatLoad;
	}


	// everything below will be reftype NETWORKELEMENT, so ignore everything else
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
	if(!m_zoneProperties.empty()) {
		InputReference inputRef;
		inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		inputRef.m_name = std::string("AirTemperature");
		inputRef.m_required = true;
		for(const ZoneProperties &zoneProp : m_zoneProperties) {
			inputRef.m_id = zoneProp.m_zoneId;
			inputRefs.push_back(inputRef);
		}
	}
	// set references to construction layer temperatures
	if(!m_activeProperties.empty()) {
		InputReference inputRef;
		inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		inputRef.m_name = std::string("ActiveLayerTemperature");
		inputRef.m_required = true;
		for(const ActiveLayerProperties &actLayerProp : m_activeProperties) {
			inputRef.m_id = actLayerProp.m_constructionInstanceId;
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
	for(unsigned int i = 0; i < m_zoneProperties.size(); ++i) {
		// set reference to zone temperature
		m_zoneProperties[i].m_zoneTemperatureRef = resultValueRefs[1 + i];
	}
	//set active layer references inside network
	for(unsigned int i = 0; i < m_activeProperties.size(); ++i) {
		// set reference to zone temperature
		m_activeProperties[i].m_activeLayerTemperatureRef = resultValueRefs[1 + i];
	}
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {

		const FlowElementProperties &elemProp = m_flowElementProperties[i];
		const ZoneProperties *zoneProp = m_flowElementProperties[i].m_zoneProperties;
		const ActiveLayerProperties *layerProp = m_flowElementProperties[i].m_activeLayerProperties;
		// set dependencies between heat exchange values and zone inputs
		if(zoneProp != nullptr) {
			// zone temperature is requested
			resultInputValueReferences.push_back(std::make_pair( &m_statesModel->m_heatExchangeRefValues[i],
																 zoneProp->m_zoneTemperatureRef));
			resultInputValueReferences.push_back(std::make_pair( &zoneProp->m_zoneHeatLoad,
																 elemProp.m_heatLossRef));
		}
		// set dependencies between heat exchange values and active layer inputs
		if(layerProp !=  nullptr) {
			// layer temperature is requested
			resultInputValueReferences.push_back(std::make_pair( &m_statesModel->m_heatExchangeRefValues[i],
																 layerProp->m_activeLayerTemperatureRef));
			resultInputValueReferences.push_back(std::make_pair( &layerProp->m_activeLayerHeatLoad,
																 elemProp.m_heatLossRef));
		}
	}

	unsigned int offset = 0;
	// we at first try use dense pattern between all internal element divergences and internal states
	for(unsigned int i = 0; i < m_statesModel->m_network->m_elements.size(); ++i) {

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
				if(elemProp.m_heatLossRef != nullptr)
					resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], elemProp.m_heatLossRef) );

				// dependencyies to ydot: heat exchange values (either externbal temperature or heat flux)
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[offset + n], &m_statesModel->m_heatExchangeRefValues[i]) );

				// TODO Anne, eigentlich könnten/müssten die rein states-Modell-spezifischen Abhängigkeiten auch
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
				if(elemProp.m_heatLossRef != nullptr)
					resultInputValueReferences.push_back(std::make_pair(elemProp.m_heatLossRef,
																		&m_statesModel->m_y[offset + n] ) );
			}
		}
		offset += fe->nInternalStates();
	}
	// retrieve dependencies of network connections
	m_statesModel->m_p->dependencies(resultInputValueReferences);
}


int ThermalNetworkBalanceModel::setTime(double t) {
	// update all spline values
	for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {
		const FlowElementProperties &elemProp = m_flowElementProperties[i];
		// no spline
		if(elemProp.m_heatExchangeSplineRef == nullptr)
			continue;
		m_statesModel->m_heatExchangeRefValues[i] =
				elemProp.m_heatExchangeSplineRef->value(t);
	}
	return 0;
}


int ThermalNetworkBalanceModel::update() {

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

	// update all network internal calculation quantities
	int res = m_statesModel->m_p->update();
	if (res != 0)
		return res;

	// update zone specific fluxes
	if(!m_zoneProperties.empty() || !m_activeProperties.empty()) {
		// set zone heat fluxes to 0
		for(ZoneProperties &zoneProp : m_zoneProperties)
			zoneProp.m_zoneHeatLoad = 0.0;

		for(unsigned int i = 0; i < m_flowElementProperties.size(); ++i) {

			const FlowElementProperties &elemProp = m_flowElementProperties[i];
			// skip invalid elements without access to zone temperature
			ZoneProperties *zoneProp = m_flowElementProperties[i].m_zoneProperties;
			ActiveLayerProperties *layerProp = m_flowElementProperties[i].m_activeLayerProperties;

			if(zoneProp != nullptr) {
				zoneProp->m_zoneHeatLoad += *elemProp.m_heatLossRef;
			}
			else if(layerProp != nullptr) {
				IBK_ASSERT(layerProp->m_activeLayerTemperatureRef != nullptr);
				layerProp->m_activeLayerHeatLoad = *elemProp.m_heatLossRef;
			}
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
