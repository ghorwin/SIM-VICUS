#ifndef ThermalNetworkAbstractFlowElementWithHeatLossH
#define ThermalNetworkAbstractFlowElementWithHeatLossH

#include "NM_ThermalNetworkAbstractFlowElement.h"

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element with heat loss over the boundary.

*/
class ThermalNetworkAbstractFlowElementWithHeatLoss : public ThermalNetworkAbstractFlowElement {
public:
	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalDerivatives(double *ydot) override;

//	// Functions below are needed when model has exchange with environment

//	/*! Set ambient conditions. DISCUSS
//		Only for models with heat exchange to surrounding (earth, air, water...).
//		\param Tamb Temperature [K]
//		\param alphaAmb Heat exchange coefficient  [W/m2K]
//	*/
//	virtual void setAmbientConditions(double Tamb, double alphaAmb) = 0;


	/*! Heat loss over surface of flow element towards the environment in [W].
		This is a loss, i.e. positive means reduction of energy in flow element.
	*/
	double							m_heatLoss = 0.0;
};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementWithHeatLossH
