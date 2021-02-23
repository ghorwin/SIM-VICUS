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

/*! Instantiated for StaticPipe elements with HeatExchangeType set. */
class TNStaticPipeElement : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNStaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid,
				  const double &TExt);

	/*! Overloaded from ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

private:

	/*! pipe length in [m] */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in [m] */
	double							m_innerDiameter;

	/*! outer diameter of pipe in [m] */
	double							m_outerDiameter;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = 0.01;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! Equivalent u-value of the pipe wall and insulation per length of pipe in [W/mK] */
	double							m_UValuePipeWall;

	/*! Heat transfer coefficient from outer pipe wall to environment in [W/m2K] */
	double							m_outerHeatTransferCoefficient;

	/*! Reference to external temperature in K */
	const double*					m_externalTemperatureRef = nullptr;

};



// **** Static Adiabatic Pipe ***

/*! Instantiated for StaticPipe elements without HeatExchangeType set. */
class TNStaticAdiabaticPipeElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNStaticAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);
};



// **** Dynamic Pipe ***

/*! Instantiated for DynamicPipe elements with HeatExchangeType set. */
class TNDynamicPipeElement : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid,
				  const double &TExt);

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const override { return m_nVolumes;}

	/*! Function for setting initial temperature
	 * for each model.*/
	virtual void setInitialTemperature(double T0) override;

	/*! Function for retrieving initial states.*/
	virtual void initialInternalStates(double *y0) override;

	/*! Function for setting internal states.*/
	virtual void setInternalStates(const double *y) override;

	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalDerivatives(double *ydot) override;

	/*! Overrides ThermalNetworkAbstractFlowElement::outflowTemperature(). */
	virtual double outflowTemperature() override;

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	virtual void setInflowTemperature(double Tinflow) override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	virtual void dependencies(const double *ydot, const double *y,
							  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
							  std::vector<std::pair<const double *, const double *> > & ) const override;

private:

	/*! Number of discretization volumes */
	unsigned int					m_nVolumes;

	/*! Volume of all pipe deiscretization elements*/
	double							m_discVolume = -999;

	/*! Lengths of of all pipe volumes*/
	double							m_discLength = -999;

	/*! Fluid temperatures for all discretization volumes J/kg */
	std::vector<double>				m_temperatures;

	/*! Fluid temperatures for all discretization volumes  W */
	std::vector<double>				m_heatLosses;

	/*! pipe length in m */
	double							m_length = -999;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_innerDiameter = -999;

	/*! outer diameter of pipe in m */
	double							m_outerDiameter = -999;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = -999;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! thermal resistance of the pipe wall in Km2/W */
	double							m_UValuePipeWall = -999;

	/*! Heat transfer coefficient from outer pipe wall to environment in W/m2K */
	double							m_outerHeatTransferCoefficient = -999;

	/*! Reference to external temperature in K */
	const double*					m_externalTemperatureRef = nullptr;

};



// **** Dynamic Adiabatic Pipe ***

class TNDynamicAdiabaticPipeElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNDynamicAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const override { return m_nVolumes; }

	/*! Function for retrieving initial states of each volume after initial temperature is set. */
	void initialInternalStates(double *y0) override;

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y) override;

	/*! Function for setting initial temperature for each volume. */
	void setInitialTemperature(double T0) override;

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot) override;

	/*! Overrides ThermalNetworkAbstractFlowElement::outflowTemperature(). */
	double outflowTemperature() override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const override;

private:

	/*! Number of discretization volumes */
	unsigned int					m_nVolumes;

	/*! Volume of all pipe deiscretization elements*/
	double							m_discVolume = -999;

	/*! Fluid temperatures for all discretization volumes J/kg */
	std::vector<double>				m_temperatures;
};


// **** Pump element with losses ***

class TNPumpWithPerformanceLoss : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNPumpWithPerformanceLoss(const NANDRAD::HydraulicFluid & fluid,
							  const NANDRAD::HydraulicNetworkComponent & comp,
							  const double &pRef);

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

private:
	/*! Reference to pressure head */
	const double *					m_pressureHeadRef = nullptr;

	/*! Pump efficiency */
	double							m_pumpEfficiency;

	/*! Motor efficiency*/
	double							m_motorEfficiency = 0.0;
};


// **** Pump element with losses ***

class TNHeatPumpIdealCarnot : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNHeatPumpIdealCarnot(const NANDRAD::HydraulicFluid & fluid,
							  const NANDRAD::HydraulicNetworkComponent & comp,
							  const double &QExt);

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

private:
	/*! Reference to external heat loss in W */
	const double*					m_externalHeatLossRef = nullptr;
	/*! Mean condender temperature [K]*/
	double							m_condenserMeanTemperature = 999;

	/*! Carnot effiency*/
	double							m_carnotEfficiency = 999;
};


// **** General adiabativ element ***

class TNAdiabaticElement : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNAdiabaticElement(const NANDRAD::HydraulicFluid & fluid, double fluidVolume);
};



// **** General element with given heat loss ***

class TNElementWithExternalHeatLoss : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNElementWithExternalHeatLoss(const NANDRAD::HydraulicFluid & fluid,
				  double fluidVolume,
				  const double &QExt);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Reference to external heat loss in W */
	const double*					m_externalHeatLossRef = nullptr;
};




// **** TNHeatPumpIdealCarnot ***

class TNHeatPumpIdealCarnot : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	TNHeatPumpIdealCarnot() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNHeatPumpIdealCarnot(const NANDRAD::HydraulicNetworkComponent & comp,
						const NANDRAD::HydraulicFluid & fluid,
						const double &heatFluxExtern);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNHeatPumpIdealCarnot();

	/*! Set fluid inlet and outlet nodal conditions. */
	void setNodalConditions(double mdot, double TInlet, double TOutlet);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Reference to external heat loss in W */
	const double*			m_externalHeatLoss = nullptr;

	/*! mean fluid temperature in condenser [C] */
	double					m_condenserMeanTemperature;

	/*! carnot efficiency factor [-] */
	double					m_carnotEfficiency;
};




} // namespace NANDRAD_MODEL

#endif // ThermalNetworkFlowElementsH
