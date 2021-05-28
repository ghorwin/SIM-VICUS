/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_HydraulicNetworkFlowElementsH
#define NM_HydraulicNetworkFlowElementsH

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


class TNElementWithExternalHeatLoss;

/*! Element that calculates the pressure loss according to a given pressure loss coefficient
	(which is in Germany usually called zeta-value).
	Optionally, there might be a mass flow controller implemented in a referenced m_thermalNetworkElement
	that provides mass-flow dependent additional zeta values.
*/
class HNPressureLossCoeffElement : public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNPressureLossCoeffElement(const NANDRAD::HydraulicNetworkComponent & component,
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

	/*! Optional pointer to the corresponding thermal network flow element. If nullptr, there is no
		additional zeta-value to be calculated.
		The referenced element computes the additional zeta value to be added to m_zeta.
	*/
	TNElementWithExternalHeatLoss * m_thermalNetworkElement = nullptr;
	friend class ThermalNetworkStatesModel;

}; // HNFixedPressureLossCoeffElement


/*! Pump model with fixed constant pressure head */
class HNConstantPressurePump: public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNConstantPressurePump(unsigned int id, const NANDRAD::HydraulicNetworkComponent & component);

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Element's ID, needed to formulated input references. */
	unsigned int					m_id;
	/*! If not nullptr, this pressure head value (from an external model) is used instead of the constant one. */
	const double					*m_pressureHeadRef = nullptr;

private:
	/*! Constant pressure head [Pa] to be added. */
	double							m_pressureHead = -999;

}; // HNConstantPressurePump


/*! A network that dictates either mass flux or pressure difference.
	Note: there must be only one of these models in the network.
*/
class HNConstantMassFluxPump: public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNConstantMassFluxPump(unsigned int id, const NANDRAD::HydraulicNetworkComponent & component);

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Element's ID, needed to formulated input references. */
	unsigned int					m_id;
	/*! If not nullptr, this mass flux value (from an external model) is used instead of the constant one. */
	const double					*m_massFluxRef = nullptr;

private:
	/*! Constant mass flux [kg/s] to be enforced. */
	double							m_massFlux = -999;

}; // HNConstantPressurePump

} // namespace NANDRAD_MODEL

#endif // NM_HydraulicNetworkFlowElementsH
