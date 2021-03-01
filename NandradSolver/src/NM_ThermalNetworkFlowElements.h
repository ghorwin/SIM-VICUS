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

// **** Pipe with single fluid volume but including a steady state temperature distribution***

/*! Instantiated for SimplePipe elements with HeatExchangeType set. */
class TNSimplePipeElement : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNSimplePipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid,
				  const double &TExt);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override{
		quantities.push_back(QuantityDescription("Velocity","m/s","Fluid velocity", false));
		quantities.push_back(QuantityDescription("Viscosity","m^2/s","Fluid dynamic viscosity", false));
		quantities.push_back(QuantityDescription("Reynolds","---","Reynolds number", false));
		quantities.push_back(QuantityDescription("Prandtl","---","Prandtl number", false));
		quantities.push_back(QuantityDescription("Nusselt","---","Nusselt number", false));
		quantities.push_back(QuantityDescription("ThermalTransmittance","W/K","Total thermal transmittance of fluid and pipe wall", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_velocity);
		valRefs.push_back(&m_viscosity);
		valRefs.push_back(&m_reynolds);
		valRefs.push_back(&m_prandtl);
		valRefs.push_back(&m_nusselt);
		valRefs.push_back(&m_thermalTransmittance);
	}

	/*! Overloaded from ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

private:

	/*! pipe length in [m] */
	double							m_length = -999;

	/*! hydraulic (inner) diameter of pipe in [m] */
	double							m_innerDiameter = -999;

	/*! outer diameter of pipe in [m] */
	double							m_outerDiameter = -999;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = 0.01;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! Equivalent u-value of the pipe wall and insulation per length of pipe in [W/mK] */
	double							m_UValuePipeWall;

	/*! Fluid velocity [m/s]*/
	double							m_velocity = -999;

	/*! Fluid dynamic viscosity in [m2/s]*/
	double							m_viscosity = -999;

	/*! Reynolds number in [---]*/
	double							m_reynolds = -999;

	/*! Prandl number in [---]*/
	double							m_prandtl = -999;

	/*! Nusselt number in [---]*/
	double							m_nusselt = -999;

	/*! Total thermal resistance in [W/K]*/
	double							m_thermalTransmittance = -999;

	/*! Heat transfer coefficient from outer pipe wall to environment in [W/m2K] */
	double							m_outerHeatTransferCoefficient = -999;

	/*! Reference to external temperature in K */
	const double*					m_externalTemperatureRef = nullptr;

};


//#define STATIC_PIPE_MODEL_ENABLED
#ifdef STATIC_PIPE_MODEL_ENABLED

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

	/*! Pipe length in [m] */
	double							m_length;

	/*! Hydraulic (inner) diameter of pipe in [m] */
	double							m_innerDiameter;

	/*! Outer diameter of pipe in [m] */
	double							m_outerDiameter;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = -999;

	/*! Fluid dynamic viscosity [m/s] (temperature dependent).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! Equivalent u-value of the pipe wall and insulation per length of pipe in [W/mK] */
	double							m_UValuePipeWall;

	/*! Heat transfer coefficient from outer pipe wall to environment in [W/m2K] */
	double							m_outerHeatTransferCoefficient;

	/*! Reference to external temperature [K] */
	const double*					m_externalTemperatureRef = nullptr;

};
#endif // STATIC_PIPE_MODEL_ENABLED


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

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override{
		quantities.push_back(QuantityDescription("Velocity","m/s","Fluid velocity", false));
		quantities.push_back(QuantityDescription("Viscosity","m^2/s","Fluid dynamic viscosity", false));
		quantities.push_back(QuantityDescription("Reynolds","---","Reynolds number", false));
		quantities.push_back(QuantityDescription("Prandtl","---","Prandtl number", false));
		quantities.push_back(QuantityDescription("Nusselt","---","Nusselt number", false));
		quantities.push_back(QuantityDescription("ThermalTransmittance","W/K","Total thermal transmittance of fluid and pipe wall", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_velocity);
		valRefs.push_back(&m_viscosity);
		valRefs.push_back(&m_reynolds);
		valRefs.push_back(&m_prandtl);
		valRefs.push_back(&m_nusselt);
		valRefs.push_back(&m_thermalTransmittance);
	}

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
	virtual double outflowTemperature() const override;

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	virtual void setInflowTemperature(double Tinflow) override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	virtual void dependencies(const double *ydot, const double *y,
							  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
							  std::vector<std::pair<const double *, const double *> > & ) const override;

private:

	/*! Number of discretization volumes */
	unsigned int					m_nVolumes;

	/*! Volume of all pipe deiscretization element [m3] s*/
	double							m_discVolume = -999;

	/*! Lengths of of all pipe volumes [m] */
	double							m_discLength = -999;

	/*! Fluid temperatures for all discretization volumes [K] */
	std::vector<double>				m_temperatures;

	/*! Fluid temperatures for all discretization volumes [W] */
	std::vector<double>				m_heatLosses;

	/*! Pipe length in [m] */
	double							m_length = -999;

	/*! hydraulic (inner) diameter of pipe [m] */
	double							m_innerDiameter = -999;

	/*! outer diameter of pipe [m] */
	double							m_outerDiameter = -999;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = -999;

	/*! Fluid dynamic viscosity [m/s] (temperature dependent).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! thermal resistance of the pipe wall in [W/mK] */
	double							m_UValuePipeWall = -999;

	/*! Heat transfer coefficient from outer pipe wall to environment in [W/m2K] */
	double							m_outerHeatTransferCoefficient = -999;

	/*! Fluid velocity [m/s]*/
	double							m_velocity = -999;

	/*! Fluid dynamic viscosity in [m2/s]*/
	double							m_viscosity = -999;

	/*! Reynolds number in [---]*/
	double							m_reynolds = -999;

	/*! Prandl number in [---]*/
	double							m_prandtl = -999;

	/*! Nusselt number in [---]*/
	double							m_nusselt = -999;

	/*! Total thermal resistance in [W/K]*/
	double							m_thermalTransmittance = -999;

	/*! Reference to external temperature in [K] */
	const double*					m_externalTemperatureRef = nullptr;

};



// **** Dynamic Adiabatic Pipe ***

/*! Instantiated for DynamicPipe elements without HeatExchangeType. */
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
	double outflowTemperature() const override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const override;

private:

	/*! Number of discretization volumes */
	unsigned int					m_nVolumes;

	/*! Volume of all pipe deiscretization elements [m3] */
	double							m_discVolume = -999;

	/*! Fluid temperatures for all discretization volumes [K] */
	std::vector<double>				m_temperatures;
};


// **** Pump element with losses ***

class TNPumpWithPerformanceLoss : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNPumpWithPerformanceLoss(const NANDRAD::HydraulicFluid & fluid,
							  const NANDRAD::HydraulicNetworkComponent & comp,
							  const double &pRef);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override{
		quantities.push_back(QuantityDescription("ElectricalPower","W","Requested electrical power for current working point", false));
		quantities.push_back(QuantityDescription("MechanicalPower","W","Mechanical power for current working point", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_electricalPower);
		valRefs.push_back(&m_mechanicalPower);
	}

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

private:
	/*! Reference to pressure head [Pa] */
	const double *					m_pressureHeadRef = nullptr;

	/*! Pump efficiency [0...1] */
	double							m_pumpEfficiency = -999;

	/*! Requested electrical power for current working point [W] */
	double							m_electricalPower = -999;

	/*! Mechanical power for current working point [W] */
	double							m_mechanicalPower = -999;
};


// **** TNHeatPumpIdealCarnot ***

class TNHeatPumpIdealCarnot : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNHeatPumpIdealCarnot(const NANDRAD::HydraulicFluid & fluid,
							const NANDRAD::HydraulicNetworkComponent & comp,
							const double &QExt);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override{
		quantities.push_back(QuantityDescription("PerformanceCoefficient","---","Performance coefficient for mechaniscal heat pumps", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_COP);
	}

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

private:
	/*! Reference to external heat loss in [W] */
	const double*					m_externalHeatLossRef = nullptr;
	/*! Mean condender temperature [K]*/
	double							m_condenserMeanTemperature = 999;

	/*! Carnot efficiency [0...1] */
	double							m_carnotEfficiency = 999;

	/*! Performance coefficient for mechaniscal heat pumps [0...1] */
	double							m_COP = 999;
};


// **** General adiabatic element ***

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

	/*! Reference to external heat loss [W] */
	const double*					m_externalHeatLossRef = nullptr;
};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkFlowElementsH
