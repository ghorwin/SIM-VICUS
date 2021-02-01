#include "NM_ThermalNetworkFlowElements.h"

#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#define PI				3.141592653589793238

namespace NANDRAD_MODEL {

// *** TNStaticPipeElement ***

TNStaticPipeElement::TNStaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			comp.m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;
	// set a small volume
	m_volume = 0.01;
}

TNStaticPipeElement::~TNStaticPipeElement()
{

}

unsigned int TNStaticPipeElement::nInternalStates() const
{
	return 1;
}

void TNStaticPipeElement::setInitialTemperature(double T0) {
	m_temperature = T0;
	m_specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// set inlet and outlet temperatures and specific enthalpies
	m_inletTemperature = T0;
	m_outletTemperature = T0;
	m_inletSpecificEnthalpy = m_specificEnthalpy;
	m_outletSpecificEnthalpy = m_specificEnthalpy;
}

void TNStaticPipeElement::initialInternalStates(double * y0) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	y0[0] = m_specificEnthalpy * m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNStaticPipeElement::setInternalStates(const double * y)
{
	// calculate specific enthalpy
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	m_temperature = m_specificEnthalpy / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNStaticPipeElement::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -m_heatLoss + m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}


void TNStaticPipeElement::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(m_massFlux >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_outletTemperature = m_temperature;
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_inletTemperature = m_temperature;
	}

	// calculate inner heat transfer and for simpicity use laminar flow equations
	// TODO Hauke: add turbulent flow equations
	// first calculate reynolds number
	const double velocity = std::fabs(m_massFlux)/(m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	// TODO Anne: we use the kinematic viscosity based on inlet temperature? Should be ok.
	const double reynolds = velocity * m_innerDiameter/m_fluid->m_kinematicViscosity.m_values.value(m_temperature);
	// calculate prandtl number
	const double prandtl = velocity * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value *
			m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value/
			m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	// calculate laminar nusselt number
	double val = 1.615 * (reynolds*prandtl * m_innerDiameter/m_length-0.7);
	const double nusselt = std::pow(49.37 + val*val*val,0.33333);
	// calculate convection coefficient for the inner side
	m_innerHeatTransferCoefficient = nusselt * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value/m_innerDiameter;

	// calculate heat transfer
	const double totalUValuePipe = (PI*m_length) / (1.0/(m_innerHeatTransferCoefficient * m_innerDiameter)
													+ 1.0/(m_outerHeatTransferCoefficient * m_outerDiameter)
													+ 1.0/(2.0*m_UValuePipeWall) );

	if(m_massFlux >= 0) {
		// calculate heat loss with given parameters
		m_heatLoss = m_massFlux * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value *
				(m_inletTemperature - m_ambientTemperature) *
				(1. - std::exp(-totalUValuePipe / (std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value )));
	}
	else {
		// calculate heat loss with given parameters
		m_heatLoss = std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value *
				(m_outletTemperature - m_ambientTemperature) *
				(1. - std::exp(-totalUValuePipe / (std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value )));
	}
}

void TNStaticPipeElement::setAmbientConditions(double Tamb, double /*alphaAmb*/)
{
	//copy ambient temperature
	m_ambientTemperature = Tamb;
}

double TNStaticPipeElement::meanTemperature() const {
	return m_temperature;
}

double TNStaticPipeElement::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNStaticPipeElement::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNStaticPipeElement::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNStaticPipeElement::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNStaticPipeElement::heatLoss() const  {
	return m_heatLoss;
}


// *** TNStaticAdiabaticPipeElement ***

TNStaticAdiabaticPipeElement::TNStaticAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & /*elem*/,
							 const NANDRAD::HydraulicNetworkComponent & /*comp*/,
							const NANDRAD::HydraulicNetworkPipeProperties & /*pipePara*/,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	// set a small volume
	m_volume = 0.01;
}

TNStaticAdiabaticPipeElement::~TNStaticAdiabaticPipeElement()
{

}

unsigned int TNStaticAdiabaticPipeElement::nInternalStates() const
{
	return 1;
}

void TNStaticAdiabaticPipeElement::setInitialTemperature(double T0) {
	m_temperature = T0;
	m_specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// set inlet and outölet tenoeratures and specific enthalpies
	m_inletTemperature = T0;
	m_outletTemperature = T0;
	m_inletSpecificEnthalpy = m_specificEnthalpy;
	m_outletSpecificEnthalpy = m_specificEnthalpy;
}

void TNStaticAdiabaticPipeElement::initialInternalStates(double * y0) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	y0[0] = m_specificEnthalpy * m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNStaticAdiabaticPipeElement::setInternalStates(const double * y)
{
	// calculate specific enthalpy
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	m_temperature = m_specificEnthalpy / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNStaticAdiabaticPipeElement::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}


void TNStaticAdiabaticPipeElement::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_outletTemperature = m_temperature;
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_inletTemperature = m_temperature;
	}
}

void TNStaticAdiabaticPipeElement::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNStaticAdiabaticPipeElement::meanTemperature() const {
	return m_temperature;
}

double TNStaticAdiabaticPipeElement::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNStaticAdiabaticPipeElement::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNStaticAdiabaticPipeElement::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNStaticAdiabaticPipeElement::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNStaticAdiabaticPipeElement::heatLoss() const  {
	return 0.0;
}



// *** DynamicPipeElement ***

TNDynamicPipeElement::TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			comp.m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;
	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	IBK_ASSERT(minDiscLength < m_length);

	// claculte number of discretization elements
	m_nVolumes = (unsigned int) (m_length/minDiscLength);
	// resize all vectors
	m_specificEnthalpies.resize(m_nVolumes, 0.0);
	m_temperatures.resize(m_nVolumes, 273.15);
	m_heatLosses.resize(m_nVolumes, 0.0);
	// calculate fluid volume inside the pipe
	m_volume = PI/4. * m_innerDiameter * m_innerDiameter * m_length;

	// calculate segment specific quantities
	m_discLength = m_length/(double) m_nVolumes;
	m_discVolume = m_volume/(double) m_nVolumes;
}

TNDynamicPipeElement::~TNDynamicPipeElement()
{

}

unsigned int TNDynamicPipeElement::nInternalStates() const
{
	return m_nVolumes;
}

void TNDynamicPipeElement::setInitialTemperature(double T0) {
	m_meanTemperature = T0;
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
	// calculte specific enthalpy
	double specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	std::fill(m_specificEnthalpies.begin(), m_specificEnthalpies.end(), specificEnthalpy);
	// set inlet and outölet tenoeratures and specific enthalpies
	m_inletTemperature = T0;
	m_outletTemperature = T0;
	m_inletSpecificEnthalpy = specificEnthalpy;
	m_outletSpecificEnthalpy = specificEnthalpy;
}

void TNDynamicPipeElement::initialInternalStates(double * y0) {
	// copy internal states
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i] = m_specificEnthalpies[i] * m_discVolume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNDynamicPipeElement::setInternalStates(const double * y)
{
	double temp = 0.0;
	// calculate specific enthalpy
	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		m_specificEnthalpies[i] = y[i] / ( m_discVolume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
		m_temperatures[i] = m_specificEnthalpies[i] / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		temp += m_temperatures[i];
	}
	m_meanTemperature = temp/(double) m_nVolumes;
}

void TNDynamicPipeElement::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	if(m_massFlux >= 0.0) {
		// first element copies boundary conditions
		ydot[0] = -m_heatLosses[0] + m_massFlux * (m_inletSpecificEnthalpy - m_specificEnthalpies[0]);
		for(unsigned int i = 1; i < m_nVolumes; ++i) {
			ydot[i] = -m_heatLosses[i] + m_massFlux * (m_specificEnthalpies[i - 1] - m_specificEnthalpies[i]);
		}
	}
	else { // m_massFlux < 0
		// last element copies boundary conditions
		ydot[m_nVolumes - 1] = -m_heatLosses[m_nVolumes - 1] + m_massFlux * (m_specificEnthalpies[m_nVolumes - 1]
				- m_outletSpecificEnthalpy);
		for(unsigned int i = 0; i < m_nVolumes - 1; ++i) {
			ydot[i] = -m_heatLosses[i] + m_massFlux * (m_specificEnthalpies[i] - m_specificEnthalpies[i + 1]);
		}
	}
}


void TNDynamicPipeElement::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpies[m_nVolumes - 1];
		// set outlet temperature
		m_outletTemperature = m_temperatures[m_nVolumes - 1];
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpies[0];
		// set outlet temperature
		m_inletTemperature = m_temperatures[0];
	}

	m_heatLoss = 0.0;
	// velocity is the same along each pipe segment
	const double velocity = std::fabs(mdot)/(m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);

	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		// calculate inner heat transfer and for simpicity use laminar flow equations
		// TODO Hauke: add turbulent flow equations
		// first calculate reynolds number
		const double reynolds = velocity * m_innerDiameter/m_fluid->m_kinematicViscosity.m_values.value(m_temperatures[i]);
		// calculate prandtl number
		const double prandtl = velocity * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value *
				m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value/
				m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
		// calculate laminar nusselt number
		double val = 1.615 * (reynolds*prandtl * m_innerDiameter/m_length-0.7);
		const double nusselt = std::pow(49.37 + val*val*val,0.33333);
		// calculate convection coefficient for the inner side
		double innerHeatTransferCoefficient = nusselt * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value/m_innerDiameter;

		// calculate heat transfer
		const double UValue = (PI*m_discLength) / (1.0/(innerHeatTransferCoefficient * m_innerDiameter)
														+ 1.0/(m_outerHeatTransferCoefficient * m_outerDiameter)
														+ 1.0/(2.0*m_UValuePipeWall) );

		// calculate heat loss with given parameters
		m_heatLosses[i] = UValue * (m_temperatures[i] - m_ambientTemperature);
		// sum up heat losses
		m_heatLoss += m_heatLosses[i];
	}
}

void TNDynamicPipeElement::setAmbientConditions(double Tamb, double /*alphaAmb*/)
{
	//copy ambient temperature
	m_ambientTemperature = Tamb;
}

double TNDynamicPipeElement::meanTemperature() const {
	return m_meanTemperature;
}

double TNDynamicPipeElement::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNDynamicPipeElement::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNDynamicPipeElement::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNDynamicPipeElement::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNDynamicPipeElement::heatLoss() const  {
	return m_heatLoss;
}


// *** DynamicAdiabaticPipeElement ***

TNDynamicAdiabaticPipeElement::TNDynamicAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	double length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	double innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;

	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	IBK_ASSERT(minDiscLength < length);

	// claculte number of discretization elements
	m_nVolumes = (unsigned int) (length/minDiscLength);
	// resize all vectors
	m_specificEnthalpies.resize(m_nVolumes, 0.0);
	m_temperatures.resize(m_nVolumes, 273.15);

	// calculate fluid volume inside the pipe
	m_volume = PI/4. * innerDiameter * innerDiameter * length;

	// calculate segment specific quantities
	m_discVolume = m_volume/(double) m_nVolumes;
}

TNDynamicAdiabaticPipeElement::~TNDynamicAdiabaticPipeElement()
{

}

unsigned int TNDynamicAdiabaticPipeElement::nInternalStates() const
{
	return m_nVolumes;
}

void TNDynamicAdiabaticPipeElement::setInitialTemperature(double T0) {
	m_meanTemperature = T0;
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
	// calculte specific enthalpy
	double specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	std::fill(m_specificEnthalpies.begin(), m_specificEnthalpies.end(), specificEnthalpy);
	// set inlet and outölet tenoeratures and specific enthalpies
	m_inletTemperature = T0;
	m_outletTemperature = T0;
	m_inletSpecificEnthalpy = specificEnthalpy;
	m_outletSpecificEnthalpy = specificEnthalpy;
}

void TNDynamicAdiabaticPipeElement::initialInternalStates(double * y0) {
	// copy internal states
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i] = m_specificEnthalpies[i] * m_discVolume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNDynamicAdiabaticPipeElement::setInternalStates(const double * y)
{
	double temp = 0.0;
	// calculate specific enthalpy
	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		m_specificEnthalpies[i] = y[i] / ( m_discVolume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
		m_temperatures[i] = m_specificEnthalpies[i] / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		temp += m_temperatures[i];
	}
	m_meanTemperature = temp/(double) m_nVolumes;
}

void TNDynamicAdiabaticPipeElement::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	if(m_massFlux >= 0.0) {
		// first element copies boundary conditions
		ydot[0] = m_massFlux * (m_inletSpecificEnthalpy - m_specificEnthalpies[0]);
		for(unsigned int i = 1; i < m_nVolumes; ++i) {
			ydot[i] = m_massFlux * (m_specificEnthalpies[i - 1] - m_specificEnthalpies[i]);
		}
	}
	else { // m_massFlux < 0
		// last element copies boundary conditions
		ydot[m_nVolumes - 1] = m_massFlux * (m_specificEnthalpies[m_nVolumes - 1]
				- m_outletSpecificEnthalpy);
		for(unsigned int i = 0; i < m_nVolumes - 1; ++i) {
			ydot[i] = m_massFlux * (m_specificEnthalpies[i] - m_specificEnthalpies[i + 1]);
		}
	}
}


void TNDynamicAdiabaticPipeElement::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpies[m_nVolumes - 1];
		// set outlet temperature
		m_outletTemperature = m_temperatures[m_nVolumes - 1];
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpies[0];
		// set outlet temperature
		m_inletTemperature = m_temperatures[0];
	}
}

void TNDynamicAdiabaticPipeElement::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNDynamicAdiabaticPipeElement::meanTemperature() const {
	return m_meanTemperature;
}

double TNDynamicAdiabaticPipeElement::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNDynamicAdiabaticPipeElement::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNDynamicAdiabaticPipeElement::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNDynamicAdiabaticPipeElement::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNDynamicAdiabaticPipeElement::heatLoss() const  {
	return 0.0;
}


// *** HeatExchanger ***

TNHeatExchanger::TNHeatExchanger(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_volume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_UAValue = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_UAValue].value;
	m_heatFluxToAmbient = elem.m_para[NANDRAD::HydraulicNetworkElement::P_HeatFlux].value;
}

TNHeatExchanger::~TNHeatExchanger()
{

}

unsigned int TNHeatExchanger::nInternalStates() const
{
	return 1;
}

void TNHeatExchanger::setInitialTemperature(double T0) {
	m_temperature = T0;
	m_specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// set inlet and outölet tenoeratures and specific enthalpies
	m_inletTemperature = T0;
	m_outletTemperature = T0;
	m_inletSpecificEnthalpy = m_specificEnthalpy;
	m_outletSpecificEnthalpy = m_specificEnthalpy;
}

void TNHeatExchanger::initialInternalStates(double * y0) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	y0[0] = m_specificEnthalpy * m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNHeatExchanger::setInternalStates(const double * y)
{
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	m_temperature = m_specificEnthalpy / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNHeatExchanger::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -heatLoss() + m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}

void TNHeatExchanger::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_outletTemperature = m_temperature;
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_inletTemperature = m_temperature;
	}
}

void TNHeatExchanger::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNHeatExchanger::meanTemperature() const {
	return m_temperature;
}

double TNHeatExchanger::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNHeatExchanger::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNHeatExchanger::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNHeatExchanger::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNHeatExchanger::heatLoss() const
{
	return m_heatFluxToAmbient;
}



// *** Pump ***

TNPump::TNPump(const NANDRAD::HydraulicNetworkElement & /*elem*/,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_volume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
}

TNPump::~TNPump()
{

}

unsigned int TNPump::nInternalStates() const
{
	return 1;
}

void TNPump::initialInternalStates(double * y0) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	y0[0] = m_specificEnthalpy * m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNPump::setInternalStates(const double * y)
{
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	m_temperature = m_specificEnthalpy / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNPump::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -heatLoss() + m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}

void TNPump::setInitialTemperature(double T0) {
	m_temperature = T0;
	m_specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// set inlet and outölet tenoeratures and specific enthalpies
	m_inletTemperature = T0;
	m_outletTemperature = T0;
	m_inletSpecificEnthalpy = m_specificEnthalpy;
	m_outletSpecificEnthalpy = m_specificEnthalpy;
}

void TNPump::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_outletTemperature = m_temperature;
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_inletTemperature = m_temperature;
	}
}

void TNPump::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNPump::meanTemperature() const {
	return m_temperature;
}

double TNPump::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNPump::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNPump::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNPump::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNPump::heatLoss() const
{
	return 0.0;
}

} // namespace NANDRAD_MODEL
