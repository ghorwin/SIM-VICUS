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

	/*! Optional function for registering dependencies between derivatives and internal states.
		Re-implemented to add dependencies to computed variable m_heatLoss and from
		m_heatExchangeValueRef.
	*/
	virtual void dependencies(const double *ydot, const double *y,
				const double *mdot, const double* TInflowLeft, const double*TInflowRight,
				std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const override;

	/*! Heat loss over surface of flow element towards the environment in [W].
		This is a loss, i.e. positive means reduction of energy in flow element.
	*/
	double							m_heatLoss = 0.0; // Important: initialize with 0, since some models may never change it!

	/*! Value reference to external quantity. */
	const double					*m_heatExchangeValueRef = nullptr;
};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementWithHeatLossH
