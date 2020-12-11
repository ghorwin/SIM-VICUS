#ifndef PIPEELEMENT_H
#define PIPEELEMENT_H

#include "AbstractFlowElement.h"

class PipeElement : public AbstractFlowElement {
public:
	PipeElement();
	PipeElement(unsigned int n_inlet, unsigned int n_outlet, double res);

	// AbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const;

	double mdot(double p_inlet, double p_outlet) const;
	void dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const;

	double m_res;
};

#endif // PIPEELEMENT_H
