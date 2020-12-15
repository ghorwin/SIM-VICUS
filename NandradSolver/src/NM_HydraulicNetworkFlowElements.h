#ifndef HydraulicNetworkFlowElementsH
#define HydraulicNetworkFlowElementsH

#include <vector>

#include "NM_HydraulicNetworkAbstractFlowElement.h"

#include "NANDRAD_HydraulicFluid.h"

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
	HNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & component,
				  const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const override;

	/*! pressure loss due to pipe wall friction in Pa. For positive mass flows, there will be a positive pressure loss
	 * mdot:	mass flow  in kg/m3
	 * temperature: temperature in C
	 */
	double pressureLossFriction(const double &mdot, const double &temperature) const;

	// friction factors are static, so I can use them from VICUS for sizing algorithm

	/*! darcy friction factor for pipe wall, swamee-jain equation (approximation of colebrook equation) */
	static double frictionFactorSwamee(const double &Re, const double &diameter, const double &roughness);

	/*! darcy friction factor for pipe wall, equation according to cheng 2008 (doi:10.1061/(asce)0733-9429(2008)134:9(1357))*/
	static double frictionFactorCheng(const double &Re, const double &diameter, const double &roughness);


private:

	/*! Some fluid flow resistance. */
	double						m_res;

	/*! the fluid, containing all physical parameters */
	NANDRAD::HydraulicFluid		m_fluid;

	/*! pipe length in m */
	double						m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double						m_diameter;

	/*! roughness of pipe wall in m */
	double						m_roughness;

	/*! Reynolds number where flow switches from laminar to transition state */
	static double m_Re1;

	/*! Reynolds number where flow switches from transition state to turbulent */
	static double m_Re2;
};


// TODO : add other flow element classes



} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkFlowElementsH
