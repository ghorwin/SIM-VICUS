#ifndef HydraulicNetworkFlowElementsH
#define HydraulicNetworkFlowElementsH

#include <vector>

#include "NM_HydraulicNetworkAbstractFlowElement.h"

namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicFluid;
}

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element. */
class HNPipeElement : public HydraulicNetworkAbstractFlowElement {
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNPipeElement(const NANDRAD::HydraulicNetworkElement & def,
				  const NANDRAD::HydraulicNetworkComponent & component,
				  const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const override;

private:

	/*! Some fluid flow resistance. */
	double m_res;
};


// TODO : add other flow element classes



} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkFlowElementsH
