#ifndef PUMP_H
#define PUMP_H

#include "AbstractFlowElement.h"

class Pump : public AbstractFlowElement {
public:
	Pump();
	Pump(unsigned int n_inlet, unsigned int n_outlet) : AbstractFlowElement(n_inlet, n_outlet) {}

	// AbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const;

	double mdot(double p_inlet, double p_outlet) const;
	void dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const;
};

#endif // PUMP_H
