#ifndef ThermalNetworkAbstractFlowElementWithHeatLossH
#define ThermalNetworkAbstractFlowElementWithHeatLossH

#include "NM_ThermalNetworkAbstractFlowElement.h"


namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element with heat loss (over the boundary).
*/
class ThermalNetworkAbstractFlowElementWithHeatLoss : public ThermalNetworkAbstractFlowElement {
public:
	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalDerivatives(double *ydot) override;

	/*! Heat loss over surface of flow element towards the environment in [W].
		This is a loss, i.e. positive means reduction of energy in flow element.
	*/
	double							m_heatLoss = 0.0; // Important: initialize with 0, since some models may never change it!
	/*! Heat exchange type of hydraulic component.
		Corresponds to NANDRAD::HydraulicComponent::HeatExchangeType enum.
	*/
	int								m_heatExchangeType = -1;
};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementWithHeatLossH
