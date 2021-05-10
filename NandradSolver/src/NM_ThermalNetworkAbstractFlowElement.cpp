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


void ThermalNetworkAbstractFlowElement::dependencies(const double *ydot, const double *y,
			const double *mdot, const double* TInflowLeft, const double*TInflowRight,
			std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	// meanTemperature is computed from y
	resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y) );

	// ydot depends on inflow conditions, mean temperature (=outflowTemperature()) and mdot
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowRight) );
	resultInputDependencies.push_back(std::make_pair(ydot, mdot) );
	resultInputDependencies.push_back(std::make_pair(ydot, &m_meanTemperature) );

	// outflow conditions depend on mean temperature
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, &m_meanTemperature) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, &m_meanTemperature) );
}




} // namespace NANDRAD_MODEL

