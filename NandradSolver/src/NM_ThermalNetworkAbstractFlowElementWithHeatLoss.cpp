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

#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

namespace NANDRAD_MODEL {

void ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(double * ydot) {
	// balance is the same as with ThermalNetworkAbstractFlowElement, yet we subtract the heat loss.
	ThermalNetworkAbstractFlowElement::internalDerivatives(ydot);
	ydot[0] -= m_heatLoss;
}


void ThermalNetworkAbstractFlowElementWithHeatLoss::dependencies(const double *ydot, const double *y,
			const double *mdot, const double* TInflowLeft, const double*TInflowRight,
			std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	// add default dependencies for computed ydot, m_meanTemperature, InflowLeft and TInflowRight
	ThermalNetworkAbstractFlowElement::dependencies(ydot, y, mdot, TInflowLeft, TInflowRight, resultInputDependencies);

	// m_heatLoss depends on all inputs, just as ydot
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, TInflowRight) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, mdot) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, &m_meanTemperature) );

	// if we have an external reference, heat loss also depends on it
	if (m_heatExchangeValueRef != nullptr)
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeValueRef) );

	// ydot also depends on m_heatLoss
	resultInputDependencies.push_back(std::make_pair(ydot, &m_heatLoss) );
}


} // namespace NANDRAD_MODEL

