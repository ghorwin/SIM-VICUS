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

#include <IBK_LinearSpline.h>
#include <IBK_messages.h>

#include "NM_ThermalNetworkStatesModel.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_ThermalNetworkPrivate.h"
#include "NM_ThermalNetworkFlowElements.h"
#include "NM_KeywordList.h"


#include "NM_HydraulicNetworkModelPrivate.h"

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
									  const HydraulicNetworkModel &hydrNetworkModel,
									  const NANDRAD::SimulationParameter & simPara)
{
	FUNCID(ThermalNetworkStatesModel::setup);

	// store network pointer
	m_network = &nw;
	m_simPara = &simPara;
	// create implementation instance
	m_p = new ThermalNetworkModelImpl; // we take ownership

	// The hydraulic network is already initialized, so the data in 'networkModel ' can
	// be used during initialization.

	// copy element ids
	m_elementIds = hydrNetworkModel.m_elementIds;

	// copy node ids
	m_nodeIds = hydrNetworkModel.m_nodeIds;

	// copy zone node ids
	m_zoneNodeIds = hydrNetworkModel.m_zoneNodeIds;

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
				case NANDRAD::HydraulicNetworkComponent::MT_SimplePipe : {
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
							TNElementWithExternalHeatLoss * pipeElement = new TNElementWithExternalHeatLoss(e.m_id, m_network->m_fluid, volume,
																											*e.m_component);

							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
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
							pipeElement->m_heatExchangeValueRef = &e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].value;
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
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating :
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
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser :
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating :
							throw IBK::Exception(IBK::FormatString("Heat exchange model %1 cannot be used with DynamicPipe components.")
												 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", e.m_heatExchange.m_modelType)), FUNC_ID);

					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe


				case NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePump :
				case NANDRAD::HydraulicNetworkComponent::MT_ConstantMassFluxPump :
				case NANDRAD::HydraulicNetworkComponent::MT_ControlledPump :
				case NANDRAD::HydraulicNetworkComponent::MT_VariablePressurePump :
				{
					// get value reference to constant pressure ref parameter for constant pressure pump
					const double * pressureHeadRef =  &e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].value;
					// set pointer to pressure head computed by controlled pump for controlled pump
					if (e.m_component->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_ConstantMassFluxPump) {
						const HNConstantMassFluxPump * pump = dynamic_cast<const HNConstantMassFluxPump *>(hydrNetworkModel.m_p->m_flowElements[i]);
						IBK_ASSERT(pump != nullptr);
						pressureHeadRef = pump->pressureHeadRef();
					}
					else if (e.m_component->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_ControlledPump) {
						const HNControlledPump * pump = dynamic_cast<const HNControlledPump *>(hydrNetworkModel.m_p->m_flowElements[i]);
						IBK_ASSERT(pump != nullptr);
						pressureHeadRef = pump->pressureHeadRef();
					}
					else if (e.m_component->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_VariablePressurePump) {
						const HNVariablePressureHeadPump * pump = dynamic_cast<const HNVariablePressureHeadPump *>(hydrNetworkModel.m_p->m_flowElements[i]);
						IBK_ASSERT(pump != nullptr);
						pressureHeadRef = pump->pressureHeadRef();
					}
					IBK_ASSERT(pressureHeadRef != nullptr);
					// create pump model with heat loss
					TNPumpWithPerformanceLoss * element = new TNPumpWithPerformanceLoss(m_network->m_fluid,
									*e.m_component, pressureHeadRef);
					// add to flow elements
					m_p->m_flowElements.push_back(element); // transfer ownership
					m_p->m_heatLossElements.push_back(element); // no heat loss

				} break; // NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePump


				case NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger : {
					switch (e.m_heatExchange.m_modelType) {
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
						{
							// create generic flow element with given heat flux
							TNElementWithExternalHeatLoss * element = new TNElementWithExternalHeatLoss(
										e.m_id, m_network->m_fluid, e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value,
										*e.m_component);

							// add to flow elements
							m_p->m_flowElements.push_back(element); // transfer ownership
							m_p->m_heatLossElements.push_back(element); // copy of pointer
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
						case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating :
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T:
							throw IBK::Exception(IBK::FormatString("Heat exchange model %1 cannot be used with HeatExchanger components.")
												 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", e.m_heatExchange.m_modelType)), FUNC_ID);

					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger


				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide :
				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide :
				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide : {
					// create general model with given heat flux
					TNHeatPumpVariable * element = new TNHeatPumpVariable(m_network->m_fluid, e);
					// add to flow elements
					m_p->m_flowElements.push_back(element); // transfer ownership
					m_p->m_heatLossElements.push_back(element); // copy of pointer

				} break; // NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnot


				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide: {
					TNHeatPumpOnOff * element = new TNHeatPumpOnOff(m_network->m_fluid, e);
					m_p->m_flowElements.push_back(element); // transfer ownership
					m_p->m_heatLossElements.push_back(element); // no heat loss
				} break;


//				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSideWithBuffer : {
//					// create general model with given heat flux
//					TNHeatPumpWithBuffer * element = new TNHeatPumpWithBuffer(m_network->m_fluid, e);
//					m_p->m_flowElements.push_back(element); // transfer ownership
//					m_p->m_heatLossElements.push_back(element); // copy of pointer

//				} break; // NANDRAD::HydraulicNetworkComponent::MT_HeatPumpWithBuffer


				case NANDRAD::HydraulicNetworkComponent::MT_ControlledValve:
				case NANDRAD::HydraulicNetworkComponent::MT_ConstantPressureLossValve:
				case NANDRAD::HydraulicNetworkComponent::MT_PressureLossElement: {
					TNAdiabaticElement * element = new TNAdiabaticElement( m_network->m_fluid, e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value);
					m_p->m_flowElements.push_back(element); // transfer ownership
					m_p->m_heatLossElements.push_back(nullptr); // no heat loss
				} break;


				case NANDRAD::HydraulicNetworkComponent::MT_IdealHeaterCooler: {
					TNIdealHeaterCooler * element = new TNIdealHeaterCooler(e.m_id, m_network->m_fluid);
					m_p->m_flowElements.push_back(element); // transfer ownership
					m_p->m_heatLossElements.push_back(nullptr); // add nullptr
				} break;

				case NANDRAD::HydraulicNetworkComponent::NUM_MT:
				break; // just to make compiler happy
			}

		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing HydraulicFlowElement with id %1")
								.arg(e.m_componentId), FUNC_ID);
		}
	}


	// *** controller setup ***

	// some of our flow elements may have a flow controller, so we do another pass over all flow elements and
	// setup the communication between HydraulicFlowElements and ThermalHydraulicFlowElements

	// setup the network
	try {
		m_p->setup(*hydrNetworkModel.network(), nw.m_fluid);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error setting up flow network.", FUNC_ID);
	}


	// resize vectors
	m_n = 0;
	for (ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		m_n += fe->nInternalStates();
	}
	if (m_n == 0)
		throw IBK::Exception("Network requires at least one flow element with energy balance enabled!", FUNC_ID);
	m_y.resize(m_n,0.0);

	// resize reference values
	m_meanTemperatureRefs.resize(m_elementIds.size(), nullptr);
	m_heatExchangeSplineValues.resize(m_elementIds.size(), 0);

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

	// add fluid temperatures to descriptions
	QuantityDescription desc("FluidTemperature", "C", "Internal fluid temperature of network element", false);
	// adjust reference type
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_elementIds.size(); ++i) {
		desc.m_id = m_elementIds[i];
		resDesc.push_back(desc);
	}

	// export all heat exchange values
	for (unsigned int i=0; i<m_elementIds.size(); ++i) {
		switch (m_network->m_elements[i].m_heatExchange.m_modelType) {
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline : {
				QuantityDescription desc("HeatExchangeHeatLoss", "W", "Pre-described heat loss.", false);
				desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
				desc.m_id = m_network->m_elements[i].m_id;
				resDesc.push_back(desc);
			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser : {
				// currently, this result value is not directly used in other models, yet may be useful for FMI exchange
				QuantityDescription desc("HeatExchangeHeatLossCondenser", "W", "Pre-described heat loss at condenser of heat pump.", false);
				desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
				desc.m_id = m_network->m_elements[i].m_id;
				resDesc.push_back(desc);
			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator : {
				// currently, this result value is not directly used in other models, yet may be useful for FMI exchange
				QuantityDescription desc("HeatExchangeTemperatureEvaporator", "K", "Pre-described evaporator temperature of heat pump.", false);
				desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
				desc.m_id = m_network->m_elements[i].m_id;
				resDesc.push_back(desc);
			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating : {
				// currently, this result value is not directly used in other models, yet may be useful for FMI exchange
				QuantityDescription desc("HeatExchangeHeatingDemandSpaceHeating", "W", "Heating demand for space heating", false);
				desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
				desc.m_id = m_network->m_elements[i].m_id;
				resDesc.push_back(desc);
			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant :
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline : {
				QuantityDescription desc("HeatExchangeTemperature", "K", "Pre-described external temperature.", false);
				desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
				desc.m_id = m_network->m_elements[i].m_id;
				resDesc.push_back(desc);
			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
			case NANDRAD::HydraulicNetworkHeatExchange::NUM_T:
			break;
		}
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
		IBK_ASSERT(quantity.m_referenceType == NANDRAD::ModelInputReference::MRT_NETWORKELEMENT);
		// access to an element temperature
		std::vector<unsigned int>::const_iterator fIt = std::find(m_elementIds.begin(), m_elementIds.end(), (unsigned int) quantity.m_id);
		// invalid index access
		if (fIt == m_elementIds.end())
			return nullptr;
		unsigned int pos = (unsigned int) std::distance(m_elementIds.begin(), fIt);
		return m_meanTemperatureRefs[pos];
	}


	if (quantityName == std::string("HeatExchangeHeatLoss") ||
		quantityName == std::string("HeatExchangeHeatLossCondenser") ||
		quantityName == std::string("HeatExchangeHeatingDemandSpaceHeating"))
	{
		for (unsigned int i=0; i<m_elementIds.size(); ++i) {
			if (quantity.m_id != m_network->m_elements[i].m_id) continue; // not our element
			// special case: constant parameter
			const NANDRAD::HydraulicNetworkHeatExchange &heatExchange = m_network->m_elements[i].m_heatExchange;
			switch (heatExchange.m_modelType) {
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant:
					IBK_ASSERT(!heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].name.empty());
					return &heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].value;

				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating:
					return &m_heatExchangeSplineValues[i];

				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
				case NANDRAD::HydraulicNetworkHeatExchange::NUM_T: ; // nothing to do, not our quantity
			}
		}
	}

	if (quantityName == std::string("HeatExchangeTemperature") ||
		quantityName == std::string("HeatExchangeTemperatureEvaporator") )
	{
		for (unsigned int i=0; i<m_elementIds.size(); ++i) {
			if (quantity.m_id != m_network->m_elements[i].m_id) continue; // not our element
			// special case: constant parameter
			const NANDRAD::HydraulicNetworkHeatExchange &heatExchange = m_network->m_elements[i].m_heatExchange;
			switch (heatExchange.m_modelType) {
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
					IBK_ASSERT(!heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].name.empty());
					return &heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].value;

				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
					return &m_heatExchangeSplineValues[i];

				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant:
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating:
				case NANDRAD::HydraulicNetworkHeatExchange::NUM_T: ; // nothing to do, not our quantity
			}

		}
	}

	return nullptr;
}


int ThermalNetworkStatesModel::setTime(double t) {
//	IBK_FUNCID_Message(ThermalNetworkStatesModel::setTime)

	// update all spline values
	for (unsigned int i=0; i<m_elementIds.size(); ++i) {
		switch (m_network->m_elements[i].m_heatExchange.m_modelType) {
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser :
			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating : {
				m_heatExchangeSplineValues[i] = m_simPara->evaluateTimeSeries(t,
					m_network->m_elements[i].m_heatExchange.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss]);
			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline :
			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator : {
				m_heatExchangeSplineValues[i] = m_simPara->evaluateTimeSeries(t,
					m_network->m_elements[i].m_heatExchange.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature]);
			} break;

			default :;
		}
	}

	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->setTime(t);
	}

	return 0;
}



void ThermalNetworkStatesModel::calculateErrorWeightFactors(std::vector<double> & weights) {
	// by default weight enlargement factors are 1
	weights = std::vector<double>(m_n, 1.0);

	// add higher weight for all elements which are not pipes (we currently don't do that, just use 1.)
	IBK_ASSERT(m_network->m_elements.size() == m_p->m_flowElements.size());
	unsigned int i=0;
	for (unsigned int n=0; n<m_p->m_flowElements.size(); ++n) {
		const ThermalNetworkAbstractFlowElement *flowElem = m_p->m_flowElements[n];
		unsigned int nStates = flowElem->nInternalStates();
		// skip elements without states
		if (nStates == 0)
			continue;
//		NANDRAD::HydraulicNetworkComponent::ModelType type = m_network->m_elements[n].m_component->m_modelType;
		IBK_ASSERT(flowElem!=nullptr);

//		// for all elements that are not pipes (ie. heat exchangers/heat pumps etc.)
//		// increase the sensitivity for weights
//		if (type != NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe &&
//			type != NANDRAD::HydraulicNetworkComponent::MT_SimplePipe )
//			weights[i] = 1.;

		// for heat pump with buffer storages, it might make sense to increase the weight for higher accuracy of the buffer temperatures
		// as they are commonly a lot larger than other component volumes
		// for better performance, we may keep them at 1.
//		if (type == NANDRAD::HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSideWithBuffer){
//			weights[i+1] = 1.;
//			weights[i+2] = 1.;
//		}
		// increment counter for number of unknowns
		i += flowElem->nInternalStates();
	}
}


void ThermalNetworkStatesModel::stepCompleted(double t) {
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->stepCompleted(t);
	}
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
//	IBK_FUNCID_Message(ThermalNetworkStatesModel::update)
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
