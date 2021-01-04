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

#include "NM_ThermalNetworkAbstractFlowElement.h"

#include <vector>

namespace NANDRAD_MODEL {

/*!	A model that computes all temperature states of hydraulic network given the internal energy density.
*/

class ThermalNetworkModelImpl {
public:
	ThermalNetworkModelImpl() { }
	~ThermalNetworkModelImpl() { }

	/*! Constant access to heat flux vector*/
	const double *heatFluxes() const;

	/*! Initialized solver based on current content of m_flowElements.
		Setup needs to be called whenever m_flowElements vector changes
		(but not, when parameters inside flow elements change!).
	*/
	void setup();

	/*! Updates all states at network nodes.
	*/
	int updateStates();

	/*! Updates all heat fluxes through the pipes.
	*/
	int updateFluxes();

	/*! Container for flow element implementation objects.
		Size must equal the number of edges.
	*/
	std::vector<ThermalNetworkAbstractFlowElement*>	m_flowElements;

	/*! Container with global pointer to calculated mass fluxes.
	*/
	const double									*m_massFluxes;

private:
	/*! Stores connectivity information. */
	struct Node {
		Node() {}
		Node(unsigned int i1, unsigned int i2) {
			m_flowElementIndexesInlet.push_back(i1);
			m_flowElementIndexesOutlet.push_back(i2);
		}

		/*! Vector with indexes of inlet flow elements. */
		std::vector<unsigned int> m_flowElementIndexesInlet;
		std::vector<unsigned int> m_flowElementIndexesOutlet;
	};


	/*! Vector of nodes.
	*/
	std::vector<Node>								m_nodes;
	/*! Container with specific enthalpy for each node.
	*/
	std::vector<double>								m_specificEnthalpy;
	/*! Vector result heat fluxes. */
	std::vector<double>								m_heatFluxes;
};



} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkPrivateH
