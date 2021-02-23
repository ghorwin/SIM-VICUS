#include "NM_ThermalNetworkAbstractFlowElement.h"

#include <cmath>

namespace NANDRAD_MODEL {

ThermalNetworkAbstractFlowElement::~ThermalNetworkAbstractFlowElement() {
}


void ThermalNetworkAbstractFlowElement::setInitialTemperature(double T0) {
	m_meanTemperature = T0;
}


void ThermalNetworkAbstractFlowElement::initialInternalStates(double * y0) {
	// base implementation used exactly one fluid volume
	y0[0] = m_meanTemperature * m_fluidHeatCapacity * m_fluidVolume * m_fluidDensity;
}


void ThermalNetworkAbstractFlowElement::setInternalStates(const double * y) {
	m_meanTemperature = y[0] / (m_fluidHeatCapacity * m_fluidVolume * m_fluidDensity);
}


void ThermalNetworkAbstractFlowElement::internalDerivatives(double * ydot) {
	// basic implementation assume adiabatic flow element
	// and balance energy into and out of the fluid: kg/s * J/kgK * K
	ydot[0] = std::fabs(m_massFlux) * m_fluidHeatCapacity * (m_inflowTemperature - outflowTemperature());
}




} // namespace NANDRAD_MODEL

