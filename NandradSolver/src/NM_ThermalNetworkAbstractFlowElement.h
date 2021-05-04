#ifndef ThermalNetworkAbstractFlowElementH
#define ThermalNetworkAbstractFlowElementH

#include <vector>
#include <utility>

#include "NM_QuantityDescription.h"

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

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &/*quantities*/) const { }

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &/*valRefs*/) const { }

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

	/*! Sets the computed mass flux in the element.
		The function may compute the temperature of outflowing fluid, whether it flows out at node 1 or 2.
		The energy/enthalpy has been set already in setInternalStates(), so a very simple implementation may
		just use the mean temperature as outflow temperature.
		\param massFlux Mass flux in [kg/s].
	*/
	virtual void setMassFlux(double massFlux) { m_massFlux = massFlux; }

	/*! Returns the temperature of outflowing fluid in [K].
		The energy/enthalpy has been set already in setInternalStates(). The default implementation assumes a
		single, well-mixed volume and simply returns the mean temperature as outflow temperature.

		This function is called from the node temperature calculation routine to compute the mixed temperatures at nodes.
	*/
	virtual double outflowTemperature() const { return m_meanTemperature; }

	/*! Once all node temperatures have been computed, the flow element gets the temperature of the inlowing fluid. */
	virtual void setInflowTemperature(double Tinflow) { m_inflowTemperature = Tinflow; }

	/*! Optional function for registering dependencies between derivatives and internal states.
		Default implementation simply adds dependencies between ydot and meanTemperature and _all_ inputs
		(y, mdot, TInflowLeft, TInflowRight).
	*/
	virtual void dependencies(const double */*ydot*/, const double */*y*/,
							  const double */*mdot*/, const double* /*TInflowLeft*/, const double*/*TInflowRight*/,
							  std::vector<std::pair<const double *, const double *> > & ) const;


	// Common variables for flow elements

	/*! Temperature of inflowing fluid in [K], regardless where it flows into element (depends on massFlux sign). */
	double							m_inflowTemperature = -999;

	/*! Fluid mass flux (positively defined from inlet to outlet). */
	double							m_massFlux = -999;

	/*! Fluid heat capacity [J/kgK].
		Cached value from fluid properties.
	*/
	double							m_fluidHeatCapacity = -999;

	/*! Fluid density [kg/m3].
		Cached value from fluid properties.
	*/
	double							m_fluidDensity = -999;

	/*! Fluid volume [m3]. */
	double							m_fluidVolume = -999;

	/*! Fluid (well-mixed) temperature in [K]. */
	double							m_meanTemperature = -999;

};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementH
