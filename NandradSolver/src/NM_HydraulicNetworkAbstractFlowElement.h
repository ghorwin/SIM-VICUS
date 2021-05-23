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

namespace NANDRAD_MODEL {

/*! Defines the abstract interface for a flow element.
	The system function and partial derivative function must be implemented by child objects.
*/
class HydraulicNetworkAbstractFlowElement {
public:
	HydraulicNetworkAbstractFlowElement() {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~HydraulicNetworkAbstractFlowElement();

	/*! The flow element's system function. Actually evaluates the residual of the system function
		for a given combination of mass flux [kg/s], inlet and output pressures [Pa].
	*/
	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const = 0;

	/*! Returns partial derivatives of the system functions w.r.t. the three dependent variables. */
	virtual void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const = 0;

	/*! Sets fluid temperature [K] for all internal pipe volumes. */
	virtual void setFluidTemperature(double fluidTemp) { (void)fluidTemp; }

};

} // namespace NANDRAD_MODEL

#endif // NM_HydraulicNetworkAbstractFlowElementH
