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

#include "NM_ThermalNetworkFlowElements.h"

#include "NM_Physics.h"
#include "NM_HydraulicNetworkFlowElements.h"

#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"
#include "NANDRAD_HydraulicNetworkControlElement.h"

#include "numeric"

#include "IBK_messages.h"


namespace NANDRAD_MODEL {

// *** TNSimplePipeElement ***

TNSimplePipeElement::TNSimplePipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & /*comp*/,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// copy number of pipes
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
	// compute fluid volume
	m_fluidCrossSection = PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;
}


void TNSimplePipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// check heat transfer type
	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs

	// note: velcoty is caluclated for a single pipe (but mass flux interpreted as flux through all parallel pipes
	m_velocity = m_volumeFlow/m_fluidCrossSection;

	m_viscosity = m_fluidViscosity.value(m_meanTemperature);
	m_reynolds = ReynoldsNumber(m_velocity, m_viscosity, m_innerDiameter);
	m_prandtl = PrandtlNumber(m_viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	m_nusselt = NusseltNumber(m_reynolds, m_prandtl, m_length, m_innerDiameter);
	// calculate inner heat transfer coefficient
	double innerHeatTransferCoefficient = m_nusselt * m_fluidConductivity /
											m_innerDiameter;

	if (m_outerHeatTransferCoefficient == 0.) {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_UAValue = m_length /
				(
					  1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI )
					+ 1.0 / m_UValuePipeWall
				);
	}
	else {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_UAValue = m_length /
				(
					  1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI)
					+ 1.0 / (m_outerHeatTransferCoefficient * m_outerDiameter * PI)
					+ 1.0 / m_UValuePipeWall
				);
	}

	IBK_ASSERT(m_heatExchangeValueRef != nullptr);
	const double externalTemperature = *m_heatExchangeValueRef;
	// calculate heat loss with given parameters
	// Q in [W] = DeltaT * UAValueTotal
	m_heatLoss = m_UAValue * (m_meanTemperature - externalTemperature) * m_nParallelPipes;
}


#ifdef STATIC_PIPE_MODEL_ENABLED

// *** TNStaticPipeElement ***

TNStaticPipeElement::TNStaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// compute fluid volume
	m_fluidVolume = 0.01;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;
}


void TNStaticPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// calculate inner heat transfer coefficient
	const double velocity = std::fabs(m_massFlux)/(m_fluidVolume * m_fluidDensity);
	const double viscosity = m_fluidViscosity.value(m_meanTemperature);
	const double reynolds = ReynoldsNumber(velocity, viscosity, m_innerDiameter);
	const double prandtl = PrandtlNumber(viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	double nusselt = NusseltNumber(reynolds, prandtl, m_length, m_innerDiameter);
	double innerHeatTransferCoefficient = nusselt * m_fluidConductivity /
											m_innerDiameter;

	// calculate heat transfer

	// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
	const double UAValueTotal = m_length /
			(
				  1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI)
				+ 1.0 / (m_outerHeatTransferCoefficient * m_outerDiameter * PI)
				+ 1.0 / m_UValuePipeWall
			);

	// Q in [W] = DeltaT * UAValueTotal
	IBK_ASSERT(m_heatExchangeValueRef != nullptr);
	const double externalTemperature = *m_heatExchangeValueRef;
	// calculate heat loss with given (for steady state model we interpret mean temperature as
	// outflow temperature and calculate a corresponding heat flux)
	m_heatLoss = m_massFlux * m_fluidHeatCapacity *
			(m_inflowTemperature - externalTemperature) *
			(1. - std::exp(-UAValueTotal / (std::fabs(m_massFlux) * m_fluidHeatCapacity )) );
}
#endif // STATIC_PIPE_MODEL_ENABLED



// *** DynamicPipeElement ***

TNDynamicPipeElement::TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// copy number of pipes
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
	// calculate fluid volume inside the pipe
	m_fluidCrossSection = PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;

	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	// in case given discretization length is larger than pipe length:
	// set discretization length to pipe length, so we have just one volume
	if (minDiscLength > m_length)
		minDiscLength = m_length;

	// claculte number of discretization elements
	m_nVolumes = (unsigned int) (m_length/minDiscLength);
	// resize all vectors
	m_temperatures.resize(m_nVolumes, 273.15);
	m_heatLosses.resize(m_nVolumes, 0.0);

	// calculate segment specific quantities
	m_discLength = m_length/(double) m_nVolumes;
	m_discVolume = m_fluidVolume/(double) m_nVolumes;
}


void TNDynamicPipeElement::setInitialTemperature(double T0) {
	// use standard implementation
	ThermalNetworkAbstractFlowElementWithHeatLoss::setInitialTemperature(T0);
	// fill vector valued quantiteis
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
}


void TNDynamicPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs
	// note: velcoty is caluclated for a single pipe (but mass flux interpreted as flux through all parallel pipes
	m_velocity = m_volumeFlow/m_fluidCrossSection;

	m_heatLoss = 0.0;

	// assume constant heat transfer coefficient along pipe, using average temperature
	m_viscosity = m_fluidViscosity.value(m_meanTemperature);
	m_reynolds = ReynoldsNumber(m_velocity, m_viscosity, m_innerDiameter);
	m_prandtl = PrandtlNumber(m_viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	m_nusselt = NusseltNumber(m_reynolds, m_prandtl, m_length, m_innerDiameter);
	double innerHeatTransferCoefficient = m_nusselt * m_fluidConductivity / m_innerDiameter;

	// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
	// see documentation above
	if(m_outerHeatTransferCoefficient == 0.) {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_UAValue = m_discLength /
				(
					  1.0 / ( innerHeatTransferCoefficient * m_innerDiameter * PI)
					+ 1.0 / m_UValuePipeWall
				);
	}
	else {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_UAValue = m_discLength /
				(
					  1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI)
					+ 1.0 / (m_outerHeatTransferCoefficient * m_outerDiameter * PI)
					+ 1.0 / m_UValuePipeWall
				);
	}


	IBK_ASSERT(m_heatExchangeValueRef != nullptr);
	const double externalTemperature = *m_heatExchangeValueRef;
	for (unsigned int i = 0; i < m_nVolumes; ++i) {
		// calculate heat loss with given parameters
		// TODO : Hauke, check equation... hier fehlt glaub ich noch der Faktor 1/m_nVolumes
		m_heatLosses[i] = m_UAValue * (m_temperatures[i] - externalTemperature) * m_nParallelPipes;
		// sum up heat losses
		m_heatLoss += m_heatLosses[i];
	}
}


void TNDynamicPipeElement::initialInternalStates(double * y0) {
	// copy internal states
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i] = m_temperatures[i] * m_fluidHeatCapacity * m_fluidDensity * m_discVolume ;
}


void TNDynamicPipeElement::setInternalStates(const double * y) {
	double temp = 0.0;
	// calculate specific enthalpy
	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		m_temperatures[i] = y[i] / ( m_discVolume * m_fluidDensity * m_fluidHeatCapacity);
		temp += m_temperatures[i];
	}
	m_meanTemperature = temp/m_nVolumes;
}


void TNDynamicPipeElement::internalDerivatives(double * ydot) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	if (m_massFlux >= 0.0) {
		// first element copies boundary conditions

		ydot[0] = -m_heatLosses[0] + m_massFlux * m_fluidHeatCapacity * (m_inflowTemperature  - m_temperatures[0]);
		for (unsigned int i = 1; i < m_nVolumes; ++i) {
			ydot[i] = -m_heatLosses[i] + m_massFlux * m_fluidHeatCapacity * (m_temperatures[i - 1] - m_temperatures[i]);
		}
	}
	else { // m_massFlux < 0
		// last element copies boundary conditions
		ydot[m_nVolumes - 1] = -m_heatLosses[m_nVolumes - 1] + m_massFlux * m_fluidHeatCapacity * (m_temperatures[m_nVolumes - 1] - m_inflowTemperature);
		for (unsigned int i = 0; i < m_nVolumes - 1; ++i) {
			ydot[i] = -m_heatLosses[i] + m_massFlux * m_fluidHeatCapacity * (m_temperatures[i] - m_temperatures[i + 1]);
		}
	}
}


double TNDynamicPipeElement::outflowTemperature() const {
	if (m_massFlux >= 0)
		return m_temperatures[m_nVolumes-1];
	else
		return m_temperatures[0];
}


void TNDynamicPipeElement::dependencies(const double *ydot, const double *y,
					const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	// The model computes ydot[...], m_meanTemperature and m_heatLoss

	// Dependencies correspond to a 1D Finite Volume discretization using first-order upwinding
	// flux approxiation. Since flow go into either direction, we use the dependencies for central differencing,
	// thus ydot[i] depends on y[i-1], y[i] and y[i+1]. The boundary elements depend on the inlet and outlet temperatures.
	// And of course, all ydot depend on the mass flux.

	// Also, since we have heat exchange to the environment, each ydot depends on the external temperature.

	// set dependency to inlet and outlet enthalpy
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, y) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, y + nInternalStates() - 1) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot + nInternalStates() - 1, TInflowRight) );

	for (unsigned int n = 0; n < nInternalStates(); ++n) {
		// set dependency to mean temperatures
		resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y + n) );

		// set dependency to ydot from y i-1, i and i+1
		if (n > 0)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n - 1) );

		resultInputDependencies.push_back(std::make_pair(ydot + n, y + n) );

		if (n < nInternalStates() - 1)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n + 1) );

		// set dependency to mdot
		resultInputDependencies.push_back(std::make_pair(ydot + n, mdot) );
		// and to external temperature
		resultInputDependencies.push_back(std::make_pair(ydot + n, m_heatExchangeValueRef));

		// set dependency to Qdot
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, y + n) );
	}

	// set dependency to Qdot
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, mdot) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeValueRef) );
}


// *** DynamicAdiabaticPipeElement ***

TNDynamicAdiabaticPipeElement::TNDynamicAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	double length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	double innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;

	m_flowCrossSection = PI/4. * innerDiameter * innerDiameter;
	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	// in case given discretization length is larger than pipe length:
	// set discretization length to pipe length, so we have just one volume
	if (minDiscLength > length)
		minDiscLength = length;

	// claculte number of discretization elements
	m_nVolumes = (unsigned int) (length/minDiscLength);
	// resize all vectors
	m_temperatures.resize(m_nVolumes, 273.15);
	// calculate fluid volume inside the pipe
	m_fluidVolume = PI/4. * innerDiameter * innerDiameter * length;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;

	// calculate segment specific quantities
	m_discVolume = m_fluidVolume/(double) m_nVolumes;
}


void TNDynamicAdiabaticPipeElement::setInitialTemperature(double T0) {
	// use standard implementation
	ThermalNetworkAbstractFlowElement::setInitialTemperature(T0);
	// fill vector valued quantiteis
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
}


void TNDynamicAdiabaticPipeElement::initialInternalStates(double * y0) {
	// copy internal states
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i] = m_temperatures[i] * m_fluidHeatCapacity * m_fluidDensity * m_discVolume ;
}


void TNDynamicAdiabaticPipeElement::setInternalStates(const double * y) {
	double temp = 0.0;
	// calculate specific enthalpy
	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		m_temperatures[i] = y[i] / ( m_discVolume * m_fluidDensity * m_fluidHeatCapacity);
		temp += m_temperatures[i];
	}
	m_meanTemperature = temp/m_nVolumes;

	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs
	m_velocity = m_volumeFlow/m_flowCrossSection;
}


void TNDynamicAdiabaticPipeElement::internalDerivatives(double * ydot) {
	if (m_massFlux >= 0.0) {
		// first element copies boundary conditions
		ydot[0] = m_massFlux * m_fluidHeatCapacity * (m_inflowTemperature  - m_temperatures[0]);
		for(unsigned int i = 1; i < m_nVolumes; ++i) {
			ydot[i] = m_massFlux * m_fluidHeatCapacity * (m_temperatures[i - 1] - m_temperatures[i]);
		}
	}
	else { // m_massFlux < 0
		// last element copies boundary conditions
		ydot[m_nVolumes - 1] = m_massFlux * m_fluidHeatCapacity * (m_temperatures[m_nVolumes - 1] - m_inflowTemperature);
		for(unsigned int i = 0; i < m_nVolumes - 1; ++i) {
			ydot[i] = m_massFlux * m_fluidHeatCapacity * (m_temperatures[i] - m_temperatures[i + 1]);
		}
	}
}


double TNDynamicAdiabaticPipeElement::outflowTemperature() const {
	if (m_massFlux >= 0)
		return m_temperatures[m_nVolumes-1];
	else
		return m_temperatures[0];
}


void TNDynamicAdiabaticPipeElement::dependencies(const double *ydot, const double *y,
			const double *mdot, const double* TInflowLeft, const double*TInflowRight,
			std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	// Dependencies correspond to a 1D Finite Volume discretization using first-order upwinding
	// flux approxiation. Since flow go into either direction, we use the dependencies for central differencing,
	// thus ydot[i] depends on y[i-1], y[i] and y[i+1]. The boundary elements depend on the inlet and outlet temperatures.
	// And of course, all ydot depend on the mass flux.

	// set dependency to inlet and outlet enthalpy
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, y) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, y + nInternalStates() - 1) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot + nInternalStates() - 1, TInflowRight) );

	for (unsigned int n = 0; n < nInternalStates(); ++n) {
		// set dependency to mean temperatures
		resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y + n) );

		// set dependency to ydot from y i-1, i and i+1
		if (n > 0)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n - 1) );

		resultInputDependencies.push_back(std::make_pair(ydot + n, y + n) );

		if (n < nInternalStates() - 1)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n + 1) );

		// set dependency to mdot
		resultInputDependencies.push_back(std::make_pair(ydot + n, mdot) );
	}
}



// *** PumpWithPerformanceLoss ***

TNPumpWithPerformanceLoss::TNPumpWithPerformanceLoss(const NANDRAD::HydraulicFluid & fluid,
							const NANDRAD::HydraulicNetworkComponent & comp,
							const double * pressureHeadRef)
{
	// copy component properties
	m_fluidVolume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_pumpEfficiency = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PumpEfficiency].value;
	m_fractionOfMotorInefficienciesToFluidStream = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_FractionOfMotorInefficienciesToFluidStream].value;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// store pressure head
	m_pressureHeadRef = pressureHeadRef;
}


void TNPumpWithPerformanceLoss::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// mechanical power of pump is pressure head times volumetric flow
	// Pa * m3/s = N/m2 * m3/s = N*m/s

	// mechanical power = volume flow * pressure head
	m_mechanicalPower = std::fabs(m_massFlux/m_fluidDensity * *m_pressureHeadRef); // positive value!

	// efficiency is defined as portion of total electrical power used for mechanical
	// Pelectrical * m_pumpEfficiency = Pmechanical
	m_electricalPower = m_mechanicalPower/m_pumpEfficiency;

	// energy balance of pump: mechanical power + heating power = electrical power
	double heatingPower = m_electricalPower - m_mechanicalPower; // always a positive value!

	// compute fraction of heat that is supplied to the fluid
	m_heatLoss = - m_fractionOfMotorInefficienciesToFluidStream * heatingPower; // negative, because we heat up the fluid

	// store heat flux to the environment as optional output
	m_heatLossToEnvironment = heatingPower + m_heatLoss; // mind: m_heatLoss is negative
}



// *** AdiabaticElement ***

TNAdiabaticElement::TNAdiabaticElement(const NANDRAD::HydraulicFluid & fluid, double fluidVolume) {
	// copy fluid parameters
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	// store fluid volume
	m_fluidVolume = fluidVolume;
}



// *** ElementWithExternalHeatLoss ***

TNElementWithExternalHeatLoss::TNElementWithExternalHeatLoss(const NANDRAD::HydraulicFluid & fluid, double fluidVolume)
{
	m_fluidVolume = fluidVolume;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}


void TNElementWithExternalHeatLoss::internalDerivatives(double * ydot) {
	// set heat loss
	IBK_ASSERT(m_heatExchangeValueRef != nullptr);
	m_heatLoss = *m_heatExchangeValueRef;
	// use basic routine
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}




// *** TNHeatPumpIdealCarnot ***

TNHeatPumpIdealCarnot::TNHeatPumpIdealCarnot(unsigned int flowElementId,
											 const NANDRAD::HydraulicFluid & fluid,
											 const NANDRAD::HydraulicNetworkElement & e) :
	TNElementWithExternalHeatLoss(fluid, e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value),
	m_flowElement(&e)
{
	m_flowElementId = flowElementId;
	m_fluidVolume = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_carnotEfficiency = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_CarnotEfficiency].value;
	m_condenserMaximumHeatFlux = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_MaximumHeatingPower].value;
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}


void TNHeatPumpIdealCarnot::setInflowTemperature(double Tinflow) {
	FUNCID(TNHeatPumpIdealCarnot::setInflowTemperature);

	ThermalNetworkAbstractFlowElementWithHeatLoss::setInflowTemperature(Tinflow);

	const double MIN_TEMPERATURE_DIFFERENCE_CONDENSER = 4; // K

	switch (m_flowElement->m_component->m_modelType) {

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSourceSide: {
			// initialize all results with 0
			m_evaporatorHeatFlux = 0;
			m_condenserHeatFlux = 0;
			m_COP = 0.0;
			m_heatLoss = 0.0;
			m_electricalPower  = 0.0;
			m_evaporatorMeanTemperature = 0;
			m_temperatureDifference = 0;

			// get scheduled temperature
			IBK_ASSERT(m_condenserMeanTemperatureRef != nullptr);
			m_condenserMeanTemperature = *m_condenserMeanTemperatureRef;

			// cut condenser heat flux
			IBK_ASSERT(m_heatExchangeValueRef != nullptr);
			m_condenserHeatFlux = *m_heatExchangeValueRef;
			if (m_condenserHeatFlux > m_condenserMaximumHeatFlux)
				m_condenserHeatFlux = m_condenserMaximumHeatFlux;

			// compute temperature level of evaporator side -> m_inflowTemperature and
			// m_meanTemperature (fluid outlet temperature) are both known and given
			m_evaporatorMeanTemperature = (m_inflowTemperature + m_meanTemperature) / 2;

			// turn off heat pump if evaporator temperature is out of valid range

			// First check that COP is always > 1:
			// COP = carnotEfficiency * Tcond / (Tcond - Tevap) > 1
			// --> TEvap > Tcond ( 1 - carnotEfficiency )
			// for a very bad heat pump with carnot efficiency = 10%, we have a lower limit of approx. Tcond = 300; 300 * 0.9 = 270 K
			// generally, we do not want to go below -20 C.
			if (m_evaporatorMeanTemperature < 273.15 - 20 ||
				(m_evaporatorMeanTemperature < m_condenserMeanTemperature*(1-m_carnotEfficiency)) )
			{
				IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("Evaporator temperature < -20 C, turning of "
																	"HeatPumpIdealCarnot, flow element with id '%1'\n").arg(m_flowElementId),
													IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
			}
			else if (m_condenserMeanTemperature - m_evaporatorMeanTemperature < MIN_TEMPERATURE_DIFFERENCE_CONDENSER) {
				IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("Evaporator temperature must be at least %1 K lower than condenser temperature (%2 C)\n")
												  .arg(MIN_TEMPERATURE_DIFFERENCE_CONDENSER).arg(m_condenserMeanTemperature - 273.15),
													IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
			}
			else {
				// always > 1
				m_COP = m_carnotEfficiency * m_condenserMeanTemperature / (m_condenserMeanTemperature - m_evaporatorMeanTemperature);

				m_evaporatorHeatFlux = m_condenserHeatFlux * (m_COP - 1) / m_COP;
				m_heatLoss = m_evaporatorHeatFlux; // energy taken out of fluid medium
				m_electricalPower  = m_condenserHeatFlux - m_evaporatorHeatFlux; // same as "m_condenserHeatFlux/m_COP", electrical power of heat pump
				m_temperatureDifference = m_inflowTemperature - m_meanTemperature;
			}

		} break; // HP_SourceSide


		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSupplySide:{
			// initialize all results with 0
			m_evaporatorHeatFlux = 0;
			m_COP = 0.0;
			m_heatLoss = 0.0;
			m_electricalPower  = 0.0;
			m_temperatureDifference = 0;

			// get scheduled temperatures
			IBK_ASSERT(m_heatExchangeValueRef != nullptr);
			IBK_ASSERT(m_condenserOutletSetpointRef != nullptr);
			m_evaporatorMeanTemperature = *m_heatExchangeValueRef;
			const double outletSetpointTemperature = *m_condenserOutletSetpointRef;

			// condenser heat flux (heating power required by building/added to fluid)
			m_condenserHeatFlux = m_massFlux * m_fluidHeatCapacity * (outletSetpointTemperature - m_inflowTemperature);
			if (m_condenserHeatFlux > m_condenserMaximumHeatFlux)
				m_condenserHeatFlux = m_condenserMaximumHeatFlux;
			// heat pump can only add heat; also, we expect positive mass flux
			if (m_condenserHeatFlux < 0 || m_massFlux < 0)
				m_condenserHeatFlux = 0;
			else {
				m_condenserMeanTemperature = (m_inflowTemperature + m_meanTemperature) / 2;
				// heat pump physics only work when condenser temperature is above evaporator temperature
				// for examplanations of checks below, see above in HP_SourceSide
				if (m_evaporatorMeanTemperature < 273.15 - 20 ||
					(m_evaporatorMeanTemperature < m_condenserMeanTemperature*(1-m_carnotEfficiency)) )
				{
					IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("Evaporator temperature < -20 C, turning of "
																		"HeatPumpIdealCarnot, flow element with id '%1'\n").arg(m_flowElementId),
														IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				}
				else if (m_condenserMeanTemperature - m_evaporatorMeanTemperature < MIN_TEMPERATURE_DIFFERENCE_CONDENSER) {
					IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("Evaporator temperature must be at least %1 K lower than condenser temperature (%2 C)\n")
													  .arg(MIN_TEMPERATURE_DIFFERENCE_CONDENSER).arg(m_condenserMeanTemperature - 273.15),
														IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				}
				else {
					m_COP = m_carnotEfficiency *  m_condenserMeanTemperature / (m_condenserMeanTemperature - m_evaporatorMeanTemperature);
					m_evaporatorHeatFlux = m_condenserHeatFlux * (m_COP - 1) / m_COP; // heat taken from source
					m_heatLoss = - m_condenserHeatFlux; // negative, because building "removes" this heat from the fluid and heat loss is defined as positive quantity
					m_electricalPower  = m_condenserHeatFlux - m_evaporatorHeatFlux;
					m_temperatureDifference = m_meanTemperature - m_inflowTemperature;
				}
			}

		} break; // HP_SupplySide

		default: ; // just to make compiler happy
	}
}


void TNHeatPumpIdealCarnot::inputReferences(std::vector<InputReference> & inputRefs) const {

	switch (m_flowElement->m_component->m_modelType) {

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSourceSide: {
			InputReference ref;
			ref.m_id = m_flowElementId;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_name.m_name = "CondenserMeanTemperatureSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
		} break;

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSupplySide: {
			InputReference ref;
			ref.m_id = m_flowElementId;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_name.m_name = "CondenserOutletSetpointSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
		} break;

		default : ;
	}
}


void TNHeatPumpIdealCarnot::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	// now store the pointer returned for our input ref request and advance the iterator by one
	switch (m_flowElement->m_component->m_modelType) {
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSourceSide:
			m_condenserMeanTemperatureRef = *(resultValueRefs++); // CondenserMeanTemperatureSchedule
			break;
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSupplySide:
			m_condenserOutletSetpointRef = *(resultValueRefs++); // CondenserOutletSetpointSchedule
			break;
		default : ;
	}
}


void TNHeatPumpIdealCarnot::internalDerivatives(double *ydot) {
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}



// *** TNSupplyTemperatureAdapter ***

TNIdealHeaterCooler::TNIdealHeaterCooler(unsigned int flowElementId, const NANDRAD::HydraulicFluid & fluid) :
	m_id(flowElementId)
{
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}


void TNIdealHeaterCooler::setInflowTemperature(double Tinflow) {

	IBK_ASSERT(m_fluidOutletSetpointScheduleRef != nullptr);
	m_meanTemperature = *m_fluidOutletSetpointScheduleRef;
	double absMassFlux = std::fabs(m_massFlux);

	// heat needed to provide the given temperature (If we are heating up the fluid, this is positive)
	m_suppliedHeat = absMassFlux * m_fluidDensity * m_fluidHeatCapacity * (m_meanTemperature - Tinflow);

	if (m_massFluxSetpointRef != nullptr) {
		// compute implied bypass flow
		double massFluxByPass = *m_massFluxSetpointRef - absMassFlux;
		// compute blended temperature

		m_mixedReturnTemperature = Tinflow*absMassFlux + *m_fluidOutletSetpointScheduleRef*massFluxByPass;
		m_mixedReturnTemperature /= *m_massFluxSetpointRef + 1e-10; // add small offset to avoid diff-by-zero
	}
	else {
		// no mass flux schedule, no mixed temperature
		m_mixedReturnTemperature = m_meanTemperature;
	}
}


void TNIdealHeaterCooler::inputReferences(std::vector<InputReference> & inputRefs) const {
	InputReference ref;
	ref.m_id = m_id;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	ref.m_name.m_name = "FluidOutletSetpointSchedule";
	ref.m_required = true;
	inputRefs.push_back(ref);

	ref.m_name.m_name = "MassFluxSetpointSchedule";
	ref.m_required = false;
	inputRefs.push_back(ref);
}


void TNIdealHeaterCooler::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	m_fluidOutletSetpointScheduleRef = *(resultValueRefs++);

	m_massFluxSetpointRef = *(resultValueRefs++); // may be nullptr
}



// *** TNHeatPumpReal ***

TNHeatPumpReal::TNHeatPumpReal(unsigned int flowElementId, const NANDRAD::HydraulicFluid &fluid, const NANDRAD::HydraulicNetworkElement &e):
	TNElementWithExternalHeatLoss(fluid, e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value),
	m_flowElement(&e)
{
	m_flowElementId = flowElementId;
	m_fluidVolume = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_coeffsQcond = e.m_component->m_polynomCoefficients.m_values.at("QdotCondensator");
	m_coeffsPel = e.m_component->m_polynomCoefficients.m_values.at("Pel");
}


void TNHeatPumpReal::setInflowTemperature(double Tinflow)
{
	ThermalNetworkAbstractFlowElementWithHeatLoss::setInflowTemperature(Tinflow);

	IBK_ASSERT(m_condenserOutletSetpointRef != nullptr);
	IBK_ASSERT(m_onOffSignalRef != nullptr);

	// convenience
	const double &Tc = *m_condenserOutletSetpointRef - 273.15;
	const double &Te = Tinflow - 273.15;

	// heat pump is ON
	if (*m_onOffSignalRef > 0.5){

		// condensator heat flux polynom (the polynom result is in kW)
		m_condenserHeatFlux = 1000.0 * (m_coeffsQcond[0] + m_coeffsQcond[1] * Te + m_coeffsQcond[2] * Tc + m_coeffsQcond[3] * Te * Tc +
				m_coeffsQcond[4] * Te * Te + m_coeffsQcond[5] * Tc * Tc);
		// electrical power polynom (the polynom result is in kW)
		m_electricalPower = 1000.0 * (m_coeffsPel[0] + m_coeffsPel[1] * Te + m_coeffsPel[2] * Tc + m_coeffsPel[3] * Te * Tc +
				m_coeffsPel[4] * Te * Te + m_coeffsPel[5] * Tc * Tc);

		m_evaporatorHeatFlux = m_condenserHeatFlux - m_electricalPower;
		m_COP = m_condenserHeatFlux / m_electricalPower;
		m_temperatureDifference = m_meanTemperature - m_inflowTemperature;
		m_condenserOutletTemperature = *m_condenserOutletSetpointRef;
		m_evaporatorInletTemperature = m_inflowTemperature;

		// heat loss depends on model type
		if (m_flowElement->m_component->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_HeatPumpRealSourceSide){
			m_heatLoss = m_evaporatorHeatFlux;
		}
		else {
			// TODO MT_HeatPumpRealSupplySide
		}

	}

	// heat pump is OFF
	else {
			m_condenserHeatFlux = 0;
			m_evaporatorHeatFlux = 0;
			m_electricalPower = 0;
			m_COP = 0;
			m_temperatureDifference = 0;
			m_condenserOutletTemperature = 0;
			m_evaporatorInletTemperature = 0;
	}
}


void TNHeatPumpReal::inputReferences(std::vector<InputReference> &inputRefs) const
{
	switch (m_flowElement->m_component->m_modelType) {

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpRealSourceSide: {
			InputReference ref;
			ref.m_id = m_flowElementId;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_name.m_name = "CondenserOutletSetpointSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);

			InputReference ref2;
			ref2.m_id = m_flowElementId;
			ref2.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref2.m_name.m_name = "HeatPumpOnOffSignalSchedule";
			ref2.m_required = true;
			inputRefs.push_back(ref2);
		} break;

		default : ;
	}
}


void TNHeatPumpReal::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	// now store the pointer returned for our input ref request and advance the iterator by one
	switch (m_flowElement->m_component->m_modelType) {
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpRealSourceSide:{
			m_condenserOutletSetpointRef = *(resultValueRefs++);
			m_onOffSignalRef = *(resultValueRefs++);
		} break;
		default : ;
	}
}

void TNHeatPumpReal::internalDerivatives(double *ydot)
{
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}


} // namespace NANDRAD_MODEL
