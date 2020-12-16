#include "NM_HydraulicNetworkFlowElements.h"

#include "NANDRAD_HydraulicNetworkElement.h"
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
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_diameter = component.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].value;
	m_roughness = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeRoughness].value;
}


double  HNPipeElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	return p_inlet - p_outlet - pressureLossFriction(mdot);	// this is the system function
}


void HNPipeElement::dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const {
	// partial derivatives of the system function are constants
	dmdp_in = 1./(2*m_res*mdot+1e-8);
	dmdp_out = -dmdp_in;
}


double HNPipeElement::pressureLossFriction(const double &mdot) const{

	double velocity = mdot / (m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value * m_diameter * m_diameter * PI / 4);
	double Re = velocity * m_diameter / m_fluid->m_kinematicViscosity.m_values.value(m_fluidTemperature);
	if (Re < 1e-6) // TODO Anne: which threshold should we use?
		return 0.0;
	else
		return m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value / 2 * velocity * velocity
				* m_length / m_diameter * frictionFactorSwamee(Re, m_diameter, m_roughness);
}

double HNPipeElement::frictionFactorSwamee(const double &Re, const double &diameter, const double &roughness){
	IBK_ASSERT(roughness>0 && diameter>0 && Re>0);
	if (Re < RE_LAMINAR)
		return 64/Re;
	else if (Re < RE_TURBULENT){
		double fTurb = 0.25 / (std::log10((roughness / diameter) / 3.7 + 5.74 / std::pow(RE_TURBULENT, 0.9) ));
		return 64/RE_LAMINAR + (Re - RE_LAMINAR) * (fTurb*fTurb - 64/RE_LAMINAR) / (RE_TURBULENT - RE_LAMINAR);
	}
	else{
		double f = 0.25 / (std::log10((roughness / diameter) / 3.7 + 5.74 / std::pow(Re, 0.9) ));
		return	f*f;
	}
}

/*! this one is probably quite expensive ? */
double HNPipeElement::frictionFactorCheng(const double &Re, const double &diameter, const double &roughness){
	IBK_ASSERT(roughness>0 && diameter>0 && Re>0);
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
	m_zeta = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].value;
	m_diameter = component.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].value;
}


double HNFixedPressureLossCoeffElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	double area = PI / 4 * m_diameter * m_diameter;
	double dp = mdot * mdot * m_zeta / (2 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value
			* (area * area));
	return p_inlet - p_outlet - dp;

}

void HNFixedPressureLossCoeffElement::dmdot_dp(double mdot, double p_inlet, double p_outlet, double &dmdp_in, double &dmdp_out) const{

}



// *** HNConstantPressurePump ***

HNConstantPressurePump::HNConstantPressurePump(const NANDRAD::HydraulicNetworkElement &elem,
												const NANDRAD::HydraulicNetworkComponent &component,
												const NANDRAD::HydraulicFluid &fluid):
	m_fluid(&fluid)
{
	m_pressureHead = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].value;
}

double HNConstantPressurePump::systemFunction(double mdot, double p_inlet, double p_outlet) const
{
	return p_inlet - p_outlet + m_pressureHead;
}

void HNConstantPressurePump::dmdot_dp(double mdot, double p_inlet, double p_outlet, double &dmdp_in, double &dmdp_out) const
{

}



} // namespace NANDRAD_MODEL
