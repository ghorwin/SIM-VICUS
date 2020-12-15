#include "NM_HydraulicNetworkFlowElements.h"

#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#define PI					3.141592653589793238

namespace NANDRAD_MODEL {

// Definition of destructor is here, so that we have the code and virtual function table
// only once.
HydraulicNetworkAbstractFlowElement::~HydraulicNetworkAbstractFlowElement() {

}


HNPipeElement::HNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkComponent & component,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].get_value(IBK::Unit("m"));
	m_diameter = component.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].get_value(IBK::Unit("m"));
	m_roughness = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeRoughness].get_value(IBK::Unit("m"));
}


double  HNPipeElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// TODO Andreas: how do we get the fluid temperature here?
	double temp = 20;
	return p_inlet - p_outlet - pressureLossFriction(mdot, temp);	// this is the system function
}


void HNPipeElement::dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const {
	// partial derivatives of the system function are constants
	dmdp_in = 1./(2*m_res*mdot+1e-8);
	dmdp_out = -dmdp_in;
}


double HNPipeElement::pressureLossFriction(const double &mdot, const double &temperature) const{
	double velocity = mdot / (m_fluid.m_para[NANDRAD::HydraulicFluid::P_Density].get_value(IBK::Unit("kg/m3")) * m_diameter * m_diameter * PI / 4);
	double Re = velocity * m_diameter / m_fluid.m_kinematicViscosity.m_values.value(temperature);
	if (Re < 1e-6) // TODO Anne: which threshold should we use?
		return 0.0;
	else
		return m_fluid.m_para[NANDRAD::HydraulicFluid::P_Density].get_value(IBK::Unit("kg/m3")) / 2 * velocity * velocity
				* m_length / m_diameter * frictionFactorSwamee(Re, m_diameter, m_roughness);
}


double HNPipeElement::frictionFactorSwamee(const double &Re, const double &diameter, const double &roughness){
	IBK_ASSERT(roughness>0 && diameter>0 && Re>0);
	if (Re < m_Re1)
		return 64/Re;
	else if (Re < m_Re2){
		double fTurb = 0.25 / (std::log10((roughness / diameter) / 3.7 + 5.74 / std::pow(m_Re2, 0.9) ));
		return 64/m_Re1 + (Re - m_Re1) * (fTurb*fTurb - 64/m_Re1) / (m_Re2 - m_Re1);
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
	double a = 1 / (1 + std::pow(Re/m_Re1, 9));
	double b = 1 / (1 + (Re / (160 * delta)) * (Re / (160 * delta)) );
	return 1 / ( std::pow(Re/64, a) * std::pow( 1.8*std::log10(Re / 6.8), 2*(1-a)*b )
			* std::pow( 2*std::log10(3.7*delta), 2*(1-a)*(1-b) ) );
}


double HNPipeElement::m_Re1 = 1700;

double HNPipeElement::m_Re2 = 4000;



} // namespace NANDRAD_MODEL
