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

#include <numeric>
#include <fstream>

#include <IBK_messages.h>
#include <IBK_FluidPhysics.h>


namespace NANDRAD_MODEL {

// *** TNSimplePipeElement ***

TNSimplePipeElement::TNSimplePipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & /*comp*/,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_flowElementId = elem.m_id;
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


void TNSimplePipeElement::modelQuantities(std::vector<QuantityDescription> & quantities) const{
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantities(quantities);
	quantities.push_back(QuantityDescription("FluidVelocity","m/s","Fluid velocity", false));
	quantities.push_back(QuantityDescription("FluidVolumeFlow","m3/h","Fluid volume flow", false));
	quantities.push_back(QuantityDescription("FluidViscosity","m2/s","Fluid dynamic viscosity", false));
	quantities.push_back(QuantityDescription("Reynolds","---","Reynolds number", false));
	quantities.push_back(QuantityDescription("Prandtl","---","Prandtl number", false));
	quantities.push_back(QuantityDescription("Nusselt","---","Nusselt number", false));
	quantities.push_back(QuantityDescription("ThermalTransmittance","W/K","Total thermal transmittance of fluid and pipe wall", false));
}


void TNSimplePipeElement::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantityValueRefs(valRefs);
	valRefs.push_back(&m_velocity);
	valRefs.push_back(&m_volumeFlow);
	valRefs.push_back(&m_viscosity);
	valRefs.push_back(&m_reynolds);
	valRefs.push_back(&m_prandtl);
	valRefs.push_back(&m_nusselt);
	valRefs.push_back(&m_UAValue);
}


void TNSimplePipeElement::inputReferences(std::vector<InputReference> & inputRefs) const {
	InputReference ref;
	ref.m_id = m_flowElementId;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// TNSimplePipeElement is instantiated for heat exchange types TemperatureConstant, TemperatureSpline, TemperatureZone,
	// TemperatureConstructionLayer. We do not know here, where the individual memory slots are. So we generically request
	// a "HeatExchangeTemperature" and let the ThermalNetworkBalanceModel decide and adjust the request as needed.
	// For example, for TemperatureZone the input reference request will be modified to ask for an AirTemperature provided
	// by a zone model.
	ref.m_name.m_name = "HeatExchangeTemperature";
	ref.m_required = true;
	inputRefs.push_back(ref);
}


void TNSimplePipeElement::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	m_heatExchangeTemperatureRef =  *(resultValueRefs++);
}


void TNSimplePipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// check heat transfer type
	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs

	// note: velocity is calculated for a single pipe (but mass flux interpreted as flux through all parallel pipes)
	m_velocity = m_volumeFlow/m_fluidCrossSection;

	m_viscosity = m_fluidViscosity.value(m_meanTemperature);
	m_reynolds = IBK::ReynoldsNumber(m_velocity, m_viscosity, m_innerDiameter);
	m_prandtl = IBK::PrandtlNumber(m_viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	m_nusselt = IBK::NusseltNumber(m_reynolds, m_prandtl, m_length, m_innerDiameter);
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

	IBK_ASSERT(m_heatExchangeTemperatureRef != nullptr);
	const double externalTemperature = *m_heatExchangeTemperatureRef;
	// calculate heat loss with given parameters
	// Q in [W] = DeltaT * UAValueTotal
	m_heatLoss = m_UAValue * (m_meanTemperature - externalTemperature) * m_nParallelPipes;
}

void TNSimplePipeElement::dependencies(const double * ydot, const double * y, const double * mdot,
									   const double * TInflowLeft, const double * TInflowRight,
									   std::vector<std::pair<const double *, const double *> > &resultInputDependencies) const
{
	ThermalNetworkAbstractFlowElementWithHeatLoss::dependencies(ydot, y , mdot, TInflowLeft, TInflowRight, resultInputDependencies);

	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeTemperatureRef) );
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
	const double velocity = std::fabs(m_massFlux)/(m_fluidCrossSection * m_fluidDensity);
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

#ifdef DETAILLED_WALL_CAPACITY

TNDynamicPipeElement::TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_flowElementId = elem.m_id;
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// copy number of pipes
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;

	// equivalent fluid volume of pipe wall
	m_densityWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_DensityPipeWall].value;
	m_heatCapacityWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_HeatCapacityPipeWall].value;

	// calculate fluid volume inside the pipe
	// Note: This is the volume of all parallel pipes. It also includes the equivalent volume of the pipe wall.
	m_fluidCrossSection = PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length;

	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	// in case given discretization length is larger than pipe length:
	// set discretization length to pipe length, so we have just one volume
	if (minDiscLength > m_length)
		minDiscLength = m_length;

	// calculate number of discretization elements
	m_nVolumes = (unsigned int) (m_length/minDiscLength);
	// resize all vectors
	m_temperaturesFluid.resize(m_nVolumes, 273.15);
	m_heatLossesFluid.resize(m_nVolumes, 0.0);
	m_temperaturesWall.resize(m_nVolumes, 273.15);
	m_heatLossesWall.resize(m_nVolumes, 0.0);

	double volumeWall = PI/4. * m_length * (m_outerDiameter*m_outerDiameter - m_innerDiameter*m_innerDiameter);

	// calculate segment specific quantities
	m_discLength = m_length/(double) m_nVolumes;
	m_discVolumeFluid = m_fluidVolume/(double) m_nVolumes;
	m_discVolumeWall = volumeWall/(double) m_nVolumes;

	// create output file for vector-value temperatures
	m_ofstream = new std::ofstream;
}


void TNDynamicPipeElement::modelQuantities(std::vector<QuantityDescription> & quantities) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantities(quantities);
	quantities.push_back(QuantityDescription("FluidVolumeFlow","m3/h","Fluid volume flow", false));
	quantities.push_back(QuantityDescription("FluidVelocity","m/s","Fluid velocity", false));
	quantities.push_back(QuantityDescription("FluidViscosity","m2/s","Fluid dynamic viscosity", false));
	quantities.push_back(QuantityDescription("Reynolds","---","Reynolds number", false));
	quantities.push_back(QuantityDescription("Prandtl","---","Prandtl number", false));
	quantities.push_back(QuantityDescription("Nusselt","---","Nusselt number", false));
	quantities.push_back(QuantityDescription("ThermalTransmittance","W/K","Total thermal transmittance of fluid and pipe wall", false));
}


void TNDynamicPipeElement::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantityValueRefs(valRefs);
	valRefs.push_back(&m_volumeFlow);
	valRefs.push_back(&m_velocity);
	valRefs.push_back(&m_viscosity);
	valRefs.push_back(&m_reynolds);
	valRefs.push_back(&m_prandtl);
	valRefs.push_back(&m_nusselt);
	valRefs.push_back(&m_UAValueFluidPipe);
}


void TNDynamicPipeElement::inputReferences(std::vector<InputReference> & inputRefs) const {
	InputReference ref;
	ref.m_id = m_flowElementId;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// see comment in TNSimplePipeElement::inputReferences()
	ref.m_name.m_name = "HeatExchangeTemperature";
	ref.m_required = true;
	inputRefs.push_back(ref);
}


void TNDynamicPipeElement::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	m_heatExchangeTemperatureRef =  *(resultValueRefs++);
}

unsigned int TNDynamicPipeElement::nInternalStates() const { return 2*m_nVolumes;}


void TNDynamicPipeElement::setInitialTemperature(double T0) {
	// use standard implementation
	ThermalNetworkAbstractFlowElementWithHeatLoss::setInitialTemperature(T0);
	// fill vector valued quantities
	std::fill(m_temperaturesFluid.begin(), m_temperaturesFluid.end(), T0);
	std::fill(m_temperaturesWall.begin(), m_temperaturesWall.end(), T0);
}


void TNDynamicPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs
	// note: velocity is caluclated for a single pipe (but mass flux interpreted as flux through all parallel pipes
	m_velocity = m_volumeFlow/m_fluidCrossSection;

	m_heatLoss = 0.0;

	// assume constant heat transfer coefficient along pipe, using average temperature
	m_viscosity = m_fluidViscosity.value(m_meanTemperature);
	m_reynolds = IBK::ReynoldsNumber(m_velocity, m_viscosity, m_innerDiameter);
	m_prandtl = IBK::PrandtlNumber(m_viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	m_nusselt = IBK::NusseltNumber(m_reynolds, m_prandtl, m_length, m_innerDiameter);
	double innerHeatTransferCoefficient = m_nusselt * m_fluidConductivity / m_innerDiameter;

	if(m_outerHeatTransferCoefficient == 0.) {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_UAValueFluidPipe = m_discLength / (1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI));
		m_UAValuePipeAmbient = m_discLength * m_UValuePipeWall;
	}
	else {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_UAValueFluidPipe = m_discLength / (1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI));

		m_UAValuePipeAmbient = m_discLength /
				(
					1.0 / (m_outerHeatTransferCoefficient * m_outerDiameter * PI)
					+ 1.0 / m_UValuePipeWall
				);
	}


	IBK_ASSERT(m_heatExchangeTemperatureRef != nullptr);
	const double externalTemperature = *m_heatExchangeTemperatureRef;
	for (unsigned int i = 0; i < m_nVolumes; ++i) {
		// calculate heat loss with given parameters
		m_heatLossesFluid[i] = m_UAValueFluidPipe * (m_temperaturesFluid[i] - m_temperaturesWall[i]) * m_nParallelPipes;
		m_heatLossesWall[i] = m_UAValuePipeAmbient * (m_temperaturesWall[i] - externalTemperature) * m_nParallelPipes;
		// sum up heat losses
		m_heatLoss += m_heatLossesWall[i];
	}
}


void TNDynamicPipeElement::initialInternalStates(double * y0) {
	// copy internal states
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i] = m_temperaturesFluid[i] * m_fluidHeatCapacity * m_fluidDensity * m_discVolumeFluid ;
	// pipe wall
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i+m_nVolumes] = m_temperaturesWall[i] * m_heatCapacityWall * m_densityWall * m_discVolumeWall ;
}


void TNDynamicPipeElement::setInternalStates(const double * y) {
	double temp = 0.0;
	// calculate specific enthalpy
	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		m_temperaturesFluid[i] = y[i] / ( m_discVolumeFluid * m_fluidDensity * m_fluidHeatCapacity);
		temp += m_temperaturesFluid[i];
	}
	m_meanTemperature = temp/m_nVolumes;
	// pipe wall
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		m_temperaturesWall[i] = y[i+m_nVolumes] / ( m_discVolumeWall * m_densityWall * m_heatCapacityWall);
}


void TNDynamicPipeElement::internalDerivatives(double * ydot) {

	// heat fluxes into the fluid and enthalpy change are heat sources
	if (m_massFlux >= 0.0) {
		// first element copies boundary conditions
		ydot[0] = -m_heatLossesFluid[0] + m_massFlux * m_fluidHeatCapacity * (m_inflowTemperature  - m_temperaturesFluid[0]);
		for (unsigned int i = 1; i < m_nVolumes; ++i) {
			ydot[i] = -m_heatLossesFluid[i] + m_massFlux * m_fluidHeatCapacity * (m_temperaturesFluid[i - 1] - m_temperaturesFluid[i]);
		}
	}
	else { // m_massFlux < 0
		// last element copies boundary conditions
		ydot[m_nVolumes - 1] = -m_heatLossesFluid[m_nVolumes - 1] + m_massFlux * m_fluidHeatCapacity * (m_temperaturesFluid[m_nVolumes - 1] - m_inflowTemperature);
		for (unsigned int i = 0; i < m_nVolumes - 1; ++i) {
			ydot[i] = -m_heatLossesFluid[i] + m_massFlux * m_fluidHeatCapacity * (m_temperaturesFluid[i] - m_temperaturesFluid[i + 1]);
		}
	}

	// pipe wall
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		ydot[i+m_nVolumes] = -m_heatLossesWall[i] + m_heatLossesFluid[i];

}


double TNDynamicPipeElement::outflowTemperature() const {
	if (m_massFlux >= 0)
		return m_temperaturesFluid[m_nVolumes-1];
	else
		return m_temperaturesFluid[0];
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
		resultInputDependencies.push_back(std::make_pair(ydot + n, m_heatExchangeTemperatureRef));

		// set dependency to Qdot
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, y + n) );
	}

	// set dependency to Qdot
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, mdot) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeTemperatureRef) );
}


void TNDynamicPipeElement::stepCompleted(double t) {
#ifdef WRITE_TEMP_PROFILE
	if (t - m_lastTimePoint < 60)
		return;
	m_lastTimePoint = t;

	m_ofstream->open("FluidTemperatures.txt", std::ofstream::out | std::ofstream::app);
	*m_ofstream << t ;
	for (const double &temp: m_temperaturesFluid)
		*m_ofstream << "\t" << temp;
	*m_ofstream << std::endl;
	m_ofstream->flush();
	m_ofstream->close();

	m_ofstream->open("WallTemperatures.txt", std::ofstream::out | std::ofstream::app);
	*m_ofstream << t ;
	for (const double &temp: m_temperaturesWall)
		*m_ofstream << "\t" << temp;
	*m_ofstream << std::endl;
	m_ofstream->flush();
	m_ofstream->close();
#endif
}



#else


TNDynamicPipeElement::TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_flowElementId = elem.m_id;
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// copy number of pipes
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;

	// equivalent fluid volume of pipe wall
	double densityWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_DensityPipeWall].value;
	double heatCapacityWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_HeatCapacityPipeWall].value;
	double volumeWall = PI/4. * m_length * (m_outerDiameter*m_outerDiameter - m_innerDiameter*m_innerDiameter);
	double equivalentVolumeWall = densityWall * heatCapacityWall * volumeWall / (m_fluidDensity * m_fluidHeatCapacity) * m_nParallelPipes;
	// calculate fluid volume inside the pipe
	// Note: This is the volume of all parallel pipes. It also includes the equivalent volume of the pipe wall.
	m_fluidCrossSection = PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length + equivalentVolumeWall;

	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	// in case given discretization length is larger than pipe length:
	// set discretization length to pipe length, so we have just one volume
	if (minDiscLength > m_length)
		minDiscLength = m_length;

	// calculate number of discretization elements
	m_nVolumes = (unsigned int) (m_length/minDiscLength);
	// resize all vectors
	m_temperatures.resize(m_nVolumes, 273.15);
	m_heatLosses.resize(m_nVolumes, 0.0);

	// calculate segment specific quantities
	m_discLength = m_length/(double) m_nVolumes;
	m_discVolume = m_fluidVolume/(double) m_nVolumes;
}


void TNDynamicPipeElement::modelQuantities(std::vector<QuantityDescription> & quantities) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantities(quantities);
	quantities.push_back(QuantityDescription("FluidVolumeFlow","m3/h","Fluid volume flow", false));
	quantities.push_back(QuantityDescription("FluidVelocity","m/s","Fluid velocity", false));
	quantities.push_back(QuantityDescription("FluidViscosity","m2/s","Fluid dynamic viscosity", false));
	quantities.push_back(QuantityDescription("Reynolds","---","Reynolds number", false));
	quantities.push_back(QuantityDescription("Prandtl","---","Prandtl number", false));
	quantities.push_back(QuantityDescription("Nusselt","---","Nusselt number", false));
	quantities.push_back(QuantityDescription("ThermalTransmittance","W/K","Total thermal transmittance of fluid and pipe wall", false));
}


void TNDynamicPipeElement::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantityValueRefs(valRefs);
	valRefs.push_back(&m_volumeFlow);
	valRefs.push_back(&m_velocity);
	valRefs.push_back(&m_viscosity);
	valRefs.push_back(&m_reynolds);
	valRefs.push_back(&m_prandtl);
	valRefs.push_back(&m_nusselt);
	valRefs.push_back(&m_UAValue);
}


void TNDynamicPipeElement::inputReferences(std::vector<InputReference> & inputRefs) const {
	InputReference ref;
	ref.m_id = m_flowElementId;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// see comment in TNSimplePipeElement::inputReferences()
	ref.m_name.m_name = "HeatExchangeTemperature";
	ref.m_required = true;
	inputRefs.push_back(ref);
}


void TNDynamicPipeElement::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	m_heatExchangeTemperatureRef =  *(resultValueRefs++);
}


unsigned int TNDynamicPipeElement::nInternalStates() const { return m_nVolumes;}


void TNDynamicPipeElement::setInitialTemperature(double T0) {
	// use standard implementation
	ThermalNetworkAbstractFlowElementWithHeatLoss::setInitialTemperature(T0);
	// fill vector valued quantities
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
}


void TNDynamicPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs
	// note: velocity is caluclated for a single pipe (but mass flux interpreted as flux through all parallel pipes
	m_velocity = m_volumeFlow/m_fluidCrossSection;

	m_heatLoss = 0.0;

	// assume constant heat transfer coefficient along pipe, using average temperature
	m_viscosity = m_fluidViscosity.value(m_meanTemperature);
	m_reynolds = IBK::ReynoldsNumber(m_velocity, m_viscosity, m_innerDiameter);
	m_prandtl = IBK::PrandtlNumber(m_viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	m_nusselt = IBK::NusseltNumber(m_reynolds, m_prandtl, m_length, m_innerDiameter);
	double innerHeatTransferCoefficient = m_nusselt * m_fluidConductivity / m_innerDiameter;

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


	IBK_ASSERT(m_heatExchangeTemperatureRef != nullptr);
	const double externalTemperature = *m_heatExchangeTemperatureRef;
	for (unsigned int i = 0; i < m_nVolumes; ++i) {
		// calculate heat loss with given parameters
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
		resultInputDependencies.push_back(std::make_pair(ydot + n, m_heatExchangeTemperatureRef));

		// set dependency to Qdot
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, y + n) );
	}

	// set dependency to Qdot
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, mdot) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeTemperatureRef) );
}

#endif



// *** DynamicAdiabaticPipeElement ***

TNDynamicAdiabaticPipeElement::TNDynamicAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	double length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	double innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	double outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;

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

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;

	// equivalent fluid volume of pipe wall
	double densityWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_DensityPipeWall].value;
	double heatCapacityWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_HeatCapacityPipeWall].value;
	double volumeWall = PI/4. * length * (outerDiameter*outerDiameter - innerDiameter*innerDiameter);
	double equivalentVolumeWall = densityWall * heatCapacityWall * volumeWall / (m_fluidDensity * m_fluidHeatCapacity);

	// calculate fluid volume inside the pipe
	m_fluidVolume = PI/4. * innerDiameter * innerDiameter * length + equivalentVolumeWall;

	// calculate segment specific quantities
	m_discVolume = m_fluidVolume/(double) m_nVolumes;
}


void TNDynamicAdiabaticPipeElement::modelQuantities(std::vector<QuantityDescription> & quantities) const{
	quantities.push_back(QuantityDescription("FluidVolumeFlow","m3/h","Fluid volume flow", false));
	quantities.push_back(QuantityDescription("FluidVelocity","m/s","Fluid velocity", false));
}


void TNDynamicAdiabaticPipeElement::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	valRefs.push_back(&m_volumeFlow);
	valRefs.push_back(&m_velocity);
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


void TNPumpWithPerformanceLoss::modelQuantities(std::vector<QuantityDescription> & quantities) const{
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantities(quantities);
	quantities.push_back(QuantityDescription("ElectricalPower","W","Requested electrical power for current working point", false));
	quantities.push_back(QuantityDescription("MechanicalPower","W","Mechanical power for current working point", false));
}


void TNPumpWithPerformanceLoss::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantityValueRefs(valRefs);
	valRefs.push_back(&m_electricalPower);
	valRefs.push_back(&m_mechanicalPower);
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

TNElementWithExternalHeatLoss::TNElementWithExternalHeatLoss(unsigned int flowElementId, const NANDRAD::HydraulicFluid & fluid, double fluidVolume)
{
	m_flowElementId = flowElementId;
	m_fluidVolume = fluidVolume;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}


void TNElementWithExternalHeatLoss::modelQuantities(std::vector<QuantityDescription> &quantities) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantities(quantities);
	quantities.push_back(QuantityDescription("TemperatureDifference", "K", "Outlet temperature minus inlet temperature", false));
}


void TNElementWithExternalHeatLoss::modelQuantityValueRefs(std::vector<const double *> &valRefs) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantityValueRefs(valRefs);
	valRefs.push_back(&m_temperatureDifference);
}


void TNElementWithExternalHeatLoss::internalDerivatives(double * ydot) {
	// set heat loss
	IBK_ASSERT(m_heatExchangeHeatLossRef != nullptr);
	m_heatLoss = *m_heatExchangeHeatLossRef;
	m_temperatureDifference = m_heatLoss / (m_massFlux * m_fluidHeatCapacity);
	// use basic routine
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}


void TNElementWithExternalHeatLoss::inputReferences(std::vector<InputReference> & inputRefs) const {
	InputReference ref;
	ref.m_id = m_flowElementId;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	ref.m_name.m_name = "HeatExchangeHeatLoss";
	ref.m_required = true;
	inputRefs.push_back(ref);
}


void TNElementWithExternalHeatLoss::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	m_heatExchangeHeatLossRef =  *(resultValueRefs++);
}


void TNElementWithExternalHeatLoss::dependencies(const double * ydot, const double * y, const double * mdot,
												 const double * TInflowLeft, const double * TInflowRight,
												 std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	ThermalNetworkAbstractFlowElementWithHeatLoss::dependencies(ydot, y , mdot, TInflowLeft, TInflowRight, resultInputDependencies);

	if (m_heatExchangeHeatLossRef != nullptr)
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeHeatLossRef) );
}





// *** TNHeatPumpVariable ***

TNHeatPumpVariable::TNHeatPumpVariable(const NANDRAD::HydraulicFluid & fluid,
									   const NANDRAD::HydraulicNetworkElement & e) :
	m_flowElement(&e)
{
	m_fluidVolume = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_condenserMaximumHeatFlux = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_MaximumHeatingPower].value;
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	switch (e.m_component->m_modelType) {
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide:
			m_coeffsCOP = e.m_component->m_polynomCoefficients.m_values.at("COP");
			break;
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide:
			m_carnotEfficiency = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_CarnotEfficiency].value;
			break;
		default:;
	}
}


void TNHeatPumpVariable::modelQuantities(std::vector<QuantityDescription> & quantities) const{
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantities(quantities);
	quantities.push_back(QuantityDescription("COP","---", "Coefficient of performance for heat pump", false));
	quantities.push_back(QuantityDescription("ElectricalPower", "W", "Electrical power for heat pump", false));
	quantities.push_back(QuantityDescription("CondenserHeatFlux", "W", "Heat Flux at condenser side of heat pump", false));
	quantities.push_back(QuantityDescription("EvaporatorHeatFlux", "W", "Heat Flux at evaporator side of heat pump", false));
	quantities.push_back(QuantityDescription("EvaporatorMeanTemperature", "C", "Mean temperature at evaporator side of heat pump", false));
	quantities.push_back(QuantityDescription("CondenserMeanTemperature", "C", "Mean temperature at condenser side of heat pump", false));
	quantities.push_back(QuantityDescription("TemperatureDifference", "K", "Outlet temperature minus inlet temperature", false));
}


void TNHeatPumpVariable::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantityValueRefs(valRefs);
	valRefs.push_back(&m_COP);
	valRefs.push_back(&m_electricalPower);
	valRefs.push_back(&m_condenserHeatFlux);
	valRefs.push_back(&m_evaporatorHeatFlux);
	valRefs.push_back(&m_evaporatorMeanTemperature);
	valRefs.push_back(&m_condenserMeanTemperature);
	valRefs.push_back(&m_temperatureDifference);
}


void TNHeatPumpVariable::setInflowTemperature(double Tinflow) {

	ThermalNetworkAbstractFlowElementWithHeatLoss::setInflowTemperature(Tinflow);

	switch (m_flowElement->m_component->m_modelType) {

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide: {
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

			// check for valid pointer
			IBK_ASSERT(m_heatExchangeCondensorHeatLossRef != nullptr);
			m_condenserHeatFlux = *m_heatExchangeCondensorHeatLossRef;

			// Passive cooling mode:
			// in case of a negative condenser heat flux, we interpet this as passive cooling, so the heat pump is off
			// and just acts as a usual heat exchanger by adding the condenser heat flux directly to the fluid
			if (m_condenserHeatFlux <= 0) {
				m_heatLoss = m_condenserHeatFlux;
			}

			// Normal heat pump mode:  we expect positive mass flux
			else if (m_massFlux > 0){
				// cut condenser heat flux
				if (m_condenserHeatFlux > m_condenserMaximumHeatFlux)
					m_condenserHeatFlux = m_condenserMaximumHeatFlux;
				// compute temperature level of evaporator side -> m_inflowTemperature and
				// m_meanTemperature (fluid outlet temperature) are both known and given
				m_evaporatorMeanTemperature = (m_inflowTemperature + m_meanTemperature) / 2;

				// Calculate COP depending on model and check if temperatures and COP are in a valid range
				// if temperatures / COP is invalid m_COP will be =0
				calculateCOP();

				// only continue for valid COP's
				if (m_COP > 1) {
					m_evaporatorHeatFlux = m_condenserHeatFlux * (m_COP - 1) / m_COP;
					m_heatLoss = m_evaporatorHeatFlux; // energy taken out of fluid medium
					m_electricalPower  = m_condenserHeatFlux - m_evaporatorHeatFlux; // same as "m_condenserHeatFlux/m_COP", electrical power of heat pump
					m_temperatureDifference = m_inflowTemperature - m_meanTemperature;
				}
			}
		} break; // HP_SourceSide


		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide:{
			// initialize all results with 0
			m_evaporatorHeatFlux = 0;
			m_COP = 0.0;
			m_heatLoss = 0.0;
			m_electricalPower  = 0.0;
			m_temperatureDifference = 0;

			// get scheduled temperatures
			IBK_ASSERT(m_heatExchangeEvaporatorTemperatureRef != nullptr);
			IBK_ASSERT(m_condenserOutletSetpointRef != nullptr);
			m_evaporatorMeanTemperature = *m_heatExchangeEvaporatorTemperatureRef;
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

				// Calculate COP, depending on model and check if temperatures and COP are in a valid range
				// if temperatures / COP is invalid m_COP will be =0
				calculateCOP();

				// only continue for valid COP's
				if (m_COP > 1) {
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


void TNHeatPumpVariable::inputReferences(std::vector<InputReference> & inputRefs) const {

	switch (m_flowElement->m_component->m_modelType) {

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide: {
			InputReference ref;
			ref.m_id = m_flowElement->m_id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_required = true;
			ref.m_name.m_name = "HeatExchangeHeatLossCondenser";
			inputRefs.push_back(ref);
			ref.m_name.m_name = "CondenserMeanTemperatureSchedule";
			inputRefs.push_back(ref);
		} break;

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide: {
			InputReference ref;
			ref.m_id = m_flowElement->m_id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_required = true;
			ref.m_name.m_name = "HeatExchangeTemperatureEvaporator";
			inputRefs.push_back(ref);
			ref.m_name.m_name = "CondenserOutletSetpointSchedule";
			inputRefs.push_back(ref);
		} break;

		default : ;
	}
}


void TNHeatPumpVariable::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {

	// now store the pointer returned for our input ref request and advance the iterator by one
	switch (m_flowElement->m_component->m_modelType) {
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide: {
			m_heatExchangeCondensorHeatLossRef = *(resultValueRefs++); // HeatLossCondenser
			m_condenserMeanTemperatureRef = *(resultValueRefs++); // CondenserMeanTemperatureSchedule
		} break;
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide: {
			m_heatExchangeEvaporatorTemperatureRef = *(resultValueRefs++); // TemperatureEvaporator
			m_condenserOutletSetpointRef = *(resultValueRefs++); // CondenserOutletSetpointSchedule
		} break;
		default : ;
	}
}


void TNHeatPumpVariable::internalDerivatives(double *ydot) {
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}


void TNHeatPumpVariable::dependencies(const double * ydot, const double * y, const double * mdot,
										 const double * TInflowLeft, const double * TInflowRight,
										 std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	ThermalNetworkAbstractFlowElementWithHeatLoss::dependencies(ydot, y, mdot, TInflowLeft, TInflowRight, resultInputDependencies);

	// add consdenser heat flux
	if (m_heatExchangeCondensorHeatLossRef != nullptr)
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeCondensorHeatLossRef));
	// add evaporator temperature
	if (m_heatExchangeEvaporatorTemperatureRef != nullptr)
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeEvaporatorTemperatureRef));
}


void TNHeatPumpVariable::calculateCOP() {
	FUNCID(TNHeatPumpVariable::calculateCOP);

	const double MIN_TEMPERATURE_DIFFERENCE_CONDENSER = 4; // K
	m_COP = 0.;

	// generally, we do not want to go below -30 C.
	if (m_evaporatorMeanTemperature < 273.15 - 30) {
		IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("Evaporator temperature < -30 C, turning of "
															"HeatPumpIdealCarnot, flow element with id '%1'\n").arg(m_flowElement->m_id),
											IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
	}
	// there must also be a minimum temperature difference
	else if (m_condenserMeanTemperature - m_evaporatorMeanTemperature < MIN_TEMPERATURE_DIFFERENCE_CONDENSER) {
		IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("Evaporator temperature must be at least %1 K lower than condenser temperature (%2 C)\n")
										  .arg(MIN_TEMPERATURE_DIFFERENCE_CONDENSER).arg(m_condenserMeanTemperature - 273.15),
											IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
	}
	// now calculate COP depending on model and check if it is valid
	else {
		switch (m_flowElement->m_component->m_modelType) {
			case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
			case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide: {
				// constant carnot efficiency
				m_COP = m_carnotEfficiency *  m_condenserMeanTemperature / (m_condenserMeanTemperature - m_evaporatorMeanTemperature);
			} break;
			case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide: {
				// bi-quadratic polynom, expects temperatures in K
				const double &Tc = m_condenserMeanTemperature;
				const double &Te = m_evaporatorMeanTemperature;
				m_COP = m_coeffsCOP[0] + m_coeffsCOP[1] * Te + m_coeffsCOP[2] * Tc + m_coeffsCOP[3] * Te * Tc +
						m_coeffsCOP[4] * Te * Te + m_coeffsCOP[5] * Tc * Tc;
			} break;
			default: ; // nothing else possible
		}

		// clipping in case of invalid COP
		if (m_COP <= 1) {
			m_COP = 0;
			IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("COP is <= 1 in HeatPumpVariable, "
																"flow element with id '%1'\n").arg(m_flowElement->m_id),
																IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
		}
	}
}






// *** TNIdealHeaterCooler ***

TNIdealHeaterCooler::TNIdealHeaterCooler(unsigned int flowElementId, const NANDRAD::HydraulicFluid & fluid) :
	m_id(flowElementId)
{
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}


void TNIdealHeaterCooler::setInflowTemperature(double Tinflow) {

	IBK_ASSERT(m_supplyTemperatureScheduleRef != nullptr);
	m_meanTemperature = *m_supplyTemperatureScheduleRef;
	double absMassFlux = std::fabs(m_massFlux);

	// heat needed to provide the given temperature (If we are heating up the fluid, this is positive)
	m_heatLoss = absMassFlux * m_fluidHeatCapacity * (Tinflow - m_meanTemperature);
}


void TNIdealHeaterCooler::inputReferences(std::vector<InputReference> & inputRefs) const {
	InputReference ref;
	ref.m_id = m_id;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	ref.m_name.m_name = "SupplyTemperatureSchedule";
	ref.m_required = true;
	inputRefs.push_back(ref);
}


void TNIdealHeaterCooler::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {
	m_supplyTemperatureScheduleRef = *(resultValueRefs++);
}


void TNIdealHeaterCooler::modelQuantities(std::vector<QuantityDescription> & quantities) const{
	quantities.push_back(QuantityDescription("FlowElementHeatLoss", "W", "Heat flux from flow element into environment", false));
}


void TNIdealHeaterCooler::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	valRefs.push_back(&m_heatLoss);
}





// *** TNHeatPumpReal ***

TNHeatPumpOnOff::TNHeatPumpOnOff(const NANDRAD::HydraulicFluid &fluid, const NANDRAD::HydraulicNetworkElement &e):
	m_flowElement(&e)
{
	m_fluidVolume = e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_coeffsQcond = e.m_component->m_polynomCoefficients.m_values.at("QdotCondensator");
	m_coeffsPel = e.m_component->m_polynomCoefficients.m_values.at("Pel");
}


void TNHeatPumpOnOff::modelQuantities(std::vector<QuantityDescription> & quantities) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantities(quantities);
	quantities.push_back(QuantityDescription("COP","---", "Coefficient of performance for heat pump", false));
	quantities.push_back(QuantityDescription("ElectricalPower", "W", "Electrical power for heat pump", false));
	quantities.push_back(QuantityDescription("CondenserHeatFlux", "W", "Heat Flux at condenser side of heat pump", false));
	quantities.push_back(QuantityDescription("EvaporatorHeatFlux", "W", "Heat Flux at evaporator side of heat pump", false));
	quantities.push_back(QuantityDescription("TemperatureDifference", "K", "Outlet temperature minus inlet temperature", false));
	quantities.push_back(QuantityDescription("CondenserOutletTemperature", "C", "Outlet temperature of condenser", false));
	quantities.push_back(QuantityDescription("EvaporatorInletTemperature", "C", "Inlet temperature of Evaporator", false));
}


void TNHeatPumpOnOff::modelQuantityValueRefs(std::vector<const double *> & valRefs) const {
	ThermalNetworkAbstractFlowElementWithHeatLoss::modelQuantityValueRefs(valRefs);
	valRefs.push_back(&m_COP);
	valRefs.push_back(&m_electricalPower);
	valRefs.push_back(&m_condenserHeatFlux);
	valRefs.push_back(&m_evaporatorHeatFlux);
	valRefs.push_back(&m_temperatureDifference);
	valRefs.push_back(&m_condenserOutletTemperature);
	valRefs.push_back(&m_evaporatorInletTemperature);
}


void TNHeatPumpOnOff::setInflowTemperature(double Tinflow) {
	ThermalNetworkAbstractFlowElementWithHeatLoss::setInflowTemperature(Tinflow);

	IBK_ASSERT(m_condenserOutletSetpointRef != nullptr);
	IBK_ASSERT(m_onOffSignalRef != nullptr);

	// convenience
	const double &Tc = *m_condenserOutletSetpointRef;
	const double &Te = Tinflow;

	// heat pump is ON
	if (*m_onOffSignalRef > 0.5){

		// The polynom result is in W and expects temperatures in K
		// condensator heat flux polynom
		m_condenserHeatFlux = m_coeffsQcond[0] + m_coeffsQcond[1] * Te + m_coeffsQcond[2] * Tc + m_coeffsQcond[3] * Te * Tc +
				m_coeffsQcond[4] * Te * Te + m_coeffsQcond[5] * Tc * Tc;
		// electrical power polynom
		m_electricalPower = m_coeffsPel[0] + m_coeffsPel[1] * Te + m_coeffsPel[2] * Tc + m_coeffsPel[3] * Te * Tc +
				m_coeffsPel[4] * Te * Te + m_coeffsPel[5] * Tc * Tc;

		m_evaporatorHeatFlux = m_condenserHeatFlux - m_electricalPower;
		m_COP = m_condenserHeatFlux / m_electricalPower;
		m_temperatureDifference = m_meanTemperature - m_inflowTemperature;
		m_condenserOutletTemperature = *m_condenserOutletSetpointRef;
		m_evaporatorInletTemperature = m_inflowTemperature;

		// heat loss depends on model type
		if (m_flowElement->m_component->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide){
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


void TNHeatPumpOnOff::inputReferences(std::vector<InputReference> &inputRefs) const {

	switch (m_flowElement->m_component->m_modelType) {

		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide: {
			InputReference ref;
			ref.m_id = m_flowElement->m_id;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_name.m_name = "CondenserOutletSetpointSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);

			InputReference ref2;
			ref2.m_id = m_flowElement->m_id;
			ref2.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref2.m_name.m_name = "HeatPumpOnOffSignalSchedule";
			ref2.m_required = true;
			inputRefs.push_back(ref2);
		} break;

		default : ;
	}
}


void TNHeatPumpOnOff::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {

	// now store the pointer returned for our input ref request and advance the iterator by one
	switch (m_flowElement->m_component->m_modelType) {
		case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide:{
			m_condenserOutletSetpointRef = *(resultValueRefs++);
			m_onOffSignalRef = *(resultValueRefs++);
		} break;
		default : ;
	}
}


void TNHeatPumpOnOff::internalDerivatives(double *ydot) {
	// Bypass parent class's internalDerivatives() function
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}


} // namespace NANDRAD_MODEL
