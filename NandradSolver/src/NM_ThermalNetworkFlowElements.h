#ifndef ThermalNetworkFlowElementsH
#define ThermalNetworkFlowElementsH

#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

#include "NANDRAD_HydraulicNetworkHeatExchange.h"


namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
	class LinearSplineParameter;
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
				  const NANDRAD::HydraulicFluid & fluid);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override{
		quantities.push_back(QuantityDescription("FluidVelocity","m/s","Fluid velocity", false));
		quantities.push_back(QuantityDescription("FluidVolumeFlow","m3/h","Fluid volume flow", false));
		quantities.push_back(QuantityDescription("FluidViscosity","m2/s","Fluid dynamic viscosity", false));
		quantities.push_back(QuantityDescription("Reynolds","---","Reynolds number", false));
		quantities.push_back(QuantityDescription("Prandtl","---","Prandtl number", false));
		quantities.push_back(QuantityDescription("Nusselt","---","Nusselt number", false));
		quantities.push_back(QuantityDescription("ThermalTransmittance","W/K","Total thermal transmittance of fluid and pipe wall", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_velocity);
		valRefs.push_back(&m_volumeFlow);
		valRefs.push_back(&m_viscosity);
		valRefs.push_back(&m_reynolds);
		valRefs.push_back(&m_prandtl);
		valRefs.push_back(&m_nusselt);
		valRefs.push_back(&m_thermalTransmittance);
	}

	/*! Overloaded from ThermalNetworkAbstractFlowElement::setInflowTemperature().
		This is the actual calculation function.
	*/
	void setInflowTemperature(double Tinflow) override;


private:

	/*! pipe length in [m] */
	double							m_length = -999;

	/*! hydraulic (inner) diameter of pipe in [m] */
	double							m_innerDiameter = -999;

	/*! outer diameter of pipe in [m] */
	double							m_outerDiameter = -999;

	/*! Effective flow cross-section [m2].
		\note This is the total cross section for fluid flow of all pipes (if m_nParallelPipes is larger than 1).
	*/
	double							m_fluidCrossSection = -999;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = 0.01;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! Equivalent u-value of the pipe wall and insulation per length of pipe in [W/mK] */
	double							m_UValuePipeWall;

	/*! Number of parallel pipes (=1 per default).*/
	unsigned int					m_nParallelPipes;

	/*! Fluid velocity [m/s]*/
	double							m_velocity = -999;

	/*! Fluid volume flow [m3/s]. */
	double							m_volumeFlow = -999;

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
				  const NANDRAD::HydraulicFluid & fluid);

	/*! Overloaded from ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

	/*! Sets the reference to external temperature. */
	void setHeatExchangeValueRef(const double * heatExchangeValueRef) override {
		m_externalTemperatureRef = heatExchangeValueRef;
	}

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

	/*! Reference to external temperature in K.
		Set to the result of a temperature-calculating object.
	*/
	const double *					m_externalTemperatureRef = nullptr;
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
				  const NANDRAD::HydraulicFluid & fluid);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override {
		quantities.push_back(QuantityDescription("FluidVolumeFlow","m3/h","Fluid volume flow", false));
		quantities.push_back(QuantityDescription("FluidVelocity","m/s","Fluid velocity", false));
		quantities.push_back(QuantityDescription("FluidViscosity","m2/s","Fluid dynamic viscosity", false));
		quantities.push_back(QuantityDescription("Reynolds","---","Reynolds number", false));
		quantities.push_back(QuantityDescription("Prandtl","---","Prandtl number", false));
		quantities.push_back(QuantityDescription("Nusselt","---","Nusselt number", false));
		quantities.push_back(QuantityDescription("ThermalTransmittance","W/K","Total thermal transmittance of fluid and pipe wall", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_volumeFlow);
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

	/*! Volume of all pipe discretization element [m3] s*/
	double							m_discVolume = -999;

	/*! Lengths of of all pipe volumes [m] */
	double							m_discLength = -999;

	/*! Effective flow cross-section [m2].
		\note This is the total cross section for fluid flow of all pipes (if m_nParallelPipes is larger than 1).
	*/
	double							m_fluidCrossSection = -999;

	/*! Fluid temperatures for all discretization volumes [K] */
	std::vector<double>				m_temperatures;

	/*! Fluid temperatures for all discretization volumes [W] */
	std::vector<double>				m_heatLosses;

	/*! Pipe length in [m] */
	double							m_length = -999;

	/*! Hydraulic (inner) diameter of pipe [m] */
	double							m_innerDiameter = -999;

	/*! Outer diameter of pipe [m] */
	double							m_outerDiameter = -999;

	/*! Number of parallel pipes (=1 per default).*/
	unsigned int					m_nParallelPipes;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double							m_fluidConductivity = -999;

	/*! Fluid dynamic viscosity [m/s] (temperature dependent).*/
	IBK::LinearSpline				m_fluidViscosity;

	/*! Fluid volume flow [m3/s]. */
	double							m_volumeFlow = -999;

	/*! Equivalent u-value of the pipe wall and insulation per length of pipe in [W/mK] */
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

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override{
		quantities.push_back(QuantityDescription("FluidVolumeFlow","m3/h","Fluid volume flow", false));
		quantities.push_back(QuantityDescription("FluidVelocity","m/s","Fluid velocity", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_volumeFlow);
		valRefs.push_back(&m_velocity);
	}

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

	/*! Cross section of pipe. */
	double							m_flowCrossSection = -999;

	/*! Fluid temperatures for all discretization volumes [K] */
	std::vector<double>				m_temperatures;

	/*! Fluid velocity [m/s]*/
	double							m_velocity = -999;

	/*! Fluid volume flow [m3/s]. */
	double							m_volumeFlow = -999;
};


// **** Pump element with losses ***

class TNPumpWithPerformanceLoss : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNPumpWithPerformanceLoss(const NANDRAD::HydraulicFluid & fluid,
							  const NANDRAD::HydraulicNetworkComponent & comp,
							  double pRef);

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
	/*! Constant pressure head [Pa] */
	double							m_pressureHead = 888;

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
	TNHeatPumpIdealCarnot(unsigned int flowElementId,
						  const NANDRAD::HydraulicFluid & fluid,
						  const NANDRAD::HydraulicNetworkComponent & comp);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override{
		quantities.push_back(QuantityDescription("COP","---", "Coefficient of performance for heat pump", false));
		quantities.push_back(QuantityDescription("ElectricalPower", "W", "Electrical power for heat pump", false));
		quantities.push_back(QuantityDescription("CondenserHeatFlux", "W", "Heat Flux at condenser side of heat pump", false));
	}

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override {
		valRefs.push_back(&m_COP);
		valRefs.push_back(&m_electricalPower);
		valRefs.push_back(&m_condenserHeatFlux);
	}

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references.
	*/
	void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

private:
	/*! Id number of flow element. */
	unsigned int							m_flowElementId = NANDRAD::INVALID_ID;

	/*! Temperatures from schedules [K]. These temperatures are interpreted within the model depending on which
	 * side of the heat pump is connected to the network (m_heatpumpIntegration) */
	const double							*m_scheduledTemperature1 = nullptr;
	const double							*m_scheduledTemperature2 = nullptr;

	/*! Mean condenser temperature [K]*/
	double									m_condenserMeanTemperature = 999;

	/*! Mean evaporator temperature [K]*/
	double									m_evaporatorMeanTemperature = 999;

	/*! Nominal evaporator temperature difference [K] */
	double									m_nominalTemperatureDifference = 999;

	/*! Maximum heating power of heat pump (condenser) in [W] */
	double m_condenserMaximumHeatFlux = 999999;

	/*! Actual heating power of heat pump (condenser) in [W] */
	double m_condenserHeatFlux = 999999;

	/*! Carnot efficiency [0...1] */
	double									m_carnotEfficiency = 999;

	/*! Coefficient of performance for heat pump */
	double									m_COP = 999;

	/*! Electrical power of the heat pump compressor [W] */
	double									m_electricalPower = 999;

	NANDRAD::HydraulicNetworkComponent::HeatPumpIntegration m_heatpumpIntegration = NANDRAD::HydraulicNetworkComponent::NUM_HP;

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
	TNElementWithExternalHeatLoss(const NANDRAD::HydraulicFluid & fluid, double fluidVolume);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot) override;
};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkFlowElementsH
