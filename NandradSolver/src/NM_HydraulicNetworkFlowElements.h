#ifndef HydraulicNetworkFlowElementsH
#define HydraulicNetworkFlowElementsH

#include <vector>

#include "NM_HydraulicNetworkAbstractFlowElement.h"

#include "NANDRAD_HydraulicFluid.h"

namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
}


namespace NANDRAD_MODEL {


/*! Element that calculates the pressure loss of a straight pipe */
class HNPipeElement : public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & component,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Sets fluid temperature [K] for all internal pipe volumes. */
	void setFluidTemperature(double fluidTemp) override;

	/*! pressure loss due to pipe wall friction in Pa. For positive mass flows, there will be a positive pressure loss
	 * mdot:	mass flow  in kg/m3
	 * temperature: temperature in C
	 */
	double pressureLossFriction(const double &mdot) const;

	// friction factors are static, so I can use them from VICUS for sizing algorithm

	/*! darcy friction factor for pipe wall, swamee-jain equation (approximation of colebrook equation) */
	static double frictionFactorSwamee(const double &Re, const double &diameter, const double &roughness);

	/*! darcy friction factor for pipe wall, equation according to cheng 2008 (doi:10.1061/(asce)0733-9429(2008)134:9(1357))
		relatively expensive ??? */
	static double frictionFactorCheng(const double &Re, const double &diameter, const double &roughness);


private:

	/*! Some fluid flow resistance. */
	double							m_res;

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! pipe length in m */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_diameter;

	/*! roughness of pipe wall in m */
	double							m_roughness;

	/*! Fluid temperature [K], for now constant, will be an input reference and retrieved from network-thermal-balances model. */
	double							m_fluidTemperature = 293.15;

};



/*! Element that calculates the pressure loss according to a given fixed pressure loss coefficient
 * (which is in Germany usually called zeta-value) */
class HNFixedPressureLossCoeffElement : public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNFixedPressureLossCoeffElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & component,
				  const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;
	void setFluidTemperature(double /*fluidTemp*/) override { }


private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! the pressure loss coefficient [-] */
	double							m_zeta;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_diameter;

};



/*! Pump model with fixed constant pressure head */
class HNConstantPressurePump: public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS

public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNConstantPressurePump(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & component,
				  const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;
	void setFluidTemperature(double /*fluidTemp*/) override { }

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	double							m_pressureHead;
};

} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkFlowElementsH
