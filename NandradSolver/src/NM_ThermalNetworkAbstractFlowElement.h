#ifndef ThermalNetworkAbstractFlowElementH
#define ThermalNetworkAbstractFlowElementH

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element. */
class ThermalNetworkAbstractFlowElement {
public:
	ThermalNetworkAbstractFlowElement() {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~ThermalNetworkAbstractFlowElement() { }

	/*! Function retrieving number of internal states.*/
	virtual unsigned int nInternalStates() const = 0;

	/*! Function for setting internal states such as internal enery.
		May be vector valued for layered temperature models.*/
	virtual void setInternalEnthalpies(const double *y) = 0;

	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalHeatLosses(double *ydot) = 0;

	/*! Set fluid inlet conditions. */
	virtual void setInletFluxes(double mdot, double Hdot) = 0;

	/*! Set ambient conditions. */
	virtual void setAmbientConditions(double Tamb, double alphaAmb) = 0;

	/*! Returns fluid outlet states: spcific enthalpy. */
	virtual void outletSpecificEnthalpy(double &h) const = 0;
};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementH
