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

#ifndef NM_ThermalNetworkAbstractFlowElementWithHeatLossH
#define NM_ThermalNetworkAbstractFlowElementWithHeatLossH

#include "NM_ThermalNetworkAbstractFlowElement.h"


namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element with heat loss (over the boundary).
*/
class ThermalNetworkAbstractFlowElementWithHeatLoss : public ThermalNetworkAbstractFlowElement {
public:
	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalDerivatives(double *ydot) override;

	/*! Publishes 'FlowElementHeatLoss' via descriptions. */
	void modelQuantities(std::vector<QuantityDescription> &quantities) const override;

	/*! Publishes individual model quantity value references: same size as quantity descriptions. */
	void modelQuantityValueRefs(std::vector<const double*> &valRefs) const override;

	/*! Optional function for registering dependencies between derivatives and internal states.
		Re-implemented to add dependencies to computed variable m_heatLoss and from
		m_heatExchangeValueRef.
	*/
	virtual void dependencies(const double *ydot, const double *y,
				const double *mdot, const double* TInflowLeft, const double*TInflowRight,
				std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const override;

	/*! Heat loss over surface of flow element towards the environment in [W].
		This is a loss, i.e. positive means reduction of energy in flow element.
	*/
	double							m_heatLoss = 0.0; // Important: initialize with 0, since some models may never change it!
};


} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkAbstractFlowElementWithHeatLossH
