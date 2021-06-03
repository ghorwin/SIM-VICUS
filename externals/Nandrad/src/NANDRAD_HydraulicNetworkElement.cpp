/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_HydraulicNetworkElement.h"

#include "NANDRAD_HydraulicNetwork.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Project.h"

#include <algorithm>


namespace NANDRAD {

HydraulicNetworkElement::HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
						unsigned int componentId, unsigned int pipeID, double length) :
	m_id(id),
	m_inletNodeId(inletNodeId),
	m_outletNodeId(outletNodeId),
	m_componentId(componentId),
	m_pipePropertiesId(pipeID)
{
	KeywordList::setParameter(m_para, "HydraulicNetworkElement::para_t", P_Length, length);
}


void HydraulicNetworkElement::checkParameters(const HydraulicNetwork & nw, const Project & prj)
{
	FUNCID(HydraulicNetworkElement::checkParameters);

	// retrieve network component
	std::vector<HydraulicNetworkComponent>::const_iterator coit =
			std::find(nw.m_components.begin(), nw.m_components.end(), m_componentId);
	if (coit == nw.m_components.end()) {
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkComponent with id #%1 does not exist.")
							 .arg(m_componentId), FUNC_ID);
	}
	// set reference
	m_component = &(*coit);

	// search for all hydraulic parameters
	switch (m_component->m_modelType) {
		case HydraulicNetworkComponent::MT_SimplePipe:
		case HydraulicNetworkComponent::MT_DynamicPipe : {
			// retrieve pipe properties
			if(m_pipePropertiesId == INVALID_ID) {
				throw IBK::Exception("Missing ID reference 'PipePropertiesId'!", FUNC_ID);
			}
			// invalid id
			std::vector<HydraulicNetworkPipeProperties>::const_iterator pit =
					std::find(nw.m_pipeProperties.begin(), nw.m_pipeProperties.end(), m_pipePropertiesId);
			if (pit == nw.m_pipeProperties.end()) {
				throw IBK::Exception(IBK::FormatString("Pipe property definition (HydraulicNetworkPipeProperties) with id #%1 does not exist.")
									 .arg(m_pipePropertiesId), FUNC_ID);
			}
			// set reference
			m_pipeProperties = &(*pit);

			// all pipes need parameter Length
			m_para[P_Length].checkedValue("Length", "m", "m", 0, false, std::numeric_limits<double>::max(), true,
										   "Length must be > 0 m.");

			// check number of parallel pipes, and if missing, default to 1
			if (m_intPara[IP_NumberParallelPipes].name.empty())
				m_intPara[IP_NumberParallelPipes].set("NumberParallelPipes", 1);
			if (m_intPara[IP_NumberParallelPipes].value <= 0)
				throw IBK::Exception("Parameter 'NumberParallelPipes' must be > 0!", FUNC_ID);
		}
		break;

		case HydraulicNetworkComponent::MT_ConstantPressurePump:
		case HydraulicNetworkComponent::MT_ConstantMassFluxPump:
		case HydraulicNetworkComponent::MT_HeatExchanger:
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnot:
		case HydraulicNetworkComponent::MT_HeatPumpReal:
			// nothing to check for
		break;

		// TODO : add checks for other components

		case HydraulicNetworkComponent::NUM_MT:
			throw IBK::Exception("Invalid network component model type!", FUNC_ID);
	}

	// check if given heat exchange type is supported for this component, but only for ThermoHydraulic networks
	if (nw.m_modelType == NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork) {
		std::vector<unsigned int> hxTypes = HydraulicNetworkHeatExchange::availableHeatExchangeTypes(m_component->m_modelType);
		if (std::find(hxTypes.begin(), hxTypes.end(), m_heatExchange.m_modelType) == hxTypes.end()) {
			if (m_heatExchange.m_modelType == HydraulicNetworkHeatExchange::NUM_T)
				throw IBK::Exception(IBK::FormatString("Heat exchange type required for component '%1'!")
									 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_component->m_modelType)), FUNC_ID);
			else
				throw IBK::Exception(IBK::FormatString("Invalid type of heat exchange '%1' for component '%2'!")
									 .arg(KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", m_heatExchange.m_modelType))
									 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_component->m_modelType)), FUNC_ID);
		}

		// check for valid heat exchange parameters
		m_heatExchange.checkParameters(prj.m_placeholders, prj.m_zones, prj.m_constructionInstances);
	}

	// set pointer to control element
	if (m_controlElementID != NANDRAD::INVALID_ID) {
		auto ctit  = std::find(nw.m_controlElements.begin(), nw.m_controlElements.end(), m_controlElementID);
		if (ctit == nw.m_controlElements.end()) {
			throw IBK::Exception(IBK::FormatString("ControlElement with id #%1 does not exist.")
								 .arg(m_controlElementID), FUNC_ID);
		}
		// set reference
		m_controlElement = &(*ctit);

		// for temperature difference we enforce heat exchange type 'HeatLossSpline' or 'HeatLossSplineCondenser'
		if(ctit->m_controlledProperty == NANDRAD::HydraulicNetworkControlElement::CP_TemperatureDifference) {
			// wrong component
			switch(m_component->m_modelType ) {
				case NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger:
				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnot:
					break;
				default:
					throw IBK::Exception("HydraulicNetworkControlElement with type 'TemperatureDifference' is only suppported for "
										 "HydraulicNetworkComponent 'HeatExchanger' or 'HeatPumpIdealCarnot'!", FUNC_ID);
			}
			// wrong heat exchange type
			switch(m_heatExchange.m_modelType ) {
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
				break;
				default:
					throw IBK::Exception(IBK::FormatString("Only HeatExchangeType 'HeatLossSpline' or 'HeatLossSplineCondenser' "
														   "is allowed in combination with HydraulicNetworkController property "
														   "'TemperatureDifference'!"), FUNC_ID);
			}
		}
		else if(ctit->m_controlledProperty == NANDRAD::HydraulicNetworkControlElement::CP_ThermostatValue) {
			// thermostat control is only allowed for pipes with temperature dependend heat exchange
			switch(m_component->m_modelType ) {
				case NANDRAD::HydraulicNetworkComponent::MT_SimplePipe:
				case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe:
					break;
				default:
					throw IBK::Exception("HydraulicNetworkControlElement with type 'ThermostatValue' is only supported for "
									 "HydraulicNetworkComponent 'SimplePipe' and 'DynamicPipe'!", FUNC_ID);
			}
			// wrong heat exchange type
			switch(m_heatExchange.m_modelType ) {
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureFMUInterface:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
				break;
				default:
					throw IBK::Exception(IBK::FormatString("Only HeatExchangeType 'Temperature' "
														   "is allowed in combination with HydraulicNetworkController property "
														   "'ThermostatValue'!"), FUNC_ID);
			}
		}
	}

}


} // namespace NANDRAD
