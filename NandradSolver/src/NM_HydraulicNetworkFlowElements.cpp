#include "NM_HydraulicNetworkFlowElements.h"

#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"
#include "NANDRAD_HydraulicFluid.h"

#define PI				3.141592653589793238

/*! Reynolds number where flow switches from laminar to transition state. */
#define RE_LAMINAR		1700

/*! Reynolds number where flow switches from transition state to turbulent */
#define RE_TURBULENT	4000

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
}


double  HNPipeElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	return p_inlet - p_outlet - pressureLossFriction(mdot);	// this is the system function
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
	double zeta = m_length / m_diameter * frictionFactorSwamee(Re, m_diameter, m_roughness);
	return zeta * fluidDensity / 2 * std::abs(velocity) * velocity;
}

double HNPipeElement::frictionFactorSwamee(const double &Re, const double &diameter, const double &roughness) {
	if (Re < RE_LAMINAR)
		return 64/Re;
	else if (Re < RE_TURBULENT){
		double fLam = 64/RE_LAMINAR; // f(RE_LAMINAR)
		double fTurb = std::log10((roughness / diameter) / 3.7 + 5.74 / std::pow(RE_TURBULENT, 0.9) );
		fTurb = 0.25/(fTurb*fTurb); // f(RE_TURBULENT)
		// now interpolate linearly between fLam and fTurb
		return fLam + (Re - RE_LAMINAR) * (fTurb - fLam) / (RE_TURBULENT - RE_LAMINAR);
	}
	else{
		double f = std::log10( (roughness / diameter) / 3.7 + 5.74 / std::pow(Re, 0.9) ) ;
		return	0.25 / (f*f);
	}
}

/*! this one is probably quite expensive ? */
double HNPipeElement::frictionFactorCheng(const double &Re, const double &diameter, const double &roughness){
	double delta = diameter / roughness;
	double a = 1 / (1 + std::pow(Re/RE_LAMINAR, 9));
	double b = 1 / (1 + (Re / (160 * delta)) * (Re / (160 * delta)) );
	return 1 / ( std::pow(Re/64, a) * std::pow( 1.8*std::log10(Re / 6.8), 2*(1-a)*b )
			* std::pow( 2*std::log10(3.7*delta), 2*(1-a)*(1-b) ) );
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
