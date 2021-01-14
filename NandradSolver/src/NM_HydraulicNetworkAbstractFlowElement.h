#ifndef HydraulicNetworkAbstractFlowElementH
#define HydraulicNetworkAbstractFlowElementH


namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element. */
class HydraulicNetworkAbstractFlowElement {
public:
	HydraulicNetworkAbstractFlowElement() {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~HydraulicNetworkAbstractFlowElement();

	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const = 0;

	/*! Returns partial derivatives. */
	virtual void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const = 0;

	/*! Function retrieving number of internal states.*/
	virtual unsigned int nInternalStates() const = 0;

	/*! Sets fluid temperature [K] for all internal pipe volumes. */
	virtual void setFluidTemperature(const double *fluidTemp) = 0;
};

} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkAbstractFlowElementH
