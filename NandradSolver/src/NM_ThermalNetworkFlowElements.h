#ifndef ThermalNetworkFlowElementsH
#define ThermalNetworkFlowElementsH

#include "NM_ThermalNetworkAbstractFlowElement.h"

namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
}


namespace NANDRAD_MODEL {

// **** Static Pipe ***

class TNStaticPipeElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	TNStaticPipeElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNStaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNStaticPipeElement();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for retrieving initial states
	 * of each model after initial temperature is set.*/
	void initialInternalStates(double *y0);

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y);

	/*! Function for setting initial temperature
	 * for each model.*/
	void setInitialTemperature(double T0);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double hInlet, double hOutlet);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid mean temperature. */
	double meanTemperature() const;

	/*! Returns fluid inlet states: spcific enthalpy. */
	double inletSpecificEnthalpy() const;

	/*! Returns fluid outlet states: spcific enthalpy. */
	double outletSpecificEnthalpy() const;

	/*! Returns fluid inlet side states: temperature. */
	double inletTemperature() const;

	/*! Returns fluid outlet side states: temperature. */
	double outletTemperature() const;

	/*! Returns overall heat loss along the flow element. */
	double heatLoss() const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! pipe length in m */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_innerDiameter;

	/*! outer diameter of pipe in m */
	double							m_outerDiameter;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! thermal resistance of the pipe wall in Km2/W */
	double							m_UValuePipeWall;

	/*! Heat transfer coefficient from outer pipe wall to environment in W/m2K */
	double							m_outerHeatTransferCoefficient;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid inlet specific enthalpy */
	double							m_inletSpecificEnthalpy = 0.0;

	/*! Fluid outlet temperature */
	double							m_outletTemperature = 273.15;

	/*! Fluid outlet specific enthalpy */
	double							m_outletSpecificEnthalpy = 0.0;

	/*! Environamental temperature */
	double							m_ambientTemperature = 273.15;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Fluid specific enthalpy */
	double							m_specificEnthalpy = 0.0;

	/*! Fluid temperature */
	double							m_temperature = 273.15;

	/*! Heat loss through the pipe */
	double							m_heatLoss = 0.0;
};



// **** Static Adiabatic Pipe ***

class TNStaticAdiabaticPipeElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	TNStaticAdiabaticPipeElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNStaticAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNStaticAdiabaticPipeElement();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for retrieving initial states
	 * of each model after initial temperature is set.*/
	void initialInternalStates(double *y0);

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y);

	/*! Function for setting initial temperature
	 * for each model.*/
	void setInitialTemperature(double T0);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double hInlet, double hOutlet);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid mean temperature. */
	double meanTemperature() const;

	/*! Returns fluid inlet states: spcific enthalpy. */
	double inletSpecificEnthalpy() const;

	/*! Returns fluid outlet states: spcific enthalpy. */
	double outletSpecificEnthalpy() const;

	/*! Returns fluid inlet side states: temperature. */
	double inletTemperature() const;

	/*! Returns fluid outlet side states: temperature. */
	double outletTemperature() const;

	/*! Returns overall heat loss along the flow element. */
	double heatLoss() const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid inlet specific enthalpy */
	double							m_inletSpecificEnthalpy = 0.0;

	/*! Fluid outlet temperature */
	double							m_outletTemperature = 273.15;

	/*! Fluid outlet specific enthalpy */
	double							m_outletSpecificEnthalpy = 0.0;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Fluid specific enthalpy */
	double							m_specificEnthalpy = 0.0;

	/*! Fluid temperature */
	double							m_temperature = 273.15;
};



// **** Dynamic Pipe ***

class TNDynamicPipeElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	TNDynamicPipeElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNDynamicPipeElement();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for retrieving initial states
	 * of each model after initial temperature is set.*/
	void initialInternalStates(double *y0);

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y);

	/*! Function for setting initial temperature
	 * for each model.*/
	void setInitialTemperature(double T0);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double hInlet, double hOutlet);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid mean temperature. */
	double meanTemperature() const;

	/*! Returns fluid inlet states: spcific enthalpy. */
	double inletSpecificEnthalpy() const;

	/*! Returns fluid outlet states: spcific enthalpy. */
	double outletSpecificEnthalpy() const;

	/*! Returns fluid inlet side states: temperature. */
	double inletTemperature() const;

	/*! Returns fluid outlet side states: temperature. */
	double outletTemperature() const;

	/*! Returns overall heat loss along the flow element. */
	double heatLoss() const;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* hInlet, const double*hOutlet,
					  const double *Qdot,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! Number of discretization volumes */
	unsigned int					m_nVolumes;

	/*! Volume of all pipe deiscretization elements*/
	double							m_discVolume = 0.0;

	/*! Lengths of of all pipe volumes*/
	double							m_discLength = 0.0;

	/*! Fluid specific enthalpies for all discretization volumes J/kg */
	std::vector<double>				m_specificEnthalpies;

	/*! Fluid temperatures for all discretization volumes J/kg */
	std::vector<double>				m_temperatures;

	/*! Fluid temperatures for all discretization volumes  W */
	std::vector<double>				m_heatLosses;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! pipe length in m */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_innerDiameter;

	/*! outer diameter of pipe in m */
	double							m_outerDiameter;

	/*! thermal resistance of the pipe wall in Km2/W */
	double							m_UValuePipeWall;

	/*! Heat transfer coefficient from outer pipe wall to environment in W/m2K */
	double							m_outerHeatTransferCoefficient;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid inlet specific enthalpy */
	double							m_inletSpecificEnthalpy = 0.0;

	/*! Fluid outlet temperature */
	double							m_outletTemperature = 273.15;

	/*! Fluid outlet specific enthalpy */
	double							m_outletSpecificEnthalpy = 0.0;

	/*! Environamental temperature */
	double							m_ambientTemperature = 273.15;

	/*! Fluid mean temperature K */
	double							m_meanTemperature = 273.15;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Heat loss through the pipe W */
	double							m_heatLoss = 0.0;
};


// **** Dynamic Adiabatic Pipe ***

class TNDynamicAdiabaticPipeElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	TNDynamicAdiabaticPipeElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNDynamicAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNDynamicAdiabaticPipeElement();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for retrieving initial states
	 * of each model after initial temperature is set.*/
	void initialInternalStates(double *y0);

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y);

	/*! Function for setting initial temperature
	 * for each model.*/
	void setInitialTemperature(double T0);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double hInlet, double hOutlet);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid mean temperature. */
	double meanTemperature() const;

	/*! Returns fluid inlet states: spcific enthalpy. */
	double inletSpecificEnthalpy() const;

	/*! Returns fluid outlet states: spcific enthalpy. */
	double outletSpecificEnthalpy() const;

	/*! Returns fluid inlet side states: temperature. */
	double inletTemperature() const;

	/*! Returns fluid outlet side states: temperature. */
	double outletTemperature() const;

	/*! Returns overall heat loss along the flow element. */
	double heatLoss() const;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* hInlet, const double*hOutlet,
					  const double *Qdot,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! Number of discretization volumes */
	unsigned int					m_nVolumes;

	/*! Volume of all pipe deiscretization elements*/
	double							m_discVolume = 0.0;

	/*! Fluid specific enthalpies for all discretization volumes J/kg */
	std::vector<double>				m_specificEnthalpies;

	/*! Fluid temperatures for all discretization volumes J/kg */
	std::vector<double>				m_temperatures;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid inlet specific enthalpy */
	double							m_inletSpecificEnthalpy = 0.0;

	/*! Fluid outlet temperature */
	double							m_outletTemperature = 273.15;

	/*! Fluid outlet specific enthalpy */
	double							m_outletSpecificEnthalpy = 0.0;

	/*! Fluid mean temperature K */
	double							m_meanTemperature = 273.15;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;
};


// **** HeatExchanger ***

class TNHeatExchanger : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	TNHeatExchanger() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNHeatExchanger(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNHeatExchanger();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for setting initial temperature
	 * for each model.*/
	void setInitialTemperature(double T0);

	/*! Function for retrieving initial states
	 * of each model after initial temperature is set.*/
	void initialInternalStates(double *y0);

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double hInlet, double hOutlet);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid mean temperature. */
	double meanTemperature() const;

	/*! Returns fluid inlet states: spcific enthalpy. */
	double inletSpecificEnthalpy() const;

	/*! Returns fluid outlet states: spcific enthalpy. */
	double outletSpecificEnthalpy() const;

	/*! Returns fluid inlet side states: temperature. */
	double inletTemperature() const;

	/*! Returns fluid outlet side states: temperature. */
	double outletTemperature() const;

	/*! Returns overall heat loss along the flow element. */
	double heatLoss() const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	double							m_heatFluxToAmbient;

	double							m_UAValue;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid inlet specific enthalpy */
	double							m_inletSpecificEnthalpy = 0.0;

	/*! Fluid outlet temperature */
	double							m_outletTemperature = 273.15;

	/*! Fluid outlet specific enthalpy */
	double							m_outletSpecificEnthalpy = 0.0;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Fluid specific enthalpy */
	double							m_specificEnthalpy = 0.0;

	/*! Fluid temperature */
	double							m_temperature = 273.15;

};




// **** Pump ***

class TNPump: public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	TNPump() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNPump(const NANDRAD::HydraulicNetworkElement & elem,
				const NANDRAD::HydraulicNetworkComponent & comp,
				const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNPump();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for setting initial temperature
	 * for each model.*/
	void setInitialTemperature(double T0);

	/*! Function for retrieving initial states
	 * of each model after initial temperature is set.*/
	void initialInternalStates(double *y0);

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double hInlet, double hOutlet);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid mean temperature. */
	double meanTemperature() const;

	/*! Returns fluid inlet states: spcific enthalpy. */
	double inletSpecificEnthalpy() const;

	/*! Returns fluid outlet states: spcific enthalpy. */
	double outletSpecificEnthalpy() const;

	/*! Returns fluid inlet side states: temperature. */
	double inletTemperature() const;

	/*! Returns fluid outlet side states: temperature. */
	double outletTemperature() const;

	/*! Returns overall heat loss along the flow element. */
	double heatLoss() const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid inlet specific enthalpy */
	double							m_inletSpecificEnthalpy = 0.0;

	/*! Fluid outlet temperature */
	double							m_outletTemperature = 273.15;

	/*! Fluid outlet specific enthalpy */
	double							m_outletSpecificEnthalpy = 0.0;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Fluid specific enthalpy */
	double							m_specificEnthalpy = 0.0;

	/*! Fluid temperature */
	double							m_temperature = 273.15;

};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkFlowElementsH
