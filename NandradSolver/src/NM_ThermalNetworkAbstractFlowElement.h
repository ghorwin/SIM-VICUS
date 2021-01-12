#ifndef ThermalNetworkAbstractFlowElementH
#define ThermalNetworkAbstractFlowElementH

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element.
	Fluid inside thermal elements is treated as volume/discretized volumes
	with storage mass and thermal states such as temperature and internal enthalpy.
	Thus, balance equations are of dynamic type and iterated by global ODE solver.
	We call abstract function in  the following sequence
	\code
	//derived class from ThermalNetworkAbstractFlowElement
	Pipe model;
	Pipe nextModel;
	// define all pipes
	// ...
	model.setup();
	nextModel.setup();
	// ...
	// global solver initialization
	// ...
	model.initialTemperatures(&T0[offset]);
	offset += model.nInternalStates();
	nextModel.initialTemperatures(&T0[offset]);
	// ...
	// global solver step
	// ...
	model.setInternalStates(&y[offset]);
	offset += model.nInternalStates();
	nextModel.setInternalStates(&y[offset]);
	// ...
	// iteration through all flow models of the network
	// calculate specific enthalpy
	// ...
	double hOut;
	model.outletSpecificEnthalpy(hOut);
	// ...
	// calculate mass fluxes mdot
	// ...
	// set ambient conditions
	// ...
	model.setAmbientConditions(....,...);
	nextModel.setAmbientConditions(...,...);
	// ...
	// transport specific enthalpy from one element to the other
	// ...
	nextModel.setInletFluxes(mdot, mdot*hOut);
	// ...
	global solver retrieves derivatives
	// ...
	model.internalDerivatives(&ydot[offset]);
	offset += model.nInternalStates();
	nextModel.internalDerivatives(&ydot[offset]);
	// ...
*/
class ThermalNetworkAbstractFlowElement {
public:
	ThermalNetworkAbstractFlowElement() {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~ThermalNetworkAbstractFlowElement() { }

	/*! Function retrieving number of internal states.*/
	virtual unsigned int nInternalStates() const = 0;

	/*! Function for retreiving initial temperatures
	 * from each model.*/
	virtual void initialTemperatures(double *T0) const = 0;

	/*! Function for setting internal states.*/
	virtual void setInternalStates(const double *y) = 0;

	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalDerivatives(double *ydot) = 0;

	/*! Set fluid inlet conditions: inlet depends on mass flux direction. */
	virtual void setInletFluxes(double mdot, double Hdot) = 0;

	/*! Set ambient conditions. */
	virtual void setAmbientConditions(double Tamb, double alphaAmb) = 0;

	/*! Returns fluid outlet states: specific enthalpy. */
	virtual double outletSpecificEnthalpy() const = 0;

	/*! Returns overall heat loss along the flow element. */
	virtual double heatLoss() const = 0;

	/*! Returns fluid volume inside the flow element. */
	virtual double volume() const = 0;
};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementH
