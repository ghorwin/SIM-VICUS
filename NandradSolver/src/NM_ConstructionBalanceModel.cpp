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


void ConstructionBalanceModel::initInputReferences(const std::vector<AbstractModel *> &)
{

}

void ConstructionBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const
{

}

const std::vector<const double *> & ConstructionBalanceModel::inputValueRefs() const
{

}

void ConstructionBalanceModel::setInputValueRef(const InputReference & inputRef, const QuantityDescription & resultDesc, const double * resultValueRef)
{

}

int ConstructionBalanceModel::update() {
	// process all interfaces
	calculateBoundaryConditions(true, m_con->m_interfaceA);
	calculateBoundaryConditions(false, m_con->m_interfaceB);
}

int ConstructionBalanceModel::ydot(double * ydot)
{

}

void ConstructionBalanceModel::calculateBoundaryConditions(bool sideA, const NANDRAD::Interface & iface) {
	// determine zone ID
	unsigned int zoneID = iface.m_zoneId;
	if (zoneID == 0) {
		// outside location - process parametrized boundary conditions
		if (iface.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
			// we need ambient temperature and our surface temperature
			double Tambient = *m_inputRefs[InputRef_AmbientTemperature];
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
				}
				case NANDRAD::InterfaceHeatConduction::NUM_MT: ;// nothing to do, just to silence compiler warning
			}

		}
	}

}



} // namespace NANDRAD_MODEL

