#include "Pump.h"

#include <cmath>

Pump::Pump()
{

}

const double MAX_DELTAP = 5000;
const double scale = MAX_DELTAP/(0.8*0.8);

double Pump::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	double deltaP = 0;
	if (mdot < 0)
		deltaP = MAX_DELTAP + mdot*mdot*mdot*mdot*100000; // max pump pressure + penalty
	else if (mdot > 0.8)
		deltaP = 0;
	else {
		// use inverse quadratic function

		deltaP = MAX_DELTAP - mdot*mdot*scale;
	}
	return p_inlet - p_outlet + deltaP; // Mind: pumps add pressure
}


double Pump::mdot(double p_inlet, double p_outlet) const {
	// problem: flow reversal, should be a switch of equations from "pump" to "flow resistance"
	if (p_outlet > p_inlet)
		return 0;

	double q = -(p_inlet - p_outlet - MAX_DELTAP)/scale;
	return std::sqrt(std::fabs(q));

}


void Pump::dmdot_dp(double, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const {
	const double EPS = 0.001;
	double m = mdot(p_inlet, p_outlet);
	double m1 = mdot(p_inlet+EPS, p_outlet);
	double m2 = mdot(p_inlet, p_outlet+EPS);
	dmdp_in = (m1-m)/EPS;
	dmdp_out = (m2-m)/EPS;
}

