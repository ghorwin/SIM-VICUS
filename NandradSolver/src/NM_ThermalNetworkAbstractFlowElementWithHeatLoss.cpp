#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

namespace NANDRAD_MODEL {

void ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(double * ydot) {
	// balance is the same as with ThermalNetworkAbstractFlowElement, yet we subtract the heat loss.
	ydot[0] = -m_heatLoss + m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}


} // namespace NANDRAD_MODEL

