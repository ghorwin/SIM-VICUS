/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NM_ThermalNetworkPrivateH
#define NM_ThermalNetworkPrivateH

#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

#include <vector>

namespace NANDRAD {
	class HydraulicFluid;
}

namespace NANDRAD_MODEL {

/*!	A model that computes all temperature states of hydraulic network given the internal energy density.
*/

struct Network;

class ThermalNetworkModelImpl {
public:
	ThermalNetworkModelImpl() { }
	~ThermalNetworkModelImpl() { }

	/*! Initialized solver based on current content of m_flowElements.
		Setup needs to be called whenever m_flowElements vector changes
		(but not, when parameters inside flow elements change!).
	*/
	void setup(const Network &nw,
			   const NANDRAD::HydraulicFluid &fluid);

	/*! Updates all states and fluxes.
	*/
	int update();

	/*! Vector of dependencies.
	*/
	void dependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const;

	/*! Container for flow element implementation objects.
		Size must equal the number of edges.
	*/
	std::vector<ThermalNetworkAbstractFlowElement*>	m_flowElements;

	/*! Container for flow element object copies with heat loss.
		Elements are nullptr if no heat loss exists.
	*/
	std::vector<const ThermalNetworkAbstractFlowElementWithHeatLoss*>
													m_heatLossElements;

	/*! Constant access to network. */
	const Network									*m_network = nullptr;
	/*! Container with specific enthalpy for each node.
		TODO : Anne, remove
	*/
	std::vector<double>								m_nodalSpecificEnthalpies;
	/*! Container with temperatures for each node.
	*/
	std::vector<double>								m_nodalTemperatures;
	/*! References to heat fluxes out of each heat flow element.
		TODO : Anne, rename flowElementHeatLossRefs
	*/
	std::vector<const double*>						m_fluidHeatFluxRefs;
	/*! References to temperatures for inlet node of each flow element.
	*/
	std::vector<const double*>						m_inletNodeTemperatureRefs;
	/*! References to with temperatures for outlet node of each flow element.
	*/
	std::vector<const double*>						m_outletNodeTemperatureRefs;
	/*! Container with global pointer to calculated mass fluxes.
	*/
	const double									*m_fluidMassFluxes;

private:

	/*! Constant access to fluid. */
	const NANDRAD::HydraulicFluid					*m_fluid = nullptr;
};



} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkPrivateH
