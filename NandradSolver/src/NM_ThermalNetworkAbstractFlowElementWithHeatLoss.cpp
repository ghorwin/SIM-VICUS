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
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, ydot) );

	// if we have an external reference, heat loss also depends on it
	if (m_heatExchangeValueRef != nullptr)
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeValueRef) );

	// ydot also depends on m_heatLoss
	resultInputDependencies.push_back(std::make_pair(ydot, &m_heatLoss) );
}


} // namespace NANDRAD_MODEL

