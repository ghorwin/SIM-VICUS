#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

namespace NANDRAD_MODEL {

void ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(double * ydot) {
	// balance is the same as with ThermalNetworkAbstractFlowElement, yet we subtract the heat loss.
	ThermalNetworkAbstractFlowElement::internalDerivatives(ydot);
	ydot[0] -= m_heatLoss;
}


} // namespace NANDRAD_MODEL

