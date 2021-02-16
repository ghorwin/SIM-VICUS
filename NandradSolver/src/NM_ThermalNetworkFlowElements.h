#ifndef ThermalNetworkFlowElementsH
#define ThermalNetworkFlowElementsH

#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

#include <IBK_LinearSpline.h>

namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
}

#define PI				3.141592653589793238

namespace NANDRAD_MODEL {

// **** Static Pipe ***

class TNStaticPipeElement : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	TNStaticPipeElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNStaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid,
				  const double &TExt);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNStaticPipeElement();

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double TInlet, double TOutlet);

private:

	/*! pipe length in m */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_innerDiameter;

	/*! outer diameter of pipe in m */
	double							m_outerDiameter;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = 0.01;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! thermal resistance of the pipe wall in Km2/W */
	double							m_UValuePipeWall;

	/*! Heat transfer coefficient from outer pipe wall to environment in W/m2K */
	double							m_outerHeatTransferCoefficient;

	/*! Reference to external temperature in K */
	const double*					m_externalTemperatureRef = nullptr;
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

};



// **** Dynamic Pipe ***

class TNDynamicPipeElement : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	TNDynamicPipeElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid,
				  const double &TExt);

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
	void setNodalConditions(double mdot, double TInlet, double TOutlet);

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* hInlet, const double*hOutlet,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const;

private:

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

	/*! pipe length in m */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_innerDiameter;

	/*! outer diameter of pipe in m */
	double							m_outerDiameter;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = 0.01;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! thermal resistance of the pipe wall in Km2/W */
	double							m_UValuePipeWall;

	/*! Heat transfer coefficient from outer pipe wall to environment in W/m2K */
	double							m_outerHeatTransferCoefficient;

	/*! Reference to external temperature in K */
	const double*					m_externalTemperatureRef = nullptr;
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

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* TInlet, const double*TOutlet,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const;

private:

	/*! Number of discretization volumes */
	unsigned int					m_nVolumes;

	/*! Volume of all pipe deiscretization elements*/
	double							m_discVolume = 0.0;

	/*! Fluid specific enthalpies for all discretization volumes J/kg */
	std::vector<double>				m_specificEnthalpies;

	/*! Fluid temperatures for all discretization volumes J/kg */
	std::vector<double>				m_temperatures;
};


// **** Pump element with losses ***

class TNPumpWithPerformanceLoss : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	TNPumpWithPerformanceLoss() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNPumpWithPerformanceLoss(const NANDRAD::HydraulicFluid & fluid,
							  const NANDRAD::HydraulicNetworkComponent & comp,
							  const double &pRef);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNPumpWithPerformanceLoss();

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double hInlet, double hOutlet);

private:
	/*! Reference to pressure head */
	const double *					m_pressureHeadRef = nullptr;

	/*! Pump efficiency */
	double							m_pumpEfficiency;

	/*! Motor efficiency*/
	double							m_motorEfficiency = 0.0;
};


// **** General adiabativ element ***

class TNAdiabaticElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	TNAdiabaticElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNAdiabaticElement(const NANDRAD::HydraulicFluid & fluid,
				  double fluidVolume);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNAdiabaticElement();


};



// **** General element with given heat loss ***

class TNElementWithExternalHeatLoss : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	TNElementWithExternalHeatLoss() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNElementWithExternalHeatLoss(const NANDRAD::HydraulicFluid & fluid,
				  double fluidVolume,
				  const double &QExt);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNElementWithExternalHeatLoss();

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Reference to external heat loss in W */
	const double*					m_externalHeatLossRef = nullptr;
};




} // namespace NANDRAD_MODEL

#endif // ThermalNetworkFlowElementsH
