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

#include <map>


namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkControlElement;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
	class Thermostat;
}

namespace NANDRAD_MODEL {

class AbstractController;

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

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references. */
	virtual void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	virtual void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	/*! Function for registering dependencies between mass flux and externally referenced input values. */
	virtual void dependencies(const double * mdot,
		std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const override;

	// HydraulicNetworkAbstractFlowElement interface
	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Called at the end of a successful Newton iteration. Allows to calculate and store results. */
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
							   const NANDRAD::HydraulicNetworkControlElement *controlElement,
							   unsigned int numberParallelElements);

	/*! D'tor, releases allocated controller objetcs. */
	~HNPressureLossCoeffElement() override;

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references. */
	virtual void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	virtual void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	/*! Function for registering dependencies between mass flux and externally referenced input values. */
	virtual void dependencies(const double * mdot,
		std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const override;

	// HydraulicNetworkAbstractFlowElement interface
	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	virtual void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Called at the end of a successful Newton iteration. Allows to calculate and store results. */
	virtual void updateResults(double mdot, double p_inlet, double p_outlet) override;

	/*! This function is called at the begin of an integrator step and signals the end time point
		of the current step.
		The implementation simply forwards the call to the controller object, if used.
	*/
	void setTime(double t) override;

	/*! Called at the end of a completed time step.
		\param t Time point at end of step, can be used to compute the step length.
	*/
	void stepCompleted(double t) override;


	/*! Id number of following flow element. This is used to obtain the outlet temperature of the follwing
		flow element in order to control e.g. its temperature difference.
	*/
	unsigned int					m_followingflowElementId = NANDRAD::INVALID_ID;

private:

	/*! Computes the controlled zeta-value if a control-model is implemented.
		Otherwise returns 0.
	*/
	double zetaControlled(double mdot) const;

	/*! Id number of flow element. */
	unsigned int					m_id = NANDRAD::INVALID_ID;

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

	/*! the calculated controller zeta value for the valve */
	double							m_controllerError = -999;

	/*! number of parallel elements (mass flux will be divided by this number) */
	unsigned int					m_numberParallelElements = 1;

	/*! Reference to the controller parametrization object.*/
	const NANDRAD::HydraulicNetworkControlElement	*m_controlElement = nullptr;

	/*! Value reference to external quantity. */
	const double					*m_heatExchangeHeatLossRef = nullptr;

	/*! Value reference to temperature difference set point. */
	const double					*m_temperatureDifferenceSetpointRef = nullptr;

	/*! Value reference to mass flux set point. */
	const double					*m_massFluxSetpointRef = nullptr;

	/*! Value reference to external quantity. */
	const double					*m_followingFlowElementFluidTemperatureRef = nullptr;

	/*! Pointer to controller object used in this element */
	AbstractController				*m_controller = nullptr;

	friend class ThermalNetworkStatesModel;

}; // HNFixedPressureLossCoeffElement





/*! Implements core functionality for maximumPressureHead calculation based on various limiting properties.
	Inherit this class in all pump models that require power limitation.
*/
class HNAbstractPowerLimitedPumpModel {
public:
	HNAbstractPowerLimitedPumpModel(const double & density, const NANDRAD::HydraulicNetworkComponent &component);

	void modelQuantities(std::vector<QuantityDescription> &quantities) const;

	void modelQuantityValueRefs(std::vector<const double *> &valRefs) const;


protected:
	/*! Fluid density in kg/m3 */
	double							m_density = -999;
	/*! Efficiency of pump in - */
	double							m_maxEfficiency = -999;
	/*! Maximum electrical power at point of best operation in W */
	double							m_maxElectricalPower = -999;
	/*! Maximum pressure head at point of minimal mass flow in Pa */
	double							m_maxPressureHeadAtZeroFlow = -999;
	/*! Coefficients of quadratic polynom for el. power, always in SI units, always 3 coefficients */
	std::vector<double>				m_coefficientsPelMax;
	/*! Coefficients of quadratic polynom for maximum pressure head, always in SI units, always 3 coefficients */
	std::vector<double>				m_coefficientsDpMax;
	/*! Determines wether pump has power limit, if not just a constant efficiency is given */
	bool							m_isPowerLimited = true;

	/*! Calculates actual maximum pressure head [Pa] which linear decreases with mass flux */
	double maximumPressureHead(const double &mdot) const;
	/*! Calculates actual efficiency */
	double efficiency(const double &mdot, const double & dp) const;
	double electricalPower(const double &mdot, const double & dp, const double & eta) const;

}; // HNAbstractPowerLimitedPumpModel





/*! Pump model with fixed/scheduled constant pressure head */
class HNConstantPressurePump: public HydraulicNetworkAbstractFlowElement, // NO KEYWORDS
							  public HNAbstractPowerLimitedPumpModel {
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNConstantPressurePump(unsigned int id, const NANDRAD::HydraulicNetworkComponent & component,
						   const NANDRAD::HydraulicFluid & fluid,
						   const NANDRAD::HydraulicNetworkControlElement * ctr);

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;
	void inputReferences(std::vector<InputReference> &) const override;
	void setInputValueRefs(std::vector<const double *>::const_iterator &resultValueRefIt) override;
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;
	void modelQuantityValueRefs(std::vector<const double *> &valRefs) const override;
	void updateResults(double mdot, double p_inlet, double p_outlet) override;

	/*! We update our nonlinear heat flux dependent control signal here. Thus, we avoid CVODE's Newton method to iterate over
		mass flux dependent heat fluxes. */
	virtual void setTime(double t) override;

	unsigned int										m_followingflowElementId = NANDRAD::INVALID_ID;

private:
	/*! Element's ID, needed to formulate input references. */
	unsigned int										m_id;
	/*! Value reference to pressure head [Pa] */
	const double										* m_pressureHeadRef = nullptr;
	/*! Controller for pump (used for on/off controlling) */
	const NANDRAD::HydraulicNetworkControlElement		* m_controlElement = nullptr;
	/*! Reference to heat loss of following element (used for on/off controlling) */
	const double										* m_followingElementHeatLossRef = nullptr;
	/*! Determines wether pump is currently in operation (used for on/off controlling) */
	bool												m_pumpIsOn = true;
	/*! The electrical power demand in current operation point in W */
	double							m_electricalPower = -999;
	/*! The efficiency in current operation point - */
	double							m_efficiency = -999;

}; // HNConstantPressurePump





/*! Valve model with fixed constant pressure head */
class HNConstantPressureLossValve: public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNConstantPressureLossValve(unsigned int id, const NANDRAD::HydraulicNetworkComponent & component);

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;
	void inputReferences(std::vector<InputReference> &) const override;
	void setInputValueRefs(std::vector<const double *>::const_iterator &resultValueRefIt) override;

private:
	/*! Element's ID, needed to formulate input references. */
	unsigned int					m_id;
	/*! Value reference to pressure head [Pa] */
	const double					*m_pressureLossRef = nullptr;
}; // HNConstantPressureLossValve






/*! Pump model where pressure head is controlled based on mass flux requirements. */
class HNConstantMassFluxPump: public HydraulicNetworkAbstractFlowElement { // NO KEYWORDS
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNConstantMassFluxPump(unsigned int id, const NANDRAD::HydraulicNetworkComponent & component);

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references. */
	virtual void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	virtual void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Called at the end of a successful Newton iteration. Allows to calculate and store results. */
	virtual void updateResults(double mdot, double p_inlet, double p_outlet) override;

	/*! Return value reference of pressure head computed by flow element. */
	const double * pressureHeadRef() const { return &m_pressureHead; }

private:
	/*! Calculated pressure head. */
	double							m_pressureHead = -999;
	/*! Element's ID, needed to formulate input references. */
	unsigned int					m_id;
	/*! Value reference to target mass flux [kg/s] */
	const double					*m_massFluxRef = nullptr;

}; // HNConstantMassFluxPump





/*! Pump model where pressure head is controlled based on mass flux requirements. */
class HNControlledPump: public HydraulicNetworkAbstractFlowElement,		// NO KEYWORDS
						public HNAbstractPowerLimitedPumpModel {
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNControlledPump(const NANDRAD::HydraulicNetworkElement & e,
					 const NANDRAD::HydraulicFluid & fluid, const std::vector<unsigned int> * networkElementIds,
					 const std::vector<double> * networkPressureDifferences);

	/*! Destructor, memory cleanup. */
	~HNControlledPump() override;

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references. */
	virtual void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & inputRefs) const override;

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	virtual void setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) override;

	/*! Function for registering dependencies between mass flux and externally referenced input values. */
	virtual void dependencies(const double * mdot,
		std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const override;

	/*! Called at the end of a successful Newton iteration. Allows to calculate and store results. */
	virtual void updateResults(double mdot, double p_inlet, double p_outlet) override;

	/*! Computes and returns serialization size in bytes. */
	virtual std::size_t serializationSize() const override;

	/*! Stores model content at memory location pointed to by dataPtr. */
	virtual void serialize(void* & dataPtr) const override;

	/*! Restores model content from memory at location pointed to by dataPtr. */
	virtual void deserialize(void* & dataPtr) override;

	/*! Return value reference of pressure head computed by flow element. */
	const double * pressureHeadRef() const { return &m_pressureHead; }

	/*! called after succesful CVODE step */
	void stepCompleted(double t) override;

	/*! Id number of flow element. */
	unsigned int							m_followingflowElementId = NANDRAD::INVALID_ID;

private:
	/*! Computes the controlled pressure head if a control-model is implemented.
		Otherwise returns 0.
	*/
	double pressureHeadControlled(double mdot) const;

	/*! Computes the minimum of all observed pressure differences, also provides the node id (meant to be VICUS node ID),
	 *  according to the map m_controlElement.m_values */
	double pressureDifferenceAtWorstPoint(double & worstPointNodeId) const;


	/*! Reference to the controller parametrization object.*/
	const NANDRAD::HydraulicNetworkControlElement
									*m_controlElement = nullptr;

	/*! Pointer to controller object used in this controlled pump. */
	AbstractController				*m_controller = nullptr;

	/*! Calculated pressure head. */
	double							m_pressureHead = -999;
	/*! Element's ID, needed to formulate input references. */
	unsigned int					m_id;
	/*! Value reference to external quantity. */
	const double					*m_followingFlowElementFluidTemperatureRef = nullptr;
	/*! Value reference to setpoint mass flux. */
	const double					*m_massFluxSetpointRef = nullptr;
	/*! Value reference to temperature difference set point. */
	const double					*m_temperatureDifferenceSetpointRef = nullptr;
	/*! Value reference to pressuer difference set point. */
	const double					*m_pressureDifferenceSetpointRef = nullptr;
	/*! The calculated temperature difference */
	double							m_temperatureDifference = -999;
	/*! Reference to observed pressured differences for WorstpointController*/
	std::vector<std::vector<const double *> >		m_observedPressureDifferenceRefs;
	/*! Element id map of observed elements for Worstpoint controller */
	std::map<std::string, std::vector<double> >		m_observedPressureDiffElementIdMap;
	/*! Reference to all element ids of network (m_elementIds), needed for worstpoint controller */
	const std::vector<unsigned int>		*m_networkElementIds;
	/*! Reference to all pressure differences of network (m_p->m_pressureDifferences), needed for worstpoint controller */
	const std::vector<double>			*m_networkPressureDifferences;
	/*! Minimum of observed pressure differences (worst point), used as output only */
	double							m_pressureDifferenceWorstpoint = -999;
	/*! Id of point with minimum pressure difference (worst point), meant to be VICUS nodeId, used as output only */
	double							m_nodeIdWorstpoint = 0;

	/*! The electrical power demand in current operation point in W */
	double							m_electricalPower = -999;
	/*! The efficiency in current operation point - */
	double							m_efficiency = -999;

}; // HNControlledPump






/*! Pump model where pressure head is controlled based on mass flux requirements. */
class HNVariablePressureHeadPump: public HydraulicNetworkAbstractFlowElement,  // NO KEYWORDS
								  public HNAbstractPowerLimitedPumpModel  {
public:
	/*! C'tor, takes and caches parameters needed for function evaluation. */
	HNVariablePressureHeadPump(unsigned int id, const NANDRAD::HydraulicNetworkComponent & component,
								const NANDRAD::HydraulicFluid & fluid);

	double systemFunction(double mdot, double p_inlet, double p_outlet) const override;
	void partials(double mdot, double p_inlet, double p_outlet,
				  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const override;

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Called at the end of a successful Newton iteration. Allows to calculate and store results. */
	virtual void updateResults(double mdot, double p_inlet, double p_outlet) override;

	/*! Return value reference of pressure head computed by flow element. */
	const double * pressureHeadRef() const { return &m_pressureHead; }

private:

	double pressureHead(double mdot) const;

	/*! Element's ID, needed to formulate input references. */
	unsigned int					m_id;
	/*! current pressure head of pump in [Pa] */
	double							m_pressureHead = -999;
	/*! pressure head at design point, needed for calculation of dp-v curve slope in [Pa] */
	double							m_designPressureHead = -999;
	/*! mass flux at design point, needed for calculation of dp-v curve slope in [kg/s] */
	double							m_designMassFlux = -999;
	/*! pressure head at zero flow in [Pa] */
	double							m_minimumPressureHead = -999;

	/*! The electrical power demand in current operation point in W */
	double							m_electricalPower = -999;
	/*! The efficiency in current operation point - */
	double							m_efficiency = -999;

}; // HNVariablePressureHeadPump




} // namespace NANDRAD_MODEL

#endif // NM_HydraulicNetworkFlowElementsH
