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

#include <algorithm>

#include <IBK_messages.h>

#include "NANDRAD_HydraulicNetwork.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Project.h"


namespace NANDRAD {

HydraulicNetworkElement::HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
						unsigned int inletZoneId, unsigned int outletZoneId,
						unsigned int componentId, unsigned int pipeID, double length) :
	m_id(id),
	m_inletNodeId(inletNodeId),
	m_outletNodeId(outletNodeId),
	m_inletZoneId(inletZoneId),
	m_outletZoneId(outletZoneId),
	m_componentId(componentId),
	m_pipePropertiesId(pipeID)
{
	KeywordList::setParameter(m_para, "HydraulicNetworkElement::para_t", P_Length, length);
	for(IBK::IntPara &i : m_intPara)
		i.value = (int) NANDRAD::INVALID_ID;
}


HydraulicNetworkElement::HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
						unsigned int inletZoneId, unsigned int outletZoneId,
						unsigned int componentId, unsigned int controlElementId):
	m_id(id),
	m_inletNodeId(inletNodeId),
	m_outletNodeId(outletNodeId),
	m_inletZoneId(inletZoneId),
	m_outletZoneId(outletZoneId),
	m_componentId(componentId),
	m_pipePropertiesId(INVALID_ID),
	m_controlElementId(controlElementId)
{
	for(IBK::IntPara &i : m_intPara)
		i.value = (int) NANDRAD::INVALID_ID;
}


bool HydraulicNetworkElement::operator!=(const HydraulicNetworkElement &other) const {
	// check ids
	if (m_id != other.m_id ||
		m_inletNodeId != other.m_inletNodeId ||
		m_inletZoneId != other.m_inletZoneId ||
		m_outletNodeId != other.m_outletNodeId ||
		m_outletZoneId != other.m_outletZoneId ||
		m_componentId != other.m_componentId ||
		m_controlElementId != other.m_controlElementId ||
		m_pipePropertiesId != other.m_pipePropertiesId)

		return true;

	// chekc parameters
	for (unsigned int i=0; i<NUM_P; ++i){
		if (m_para[i] != other.m_para[i])
			return true;
	}

	for (unsigned int i=0; i<NUM_IP; ++i){
		if (m_intPara[i] != other.m_intPara[i])
			return true;
	}

	if	(m_heatExchange != other.m_heatExchange)
		return true;

	return false;
}


void HydraulicNetworkElement::checkParameters(const HydraulicNetwork & nw, const Project & prj) {
	FUNCID(HydraulicNetworkElement::checkParameters);

	// check whether inlet and oulet node/inlet and oulet zone are defined
	if(m_inletNodeId == NANDRAD::INVALID_ID && m_inletZoneId == NANDRAD::INVALID_ID)
		throw IBK::Exception(IBK::FormatString("Missing inletNodeId or inletZoneId."), FUNC_ID);
	if(m_outletNodeId == NANDRAD::INVALID_ID && m_outletZoneId == NANDRAD::INVALID_ID)
		throw IBK::Exception(IBK::FormatString("Missing outletNodeId or outletZoneId."), FUNC_ID);
	// censure that only one inlet and one outlet are defined
	if(m_inletNodeId != NANDRAD::INVALID_ID && m_inletZoneId != NANDRAD::INVALID_ID)
		throw IBK::Exception(IBK::FormatString("Only inletNodeId or inletZoneId is allowed!"), FUNC_ID);
	if(m_outletNodeId != NANDRAD::INVALID_ID && m_outletZoneId != NANDRAD::INVALID_ID)
		throw IBK::Exception(IBK::FormatString("Only outletNodeId or outletZoneId is allowed!"), FUNC_ID);

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

		case HydraulicNetworkComponent::MT_PressureLossElement :
		case HydraulicNetworkComponent::MT_ConstantPressurePump:
		case HydraulicNetworkComponent::MT_ControlledPump:
		case HydraulicNetworkComponent::MT_VariablePressurePump:
		{
			// check number of parallel elements, and if missing, default to 1
			if (m_intPara[IP_NumberParallelElements].name.empty())
				m_intPara[IP_NumberParallelElements].set("NumberParallelElements", 1);
			if (m_intPara[IP_NumberParallelElements].value <= 0)
				throw IBK::Exception("Parameter 'NumberParallelElements' must be > 0!", FUNC_ID);
		}
			break;
		case HydraulicNetworkComponent::MT_ConstantMassFluxPump:
		case HydraulicNetworkComponent::MT_HeatExchanger:
		case HydraulicNetworkComponent::MT_ControlledValve:
		case HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
		case HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide:
		case HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide:
		case HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide:
//		case HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSideWithBuffer:
		case HydraulicNetworkComponent::MT_IdealHeaterCooler:
		case HydraulicNetworkComponent::MT_ConstantPressureLossValve:
			// nothing to check for
			break;

		case HydraulicNetworkComponent::NUM_MT:
			throw IBK::Exception("Invalid network component model type!", FUNC_ID);
	}

	// *** Heat Exchange Parameter compatibility checks ***

	// check if given heat exchange type is supported for this component, but only for ThermoHydraulic networks
	if (nw.m_modelType == NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork) {
		std::vector<HydraulicNetworkHeatExchange::ModelType> hxTypes = HydraulicNetworkHeatExchange::availableHeatExchangeTypes(m_component->m_modelType);
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
		if (m_heatExchange.m_modelType != HydraulicNetworkHeatExchange::NUM_T)
			m_heatExchange.checkParameters(prj.m_placeholders, prj.m_zones, prj.m_constructionInstances, true);
	}
	else if (m_heatExchange.m_modelType != HydraulicNetworkHeatExchange::NUM_T) {
		IBK::IBK_Message("HydraulicNetworkHeatExchange parameter in element #%1 has no effect for HydraulicNetwork calculation.", IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
	}


	// *** Flow Controller Parameter compatibility checks ***

	if (m_controlElementId != NANDRAD::INVALID_ID) {
		// first check if referenced controller exists
		auto ctit  = std::find(nw.m_controlElements.begin(), nw.m_controlElements.end(), m_controlElementId);
		if (ctit == nw.m_controlElements.end()) {
			throw IBK::Exception(IBK::FormatString("ControlElement with id #%1 does not exist.")
								 .arg(m_controlElementId), FUNC_ID);
		}
		// set pointer to control element
		m_controlElement = &(*ctit);

		// get avaiable controlledProperties
		std::vector<NANDRAD::HydraulicNetworkControlElement::ControlledProperty> availableProps =
				NANDRAD::HydraulicNetworkControlElement::availableControlledProperties(m_component->m_modelType);

		// check if given controlledProperty is allowed
		if (std::find(availableProps.begin(), availableProps.end(), m_controlElement->m_controlledProperty) == availableProps.end()) {
				throw IBK::Exception(IBK::FormatString("Given ControlledProperty '%1' is not allowed for component '%2' with id #%3!")
									 .arg(KeywordList::Keyword("HydraulicNetworkControlElement::ControlledProperty", m_controlElement->m_controlledProperty))
									 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_component->m_modelType))
									 .arg(m_componentId), FUNC_ID);
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
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
				case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
				break;

				case NANDRAD::HydraulicNetworkHeatExchange::NUM_T:
				break;

				default:
					throw IBK::Exception(IBK::FormatString("Flow controller #%1 is of type '%2'. However, this controller is not compatible with "
														   "HeatExchangeType '%3'!")
										 .arg(m_controlElementId)
										 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkControlElement::ControlledProperty", m_controlElement->m_controlledProperty))
										 .arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", m_heatExchange.m_modelType)), FUNC_ID);
			}
		}

		else if (ctit->m_controlledProperty == HydraulicNetworkControlElement::CP_PressureDifferenceWorstpoint) {
			if (m_observedPressureDiffElementIds.m_values.empty())
				throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id #%1 has controller with type '%2' but empty ObservedPressureDiffElementIds. "
													   "You need to provide ObservedPressureDiffElementIds!")
													   .arg(m_id)
													   .arg(KeywordList::Keyword("HydraulicNetworkControlElement::ControlledProperty", m_controlElement->m_controlledProperty))
														, FUNC_ID);
		}
	}

}


} // namespace NANDRAD
