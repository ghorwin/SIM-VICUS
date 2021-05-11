#ifndef HydraulicNetworkFlowElementsH
#define HydraulicNetworkFlowElementsH

#include "NM_HydraulicNetworkAbstractFlowElement.h"

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
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Sets fluid temperature [K] for all internal pipe volumes. */
	void setFluidTemperature(double fluidTemp) override;

private:
	/*! Pressure loss due to pipe wall friction in [Pa]. For positive mass flows, there will be a positive pressure loss.
		\param mdot Mass flow in [kg/s]
	 */
	double pressureLossFriction(const double &mdot) const;


	/*! The fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! Pipe length in [m] */
	double							m_length;

	/*! Hydraulic (inner) diameter of pipe in [m] */
	double							m_diameter;

	/*! Roughness of pipe wall in [m] */
	double							m_roughness;

	/*! Number of parallel pipes (=1 per default).*/
	unsigned int					m_nParallelPipes;

	/*! Fluid temperature [K], will be updated in each call to setFluidTemperature(). */
	double							m_fluidTemperature = -999;

}; // HNPipeElement



/*! Element that calculates the pressure loss according to a given fixed pressure loss coefficient
	(which is in Germany usually called zeta-value)
*/
class HNFixedPressureLossCoeffElement : public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNFixedPressureLossCoeffElement(const NANDRAD::HydraulicNetworkComponent & component,
		const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

private:
	/*! Cached fluid density [kg/m3] */
	double							m_fluidDensity = -999;

	/*! The pressure loss coefficient [-] */
	double							m_zeta = -999;

	/*! Effective hydraulic (inner) diameter of pipe in [m] */
	double							m_diameter = -999;

}; // HNFixedPressureLossCoeffElement



/*! Element that calculates the pressure loss according to a given fixed pressure loss coefficient
	(which is in Germany usually called zeta-value)
*/
class HNControlledPressureLossCoeffElement : public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNControlledPressureLossCoeffElement(const NANDRAD::HydraulicNetworkComponent & component,
		const NANDRAD::HydraulicFluid & fluid);

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! The controlled pressure loss coefficient [-] */
	double							m_zetaControlled = 0;

private:
	/*! Cached fluid density [kg/m3] */
	double							m_fluidDensity = -999;

	/*! The fixed pressure loss coefficient [-] */
	double							m_zetaFix = -999;

	/*! Effective hydraulic (inner) diameter of pipe in [m] */
	double							m_diameter = -999;

	double							m_fluidHeatCapacity = -999;


}; // HNFixedPressureLossCoeffElement





/*! Pump model with fixed constant pressure head */
class HNConstantPressurePump: public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS

public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNConstantPressurePump(const NANDRAD::HydraulicNetworkComponent & component);

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

private:
	/*! Constant pressure head [Pa] to be added. */
	double							m_pressureHead = -999;

}; // HNConstantPressurePump

} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkFlowElementsH
