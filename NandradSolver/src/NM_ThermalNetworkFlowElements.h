/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_ThermalNetworkFlowElementsH
#define NM_ThermalNetworkFlowElementsH

#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

#include "NANDRAD_HydraulicNetworkHeatExchange.h"


namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
	class LinearSplineParameter;
	class HydraulicNetworkControlElement;
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
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references.*/
	void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs. */
	void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	/*! Overloaded from ThermalNetworkAbstractFlowElement::setInflowTemperature().
		This is the actual calculation function.
	*/
	void setInflowTemperature(double Tinflow) override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const override;


private:

	/*! Id number of flow element. */
	unsigned int					m_flowElementId = NANDRAD::INVALID_ID;

	/*! Value reference to external temperature. */
	const double					*m_heatExchangeTemperatureRef = nullptr;

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

	/*! Total thermal transmittance multiplied with the surface area of a pipe segment in [W/K]. */
	double							m_UAValue = -999;

	/*! Heat transfer coefficient from outer pipe wall to environment in [W/m2K] */
	double							m_outerHeatTransferCoefficient = -999;

	friend class ThermalNetworkBalanceModel;
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
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references.*/
	void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs. */
	void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const override { return m_nVolumes;}

	/*! Function for setting initial temperature
	 * for each model.*/
	void setInitialTemperature(double T0) override;

	/*! Function for retrieving initial states.*/
	void initialInternalStates(double *y0) override;

	/*! Function for setting internal states.*/
	void setInternalStates(const double *y) override;

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot) override;

	/*! Overrides ThermalNetworkAbstractFlowElement::outflowTemperature(). */
	double outflowTemperature() const override;

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const override;

private:

	/*! Id number of flow element. */
	unsigned int					m_flowElementId = NANDRAD::INVALID_ID;

	/*! Value reference to external temperature. */
	const double					*m_heatExchangeTemperatureRef = nullptr;

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

	/*! Total thermal transmittance multiplied with the surface area of a pipe segment in [W/K]. */
	double							m_UAValue = -999;

	friend class ThermalNetworkBalanceModel;
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
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

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
							  const double * pressureHeadRef);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Overrides ThermalNetworkAbstractFlowElement::setInflowTemperature(). */
	void setInflowTemperature(double Tinflow) override;

private:
	/*! Value reference to pressure head [Pa] */
	const double					*m_pressureHeadRef = nullptr;

	/*! Pump efficiency [0...1] */
	double							m_pumpEfficiency = -999;

	/*! FractionOfMotorInefficienciesToFluidStream [0...1] */
	double							m_fractionOfMotorInefficienciesToFluidStream = -999;

	/*! Requested electrical power for current working point [W] */
	double							m_electricalPower = -999;

	/*! Mechanical power for current working point [W] */
	double							m_mechanicalPower = -999;

	/*! Heat loss to the environment [W]. */
	double							m_heatLossToEnvironment = 888;
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
	TNElementWithExternalHeatLoss(unsigned int flowElementId, const NANDRAD::HydraulicFluid & fluid, double fluidVolume);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot) override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references.*/
	void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs. */
	void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const override;

protected:

	/*! Id number of flow element. */
	unsigned int					m_flowElementId = NANDRAD::INVALID_ID;

	/*! Value reference to external temperature. */
	const double					*m_heatExchangeHeatLossRef = nullptr;

	/*! Temperature difference across flow element [K]. */
	double									m_temperatureDifference = 999;
};




// **** TNHeatPumpIdealCarnot ***

class TNHeatPumpIdealCarnot : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS

public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNHeatPumpIdealCarnot(const NANDRAD::HydraulicFluid & fluid,
						  const NANDRAD::HydraulicNetworkElement & e);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

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

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot) override;

	/*! Function for registering dependencies between derivaites, internal states and modelinputs.*/
	void dependencies(const double *ydot, const double *y,
					  const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					  std::vector<std::pair<const double *, const double *> > &resultInputDependencies ) const override;

private:
	/*! Cached parametrization for heat pump flow element. */
	const NANDRAD::HydraulicNetworkElement	*m_flowElement = nullptr;

	/*! Value reference to condenser heat flux. */
	const double							*m_heatExchangeCondensorHeatLossRef = nullptr;

	/*! Value reference to evaporator temperature. */
	const double							*m_heatExchangeEvaporatorTemperatureRef = nullptr;

	/*! Temperatures from schedules [K] which will be set through input references */
	const double							*m_condenserMeanTemperatureRef = nullptr;
	const double							*m_condenserOutletSetpointRef = nullptr;

	/*! Mean condenser temperature [K], can also be used as output */
	double									m_condenserMeanTemperature = 999;

	/*! Mean evaporator temperature [K], can also be used as output */
	double									m_evaporatorMeanTemperature = 999;

	/*! Maximum heating power of heat pump (condenser) in [W] */
	double									m_condenserMaximumHeatFlux = 999999;

	/*! Actual heating power of heat pump (condenser) in [W] */
	double									m_condenserHeatFlux = 999999;

	/*! Actual heating power of heat pump (condenser) in [W] */
	double									m_evaporatorHeatFlux = 999999;

	/*! Carnot efficiency [0...1] */
	double									m_carnotEfficiency = 999;

	/*! Coefficient of performance for heat pump */
	double									m_COP = 999;

	/*! Electrical power of the heat pump compressor [W] */
	double									m_electricalPower = 999;

	/*! Temperature difference across flow element [K]. */
	double									m_temperatureDifference = 999;

};




// **** TNHeatPumpReal ***

class TNHeatPumpReal : public ThermalNetworkAbstractFlowElementWithHeatLoss { // NO KEYWORDS

public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNHeatPumpReal(const NANDRAD::HydraulicFluid & fluid,
				   const NANDRAD::HydraulicNetworkElement & e);

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

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

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot) override;

private:
	/*! Cached parametrization for heat pump flow element. */
	const NANDRAD::HydraulicNetworkElement	*m_flowElement = nullptr;

	/*! Temperatures from schedules [K] which will be set through input references */
	const double							*m_condenserOutletSetpointRef = nullptr;
	const double							*m_onOffSignalRef = nullptr;

	/*! Actual heating power of heat pump (condenser) in [W] */
	double									m_condenserHeatFlux = 999999;

	/*! Actual heating power of heat pump (condenser) in [W] */
	double									m_evaporatorHeatFlux = 999999;

	/*! Coefficient of performance for heat pump */
	double									m_COP = 999;

	/*! Electrical power of the heat pump compressor [W] */
	double									m_electricalPower = 999;

	/*! Temperature difference across flow element [K]. */
	double									m_temperatureDifference = 999;

	double									m_condenserOutletTemperature = 999;
	double									m_evaporatorInletTemperature = 999;

	/*! polynom coefficients */
	std::vector<double>						m_coeffsQcond;
	std::vector<double>						m_coeffsPel;

};



// **** Ideal Heater / Cooler Model ***

class TNIdealHeaterCooler : public ThermalNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNIdealHeaterCooler(unsigned int flowElementId, const NANDRAD::HydraulicFluid & fluid);

	// *** ThermalNetworkAbstractFlowElement interface

	/*! We have no internal state. */
	unsigned int nInternalStates() const override { return 0; }
	void setInflowTemperature(double Tinflow) override;
	/*! Simply return given supply temperature. */
	double outflowTemperature() const override { return *m_supplyTemperatureScheduleRef; }
	/*! Publish request to supply temperature schedule and optional mass flux setpoint. */
	void inputReferences(std::vector<InputReference> & inputRefs) const override;
	void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	/*! Publishes individual model quantities via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

private:
	unsigned int	m_id = NANDRAD::INVALID_ID;
	const double	*m_supplyTemperatureScheduleRef = nullptr;

	/*! Heat loss needed to provide the given supply temperature (If we add heat this is negative).
		 Is not part of the base class since we use ThermalNetworkAbstractFlowElement */
	double			m_heatLoss = 888;

};



} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkFlowElementsH
