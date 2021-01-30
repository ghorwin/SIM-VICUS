#include "NANDRAD_HydraulicNetwork.h"

#include <algorithm>

namespace NANDRAD {


void HydraulicNetwork::checkParameters()  {
	FUNCID(HydraulicNetwork::checkParameters);

	if(m_elements.empty()) {
		throw IBK::Exception("Empty network is not allowed!", FUNC_ID);
	}

	// check our own properties, first
	switch (m_modelType) {
		case MT_HydraulicNetwork :
			m_para[P_DefaultFluidTemperature].checkedValue("DefaultFluidTemperature", "K",
														   "C", -50, true, 500, true,
														   "Fluid temperature should be in the range of -50..500 C.");
		break;

		case NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork:
			m_para[P_InitialFluidTemperature].checkedValue("InitialFluidTemperature", "K",
														   "C", -50, true, 500, true,
														   "Fluid temperature should be in the range of -50..500 C.");
		break;

		case NANDRAD::HydraulicNetwork::NUM_MT: break; // just to make compiler warnings disappear
	}

	// check reference pressure
	if (m_para[P_ReferencePressure].empty())
		throw IBK::Exception(IBK::FormatString("ReferencePressure must be determined"), FUNC_ID);

	// do not allow an empty network
	if (m_elements.empty())
		throw IBK::Exception(IBK::FormatString("Network has no elements"), FUNC_ID);
	if (m_components.empty())
		throw IBK::Exception(IBK::FormatString("Network has no components"), FUNC_ID);

	// check reference element id
	if (std::find(m_elements.begin(), m_elements.end(), m_referenceElementId) == m_elements.end())
		throw IBK::Exception(IBK::FormatString("referenceElementId must be the id of an existing element"), FUNC_ID);

	// check parameters of fluid
	try {
		m_fluid.checkParameters(m_modelType);
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error intializing fluid with id #%1.")
							 .arg(m_fluid.m_id), FUNC_ID);
	}
	// check all elements and fill references to components and pipe properties
	for(HydraulicNetworkElement &e : m_elements) {
		try {
			e.checkParameters(*this);
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error intializing network element with id #%1.")
								 .arg(e.m_id), FUNC_ID);
		}
	}

	// check parameters of all network elements
	for(HydraulicNetworkComponent &c : m_components) {
		try {
			c.checkParameters(m_modelType);
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error intializing network component with id #%1.")
								 .arg(c.m_id), FUNC_ID);
		}
	}
	// check parameters of all pipe properties
	for(HydraulicNetworkPipeProperties &p : m_pipeProperties) {
		try {
			p.checkParameters(m_modelType);
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error intializing pipe property with id #%1.")
								 .arg(p.m_id), FUNC_ID);
		}
	}
}


} // namespace NANDRAD
