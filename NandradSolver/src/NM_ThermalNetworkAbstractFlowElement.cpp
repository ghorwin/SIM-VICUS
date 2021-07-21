/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NM_ThermalNetworkAbstractFlowElement.h"

#include <cmath>

namespace NANDRAD_MODEL {

ThermalNetworkAbstractFlowElement::~ThermalNetworkAbstractFlowElement() {
}


void ThermalNetworkAbstractFlowElement::setInitialTemperature(double T0) {
	m_meanTemperature = T0;
}


void ThermalNetworkAbstractFlowElement::initialInternalStates(double * y0) {
	if (nInternalStates() == 0)
		return;
	// base implementation used exactly one fluid volume
	y0[0] = m_meanTemperature * m_fluidHeatCapacity * m_fluidVolume * m_fluidDensity;
}


void ThermalNetworkAbstractFlowElement::setInternalStates(const double * y) {
	if (nInternalStates() == 0)
		return;
	m_meanTemperature = y[0] / (m_fluidHeatCapacity * m_fluidVolume * m_fluidDensity);
}


void ThermalNetworkAbstractFlowElement::internalDerivatives(double * ydot) {
	if (nInternalStates() == 0)
		return;
	// basic implementation assume adiabatic flow element
	// and balance energy into and out of the fluid: kg/s * J/kgK * K
	ydot[0] = std::fabs(m_massFlux) * m_fluidHeatCapacity * (m_inflowTemperature - outflowTemperature());
}


void ThermalNetworkAbstractFlowElement::dependencies(const double *ydot, const double *y,
			const double *mdot, const double* TInflowLeft, const double*TInflowRight,
			std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	// outflow conditions depend on mean temperature
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, &m_meanTemperature) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, &m_meanTemperature) );

	// stop here if we have no internal states
	if (nInternalStates() == 0)
		return;

	// meanTemperature is computed from y
	resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y) );

	// ydot depends on inflow conditions, mean temperature (=outflowTemperature()) and mdot.
	// Dependency on mean temperature is already defined via TInflowLeft and TInflowRight above.
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowRight) );
	resultInputDependencies.push_back(std::make_pair(ydot, mdot) );
}




} // namespace NANDRAD_MODEL

