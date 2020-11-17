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
#include <NANDRAD_Constants.h>
#include <NANDRAD_SimulationParameter.h>

#include "NM_ConstructionStatesModel.h"
#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {


void ConstructionBalanceModel::setup(const NANDRAD::ConstructionInstance & con,
									 double totalAdsorptionAreaA,
									 double totalAdsorptionAreaB,
									 const ConstructionStatesModel * statesModel)
{
	m_con = &con;
	m_statesModel = statesModel;
	m_moistureBalanceEnabled = statesModel->m_moistureBalanceEnabled;

	// cache total absorption surface areas
	m_totalAdsorptionAreaA = totalAdsorptionAreaA;
	m_totalAdsorptionAreaB = totalAdsorptionAreaB;

	// cross section area in [m2] - this is the net area not including embedded objects
	// this area is needed to compute the heat flow [W] towards the zone
	m_area = con.m_netHeatTransferArea;

	// resize storage vectors for divergences, sources, and initialize boundary conditions
	m_ydot.resize(m_statesModel->m_n);
	m_results.resize(NUM_R);

	// Initialize results
	for (unsigned int i=0; i<NUM_R; ++i)
		m_results[i] = 0;
	m_fluxDensityShortWaveRadiationA = 0;
	m_fluxDensityShortWaveRadiationB = 0;
}


void ConstructionBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// currently, we only publish boundary fluxes
	for (int i=0; i<NUM_R; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("ConstructionBalanceModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionBalanceModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("ConstructionBalanceModel::Results", i);

		resDesc.push_back(result);
	}
}


void ConstructionBalanceModel::resultValueRefs(std::vector<const double *> & res) const {
	for (const double & r : m_results)
		res.push_back(&r);
}


const double * ConstructionBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "ConstructionBalanceModel::Results";

	if (quantityName.m_name == "ydot") {
		return &m_ydot[0];
	}
	else if (KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}

	return nullptr; // not found
}


int ConstructionBalanceModel::priorityOfModelEvaluation() const {
	// we are one step above room balance model
	return AbstractStateDependency::priorityOffsetTail+4;
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
				inputRefs[InputRef_RoomBTemperature] = ref;
			}
		}
	}

	// we take solar radiation load from RoomRadiationLoadsModel and then split it up according to the distribution rules

	// We may have a zone connected on either side of the construction (or even on both).
	// So, if we have a connected zone, we create an optional input reference, just in case there is no
	// radiation loads model instantiated (when there is no window, there is no radiation summation model).
	// If there is no construction, we simply add an invalid InputReference which will get filtered out and yield a
	// nullptr as value reference.

	InputReference ref;
	if (interfaceAZoneID() != 0) {
		ref.m_id = m_con->m_interfaceA.m_zoneId;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ref.m_name.m_name = "WindowSolarRadiationFluxSum";
		ref.m_required = false; // we may not have any windows -> hence to model
	}
	inputRefs[InputRef_SideASolarRadiationFromWindowLoads] = ref;

	ref = InputReference();
	if (interfaceBZoneID() != 0) {
		ref.m_id = m_con->m_interfaceB.m_zoneId;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ref.m_name.m_name = "WindowSolarRadiationFluxSum";
		ref.m_required = false; // we may not have any windows -> hence to model
	}
	inputRefs[InputRef_SideBSolarRadiationFromWindowLoads] = ref;
}


void ConstructionBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												 const std::vector<const double *> & resultValueRefs)
{
	m_valueRefs = resultValueRefs;
}


void ConstructionBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	// this model computes:
	//
	// - ydot
	// - FluxHeatConductionA
	// - FluxHeatConductionB

	// and takes input:
	// - Zone temperatures
	// - (indirectly) y-dots from ConstructionStatesModel through temperatures and calculated heat fluxes
	//
	// see update()

	// Mind: we access some results of the ConstructionStatesModel directly, like surface temperatures and computed heat fluxes.
	//       In the case of variables that are *not* exported (internal construction heat fluxes), we simply compute the dependencies
	//       from the layer temperatures ourselves.

	if (!m_moistureBalanceEnabled) {

		// first we publish the dependencies of the boundary fluxes
		if (m_con->m_interfaceA.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
			if (m_valueRefs[InputRef_RoomATemperature] != nullptr) {
				resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionA], m_valueRefs[InputRef_RoomATemperature]));
				resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionA], &m_statesModel->m_results[ConstructionStatesModel::R_SurfaceTemperatureA]));
			}
		}
		if (m_con->m_interfaceB.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
			if (m_valueRefs[InputRef_RoomBTemperature] != nullptr) {
				resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionB], m_valueRefs[InputRef_RoomBTemperature]));
				resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionB], &m_statesModel->m_results[ConstructionStatesModel::R_SurfaceTemperatureB]));
			}
		}

		// remaining dependency pattern
		for (unsigned int i=0; i<m_statesModel->m_nElements; ++i) {
			// each ydot depends on the temperature in the cell itself
			resultInputValueReferences.push_back(std::make_pair(&m_ydot[i], m_statesModel->m_vectorValuedResults[ConstructionStatesModel::VVR_ElementTemperature].dataPtr() + i ) );
			// and on right-side element
			if (i<m_statesModel->m_nElements-1)
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[i], m_statesModel->m_vectorValuedResults[ConstructionStatesModel::VVR_ElementTemperature].dataPtr() + i+1 ) );
			// and on left-side element
			if (i > 0)
				resultInputValueReferences.push_back(std::make_pair(&m_ydot[i], m_statesModel->m_vectorValuedResults[ConstructionStatesModel::VVR_ElementTemperature].dataPtr() + i-1 ) );
		}
	}
	else {
		/// \todo hygrothermal code
	}

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
		ydot[0] = m_fluxDensityHeatConductionA + m_fluxDensityShortWaveRadiationA; // left BC fluxes
		for (unsigned int i=1; i<nElements; ++i) {
			ydot[i-1] -= qHeatCond[i];	// Mind: we _subtract_ flux
			ydot[i] = qHeatCond[i];		// Mind: we _set_ the positive flux
			// finally divide by element volume (volume = dx * 1m2)
			ydot[i-1] /= E[i-1].dx;
		}
		ydot[nElements-1] -= m_fluxDensityHeatConductionB + m_fluxDensityShortWaveRadiationB; // right BC fluxes
		ydot[nElements-1] /= E[nElements-1].dx;
	}
	return 0; // signal success
}


unsigned int ConstructionBalanceModel::interfaceAZoneID() const {
	return m_con->interfaceAZoneID();
}


unsigned int ConstructionBalanceModel::interfaceBZoneID() const {
	return m_con->interfaceBZoneID();
}


int ConstructionBalanceModel::ydot(double * ydot) {
	std::memcpy(ydot, &m_ydot[0], sizeof(double)*m_ydot.size());
	return 0; // signal success
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
			case NANDRAD::InterfaceHeatConduction::MT_Constant : {
				// transfer coefficient
				double alpha = iface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
				double Ts = sideA ? m_statesModel->m_TsA : m_statesModel->m_TsB;

				// flux density [W/m2] into left side construction
				double fluxDensity = alpha*(Tambient - Ts);

				// store results
				if (sideA) {
					m_fluxDensityHeatConductionA = fluxDensity;
					m_results[R_FluxHeatConductionA] = fluxDensity*m_area; // total flux [W]
				}
				else {
					m_fluxDensityHeatConductionB = -fluxDensity;
					m_results[R_FluxHeatConductionB] = fluxDensity*m_area; // total flux [W], mind sign convention
				}
			} break;

			case NANDRAD::InterfaceHeatConduction::NUM_MT: ;// nothing to do, just to silence compiler warning
		}
	}


	// *** outside solar radiation boundary condition

	if (iface.m_zoneId == 0 && iface.m_solarAbsorption.m_modelType != NANDRAD::InterfaceSolarAbsorption::NUM_MT) {
		// different calculation from left or right side
		if (sideA) {
			double fluxDensity = m_statesModel->m_results[NANDRAD_MODEL::ConstructionStatesModel::R_SolarRadiationFluxA];
			m_fluxDensityShortWaveRadiationA = fluxDensity; // positive from left to right (into construction)
			m_results[R_FluxShortWaveRadiationA] = fluxDensity*m_area; // total flux [W], positive into construction
		}
		else {
			double fluxDensity = m_statesModel->m_results[NANDRAD_MODEL::ConstructionStatesModel::R_SolarRadiationFluxB];
			m_fluxDensityShortWaveRadiationB = -fluxDensity; // positive from left to right (out of construction)
			m_results[R_FluxShortWaveRadiationB] = fluxDensity*m_area; // total flux [W], positive into construction
		}
	}

	// *** inside solar radiation boundary condition

	if (m_valueRefs[InputRef_SideASolarRadiationFromWindowLoads] != nullptr) {
		// we got radiation load (positive into zone and hence positive into construction surface as well)

		// retrieve total solar radiation load into the zone [W]
		double radFraction2Zone = m_statesModel->m_simPara->m_para[NANDRAD::SimulationParameter::P_RadiationLoadFractionZone].value;
		// compute fraction that is applied to surfaces directly (the rest goes to the room air balance) [W]
		double radLoad2AllSurfaces = (1-radFraction2Zone)* (*m_valueRefs[InputRef_SideASolarRadiationFromWindowLoads]);

		// \todo split the load up according to splitting rule

		// for area-weighted distribution we need to know the total area of all opaque surfaces to the zone connected at
		// side A

		IBK_ASSERT(m_totalAdsorptionAreaA != 0);
		double radLoadFraction = radLoad2AllSurfaces*m_area/m_totalAdsorptionAreaA; // in [W]
		m_results[R_FluxShortWaveRadiationA] = radLoadFraction; // this is into the construction
		m_fluxDensityShortWaveRadiationA = radLoadFraction/m_area;
	}

	if (m_valueRefs[InputRef_SideBSolarRadiationFromWindowLoads] != nullptr) {
		// we got radiation load (positive into zone and hence positive into construction surface as well)

		// retrieve total solar radiation load into the zone [W]
		double radFraction2Zone = m_statesModel->m_simPara->m_para[NANDRAD::SimulationParameter::P_RadiationLoadFractionZone].value;
		// compute fraction that is applied to surfaces directly (the rest goes to the room air balance) [W]
		double radLoad2AllSurfaces = (1-radFraction2Zone)* (*m_valueRefs[InputRef_SideBSolarRadiationFromWindowLoads]);

		// \todo split the load up according to splitting rule

		// for area-weighted distribution we need to know the total area of all opaque surfaces to the zone connected at
		// side B

		IBK_ASSERT(m_totalAdsorptionAreaB != 0);
		double radLoadFraction = radLoad2AllSurfaces*m_area/m_totalAdsorptionAreaB; // in [W]
		m_results[R_FluxShortWaveRadiationB] = radLoadFraction; // this is into the construction
		m_fluxDensityShortWaveRadiationB = -radLoadFraction/m_area; // mind sign convention, positive from left to right
	}
}



} // namespace NANDRAD_MODEL

