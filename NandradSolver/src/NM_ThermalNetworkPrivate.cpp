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
	m_inletNodeTemperatures.resize(nw.m_elements.size());
	// resize temperatures
	m_outletNodeTemperatures.resize(nw.m_elements.size());
	// resize heat fluxes
	m_fluidHeatFluxes.resize(nw.m_elements.size());
	// get fluid heat capacity
	m_fluid = &fluid;
}


int ThermalNetworkModelImpl::updateStates() {

	// set ambient conditions and calculate outlet temperatures
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *flowElem = m_flowElements[i];

		// set ambient conditions
		const double* Tamb = m_ambientTemperatureRefs[i];
		const double* alphaAmb = m_ambientHeatTransferRefs[i];

		// ambient temperature is given
		if(Tamb != nullptr) {
			IBK_ASSERT(alphaAmb != nullptr);

			flowElem->setAmbientConditions(*Tamb, *alphaAmb);
		}
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
				double specEnthalpy = m_flowElements[idx]->outletSpecificEnthalpy();
				// sum up
				specEnthalp += massFlux * specEnthalpy;
			}
		}
		// select all pipes with negative flux into element
		for(unsigned int idx : outletIdxs) {
			const double massFlux = m_fluidMassFluxes[idx];
			if(massFlux < 0) {
				massFluxInlet -= massFlux;
				// and retrieve specfic enthalpy
				double specEnthalpy = m_flowElements[idx]->outletSpecificEnthalpy();
				// sum up
				specEnthalp -= massFlux * specEnthalpy;
			}
		}
		IBK_ASSERT(massFluxInlet != 0.0);
		specEnthalp/=massFluxInlet;

		m_nodalSpecificEnthalpies[i] = specEnthalp;
		m_nodalTemperatures[i] = specEnthalp/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	}

	return 0;
}


int ThermalNetworkModelImpl::updateFluxes() 	{

	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *flowElem = m_flowElements[i];
		// set enthalpy and mass fluxes for all flow elements
		// and update their simulation results
		const Element &fe = m_network->m_elements[i];
		// get inlet node
		const double massFlux = m_fluidMassFluxes[i];
		const double specEnthalpInlet = m_nodalSpecificEnthalpies[fe.m_nInlet];
		const double specEnthalpOutlet = m_nodalSpecificEnthalpies[fe.m_nOutlet];
		// positive mass flux
		if(massFlux >= 0) {
			flowElem->setInletFluxes(massFlux, specEnthalpInlet * massFlux);
		}
		// negative mass flux
		else {
			flowElem->setInletFluxes(massFlux, specEnthalpOutlet * massFlux);
		}
		// copy nodal temperatures
		m_inletNodeTemperatures[i] = specEnthalpInlet/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		m_outletNodeTemperatures[i] = specEnthalpOutlet/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// copy heat fluxes
		m_fluidHeatFluxes[i] = flowElem->heatLoss();
	}
	return 0;
}


void ThermalNetworkModelImpl::dependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	// dependencies of all nodal enthalpies
	for(unsigned int i = 0; i < m_network->m_nodes.size(); ++i) {
		// result quantities
		const double* specEnthPtr = &m_nodalSpecificEnthalpies[i];
		const double* tempPtr = &m_nodalTemperatures[i];

		std::vector<unsigned int> inletIdxs =
				m_network->m_nodes[i].m_elementIndexesInlet;
		std::vector<unsigned int> outletIdxs =
				m_network->m_nodes[i].m_elementIndexesOutlet;

		for(unsigned int idx : inletIdxs) {
			// mass flux dependencies
			const double* massFluxRef = m_fluidMassFluxes +idx;
			resultInputValueReferences.push_back(std::make_pair(specEnthPtr, massFluxRef) );
			resultInputValueReferences.push_back(std::make_pair(tempPtr, massFluxRef) );
		}
		for(unsigned int idx : outletIdxs) {
			// mass flux dependencies
			const double* massFluxRef = m_fluidMassFluxes +idx;
			resultInputValueReferences.push_back(std::make_pair(specEnthPtr, massFluxRef) );
			resultInputValueReferences.push_back(std::make_pair(tempPtr, massFluxRef) );
		}
	}
	// dependencies of flow element inlet enthalpies
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		// set enthalpy and mass fluxes for all flow elements
		// and update their simulation results
		const Element &fe = m_network->m_elements[i];
		// get inlet node
		const double *massFluxRef = m_fluidMassFluxes + i;
		const double *specEnthalpRef = &m_nodalSpecificEnthalpies[fe.m_nInlet];
		const double *tempInletRef = &m_inletNodeTemperatures[i];
		// enthalpy and heat flux dependencies
		const double *heatFluxRef = &m_fluidHeatFluxes[i];
		resultInputValueReferences.push_back(std::make_pair(heatFluxRef, massFluxRef) );
		resultInputValueReferences.push_back(std::make_pair(heatFluxRef, specEnthalpRef) );
		// temperature dependencies
		resultInputValueReferences.push_back(std::make_pair(tempInletRef, specEnthalpRef) );
		// inverse direction
		specEnthalpRef = &m_nodalSpecificEnthalpies[fe.m_nOutlet];
		const double *tempOutletRef = &m_outletNodeTemperatures[i];
		// enthalpy dependencies
		resultInputValueReferences.push_back(std::make_pair(heatFluxRef, specEnthalpRef) );
		// temperature dependencies
		resultInputValueReferences.push_back(std::make_pair(tempOutletRef, specEnthalpRef) );

		// set dependencies to ambient conditions
		const double* Tamb = m_ambientTemperatureRefs[i];
		const double* alphaAmb = m_ambientHeatTransferRefs[i];
		// ambient temperature is given
		if(Tamb != nullptr) {
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, Tamb) );
			resultInputValueReferences.push_back(std::make_pair(heatFluxRef, alphaAmb) );
		}
	}
}


} // namespace NANDRAD_MODEL
