#include "NM_HydraulicNetworkFlowElements.h"

#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

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
							const NANDRAD::HydraulicNetworkComponent & component,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	FUNCID(HNPipeElement::HNPipeElement);

	// TODO : Hauke, check if parameters are actually given and issue error message like "Missing 'xxx' parameter in
	//        network element 'displayname (id=xxx)'." so that users know how to fix the problem.

	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_diameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_roughness = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeRoughness].value;

	if (m_length<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has length <= 0").arg(elem.m_id),FUNC_ID);
	if (m_diameter<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has diameter <= 0").arg(elem.m_id),FUNC_ID);
	if (m_roughness<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has roughness <= 0").arg(elem.m_id),FUNC_ID);
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

double HNPipeElement::pressureLossFriction(const double &mdot) const{
	// for negative mass flow: Reynolds number is positive, velocity and pressure loss are negative
	double velocity = mdot / (m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value * m_diameter * m_diameter * PI / 4);
	double Re = std::abs(velocity) * m_diameter / m_fluid->m_kinematicViscosity.m_values.value(m_fluidTemperature);
	return m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value / 2 * std::abs(velocity) * velocity
				* m_length / m_diameter * frictionFactorSwamee(Re, m_diameter, m_roughness);
}

double HNPipeElement::frictionFactorSwamee(const double &Re, const double &diameter, const double &roughness){
	if (Re < RE_LAMINAR)
		return 64/Re;
	else if (Re < RE_TURBULENT) {
		// TODO : rewrite
		double fTurb = std::log10((roughness / diameter) / 3.7 + 5.74 / std::pow(RE_TURBULENT, 0.9) );
		return 64/RE_LAMINAR + (Re - RE_LAMINAR) * (0.25/(fTurb*fTurb) - 64/RE_LAMINAR) / (RE_TURBULENT - RE_LAMINAR);
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

HNFixedPressureLossCoeffElement::HNFixedPressureLossCoeffElement(const NANDRAD::HydraulicNetworkElement &elem,
																 const NANDRAD::HydraulicNetworkComponent &component,
																 const NANDRAD::HydraulicFluid &fluid):
	m_fluid(&fluid)
{
	FUNCID(HNFixedPressureLossCoeffElement::HNFixedPressureLossCoeffElement);

	m_zeta = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].value;
	m_diameter = component.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].value;

	if (m_diameter<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has diameter <= 0").arg(elem.m_id),FUNC_ID);
	if (m_zeta<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has PressureLossCoefficient <= 0").arg(elem.m_id),FUNC_ID);
}


double HNFixedPressureLossCoeffElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// for negative mass flow: dp is negative
	double area = PI / 4 * m_diameter * m_diameter;
	double dp = std::abs(mdot) * mdot * m_zeta / (2 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value
			* (area * area));
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

HNConstantPressurePump::HNConstantPressurePump(const NANDRAD::HydraulicNetworkElement &elem,
												const NANDRAD::HydraulicNetworkComponent &component,
												const NANDRAD::HydraulicFluid &fluid):
	m_fluid(&fluid)
{
	m_pressureHead = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].value;
}


double HNConstantPressurePump::systemFunction(double /*mdot*/, double p_inlet, double p_outlet) const
{
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
