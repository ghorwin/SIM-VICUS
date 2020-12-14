#include "NM_HydraulicNetworkFlowElements.h"

namespace NANDRAD_MODEL {

// Definition of destructor is here, so that we have the code and virtual function table
// only once.
HydraulicNetworkAbstractFlowElement::~HydraulicNetworkAbstractFlowElement() {

}




HNPipeElement::HNPipeElement(const NANDRAD::HydraulicNetworkElement & def,
											const std::vector<NANDRAD::HydraulicNetworkComponent> & components,
											const NANDRAD::HydraulicFluid & fluid)
{


}


double  HNPipeElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// TODO : do we need to enfore flow direction?
//	if (mdot < 0)
//		return mdot*mdot*100000; // penalty term

	// simple quadratic relationship defines pressure drop - mass flow relation
	double deltaP = mdot*mdot*m_res;
	return p_inlet - p_outlet - deltaP;	// this is the system function
}


void HNPipeElement::dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const {
	// partial derivatives of the system function are constants
	dmdp_in = 1./(2*m_res*mdot+1e-8);
	dmdp_out = -dmdp_in;
}


} // namespace NANDRAD_MODEL
