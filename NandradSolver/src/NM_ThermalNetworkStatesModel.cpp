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

#include "NM_HydraulicNetworkModel.h"
#include "NM_ThermalNetworkPrivate.h"
#include "NM_ThermalNetworkFlowElements.h"
#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordList.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



// *** ThermalNetworkStatesModel members ***

ThermalNetworkStatesModel::~ThermalNetworkStatesModel() {
	delete m_p; // delete pimpl object
}


void ThermalNetworkStatesModel::setup(const NANDRAD::HydraulicNetwork & nw,
									  const HydraulicNetworkModel &networkModel) {
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

	// resize vectors
	m_p->m_ambientHeatFluxRefs.resize(nw.m_elements.size(),nullptr);
	m_p->m_ambientTemperatureRefs.resize(nw.m_elements.size(),nullptr);
	m_p->m_ambientHeatTransferRefs.resize(nw.m_elements.size(),nullptr);

	// now populate the m_edges vector of the network solver

	// process all hydraulic network elements and instatiate respective flow equation classes
	for (unsigned int i =0; i < nw.m_elements.size(); ++i) {
		const NANDRAD::HydraulicNetworkElement & e = nw.m_elements[i];
		// - instance-specific parameters from HydraulicNetworkElement e
		// - fluid property object from nw.m_fluid
		// - component definition (via reference from e.m_componentId) and component DB stored
		//   in network

		try {
			// retrieve component

			std::vector<NANDRAD::HydraulicNetworkComponent>::const_iterator itComp =
					std::find(nw.m_components.begin(), nw.m_components.end(), e.m_componentId);
			IBK_ASSERT(itComp != nw.m_components.end());

			switch (itComp->m_modelType) {
				case NANDRAD::HydraulicNetworkComponent::MT_StaticPipe :
				case NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe :
				case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
				case NANDRAD::HydraulicNetworkComponent::MT_DynamicAdiabaticPipe :
				{
					// lookup pipe
					std::vector<NANDRAD::HydraulicNetworkPipeProperties>::const_iterator itPipe =
							std::find(m_network->m_pipeProperties.begin(), m_network->m_pipeProperties.end(), e.m_pipeId);
					if (itPipe == m_network->m_pipeProperties.end()) {
						throw IBK::Exception(IBK::FormatString("Missing pipe properties reference in hydraulic network element '%1' (id=%2).")
											 .arg(e.m_displayName).arg(e.m_id), FUNC_ID);
					}

					// create hydraulic pipe model
					TNPipeElement * pipeElement = new TNPipeElement(e, *itComp,  *itPipe, m_network->m_fluid);
					// add to flow elements
					m_p->m_flowElements.push_back(pipeElement); // transfer ownership
				} break;


				case NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel :
				{
					TNPump * pumpElement = new TNPump(e, *itComp, m_network->m_fluid);
					m_p->m_flowElements.push_back(pumpElement);
					break;
				}


				case NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger:
				{
					TNHeatExchanger * heatEx = new TNHeatExchanger(e, *itComp, m_network->m_fluid);
					m_p->m_flowElements.push_back(heatEx);
					break;
				}
				default: {
					throw IBK::Exception(IBK::FormatString("Model of type %1 is not supported, yet!")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::ModelType",
								itComp->m_modelType)),
								FUNC_ID);
				}
			}
			// decide which heat exchange is chosen
			switch(itComp->m_heatExchangeType) {
				case NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant: {
					// retrieve constant temperature
					if(!e.m_para[NANDRAD::HydraulicNetworkElement::P_Temperature].name.empty()) {
						m_p->m_ambientTemperatureRefs.push_back(
							&e.m_para[NANDRAD::HydraulicNetworkElement::P_Temperature].value);
						// retrieve external heat transfer coefficient
						if(itComp->m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].name.empty()){
							throw IBK::Exception(IBK::FormatString("Missing parameteter '%1 for exchange type %2!")
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::para_t",
										NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient))
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
										itComp->m_heatExchangeType)),
										FUNC_ID);
						}
						m_p->m_ambientHeatTransferRefs[i] = &itComp->m_para[
							NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;
					}
					else if(!e.m_para[NANDRAD::HydraulicNetworkElement::P_HeatFlux].name.empty()) {
						m_p->m_ambientHeatFluxRefs[i] =
							&e.m_para[NANDRAD::HydraulicNetworkElement::P_HeatFlux].value;
					}
				} break;
				case NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature: {

					// check for zone id
					if(!e.m_intPara[NANDRAD::HydraulicNetworkElement::IP_ZoneId].name.empty()) {
						// parameters are checked, already
						unsigned int zoneId = (unsigned int) e.m_intPara[NANDRAD::HydraulicNetworkElement::IP_ZoneId].value;
						// check whether zone is registered
						if(!m_zoneIds.empty()) {
							std::vector<unsigned int>::iterator fIt = std::find(m_zoneIds.begin(),
																					  m_zoneIds.end(),zoneId);
							// add a new entry
							if(fIt == m_zoneIds.end()) {
								m_zoneIdxs[i] = m_zoneIds.size();
								m_zoneIds.push_back(zoneId);
							}
							else {
								unsigned int index = std::distance(m_zoneIds.begin(), fIt);
								m_zoneIdxs[i] = index;
							}
						}
						else {
							// resize zone idx vector
							m_zoneIdxs.resize(m_network->m_elements.size(), (unsigned int) (-1));
							m_zoneIdxs[i] = 0;
							m_zoneIds.push_back(zoneId);
						}
					}
					else {
						throw IBK::Exception(IBK::FormatString("Missing IntParameter '%1 for exchange type %2!")
									.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkElement::intpara_t",
									NANDRAD::HydraulicNetworkElement::IP_ZoneId))
									.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
									itComp->m_heatExchangeType)),
									FUNC_ID);

					}
					// retrieve external heat transfer coefficient
					if(itComp->m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].name.empty()){
						throw IBK::Exception(IBK::FormatString("Missing parameteter '%1 for exchange type %2!")
									.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::para_t",
									NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient))
									.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
									itComp->m_heatExchangeType)),
									FUNC_ID);
					}
					m_p->m_ambientHeatTransferRefs[i] = &itComp->m_para[
						NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;

				} break;
				case NANDRAD::HydraulicNetworkComponent::NUM_HT:
					// No thermal exchange, nothing to initialize
				break;
				default: {
					throw IBK::Exception(IBK::FormatString("Heat exchange type %1 is not supported, yet!")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								itComp->m_heatExchangeType)),
								FUNC_ID);
				}
			}
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing HydraulicFlowElement with id %1")
								.arg(e.m_componentId), FUNC_ID);
		}
	}
	m_elementIds = networkModel.m_elementIds;

	// setup the enetwork
	try {
		m_p->setup(*networkModel.network(), nw.m_fluid);
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
	m_fluidTemperatures.resize(m_p->m_flowElements.size(), 293.15);

	// initialize all fluid temperatures
	unsigned int offset = 0;
	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *fe = m_p->m_flowElements[i];
		std::vector<double> fluidTemp(fe->nInternalStates());
		fe->initialTemperatures(&fluidTemp[0]);
		unsigned int nStates = fe->nInternalStates();
		double meanTemperature = 0;
		for(unsigned int n = 0; n < nStates; ++n) {
			meanTemperature += fluidTemp[n];
		}
		meanTemperature/= (double) nStates;
		m_fluidTemperatures[i] = meanTemperature;
		offset += nStates;
	}
}


void ThermalNetworkStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	if(!resDesc.empty())
		resDesc.clear();
	// mass flux vector is a result
	QuantityDescription desc("FluidTemperature", "C", "Internal fluid temperatures fo all network elements", false);
	// deactivate description;
	if(m_p->m_flowElements.empty())
		desc.m_size = 0;
	resDesc.push_back(desc);
	// set a description for each flow element
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_elementIds.size(); ++i) {
		desc.m_id = m_elementIds[i];
		resDesc.push_back(desc);
	}
}


const double * ThermalNetworkStatesModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// return y
	if(quantityName == std::string("y")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return &m_y[0];
		return nullptr;
	}
	if(quantityName == std::string("FluidTemperature")) {
		// reference to network
		if(quantity.m_id == id() && quantity.m_referenceType ==
		   NANDRAD::ModelInputReference::MRT_NETWORK) {
			if(!m_fluidTemperatures.empty())
				return &m_fluidTemperatures[0];
			return nullptr;
		}
		IBK_ASSERT(quantity.m_referenceType == NANDRAD::ModelInputReference::MRT_NETWORKELEMENT);
		// access to an element temperature
		std::vector<unsigned int>::const_iterator fIt =
				std::find(m_elementIds.begin(), m_elementIds.end(),
						  (unsigned int) quantity.m_id);
		// invalid index access
		if(fIt == m_elementIds.end())
			return nullptr;
		unsigned int pos = (unsigned int) std::distance(m_elementIds.begin(), fIt);
		return &m_fluidTemperatures[pos];
	}
	return nullptr;
}


unsigned int ThermalNetworkStatesModel::nPrimaryStateResults() const {
	return m_n;
}


void ThermalNetworkStatesModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// TODO: implement
}


void ThermalNetworkStatesModel::yInitial(double * y) const {
	// calculate specific enthalpy
	const double heatCapacity = m_network->m_fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	const double density = m_network->m_fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	// set internal states
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->initialTemperatures(y + offset);
		// calculate internal enthalpies for all flow elements
		const double volume = fe->volume();
		for(unsigned int n = 0; n < fe->nInternalStates(); ++n) {
			y[offset + n] *= volume * heatCapacity * density;
		}
		offset += fe->nInternalStates();
	}
}


int ThermalNetworkStatesModel::update(const double * y) {
	// copy states vector
	std::memcpy(&m_y[0], y, m_n*sizeof(double));
	// set internal states
	const double heatCapacity = m_network->m_fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	const double density = m_network->m_fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;

	unsigned int offset = 0;
	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *fe = m_p->m_flowElements[i];
		// calculate internal enthalpies for all flow elements
		fe->setInternalStates(y + offset);
		// calulte fluid temperatures
		const double volume = fe->volume();
		unsigned int nStates = fe->nInternalStates();
		double meanTemperature = 0.0;
		for(unsigned int n = 0; n < nStates; ++n) {
			meanTemperature += y[offset + n]/(volume * heatCapacity * density);
		}
		meanTemperature/= (double) nStates;
		m_fluidTemperatures[i] = meanTemperature;
		offset += nStates;
	}
	return 0;
}


} // namespace NANDRAD_MODEL
