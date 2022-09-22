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

#ifndef NM_ThermalNetworkPrivateH
#define NM_ThermalNetworkPrivateH

#include "NM_ThermalNetworkAbstractFlowElementWithHeatLoss.h"

#include <vector>

namespace NANDRAD {
	class HydraulicFluid;
}

namespace NANDRAD_MODEL {

struct Network;

/*!	This is a class that hides common variables and member functions used by both ThermalNetworkStatesModel
	and ThermalNetworkBalanceModel, so that the declarations and includes do not clutter the interfaces of
	the model.

	When looking at the code, just think of these variables being part of ThermalNetworkStatesModel.
*/
class ThermalNetworkModelImpl {
public:
	~ThermalNetworkModelImpl();

	/*! Initialized solver based on current content of m_flowElements.
		Setup needs to be called whenever m_flowElements vector changes
		(but not, when parameters inside flow elements change!).
	*/
	void setup(const Network &nw,
			   const NANDRAD::HydraulicFluid &fluid);

	/*! Updates all states and fluxes. */
	int update();

	/*! Returns vector of dependencies. */
	void dependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const;

	/*! Container for flow element implementation objects.
		Size must equal the number of edges.
	*/
	std::vector<ThermalNetworkAbstractFlowElement*>	m_flowElements;

	/*! Container for flow element object copies with heat loss.
		Elements are nullptr if no heat loss exists. Size = m_flowElements.size().
	*/
	std::vector<const ThermalNetworkAbstractFlowElementWithHeatLoss*>
													m_heatLossElements;

	/*! Constant access to network. */
	const Network									*m_network = nullptr;

	/*! Container with external references to temperatures for each node (may be null if
		no reference is given).

		For normal hydraulic networks, nodes do not have a capacity or own state. Hence,
		the nodal temperatures are computed in update() based on the mixer rule of all inflowing fluids
		whereby the temperature is taken from the elements upstream.

		However, for zones, the temperature at the zone node corresponds to the well-mixed zone temperature.
		Since now the outflow temperature from the zone node is given, it may differ from inflowing enthalpy and
		hence we have an excees heat load which is recorded in m_heatLoads.

		Vector size matches number of nodes. Plain nodes (no zones) have a nullptr.
	*/
	std::vector<const double*>						m_nodalTemperatureRefs;
	/*! Container with temperatures for each node (size = number of nodes). */
	std::vector<double>								m_nodalTemperatures;
	/*! Container with heat load for each node (size = number of nodes).
		Value is zero for plain nodes (i.e. where m_nodalTemperatureRefs[i] == nullptr).
	*/
	std::vector<double>								m_heatLoads;
	/*! Container with global pointer to calculated mass fluxes.
		Pointer maps to calculated fluid mass fluxes from HydraulicNetworkModel.
	*/
	const double									*m_fluidMassFluxes;

private:

	/*! Constant access to fluid. */
	const NANDRAD::HydraulicFluid					*m_fluid = nullptr;
};



} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkPrivateH
