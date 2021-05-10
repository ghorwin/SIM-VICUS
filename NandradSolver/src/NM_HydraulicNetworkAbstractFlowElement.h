#ifndef HydraulicNetworkAbstractFlowElementH
#define HydraulicNetworkAbstractFlowElementH

namespace NANDRAD_MODEL {

/*! Defines the abstract interface for a flow element.
	The system function and partial derivative function must be implemented by child objects.
*/
class HydraulicNetworkAbstractFlowElement {
public:
	HydraulicNetworkAbstractFlowElement() {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~HydraulicNetworkAbstractFlowElement();

	/*! The flow element's system function. Actually evaluates the residual of the system function
		for a given combination of mass flux [kg/s], inlet and output pressures [Pa].
	*/
	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const = 0;

	/*! Returns partial derivatives of the system functions w.r.t. the three dependent variables. */
	virtual void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const = 0;

	/*! Sets fluid temperature [K] for all internal pipe volumes. */
	virtual void setFluidTemperature(double fluidTemp) { (void)fluidTemp; }

	/*! Sets fluid temperature [K] for all internal pipe volumes. */
	virtual void setHeatLoss(double heatLoss) { (void)heatLoss; }
};

} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkAbstractFlowElementH
