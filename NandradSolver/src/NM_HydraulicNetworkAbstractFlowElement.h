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

#ifndef NM_HydraulicNetworkAbstractFlowElementH
#define NM_HydraulicNetworkAbstractFlowElementH

#include "NM_InputReference.h"
#include "NM_QuantityDescription.h"

namespace NANDRAD_MODEL {

/*! Defines the abstract interface for a flow element.
	The system function and partial derivative function must be implemented by child objects.
*/
class HydraulicNetworkAbstractFlowElement {
public:
	HydraulicNetworkAbstractFlowElement() {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~HydraulicNetworkAbstractFlowElement();

	/*! Publishes individual model quantities via descriptions. */
	virtual void modelQuantities(std::vector<QuantityDescription> &/*quantities*/) const { }

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	virtual void modelQuantityValueRefs(std::vector<const double*> &/*valRefs*/) const { }

	/*! The flow element's system function. Actually evaluates the residual of the system function
		for a given combination of mass flux [kg/s], inlet and output pressures [Pa].
	*/
	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const = 0;

	/*! Returns partial derivatives of the system functions w.r.t. the three dependent variables. */
	virtual void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const = 0;

	/*! Adds flow-element-specific input references (schedules etc.) to the list of input references.
		Default implementation does nothing.
	*/
	virtual void inputReferences(std::vector<NANDRAD_MODEL::InputReference> & /*inputRefs*/) const {}

	/*! Provides the element with its own requested model inputs.
		The element must take exactly as many input values from the vector and move the iterator forward.
		When the function returns, the iterator must point to the first input reference past this element's inputs.
	*/
	virtual void setInputValueRefs(std::vector<const double *>::const_iterator & /*resultValueRefs*/) {}

	/*! Called at the end of a successful Newton iteration. Allows to calculate and store results for other model objects. */
	virtual void updateResults(double mdot, double p_inlet, double p_outlet) { (void)mdot; (void)p_inlet; (void)p_outlet; }

	/*! Called at the beginning of each CVODE time step. Store current time point for derivatives calculations here.*/
	virtual void setTime(double t) { (void)t; }

	/*! Called at the end of a successful CVODE time step. Implement hysteresis here.*/
	virtual void stepCompleted(double t) { (void)t; }

	/*! Optional function for registering dependencies between mass flux and externally referenced input values.
		Default implementation does nothing. Re-implement for elements with mass-flow controllers.
	*/
	virtual void dependencies(const double */*mdot*/,
							  std::vector<std::pair<const double *, const double *> > & ) const {}

	/*! This function is called after each integration step (default implementation does nothing). */
	virtual void stepCompleted(double t) { (void)t; }

	/*! Computes and returns serialization size in bytes, by default returns  returns an invalid value (-1). */
	virtual std::size_t serializationSize() const { return 0; }

	/*! Stores model content at memory location pointed to by dataPtr.
	*/
	virtual void serialize(void* & dataPtr) const { (void)dataPtr; }

	/*! Restores model content from memory at location pointed to by dataPtr.
	*/
	virtual void deserialize(void* & dataPtr) { (void)dataPtr; }

	/*! Reference to memory slot containing the (average) fluid temperature in [K] of the flow element.
		For MT_HydraulicNetwork this points to the Network parameter HydraulicNetwork::P_DefaultFluidTemperature.
		For MT_ThermalHydraulicNetwork this is a reference to the result "FluidTemperature" computed by
		ThermalNetworkStatesModel.
	*/
	const double * m_fluidTemperatureRef = nullptr;
};

} // namespace NANDRAD_MODEL

#endif // NM_HydraulicNetworkAbstractFlowElementH
