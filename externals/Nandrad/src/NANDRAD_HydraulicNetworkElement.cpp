#include "NANDRAD_HydraulicNetworkElement.h"

#include "NANDRAD_HydraulicNetwork.h"

#include <NANDRAD_KeywordList.h>

#include <IBK_UnitList.h>

namespace NANDRAD {

HydraulicNetworkElement::HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
						unsigned int componentId, unsigned int pipeID, double length) :
	m_id(id),
	m_inletNodeId(inletNodeId),
	m_outletNodeId(outletNodeId),
	m_componentId(componentId),
	m_pipePropertiesId(pipeID)
{
	NANDRAD::KeywordList::setParameter(m_para, "HydraulicNetworkElement::para_t", P_Length, length);
}


void HydraulicNetworkElement::checkParameters(const HydraulicNetwork & nw) {
	FUNCID(HydraulicNetworkElement::checkParameters);

	// retrieve network component
	std::vector<NANDRAD::HydraulicNetworkComponent>::const_iterator coit =
			std::find(nw.m_components.begin(), nw.m_components.end(), m_componentId);
	if (coit == nw.m_components.end()) {
		throw IBK::Exception(IBK::FormatString("Missing component reference to id%1.")
							 .arg(m_componentId), FUNC_ID);
	}
	// set reference
	m_component = &(*coit);

	// search for all hydraulic parameters
	switch (m_component->m_modelType) {
		case NANDRAD::HydraulicNetworkComponent::MT_StaticPipe:
		case NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe :
		case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
		case NANDRAD::HydraulicNetworkComponent::MT_DynamicAdiabaticPipe : {
			// retrieve pipe properties
			if(m_pipePropertiesId == NANDRAD::INVALID_ID) {
				throw IBK::Exception("Missing pipe property reference!", FUNC_ID);
			}
			// invalid id
			std::vector<NANDRAD::HydraulicNetworkPipeProperties>::const_iterator pit =
					std::find(nw.m_pipeProperties.begin(), nw.m_pipeProperties.end(), m_pipePropertiesId);
			if (pit == nw.m_pipeProperties.end()) {
				throw IBK::Exception(IBK::FormatString("Invalid pipe properties reference with id %1.")
									 .arg(m_pipePropertiesId), FUNC_ID);
			}
			// set reference
			m_pipeProperties = &(*pit);

			// all pipes need parameter 'Lenght
			m_para[P_Length].checkedValue("Length", "m", "m", 0, false, std::numeric_limits<double>::max(), true,
										   "Length must be > 0 m.");

			// Note: Pipe properties have been checked already.
		}
		break;

		// TODO : add checks for other components
		default: break;
	}

	// check parameters required for thermal balances/heat exchange
	if(nw.m_modelType == HydraulicNetwork::MT_ThermalHydraulicNetwork) {

		// TODO : clarify which parameters are required for which model and check accordingly

		// decide which heat exchange is chosen
		switch(m_component->m_heatExchangeType) {

			case NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant: {
				// retrieve constant temperature
				if(!m_para[P_Temperature].name.empty()) {
					// check temperature
					m_para[P_Temperature].checkedValue("Temperature", "C", "C", -200.0, true, std::numeric_limits<double>::max(), true,
												   "Temperature must be >= -200 C.");
					// check external heat transfer coefficient
					if(m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].name.empty()){
						throw IBK::Exception(IBK::FormatString("Missing parameteter '%1 for exchange type %2!")
									.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::para_t",
									NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient))
									.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
									m_component->m_heatExchangeType)),
									FUNC_ID);
					}
				}
				else if(!m_para[P_HeatFlux].name.empty()) {
					m_para[P_HeatFlux].checkedValue("HeatFlux", "W",
						"W", std::numeric_limits<double>::lowest(), false, std::numeric_limits<double>::max(), false,
						nullptr);
				}
			} break;


			case NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature: {

				// check for zone id
				if(m_intPara[IP_ZoneId].name.empty()) {
					throw IBK::Exception(IBK::FormatString("Missing IntParameter '%1 for exchange type %2!")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkElement::intpara_t",
								IP_ZoneId))
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								m_component->m_heatExchangeType)),
								FUNC_ID);

				}
				// check for external heat transfer coefficient
				if(m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].name.empty()){
					throw IBK::Exception(IBK::FormatString("Missing parameteter '%1 for exchange type %2!")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::para_t",
								NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient))
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								m_component->m_heatExchangeType)),
								FUNC_ID);
				}
			} break;

			case NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile:

			case NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature: {
				throw IBK::Exception(IBK::FormatString("Heat exchange type %1 is not supported, yet!")
							.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
							m_component->m_heatExchangeType)),
							FUNC_ID);
			}
			case NANDRAD::HydraulicNetworkComponent::NUM_HT:
				// No thermal exchange, nothing to initialize
			break;
		}
	}
}


} // namespace NANDRAD
