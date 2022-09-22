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

#include <IBK_messages.h>

#include "NM_ThermalNetworkPrivate.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_HydraulicNetworkModelPrivate.h"

#include <NANDRAD_HydraulicFluid.h>

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {


ThermalNetworkModelImpl::~ThermalNetworkModelImpl() {
	for (ThermalNetworkAbstractFlowElement* e : m_flowElements)
		delete e;
}


void ThermalNetworkModelImpl::setup(const Network &nw,
									const NANDRAD::HydraulicFluid &fluid) {
	// copy nodes pointer from network
	m_network = &nw;
	// resize temperatures
	m_nodalTemperatures.resize(nw.m_nodes.size());
	// resize temperature references
	m_nodalTemperatureRefs.resize(nw.m_nodes.size(), nullptr);
	// resize heat loads
	m_heatLoads.resize(nw.m_nodes.size(), 0.0);
	// get fluid heat capacity
	m_fluid = &fluid;
}


int ThermalNetworkModelImpl::update() {

	// udpate mass flux
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *flowElem = m_flowElements[i];
		// update mass fluxes for each flow element
		const double massFlux = m_fluidMassFluxes[i];
		// set all nodal conditions
		flowElem->setMassFlux(massFlux);
	}

	// calculate enthalpy fluxes for all nodes
	for(unsigned int i = 0; i < m_network->m_nodes.size(); ++i) {

		// set enthalpy flux to 0
		double enthalpyFluxInlet = 0;

		// vector of elements that push fluid into the node
		std::vector<unsigned int> inletIdxs =
				m_network->m_nodes[i].m_elementIndexesInlet;
		// vector of elements where the node fluid goes into
		std::vector<unsigned int> outletIdxs =
				m_network->m_nodes[i].m_elementIndexesOutlet;

		// Note: the actual mass flux direction determines what will be inlet/outlet.

		// first we sum up all mass fluxes *into* the node, wether they flow from 'inlet' or 'outlet' elements
		// we also sum up the enthalpies

		double massFluxInlet = 0.0;
		// select all pipes with positive flux into element
		for(unsigned int idx : inletIdxs) {
			const double massFlux = m_fluidMassFluxes[idx];
			if(massFlux > 0) {
				massFluxInlet += massFlux;
				// and retrieve specific enthalpy
				double temp = m_flowElements[idx]->outflowTemperature();
				// sum up
				enthalpyFluxInlet += massFlux * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value * temp;
			}
		}
		// select all pipes with negative flux into element
		for(unsigned int idx : outletIdxs) {
			const double massFlux = m_fluidMassFluxes[idx];
			if(massFlux < 0) {
				massFluxInlet -= massFlux; // mind negative sign of mass flux
				// and retrieve specific enthalpy
				double temp = m_flowElements[idx]->outflowTemperature();
				// sum up (mind negative sign of mass flux!)
				enthalpyFluxInlet += -massFlux * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value * temp;
			}
		}

		// special case: temperature is given by a transient balance equation
		if(m_nodalTemperatureRefs[i] != nullptr) {
			double temperatureOutlet = *m_nodalTemperatureRefs[i];
			double enthalpyFluxOutlet = massFluxInlet * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value
					* temperatureOutlet;
			// copy values
			m_nodalTemperatures[i] = temperatureOutlet;
			m_heatLoads[i] = enthalpyFluxInlet - enthalpyFluxOutlet;
		}
		// calulate node temperature
		else {
			// if we encounter a trivial flow solution (massFluxes all zero), then we just keep the temperatures the same
			if (massFluxInlet != 0.0) {
				m_nodalTemperatures[i] = enthalpyFluxInlet/(massFluxInlet * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value);
			}
		}
	}

	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *flowElem = m_flowElements[i];
		// set enthalpy and mass fluxes for all flow elements
		// and update their simulation results
		const Element &fe = m_network->m_elements[i];
		// get inlet node
		const double massFlux = m_fluidMassFluxes[i];

		double	inflowTemp = 0.0;
		// dependening on mass flux set inflow temperature
		if(massFlux >= 0)
			inflowTemp = m_nodalTemperatures[fe.m_nodeIndexInlet];
		else
			inflowTemp = m_nodalTemperatures[fe.m_nodeIndexOutlet];
		// set all nodal conditions
		flowElem->setInflowTemperature(inflowTemp);
	}
	return 0;
}


void ThermalNetworkModelImpl::dependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	// NOTE: we only use one representative mass flux entry, see documentation in HydraulicNetworkModel::stateDependencies()
	const double* massFluxRef = m_fluidMassFluxes;
	// dependencies of all nodal temperatures to flow element mass fluxes and temperatures
	for (unsigned int i = 0; i < m_network->m_nodes.size(); ++i) {
		// result quantities
		const double* tempPtr = &m_nodalTemperatures[i];
		// we either have a given reference in case of zone nodes, in which case our node temperature only depends
		// on the zone temperature (Note: m_nodalTemperatureRefs[i] holds pointers to zone temperatures)
		// Otherwise, wie calculate nodal temperature directly and hence node temperature depends on (all) mass fluxes
		if(m_nodalTemperatureRefs[i] != nullptr)
			resultInputValueReferences.push_back(std::make_pair(tempPtr, m_nodalTemperatureRefs[i]) );
		else
			resultInputValueReferences.push_back(std::make_pair(tempPtr, massFluxRef) );
	}
}


} // namespace NANDRAD_MODEL
