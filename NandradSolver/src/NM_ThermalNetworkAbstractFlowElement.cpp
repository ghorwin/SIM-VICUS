#include "NM_ThermalNetworkAbstractFlowElement.h"

namespace NANDRAD_MODEL {

ThermalNetworkAbstractFlowElement::~ThermalNetworkAbstractFlowElement() {
}


void ThermalNetworkAbstractFlowElement::setInitialTemperature(double T0) {
	m_meanTemperature = T0;
	m_inletTemperature = T0;
	m_outletTemperature = T0;
	m_inletSpecificEnthalpy = T0 * m_fluidHeatCapacity;
	m_outletSpecificEnthalpy = T0 * m_fluidHeatCapacity;
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
	// and balance energy into and out of the fluid: kg/s * J/kg
	ydot[0] = m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}


void ThermalNetworkAbstractFlowElement::setNodalConditions(double mdot, double TInletNode, double TOutletNode) {
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if (mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = TInletNode * m_fluidHeatCapacity; // K * J/kgK = J/kg
		// calculate inlet temperature
		m_inletTemperature = TInletNode;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_meanTemperature * m_fluidHeatCapacity;
		// set outlet temperature
		m_outletTemperature = m_meanTemperature;
	}
	// reverse direction
	else {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = m_meanTemperature * m_fluidHeatCapacity;
		// calculate inlet temperature
		m_inletTemperature = m_meanTemperature;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = TOutletNode * m_fluidHeatCapacity; // K * J/kgK = J/kg
		// set outlet temperature
		m_outletTemperature = TOutletNode;
	}
}



} // namespace NANDRAD_MODEL

