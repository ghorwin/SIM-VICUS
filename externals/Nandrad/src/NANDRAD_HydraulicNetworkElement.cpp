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
											  const std::vector<ConstructionInstance> &conInstances) {
	FUNCID(HydraulicNetworkElement::checkParameters);

	// retrieve network component
	std::vector<HydraulicNetworkComponent>::const_iterator coit =
			std::find(nw.m_components.begin(), nw.m_components.end(), m_componentId);
	if (coit == nw.m_components.end()) {
		throw IBK::Exception(IBK::FormatString("Missing/invalid reference to component with id #%1.")
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
				throw IBK::Exception("Missing pipe property reference!", FUNC_ID);
			}
			// invalid id
			std::vector<HydraulicNetworkPipeProperties>::const_iterator pit =
					std::find(nw.m_pipeProperties.begin(), nw.m_pipeProperties.end(), m_pipePropertiesId);
			if (pit == nw.m_pipeProperties.end()) {
				throw IBK::Exception(IBK::FormatString("Missing/invalid reference to pipe property with id #%1.")
									 .arg(m_pipePropertiesId), FUNC_ID);
			}
			// set reference
			m_pipeProperties = &(*pit);

			// all pipes need parameter Length
			m_para[P_Length].checkedValue("Length", "m", "m", 0, false, std::numeric_limits<double>::max(), true,
										   "Length must be > 0 m.");

			// check number of parallel pipes, and if missing, default to 1
			if(m_intPara[IP_NumberParallelPipes].value == 0)
				throw IBK::Exception("Value 0 is not allowed for parameter 'NumberOfParallelPipes!", FUNC_ID);
			if((unsigned int) m_intPara[IP_NumberParallelPipes].value == NANDRAD::INVALID_ID)
				m_intPara[IP_NumberParallelPipes].set("NumberParallelPipes",1);

		}
		break;

		// TODO : add checks for other components
		case HydraulicNetworkComponent::MT_ConstantPressurePump:
		case HydraulicNetworkComponent::MT_HeatExchanger:
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnot:	{
			if(m_pipePropertiesId != INVALID_ID) {
				throw IBK::Exception("Invalid attribute pipePropertiesId. This is only possible for pipes!", FUNC_ID);
			}
		} break;
		case HydraulicNetworkComponent::NUM_MT: {
			throw IBK::Exception("Invalid network component model type!", FUNC_ID);
		}
	}

	// finally check for valid heat exchange parameters
	m_heatExchange.checkParameters(placeholders, zones, conInstances);
}


} // namespace NANDRAD
