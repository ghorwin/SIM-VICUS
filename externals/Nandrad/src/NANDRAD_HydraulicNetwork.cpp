#include "NANDRAD_HydraulicNetwork.h"

namespace NANDRAD {


void HydraulicNetwork::checkParameters()  {
	FUNCID(HydraulicNetwork::checkParameters);

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
