#include "NANDRAD_HydraulicNetworkElement.h"

#include "NANDRAD_HydraulicNetwork.h"

#include <NANDRAD_KeywordList.h>

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


void HydraulicNetworkElement::checkParameters(const HydraulicNetwork & nw,
											  const std::map<std::string, IBK::Path> &placeholders,
											  const std::vector<Zone> &zones,
											  const std::vector<ConstructionInstance> &conInstances)
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
		case HydraulicNetworkComponent::MT_HeatExchanger:
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnot:
			// nothing to check for
		break;

		// TODO : add checks for other components

		case HydraulicNetworkComponent::NUM_MT:
			throw IBK::Exception("Invalid network component model type!", FUNC_ID);
	}

	// check if given heat exchange type is supported for this component
	if (m_heatExchange.m_modelType != HydraulicNetworkHeatExchange::NUM_T){

		std::vector<unsigned int> hxTypes = HydraulicNetworkHeatExchange::availableHeatExchangeTypes(m_component->m_modelType);
		if (std::find(hxTypes.begin(), hxTypes.end(), m_heatExchange.m_modelType) == hxTypes.end())
			throw IBK::Exception(IBK::FormatString("Invalid type of heat exchange '%1' for component '%2'!")
								 .arg(KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", m_heatExchange.m_modelType))
								 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_component->m_modelType)), FUNC_ID);
	}

	// finally check for valid heat exchange parameters
	m_heatExchange.checkParameters(placeholders, zones, conInstances);
}


} // namespace NANDRAD
