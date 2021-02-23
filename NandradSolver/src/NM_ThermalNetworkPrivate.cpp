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

#include <IBK_messages.h>

#include "NM_ThermalNetworkPrivate.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_HydraulicNetworkModel_p.h"

#include <NANDRAD_HydraulicFluid.h>

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



void ThermalNetworkModelImpl::setup(const Network &nw,
									const NANDRAD::HydraulicFluid &fluid) {
	// copy nodes pointer from network
	m_network = &nw;
	// resize temperatures
	m_nodalTemperatures.resize(nw.m_nodes.size());
	// resize specific enthalpy
	m_nodalSpecificEnthalpies.resize(nw.m_nodes.size());
	// resize temperatures
	m_inletNodeTemperatureRefs.resize(nw.m_elements.size(), nullptr);
	// resize temperatures
	m_outletNodeTemperatureRefs.resize(nw.m_elements.size(), nullptr);
	// resize heat fluxes
	m_fluidHeatFluxRefs.resize(nw.m_elements.size(), nullptr);

	// copy nodal temperatures
	for(unsigned int i = 0; i < nw.m_elements.size(); ++i) {
		const Element &elem = nw.m_elements[i];
		// copy heat fluxes
		m_inletNodeTemperatureRefs[i] = &m_nodalTemperatures[elem.m_nodeIndexInlet];
		m_outletNodeTemperatureRefs[i] = &m_nodalTemperatures[elem.m_nodeIndexOutlet];
	}
	for(unsigned int i = 0; i < m_heatLossElements.size(); ++i) {
		const ThermalNetworkAbstractFlowElementWithHeatLoss *heatLossElem
				= m_heatLossElements[i];
		// skip empty elements
		if(heatLossElem == nullptr)
			continue;
		// copy heat fluxes
		m_fluidHeatFluxRefs[i] = &heatLossElem->m_heatLoss;
	}
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
		double specEnthalp = 0;

		std::vector<unsigned int> inletIdxs =
				m_network->m_nodes[i].m_elementIndexesInlet;
		std::vector<unsigned int> outletIdxs =
				m_network->m_nodes[i].m_elementIndexesOutlet;

		double massFluxInlet = 0.0;
		// select all pipes with positive flux into element
		for(unsigned int idx : inletIdxs) {
			const double massFlux = m_fluidMassFluxes[idx];
			if(massFlux > 0) {
				massFluxInlet += massFlux;
				// and retrieve specfic enthalpy
				double temp = m_flowElements[idx]->outflowTemperature();
				// sum up
				specEnthalp += massFlux * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value * temp;
			}
		}
		// select all pipes with negative flux into element
		for(unsigned int idx : outletIdxs) {
			const double massFlux = m_fluidMassFluxes[idx];
			if(massFlux < 0) {
				massFluxInlet -= massFlux;
				// and retrieve specfic enthalpy
				double temp = m_flowElements[idx]->outflowTemperature();
				// sum up
				specEnthalp += massFlux * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value * temp;
			}
		}
		IBK_ASSERT(massFluxInlet != 0.0);
		specEnthalp/=massFluxInlet;

		m_nodalSpecificEnthalpies[i] = specEnthalp;
		m_nodalTemperatures[i] = specEnthalp/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
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

	// dependencies of all nodal enthalpies
	for(unsigned int i = 0; i < m_network->m_nodes.size(); ++i) {
		// result quantities
		const double* specEnthPtr = &m_nodalSpecificEnthalpies[i];
		const double* tempPtr = &m_nodalTemperatures[i];
		// set dependency between specific enthalpy and tempretuare
		resultInputValueReferences.push_back(std::make_pair(tempPtr, specEnthPtr) );
		resultInputValueReferences.push_back(std::make_pair(specEnthPtr, tempPtr) );

		std::vector<unsigned int> inletIdxs =
				m_network->m_nodes[i].m_elementIndexesInlet;
		std::vector<unsigned int> outletIdxs =
				m_network->m_nodes[i].m_elementIndexesOutlet;

		for(unsigned int idx : inletIdxs) {
			// mass flux dependencies
			const double* massFluxRef = m_fluidMassFluxes +idx;
			resultInputValueReferences.push_back(std::make_pair(specEnthPtr, massFluxRef) );
		}
		for(unsigned int idx : outletIdxs) {
			// mass flux dependencies
			const double* massFluxRef = m_fluidMassFluxes +idx;
			resultInputValueReferences.push_back(std::make_pair(specEnthPtr, massFluxRef) );
		}
	}
	// dependencies of flow element inlet enthalpies
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		// set enthalpy and mass fluxes for all flow elements
		// and update their simulation results
		const Element &fe = m_network->m_elements[i];
		// get inlet node
		const double *massFluxRef = m_fluidMassFluxes + i;
		const double *specEnthalpRef = &m_nodalSpecificEnthalpies[fe.m_nodeIndexInlet];
		const double *tempInletRef = m_inletNodeTemperatureRefs[i];
		// enthalpy and heat flux dependencies
		const double *heatFluxRef = m_fluidHeatFluxRefs[i];

		if(heatFluxRef != nullptr) {
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, massFluxRef) );
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, specEnthalpRef) );
		}
		// temperature dependencies
		resultInputValueReferences.push_back(std::make_pair(tempInletRef, specEnthalpRef) );
		// inverse direction
		specEnthalpRef = &m_nodalSpecificEnthalpies[fe.m_nodeIndexOutlet];
		const double *tempOutletRef = m_outletNodeTemperatureRefs[i];
		if(heatFluxRef != nullptr) {
			// enthalpy dependencies
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, specEnthalpRef) );
		}
		// temperature dependencies
		resultInputValueReferences.push_back(std::make_pair(tempOutletRef, specEnthalpRef) );
	}
	// dependencies of flow element heat fluxes
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		// set enthalpy and mass fluxes for all flow elements
		// and update their simulation results
		const Element &fe = m_network->m_elements[i];
		// get inlet node
		const double *massFluxRef = m_fluidMassFluxes + i;
		const double *specEnthalpRef = &m_nodalSpecificEnthalpies[fe.m_nodeIndexInlet];
		const double *tempInletRef = m_inletNodeTemperatureRefs[i];
		// enthalpy and heat flux dependencies
		const double *heatFluxRef = m_fluidHeatFluxRefs[i];
		if(heatFluxRef != nullptr) {
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, massFluxRef) );
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, specEnthalpRef) );
		}
		// temperature dependencies
		resultInputValueReferences.push_back(std::make_pair(tempInletRef, specEnthalpRef) );
		// inverse direction
		specEnthalpRef = &m_nodalSpecificEnthalpies[fe.m_nodeIndexOutlet];
		const double *tempOutletRef = m_outletNodeTemperatureRefs[i];
		// enthalpy dependencies
		if(heatFluxRef != nullptr) {
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, specEnthalpRef) );
		}
		// temperature dependencies
		resultInputValueReferences.push_back(std::make_pair(tempOutletRef, specEnthalpRef) );

//		// set dependencies to ambient conditions
//		const double* Tamb = m_ambientTemperatureRefs[i];
//		const double* alphaAmb = m_ambientHeatTransferRefs[i];
//		// ambient temperature is given
//		if(Tamb != nullptr) {
//			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, Tamb) );
//			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, alphaAmb) );
//		}
	}
}


} // namespace NANDRAD_MODEL
