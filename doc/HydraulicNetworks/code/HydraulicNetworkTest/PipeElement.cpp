#include "PipeElement.h"

#include <cmath>

PipeElement::PipeElement()
{

}

PipeElement::PipeElement(unsigned int n_inlet, unsigned int n_outlet, double res) :
	AbstractFlowElement(n_inlet, n_outlet),
	m_res(res)
{
}

double PipeElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// simple quadratic relationship
	if (mdot < 0)
		return mdot*mdot*100000; // penalty term

	double deltaP = mdot*mdot*m_res;
	return p_inlet - p_outlet - deltaP;	// this is the system function

}

double PipeElement::mdot(double p_inlet, double p_outlet) const {
	double q = (p_inlet - p_outlet)/m_res;
	// problem: flow reversal
	return std::sqrt( std::fabs(q));
}


void PipeElement::dmdot_dp(double mdot, double, double, double & dmdp_in, double & dmdp_out) const {
	// partial derivatives of the system function are constants
	dmdp_in = 1./(2*m_res*mdot+1e-8);
	dmdp_out = -dmdp_in;
}
