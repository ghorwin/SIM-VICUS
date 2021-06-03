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

#include <NANDRAD_Constants.h>


namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkControlElement;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
	class Thermostat;
}

namespace NANDRAD_MODEL {

/*! Element that calculates the pressure loss of a straight pipe */
class HNPipeElement : public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid,
				  const NANDRAD::HydraulicNetworkControlElement *controller,
				  const std::vector<NANDRAD::Thermostat> &thermostats);

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references.
		Default implementation does nothing.
	*/
	virtual void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	virtual void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Called at the end of a successful Newton iteration. Allows to calculte and store results.
	*/
	virtual void updateResults(double mdot, double p_inlet, double p_outlet) override;

private:
	/*! Pressure loss due to pipe wall friction in [Pa]. For positive mass flows, there will be a positive pressure loss.
		\param mdot Mass flow in [kg/s]
	 */
	double pressureLossFriction(const double &mdot) const;

	/*! Computes the controlled zeta-value if a control-model is implemented.
		Otherwise returns 0.
	*/
	double zetaControlled() const;

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

	/*! the calculated controller zeta value for the valve */
	double							m_zetaControlled = -999;

	/*! Reference to the heating thermostat control value.*/
	const double					*m_heatingThermostatControlValueRef = nullptr;

	/*! Reference to the cooling thermostat control value.*/
	const double					*m_coolingThermostatControlValueRef = nullptr;

	/*! Reference to the controller parametrization object.*/
	const NANDRAD::HydraulicNetworkControlElement
									*m_controlElement = nullptr;

	/*! Reference to all thermsotat para,etrization obejcts.*/
	const std::vector<NANDRAD::Thermostat> &m_thermostats;

}; // HNPipeElement


/*! Element that calculates the pressure loss according to a given pressure loss coefficient
	(which is in Germany usually called zeta-value).
	Optionally, there might be a mass flow controller implemented in a referenced m_thermalNetworkElement
	that provides mass-flow dependent additional zeta values.
*/
class HNPressureLossCoeffElement : public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNPressureLossCoeffElement(unsigned int flowElementId,
							   const NANDRAD::HydraulicNetworkComponent & component,
							   const NANDRAD::HydraulicFluid & fluid,
							   const NANDRAD::HydraulicNetworkControlElement *controlElement);

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references.
		Default implementation does nothing.
	*/
	virtual void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	virtual void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	// HydraulicNetworkAbstractFlowElement interface
	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	virtual void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Called at the end of a successful Newton iteration. Allows to calculte and store results.
	*/
	virtual void updateResults(double mdot, double p_inlet, double p_outlet) override;

	/*! Id number of following flow element. This is used to obtain the outlet temperature of the follwing
	flow element in order to control e.g. its temperature difference */
	unsigned int					m_followingflowElementId = NANDRAD::INVALID_ID;

private:
	/*! Computes the controlled zeta-value if a control-model is implemented.
		Otherwise returns 0.
	*/
	double zetaControlled(double mdot) const;

	/*! Id number of flow element. */
	unsigned int					m_flowElementId = NANDRAD::INVALID_ID;

	/*! Cached fluid density [kg/m3] */
	double							m_fluidDensity = -999;

	/*! Fluid heat capacity [J/kgK].
		Cached value from fluid properties.
	*/
	double							m_fluidHeatCapacity = -999;

	/*! The pressure loss coefficient [-] */
	double							m_zeta = -999;

	/*! Effective hydraulic (inner) diameter of pipe in [m] */
	double							m_diameter = -999;

	/*! the calculated controller zeta value for the valve */
	double							m_zetaControlled = -999;

	/*! the calculated temperature difference */
	double							m_temperatureDifference = -999;

	/*! Reference to the controller parametrization object.*/
	const NANDRAD::HydraulicNetworkControlElement
									*m_controlElement = nullptr;

	/*! Value reference to external quantity. */
	const double					*m_heatExchangeHeatLossRef = nullptr;

	/*! Value reference to external quantity. */
	const double					*m_followingFlowElementFluidTemperatureRef = nullptr;

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
	void inputReferences(std::vector<InputReference> &) const override;
	void setInputValueRefs(std::vector<const double *>::const_iterator &resultValueRefIt) override;

	/*! Element's ID, needed to formulated input references. */
	unsigned int					m_id;
	/*! If not nullptr, this pressure head value (from an external model) is used instead of the constant one. */
	const double					*m_pressureHeadRef = nullptr;

private:
	/*! Constant pressure head [Pa] to be added. */
	double							m_pressureHead = -999;
}; // HNConstantPressurePump

} // namespace NANDRAD_MODEL

#endif // NM_HydraulicNetworkFlowElementsH
