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


#include "NM_ConstructionBalanceModel.h"

#include <NANDRAD_ConstructionInstance.h>

#include "NM_ConstructionStatesModel.h"

namespace NANDRAD_MODEL {


void ConstructionBalanceModel::setup(const NANDRAD::ConstructionInstance & con,
									 const NANDRAD::SimulationParameter & simPara,
									 const ConstructionStatesModel * statesModel)
{
	m_con = &con;
	m_statesModel = statesModel;

	// cross section area in [m2]
	m_area = con.m_para[NANDRAD::ConstructionInstance::CP_AREA].value;
	/// \todo subtract areas of embedded objects to get net area

	// resize storage vectors for divergences, sources, and initialize boundary conditions
	m_ydot.resize(m_statesModel->m_n);
	m_results.resize(NUM_R);
}

void ConstructionBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const
{

}

void ConstructionBalanceModel::resultValueRefs(std::vector<const double *> & res) const
{

}

const double * ConstructionBalanceModel::resultValueRef(const QuantityName & quantityName) const
{

}


int ConstructionBalanceModel::priorityOfModelEvaluation() const {
	// we are one step above room balance model
	return AbstractStateDependency::priorityOffsetTail+4;
}


void ConstructionBalanceModel::initInputReferences(const std::vector<AbstractModel *> &) {

}


void ConstructionBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// first add scalar input references
	inputRefs.resize(NUM_InputRef);

	// compute input references depending on requirements of interfaces

	// side A

	if (m_con->m_interfaceA.haveBCParameters()) {
		// check if we have heat conduction
		if (m_con->m_interfaceA.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
			// if ambient zone, create an input reference for
			if (m_con->m_interfaceA.m_zoneId == 0) {
				InputReference ref;
				ref.m_id = 0;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
				ref.m_name.m_name = "Temperature";
				inputRefs[InputRef_AmbientTemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceA.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				inputRefs[InputRef_RoomATemperature] = ref;
			}
		}
	}

	// side B

	if (m_con->m_interfaceB.haveBCParameters()) {
		// check if we have heat conduction
		if (m_con->m_interfaceB.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
			// if ambient zone, create an input reference for
			if (m_con->m_interfaceB.m_zoneId == 0) {
				InputReference ref;
				ref.m_id = 0;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
				ref.m_name.m_name = "Temperature";
				inputRefs[InputRef_AmbientTemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceB.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				inputRefs[InputRef_RoomATemperature] = ref;
			}
		}
	}
}


void ConstructionBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												 const std::vector<const double *> & resultValueRefs)
{
	m_valueRefs = resultValueRefs;
}


int ConstructionBalanceModel::update() {
	// process all interfaces and compute boundary fluxes
	calculateBoundaryConditions(true, m_con->m_interfaceA);
	calculateBoundaryConditions(false, m_con->m_interfaceB);

	unsigned int nElements = m_statesModel->m_nElements;

	// compute internal sources

	// now compute all divergences in all elements

	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal code
	}
	else {
		double * ydot = &m_ydot[0];
		const double * qHeatCond = &m_statesModel->m_fluxes_q[0];
		const ConstructionStatesModel::Element * E = &m_statesModel->m_elements[0];
		ydot[0] = m_fluxDensityHeatConductionA;
		for (unsigned int i=1; i<nElements; ++i) {
			ydot[i-1] -= qHeatCond[i];	// Mind: we _subtract_ flux
			ydot[i] = qHeatCond[i];		// Mind: we _set_ the positive flux
			// finally divide by element volume (volume = dx * 1m2)
			ydot[i-1] /= E[i-1].dx;
		}
		ydot[nElements-1] -= m_fluxDensityHeatConductionB;
	}
	return 0; // signal success
}


int ConstructionBalanceModel::ydot(double * ydot) {
	std::memcpy(ydot, &m_ydot[0], sizeof(double)*m_ydot.size());
}


void ConstructionBalanceModel::calculateBoundaryConditions(bool sideA, const NANDRAD::Interface & iface) {

	// *** heat conduction boundary condition ***

	if (iface.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {

		// determine zone ID
		unsigned int zoneID = iface.m_zoneId;
		double Tambient;
		if (zoneID == 0) {
			// we need ambient temperature and our surface temperature
			Tambient = *m_valueRefs[InputRef_AmbientTemperature];
		}
		else {
			if (sideA)
				Tambient = *m_valueRefs[InputRef_RoomATemperature];
			else
				Tambient = *m_valueRefs[InputRef_RoomBTemperature];
		}
		switch (iface.m_heatConduction.m_modelType) {
			case NANDRAD::InterfaceHeatConduction::MT_CONSTANT : {
				// transfer coefficient
				double alpha = iface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
				double Ts = sideA ? m_statesModel->m_TsA : m_statesModel->m_TsB;

				// flux density [W/m2] into left side construction
				double fluxDensity = alpha*(Tambient - Ts);
				// total flux [W]
				double flux = fluxDensity*m_area;

				// store results
				if (sideA) {
					m_fluxDensityHeatConductionA = fluxDensity;
					m_results[R_FluxHeatConductionA] = flux;
				}
				else {
					m_fluxDensityHeatConductionB = fluxDensity;
					m_results[R_FluxHeatConductionB] = -flux; // Mind sign convention
				}
			} break;

			case NANDRAD::InterfaceHeatConduction::NUM_MT: ;// nothing to do, just to silence compiler warning
		}
	}


	// *** solar radiation boundary condition
}



} // namespace NANDRAD_MODEL

