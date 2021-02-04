#ifndef ThermalNetworkAbstractFlowElementH
#define ThermalNetworkAbstractFlowElementH

#include <vector>
#include <utility>


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
	virtual ~ThermalNetworkAbstractFlowElement();

	/*! Function retrieving number of internal states.*/
	virtual unsigned int nInternalStates() const { return 1; }

	/*! Function for setting initial temperature
		for each model.*/
	virtual void setInitialTemperature(double T0);

	/*! Function for retrieving initial states
	 * of each model after initial temperature is set.*/
	virtual void initialInternalStates(double *y0);

	/*! Function for setting internal states.*/
	virtual void setInternalStates(const double *y);

	/*! Computes time derivatives and includes the entire model calculation functionality.
		\note Call this function as infrequently as possible.
	*/
	virtual void internalDerivatives(double *ydot);

	/*! Set fluid inlet and outlet nodal conditions.
		Convenience function to setting m_inletTemperature ... m_massFlux manually.
		Also computes m_inletSpecificEnthalpy and m_outletSpecificEnthalpy.
		\param TInletNode Temperature at inlet node [K].
		\param TOutletNode Temperature at outlet node [K].

		\note The calculated inletXXX and outletXXX values depend on flow direction, because
			we are implementing upwinding. That means for positive mdot, the provided
			values of TOutletNode is ignored.
	*/
	virtual void setNodalConditions(double mdot, double TInletNode, double TOutletNode);


	/*! Optional function for registering dependencies between derivatives and internal states.*/
	virtual void dependencies(const double */*ydot*/, const double */*y*/,
							  const double */*mdot*/, const double* /*hInlet*/, const double*/*hOutlet*/,
							  const double */*Qdot*/,
							  std::vector<std::pair<const double *, const double *> > & ) const
	{ }

	// Common variables for flow elements

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid inlet specific enthalpy [J/kg].
		The value depends on flow direction (upwinding).
	*/
	double							m_inletSpecificEnthalpy = 0.0;

	/*! Fluid outlet temperature
		The value depends on flow direction (upwinding).
	*/
	double							m_outletTemperature = 273.15;

	/*! Fluid outlet specific enthalpy [J/kg]. */
	double							m_outletSpecificEnthalpy = 0.0;

	/*! Fluid mass flux (positively defined from inlet to outlet). */
	double							m_massFlux = 0.0;

	/*! Fluid heat capacity [J/kgK].
		Cached value from fluid properties.
	*/
	double							m_fluidHeatCapacity = 0.0;

	/*! Fluid specific enthalpy [J/kg].
	*/
	double							m_fluidSpecificEnthalpy = 0.0;

	/*! Fluid density [kg/m3].
		Cached value from fluid properties.
	*/
	double							m_fluidDensity = 1000;

	/*! Fluid volume [m3].
	*/
	double							m_fluidVolume = 1000;


	/*! Fluid temperature */
	double							m_meanTemperature = 273.15;

};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementH
