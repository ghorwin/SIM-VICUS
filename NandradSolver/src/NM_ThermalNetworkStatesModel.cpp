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

#include <IBK_LinearSpline.h>
#include <IBK_messages.h>

#include "NM_ThermalNetworkStatesModel.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_HydraulicNetworkModel_p.h"
#include "NM_ThermalNetworkPrivate.h"
#include "NM_ThermalNetworkFlowElements.h"
#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_SimulationParameter.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



// *** ThermalNetworkStatesModel members ***

ThermalNetworkStatesModel::~ThermalNetworkStatesModel() {
	delete m_p; // delete pimpl object
}


void ThermalNetworkStatesModel::setup(const NANDRAD::HydraulicNetwork & nw,
									  const HydraulicNetworkModel &networkModel,
									  const NANDRAD::SimulationParameter &simPara)
{
	FUNCID(ThermalNetworkStatesModel::setup);

	// store network pointer
	m_network = &nw;
	// create implementation instance
	m_p = new ThermalNetworkModelImpl; // we take ownership

	// The hydraulic network is already initialized, so the data in 'networkModel ' can
	// be used during initialization.

	// copy element ids
	m_elementIds = networkModel.m_elementIds;

	// We now loop over all flow elements of the network and create a corresponding thermal
	// model objects for _each_ of the hydraulic calculation objects.
	// The model objects are stored in m_p->m_flowElements vector.

	for (unsigned int i =0; i < nw.m_elements.size(); ++i) {
		const NANDRAD::HydraulicNetworkElement & e = nw.m_elements[i];
		IBK_ASSERT(e.m_component != nullptr);

		try {

			// Instantiate thermal flow element calculation objects.
			// The objects are selected based on a **combination** of modelType and heatExchangeType and
			// the parametrization of the calculation objects differs.

			switch (e.m_component->m_modelType) {
				case NANDRAD::HydraulicNetworkComponent::MT_SimplePipe :
				{
					IBK_ASSERT(e.m_pipeProperties != nullptr);

					// distinguish based on heat exchange type
					switch (e.m_heatExchange.m_modelType) {
						// create adiabatic pipe model
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T : {
							// calculate pipe volume
							const double d = e.m_pipeProperties->m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
							const double l = e.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
							double volume = PI/4. * d * d * l;
							// create pipe model with given heat flux
							TNAdiabaticElement * pipeElement = new TNAdiabaticElement( m_network->m_fluid, volume);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(nullptr); // no heat loss
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
						{
							// calculate pipe volume
							const double d = e.m_pipeProperties->m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
							const double l = e.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
							double volume = PI/4. * d * d * l;

							// create generic flow element with given heat flux
							TNElementWithExternalHeatLoss * pipeElement = new TNElementWithExternalHeatLoss(
										m_network->m_fluid, volume);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
							// for constant heat loss, already pass pointer to existing constant value
							if (e.m_heatExchange.m_modelType == NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant)
								pipeElement->m_heatExchangeValueRef = &e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].value;
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
							// toggle the define in NM_ThermalNetworkFlowElements.h
#ifdef STATIC_PIPE_MODEL_ENABLED
						{
							// create pipe model with heat exchange and static properties
							TNStaticPipeElement * pipeElement = new TNStaticPipeElement(e, *e.m_component,
																						*e.m_pipeProperties, m_network->m_fluid);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
							pipeElement->m_externalTemperatureRef = &e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].value;
						} break;
#endif // STATIC_PIPE_MODEL_ENABLED


						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
						{
							// create pipe model with heat exchange
							TNSimplePipeElement * pipeElement = new TNSimplePipeElement(e, *e.m_component,
																						*e.m_pipeProperties, m_network->m_fluid);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
							// for constant heat exchange type already store the pointer to the given temperature
							if (e.m_heatExchange.m_modelType ==NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant)
								pipeElement->m_heatExchangeValueRef = &e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].value;
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureFMUInterface :
							// TODO : Andreas, Milestone FMU-Networks
						break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
							throw IBK::Exception(IBK::FormatString("Heat exchange model %1 cannot be used with SimplePipe components.")
												 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", e.m_heatExchange.m_modelType)), FUNC_ID);


					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_SimplePipe


				case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
				{
					IBK_ASSERT(e.m_pipeProperties != nullptr);
					// distinguish based on heat exchange type
					switch (e.m_heatExchange.m_modelType) {
						// create adiabatic pipe model
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T : {
							TNDynamicAdiabaticPipeElement * pipeElement = new TNDynamicAdiabaticPipeElement(e,
																											*e.m_component,  *e.m_pipeProperties, m_network->m_fluid);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(nullptr); // no heat loss
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser :
							throw IBK::Exception(IBK::FormatString("Heat exchange model %1 cannot be used with DynamicPipe components.")
												 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", e.m_heatExchange.m_modelType)), FUNC_ID);

						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
						{
							// create pipe model with heat exchange
							TNDynamicPipeElement * pipeElement = new TNDynamicPipeElement(e,
																						  *e.m_component, *e.m_pipeProperties, m_network->m_fluid);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer

							// for the constant variant, store already reference to the given parameter value
							if (e.m_heatExchange.m_modelType ==NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant)
								pipeElement->m_heatExchangeValueRef = &e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].value;
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureFMUInterface :
							// TODO : Andreas, Milestone FMU-Networks
						break;

					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe


				case NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePump :
				{
					// create pump model with heat loss
					TNPumpWithPerformanceLoss * element = new TNPumpWithPerformanceLoss(m_network->m_fluid,
																						*e.m_component, e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].value);
					// add to flow elements
					m_p->m_flowElements.push_back(element); // transfer ownership
					m_p->m_heatLossElements.push_back(element); // no heat loss

				} break; // NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePump


				case NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger :
				{
					switch (e.m_heatExchange.m_modelType) {
						// create general adiabatic model
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T : {
							TNAdiabaticElement * element = new TNAdiabaticElement(m_network->m_fluid,
																				  e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value);
							// add to flow elements
							m_p->m_flowElements.push_back(element); // transfer ownership
							m_p->m_heatLossElements.push_back(nullptr); // no heat loss
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
						{
							// create general model with given heat flux
							TNElementWithExternalHeatLoss * element = new TNElementWithExternalHeatLoss(m_network->m_fluid,
																										e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value);
							// add to flow elements
							m_p->m_flowElements.push_back(element); // transfer ownership
							m_p->m_heatLossElements.push_back(element); // copy of pointer
							if (e.m_heatExchange.m_modelType == NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant)
								element->m_heatExchangeValueRef = &e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].value;
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureFMUInterface:
							throw IBK::Exception(IBK::FormatString("Heat exchange model %1 cannot be used with HeatExchanger components.")
												 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", e.m_heatExchange.m_modelType)), FUNC_ID);

					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger


				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnot :
				{
					switch (e.m_component->m_heatPumpIntegration) {
						case (NANDRAD::HydraulicNetworkComponent::HP_SourceSide): {

							// create general model with given heat flux
							TNHeatPumpIdealCarnot * element = new TNHeatPumpIdealCarnot(e.m_id, m_network->m_fluid, *e.m_component);
							// add to flow elements
							m_p->m_flowElements.push_back(element); // transfer ownership
							m_p->m_heatLossElements.push_back(element); // copy of pointer

						} break;
						case NANDRAD::HydraulicNetworkComponent::HP_SupplySide:
						case NANDRAD::HydraulicNetworkComponent::HP_SupplyAndSourceSide:
						case NANDRAD::HydraulicNetworkComponent::NUM_HP:
						{
							throw IBK::Exception(IBK::FormatString("Heat pump integration type %1 is not supported yet!")
												 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatPumpIntegration",
																					e.m_component->m_heatPumpIntegration)), FUNC_ID);
						}

					} // switch heat pump integration type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnot


				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpReal:
					// TODO Hauke
				break;


				case NANDRAD::HydraulicNetworkComponent::NUM_MT:
				break; // just to make compiler happy
			}

		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing HydraulicFlowElement with id %1")
								.arg(e.m_componentId), FUNC_ID);
		}
	}

	// setup the enetwork
	try {
		m_p->setup(*networkModel.network(), nw.m_fluid);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error setting up flow network.", FUNC_ID);
	}


	// resize vectors
	m_n = 0;
	for (ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		m_n += fe->nInternalStates();
	}
	m_y.resize(m_n,0.0);

	// resize reference values
	m_meanTemperatureRefs.resize(m_elementIds.size(), nullptr);

	// initialize all fluid temperatures
	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *fe = m_p->m_flowElements[i];

		double fluidTemp = m_network->m_para[NANDRAD::HydraulicNetwork::P_InitialFluidTemperature].value;
		fe->setInitialTemperature(fluidTemp);
		m_meanTemperatureRefs[i] = &fe->m_meanTemperature;
	}

	// remaining initialization related to flow element result value communication within NANDRAD model world
	// is done by ThermalNetworkBalanceModel
}


void ThermalNetworkStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	if(!resDesc.empty())
		resDesc.clear();
	QuantityDescription desc("FluidTemperature", "C", "Internal fluid temperature of network element", false);
	// adjust reference type
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
	if (quantityName == std::string("y")) {
		// whole vector access
		if (quantityName.m_index == -1)
			return &m_y[0];
		return nullptr;
	}
	if (quantityName == std::string("FluidTemperature")) {
		if (quantity.m_referenceType != NANDRAD::ModelInputReference::MRT_NETWORKELEMENT)
			return nullptr;
		// access to an element temperature
		std::vector<unsigned int>::const_iterator fIt = std::find(m_elementIds.begin(), m_elementIds.end(), (unsigned int) quantity.m_id);
		// invalid index access
		if (fIt == m_elementIds.end())
			return nullptr;
		unsigned int pos = (unsigned int) std::distance(m_elementIds.begin(), fIt);
		return m_meanTemperatureRefs[pos];
	}
	return nullptr;
}


void ThermalNetworkStatesModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & /*resultInputValueReferences*/) const {
	// the mean temperatures depend on the internal energies, but some elements like dynamic pipe
	// have several states that impact a single mean temperature

	// Note: This dependency is currently formulated in ThermalNetworkBalanceModel::stateDependencies().
	//       It won't work to publish the dependency here, since NandradModel::initSolverMatrix() does not
	//       yet take the m_meanTemperatureRefs into the result vector, causing an assert while building the matrix

	// TODO Anne, for consistency-sake, move dependency information here and adjust NandradModel::initSolverMatrix() ???
#if 0
	// offset always points to y-vector memory range of the next element
	unsigned int offset = 0;
	// loop over all elements
	for (unsigned int i=0; i<m_p->m_flowElements.size(); ++i) {
		for (unsigned int j=0; j<m_p->m_flowElements[i]->nInternalStates(); ++j)
			resultInputValueReferences.push_back(std::make_pair(m_meanTemperatureRefs[i], &m_y[offset + j]));
		offset += m_p->m_flowElements[i]->nInternalStates();
	}
#endif
}


void ThermalNetworkStatesModel::yInitial(double * y) {
	// set internal states
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->initialInternalStates(y + offset);
		offset += fe->nInternalStates();
	}
	// copy states
	std::memcpy(&m_y[0], y, m_n * sizeof(double));
}


int ThermalNetworkStatesModel::update(const double * y) {
	// copy states vector
	std::memcpy(&m_y[0], y, m_n*sizeof(double));

	// set internal states
	unsigned int offset = 0;
	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *fe = m_p->m_flowElements[i];
		// calculate internal enthalpies for all flow elements
		fe->setInternalStates(y + offset);
		// retrieve fluid temperatures
		unsigned int nStates = fe->nInternalStates();
		offset += nStates;
	}
	return 0;
}

} // namespace NANDRAD_MODEL
