#include "NM_HydraulicNetworkFlowElements.h"
#include "NM_Physics.h"

#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"
#include "NANDRAD_HydraulicFluid.h"

#define PI				3.141592653589793238


namespace NANDRAD_MODEL {

// Definition of destructor is here, so that we have the code and virtual function table
// only once.
HydraulicNetworkAbstractFlowElement::~HydraulicNetworkAbstractFlowElement() {
}


// *** HNPipeElement ***

HNPipeElement::HNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_diameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_roughness = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeRoughness].value;
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
}


double  HNPipeElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// In case of multiple parallel pipes, mdot is the mass flux through *all* pipes
	// (but all parallel sections together need the same pressure drop as a single one
	return p_inlet - p_outlet - pressureLossFriction(mdot/m_nParallelPipes);	// this is the system function
}


void HNPipeElement::partials(double mdot, double p_inlet, double p_outlet,
							 double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	// partial derivatives of the system function to pressures are constants
	df_dp_inlet = 1;
	df_dp_outlet = -1;

	// generic DQ approximation of partial derivative
	const double EPS = 1e-5; // in kg/s
	double f_eps = systemFunction(mdot+EPS, p_inlet, p_outlet);
	double f = systemFunction(mdot, p_inlet, p_outlet);
	df_dmdot = (f_eps - f)/EPS;
}


void HNPipeElement::setFluidTemperature(double fluidTemp) {
	m_fluidTemperature = fluidTemp;
}


double HNPipeElement::pressureLossFriction(const double &mdot) const {
	// for negative mass flow: Reynolds number is positive, velocity and pressure loss are negative
	double fluidDensity = m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
	double velocity = mdot / (fluidDensity * m_diameter * m_diameter * PI / 4);
	double Re = std::abs(velocity) * m_diameter / m_fluid->m_kinematicViscosity.m_values.value(m_fluidTemperature);
	double zeta = m_length / m_diameter * FrictionFactorSwamee(Re, m_diameter, m_roughness);
	return zeta * fluidDensity / 2 * std::abs(velocity) * velocity;
}



// *** HNFixedPressureLossCoeffElement ***

HNFixedPressureLossCoeffElement::HNFixedPressureLossCoeffElement(const NANDRAD::HydraulicNetworkComponent &component,
																 const NANDRAD::HydraulicFluid &fluid)
{
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_zeta = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].value;
	m_diameter = component.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].value;
}


double HNFixedPressureLossCoeffElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// for negative mass flow: dp is negative
	double area = PI / 4 * m_diameter * m_diameter;
	double velocity = mdot / (m_fluidDensity * area); // signed!
	double dp = m_zeta * m_fluidDensity / 2 * std::abs(velocity) * velocity;
	return p_inlet - p_outlet - dp;
}


void HNFixedPressureLossCoeffElement::partials(double mdot, double p_inlet, double p_outlet,
							 double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	// partial derivatives of the system function to pressures are constants
	df_dp_inlet = 1;
	df_dp_outlet = -1;
	// generic DQ approximation of partial derivative
	const double EPS = 1e-5; // in kg/s
	double f_eps = systemFunction(mdot+EPS, p_inlet, p_outlet);
	double f = systemFunction(mdot, p_inlet, p_outlet);
	df_dmdot = (f_eps - f)/EPS;
}



// *** HNControlledPressureLossCoeffElement ***

HNControlledPressureLossCoeffElement::HNControlledPressureLossCoeffElement(const NANDRAD::HydraulicNetworkComponent &component,
																		   const NANDRAD::HydraulicFluid &fluid)
{
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_zetaFix = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].value;
	m_diameter = component.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].value;
}


double HNControlledPressureLossCoeffElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {

	// NOTE: m_zetaControlled is set by the ThermalNetwork in TNElementWithExternalHeatLoss::setInflowTemperature()
	double area = PI / 4 * m_diameter * m_diameter;
	double velocity = mdot / (m_fluidDensity * area); // signed!
	double dp = (m_zetaFix + m_zetaControlled) * m_fluidDensity / 2 * std::abs(velocity) * velocity;
	return p_inlet - p_outlet - dp;
}


void HNControlledPressureLossCoeffElement::partials(double mdot, double p_inlet, double p_outlet,
							 double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	// partial derivatives of the system function to pressures are constants
	df_dp_inlet = 1;
	df_dp_outlet = -1;
	// generic DQ approximation of partial derivative
	const double EPS = 1e-5; // in kg/s
	double f_eps = systemFunction(mdot+EPS, p_inlet, p_outlet);
	double f = systemFunction(mdot, p_inlet, p_outlet);
	df_dmdot = (f_eps - f)/EPS;
}



// *** HNConstantPressurePump ***

HNConstantPressurePump::HNConstantPressurePump(const NANDRAD::HydraulicNetworkComponent &component) {
	m_pressureHead = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].value;
}


double HNConstantPressurePump::systemFunction(double /*mdot*/, double p_inlet, double p_outlet) const {
	return p_inlet - p_outlet + m_pressureHead;
}


void HNConstantPressurePump::partials(double /*mdot*/, double /*p_inlet*/, double /*p_outlet*/,
							 double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	// partial derivatives of the system function to pressures are constants
	df_dp_inlet = 1;
	df_dp_outlet = -1;
	df_dmdot = 0;
}



} // namespace NANDRAD_MODEL
