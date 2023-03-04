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

#include "NM_ConstructionBalanceModel.h"

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_ConstructionType.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_SimulationParameter.h>

#include "NM_ConstructionStatesModel.h"
#include "NM_IdealPipeRegisterModel.h"
#include "NM_IdealSurfaceHeatingCoolingModel.h"
#include "NM_InternalLoadsModel.h"
#include "NM_KeywordList.h"
#include "NM_ThermalNetworkBalanceModel.h"

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
	m_vectorValuedResults.resize(NUM_VVR);
	m_vectorValuedResults[VVR_ThermalLoad] = VectorValuedQuantity(con.m_constructionType->m_materialLayers.size(), 0);

	// Initialize results
	for (unsigned int i=0; i<NUM_R; ++i)
		m_results[i] = 0;
	m_fluxDensityShortWaveRadiationA = 0;
	m_fluxDensityShortWaveRadiationB = 0;
	m_fluxDensityLongWaveRadiationA = 0;
	m_fluxDensityLongWaveRadiationB = 0;

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
	// add vector valued quantities
	QuantityDescription res;
	res.m_constant = true;
	res.m_description = NANDRAD_MODEL::KeywordList::Description("ConstructionBalanceModel::VectorValuedResults", VVR_ThermalLoad);
	res.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionBalanceModel::VectorValuedResults", VVR_ThermalLoad);
	res.m_unit = NANDRAD_MODEL::KeywordList::Unit("ConstructionBalanceModel::VectorValuedResults", VVR_ThermalLoad);
	// this is a vector-valued quantity with as many elements as material layers
	// and the temperatures returned are actually mean temperatures of the individual elements of the material layer
	res.resize(m_con->m_constructionType->m_materialLayers.size());
	resDesc.push_back(res);

	if(m_statesModel->m_activeLayerIndex != NANDRAD::INVALID_ID)  {
		// add active layer heat load to quantity descriptions
		res = QuantityDescription("ActiveLayerThermalLoad", "W", "Thermal load of active layer", true);
		resDesc.push_back(res);
	}
}


const double * ConstructionBalanceModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
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
	else if (quantityName.m_name == KeywordList::Keyword("ConstructionBalanceModel::VectorValuedResults",
														VVR_ThermalLoad))
	{
		// index check
		if (quantityName.m_index == -1)
			return m_vectorValuedResults[VVR_ThermalLoad].dataPtr();

		unsigned int index = (unsigned int) quantityName.m_index;
		// index exceeds vector size
		if (index >= m_vectorValuedResults[VVR_ThermalLoad].size())
			return nullptr;
		return &m_vectorValuedResults[VVR_ThermalLoad].data()[index];
	}
	else if (quantityName.m_name == "ActiveLayerThermalLoad") {
		// no active layer
		if (m_statesModel->m_activeLayerIndex == NANDRAD::INVALID_ID)
			return nullptr;

		// get index of first disc element of the active layer
		return &m_vectorValuedResults[VVR_ThermalLoad].data()[m_statesModel->m_activeLayerIndex];
	}

	return nullptr; // not found
}


int ConstructionBalanceModel::priorityOfModelEvaluation() const {
	// we are one step above room balance model
	return AbstractStateDependency::priorityOffsetTail+4;
}


void ConstructionBalanceModel::initInputReferences(const std::vector<AbstractModel *> & models) {

	FUNCID(ConstructionBalanceModel::initInputReferences);
	// return already generated references
	m_inputRefs.resize(NUM_InputRef);

	// Input references to values of long wave radiation that is emitted by other instances and received by this one at side A.
	std::vector<InputReference>						inputRefsAbsorbedLWRadiationA;
	// Input references to values of long wave radiation that is emitted by other instances and received by this one at side B.
	std::vector<InputReference>						inputRefsAbsorbedLWRadiationB;

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
				m_inputRefs[InputRef_AmbientTemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceA.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				m_inputRefs[InputRef_RoomATemperature] = ref;
			}
		}
		// check for inside long wave radiation, Note: outside long wave radiation is retrieved directly from corresponding states model
		if (m_con->m_interfaceA.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
			// Note: for LW exchange with ambient we don't need input references, as we access loads model directly

			if (m_con->m_interfaceA.m_zoneId != 0) {
				// for internal LW exchange we need emitted LW fluxes from each connected interface
				const auto &connectedInterfacesA = m_con->m_interfaceA.m_connectedInterfaces;
				for (auto it=connectedInterfacesA.begin(); it!=connectedInterfacesA.end(); ++it) {
					InputReference ref;
					ref.m_id = it->first; // id of emitting construction instance
					ref.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
					// we need to find out if this interface is side A or B or connected construction
					bool sideA = it->second->m_isSideA;
					if (sideA)
						ref.m_name.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::VectorValuedResults", ConstructionStatesModel::VVR_EmittedLongWaveRadiationA);
					else
						ref.m_name.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::VectorValuedResults", ConstructionStatesModel::VVR_EmittedLongWaveRadiationB);
					ref.m_name.m_index = int(m_id); // id of absorbing instance (ourself)
					inputRefsAbsorbedLWRadiationA.push_back(ref);
				}

				InputReference ref;
				ref.m_id = m_con->m_id;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				ref.m_name.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::Results", ConstructionStatesModel::R_EmittedLongWaveRadiationFluxA);
				ref.m_required = true;
				m_inputRefs[InputRef_SideAEmittedLongWaveRadiation] = ref;
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
				m_inputRefs[InputRef_AmbientTemperature] = ref;
			}
			else {
				InputReference ref;
				ref.m_id = m_con->m_interfaceB.m_zoneId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
				ref.m_name.m_name = "AirTemperature";
				m_inputRefs[InputRef_RoomBTemperature] = ref;
			}
		}
		// check for inside long wave radiation, Note: outside long wave radiation is retrieved directly from corresponding states model
		if (m_con->m_interfaceB.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {

			// dependencies only for inside radiation exchange
			if (m_con->m_interfaceB.m_zoneId != 0) {
				const auto &connectedInterfacesB = m_con->m_interfaceB.m_connectedInterfaces;
				for (auto it=connectedInterfacesB.begin(); it!=connectedInterfacesB.end(); ++it) {
					InputReference ref;
					ref.m_id = it->first;  // id of emitting construction instance
					ref.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
					// we need to find out if this interface is side A or B or connected construction
					bool sideA = it->second->m_isSideA;
					if (sideA)
						ref.m_name.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::VectorValuedResults", ConstructionStatesModel::VVR_EmittedLongWaveRadiationA);
					else
						ref.m_name.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::VectorValuedResults", ConstructionStatesModel::VVR_EmittedLongWaveRadiationB);
					ref.m_name.m_index = int(m_id); // id of absorbing instance (ourself)
					inputRefsAbsorbedLWRadiationB.push_back(ref);
				}

				InputReference ref;
				ref.m_id = m_con->m_id;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				ref.m_name.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::Results", ConstructionStatesModel::R_EmittedLongWaveRadiationFluxB);
				ref.m_required = true;
				m_inputRefs[InputRef_SideBEmittedLongWaveRadiation] = ref;
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
	m_inputRefs[InputRef_SideASolarRadiationFromWindowLoads] = ref;

	ref = InputReference();
	if (interfaceBZoneID() != 0) {
		ref.m_id = m_con->m_interfaceB.m_zoneId;
		ref.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ref.m_name.m_name = "WindowSolarRadiationFluxSum";
		ref.m_required = false; // we may not have any windows -> hence to model
	}
	m_inputRefs[InputRef_SideBSolarRadiationFromWindowLoads] = ref;

	// container for optional surface heat loads
	std::vector<InputReference> surfaceHeatLoadRH;

	// *** internal loads radiation ***
	// search all models for construction models that have an interface to this zone
	for (AbstractModel * model : models) {

		// create input references for model that generate zone-specific inputs (optional)
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_MODEL) {

			InternalLoadsModel * intLoadsModel = dynamic_cast<InternalLoadsModel *>(model);
			IdealSurfaceHeatingCoolingModel * idealSurfaceHeatingCoolingModel = dynamic_cast<IdealSurfaceHeatingCoolingModel *>(model);
			IdealPipeRegisterModel * idealPipeRegisterModel = dynamic_cast<IdealPipeRegisterModel *>(model);

			if (intLoadsModel != nullptr) {
				InputReference r;

				// we have load on interface side A
				if (interfaceAZoneID() != 0 &&
					intLoadsModel->objectList().m_filterID.contains(interfaceAZoneID())) {

					// ensure that no more than one loads definition is defined
					if (m_inputRefs[InputRef_SideARadiationFromEquipmentLoads].m_referenceType
						!= NANDRAD::ModelInputReference::NUM_MRT)
						throw IBK::Exception(IBK::FormatString("Duplicate equipment load result generated by different models "
											"for zone id=%1.").arg(interfaceAZoneID()), FUNC_ID);
					// if equipment load is not referenced, all other internal loads must not be defined either
					IBK_ASSERT(m_inputRefs[InputRef_SideARadiationFromPersonLoads].m_referenceType
							   == NANDRAD::ModelInputReference::NUM_MRT);
					IBK_ASSERT(m_inputRefs[InputRef_SideARadiationFromLightingLoads].m_referenceType
							   == NANDRAD::ModelInputReference::NUM_MRT);

					r.m_id = model->id();
					r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
					r.m_name.m_index = (int)interfaceAZoneID(); // select result for us (our zone id)
					r.m_required = true;
					// equipment load
					r.m_name.m_name = "RadiantEquipmentHeatLoad";
					m_inputRefs[InputRef_SideARadiationFromEquipmentLoads] = r;
					// person load
					r.m_name.m_name = "RadiantPersonHeatLoad";
					m_inputRefs[InputRef_SideARadiationFromPersonLoads] = r;
					// lighting load
					r.m_name.m_name = "RadiantLightingHeatLoad";
					m_inputRefs[InputRef_SideARadiationFromLightingLoads] = r;
				}
				if (interfaceBZoneID() != 0 &&
					intLoadsModel->objectList().m_filterID.contains(interfaceBZoneID())) {

					// ensure that no more than one loads definition is defined
					if (m_inputRefs[InputRef_SideBRadiationFromEquipmentLoads].m_referenceType
						!= NANDRAD::ModelInputReference::NUM_MRT)
						throw IBK::Exception(IBK::FormatString("Duplicate equipment load result generated by different models "
											"for zone id=%1.").arg(interfaceBZoneID()), FUNC_ID);
					// if equipment load is not referenced, all other internal loads must not be defined either
					IBK_ASSERT(m_inputRefs[InputRef_SideBRadiationFromPersonLoads].m_referenceType
							   == NANDRAD::ModelInputReference::NUM_MRT);
					IBK_ASSERT(m_inputRefs[InputRef_SideBRadiationFromLightingLoads].m_referenceType
							   == NANDRAD::ModelInputReference::NUM_MRT);

					// fill references
					r.m_id = model->id();
					r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
					r.m_name.m_index = (int)interfaceBZoneID(); // select result for us (our zone id)
					r.m_required = true;
					// equipment load
					r.m_name.m_name = "RadiantEquipmentHeatLoad";
					m_inputRefs[InputRef_SideBRadiationFromEquipmentLoads] = r;
					// person load
					r.m_name.m_name = "RadiantPersonHeatLoad";
					m_inputRefs[InputRef_SideBRadiationFromPersonLoads] = r;
					// lighting load
					r.m_name.m_name = "RadiantLightingHeatLoad";
					m_inputRefs[InputRef_SideBRadiationFromLightingLoads] = r;
				}
			}
			// create input references for heat fluxes from ideal surface heatings
			else if (idealSurfaceHeatingCoolingModel != nullptr) {
				++m_surfaceHeatingCoolingModelCount;
				InputReference r;
				r.m_name.m_name = "ActiveLayerThermalLoad";
				// add current id as index so that we can sum uphat fluxes from all hetaing models
				r.m_name.m_index = (int) id();
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
				// this reference is only provided if the corresponding network with element
				// heat flux into current construction
				r.m_required = false;
				surfaceHeatLoadRH.push_back(r);
			}
			// create input references for heat fluxes from ideal pipe register model
			else if (idealPipeRegisterModel != nullptr) {
				++m_surfaceHeatingCoolingModelCount;
				InputReference r;
				r.m_name.m_name = "ActiveLayerThermalLoad";
				// add current id as index so that we can sum uphat fluxes from all hetaing models
				r.m_name.m_index = (int) id();
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
				// this reference is only provided if the corresponding network with element
				// heat flux into current construction
				r.m_required = false;
				surfaceHeatLoadRH.push_back(r);
			}
		}
		// create input references for heat fluxes out of hydraulic networks
		else if (model->referenceType() == NANDRAD::ModelInputReference::MRT_NETWORK) {
			ThermalNetworkBalanceModel * thermNetworkModel = dynamic_cast<ThermalNetworkBalanceModel *>(model);
			if (thermNetworkModel != nullptr) {
				++m_surfaceHeatingCoolingModelCount;
				InputReference r;
				r.m_name.m_name = "ActiveLayerThermalLoad";
				// add current id as index so that we can sum uphat fluxes from all networks
				r.m_name.m_index = (int) id();
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORK;
				// this reference is only provided if the corresponding network with element
				// heat flux into current construction
				r.m_required = false;
				surfaceHeatLoadRH.push_back(r);
			}
		}
	} // model object loop

	// insert references for absorbed long wave radiation
	m_inputRefs.insert(m_inputRefs.end(), inputRefsAbsorbedLWRadiationA.begin(), inputRefsAbsorbedLWRadiationA.end());
	m_inputRefs.insert(m_inputRefs.end(), inputRefsAbsorbedLWRadiationB.begin(), inputRefsAbsorbedLWRadiationB.end());
	m_absorbedLWRadiationACount = inputRefsAbsorbedLWRadiationA.size();
	m_absorbedLWRadiationBCount = inputRefsAbsorbedLWRadiationB.size();

	// insert references to all surface heating load mdoels
	m_inputRefs.insert(m_inputRefs.end(), surfaceHeatLoadRH.begin(), surfaceHeatLoadRH.end());
}


void ConstructionBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void ConstructionBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												 const std::vector<const double *> & resultValueRefs)
{
	FUNCID(ConstructionBalanceModel::setInputValueRefs);

	// resize value references
	m_valueRefs.resize(NUM_InputRef, nullptr);

	// copy required values
	for(unsigned int i = 0; i < NUM_InputRef; ++i)
		m_valueRefs[i] = resultValueRefs[i];

	// copy optional values for inside long wave radiation, side A and B
	unsigned int lastIdx = NUM_InputRef;
	for (unsigned int i = lastIdx; i < lastIdx + m_absorbedLWRadiationACount; ++i)
		m_valueRefsAbsorbedLWRadiationA.push_back( resultValueRefs[i] );
	lastIdx += m_absorbedLWRadiationACount;

	for (unsigned int i = lastIdx; i < lastIdx + m_absorbedLWRadiationBCount; ++i)
		m_valueRefsAbsorbedLWRadiationB.push_back( resultValueRefs[i] );
	lastIdx += m_absorbedLWRadiationBCount;

	// copy optional values for active layer
	if (m_statesModel->m_activeLayerIndex != NANDRAD::INVALID_ID) {
		// check all surface heating loads
		for (unsigned int i= lastIdx; i < lastIdx + m_surfaceHeatingCoolingModelCount; ++i) {
			// check that only one active layer is references
			if (resultValueRefs[i] != nullptr) {
				if (m_valueRefs[InputRef_ActiveLayerHeatLoads] != nullptr)
					throw IBK::Exception(IBK::FormatString("Active layer is referenced twice from a network component or surface heating component "
														   "for construction instance id=%1.").arg(m_id), FUNC_ID);
				// copy pointer
				m_valueRefs[InputRef_ActiveLayerHeatLoads] = resultValueRefs[i];
			}
		}
	}
}


void ConstructionBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// resultInputValueReferences = pair(target variable, source variable)

	// Note: The construction balance model is explicitely evaluated AFTER the model graph has been processed (see
	//       NandradModel::m_constructionBalanceModelContainer and its use in NandradModel::updateStateDependentModels()
	//       So, for the purpose of evaluation order, we would not have to define resultInputValueReferences.
	//       However, for setting up the Jacobian matrix correctly, we need to know which variable depends on which other variables
	//       in order to fill the respective positions in

	// Mind: we access some results of the ConstructionStatesModel directly, like surface temperatures and computed heat fluxes.
	//       In the case of variables that are *not* exported (internal construction heat fluxes), we simply compute the dependencies
	//       from the layer temperatures ourselves.

	// ydot  (depend on element temperatures), boundary ydot also from boundary fluxes (added below)
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

	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal code
	}



	// dependencies of computed fluxes

	// R_FluxHeatConductionA
	if (m_con->m_interfaceA.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
		if (m_valueRefs[InputRef_RoomATemperature] != nullptr) {
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionA], m_valueRefs[InputRef_RoomATemperature]));
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionA], &m_statesModel->m_results[ConstructionStatesModel::R_SurfaceTemperatureA]));
			// ydot of first element depends on boundary flux
			resultInputValueReferences.push_back(std::make_pair(&m_ydot[0], &m_results[R_FluxHeatConductionA] ) );
		}
	}
	// R_FluxHeatConductionB
	if (m_con->m_interfaceB.m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
		if (m_valueRefs[InputRef_RoomBTemperature] != nullptr) {
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionB], m_valueRefs[InputRef_RoomBTemperature]));
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxHeatConductionB], &m_statesModel->m_results[ConstructionStatesModel::R_SurfaceTemperatureB]));
			// ydot of last element depends on boundary flux
			resultInputValueReferences.push_back(std::make_pair(&m_ydot[m_statesModel->m_nElements-1], &m_results[R_FluxHeatConductionB] ) );
		}
	}

	// R_FluxLongWaveRadiationA
	if (m_con->m_interfaceA.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxLongWaveRadiationA], &m_statesModel->m_results[ConstructionStatesModel::R_SurfaceTemperatureA]));
		// ydot of first element depends on boundary flux
		resultInputValueReferences.push_back(std::make_pair(&m_ydot[0], &m_results[R_FluxLongWaveRadiationA] ) );
	}
	// R_FluxLongWaveRadiationB
	if (m_con->m_interfaceB.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxLongWaveRadiationB], &m_statesModel->m_results[ConstructionStatesModel::R_SurfaceTemperatureB]));
		// ydot of first element depends on boundary flux
		resultInputValueReferences.push_back(std::make_pair(&m_ydot[m_statesModel->m_nElements-1], &m_results[R_FluxLongWaveRadiationB] ) );
	}

	// R_FluxShortWaveRadiationA and R_FluxShortWaveRadiationB depend on Loads -> not a state dependency, so we do not need this
	// but these fluxes also depend on internal load models

	// R_FluxShortWaveRadiationA
	bool haveSideAShortWaveRadiationFlux = false;
	if (m_valueRefs[InputRef_SideARadiationFromEquipmentLoads] != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxShortWaveRadiationA], m_valueRefs[InputRef_SideARadiationFromEquipmentLoads]));
		haveSideAShortWaveRadiationFlux = true;
	}
	if (m_valueRefs[InputRef_SideARadiationFromPersonLoads] != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxShortWaveRadiationA], m_valueRefs[InputRef_SideARadiationFromPersonLoads]));
		haveSideAShortWaveRadiationFlux = true;
	}
	if (m_valueRefs[InputRef_SideARadiationFromLightingLoads] != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxShortWaveRadiationA], m_valueRefs[InputRef_SideARadiationFromLightingLoads]));
		haveSideAShortWaveRadiationFlux = true;
	}
	// ydot of side A element depends on short wave radiation flux, if computed
	if (haveSideAShortWaveRadiationFlux)
		resultInputValueReferences.push_back(std::make_pair(&m_ydot[0], &m_results[R_FluxShortWaveRadiationA] ) );

	// R_FluxShortWaveRadiationB
	bool haveSideBShortWaveRadiationFlux = false;
	if (m_valueRefs[InputRef_SideBRadiationFromEquipmentLoads] != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxShortWaveRadiationB], m_valueRefs[InputRef_SideBRadiationFromEquipmentLoads]));
		haveSideBShortWaveRadiationFlux = true;
	}
	if (m_valueRefs[InputRef_SideBRadiationFromPersonLoads] != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxShortWaveRadiationB], m_valueRefs[InputRef_SideBRadiationFromPersonLoads]));
		haveSideBShortWaveRadiationFlux = true;
	}
	if (m_valueRefs[InputRef_SideBRadiationFromLightingLoads] != nullptr) {
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_FluxShortWaveRadiationB], m_valueRefs[InputRef_SideBRadiationFromLightingLoads]));
		haveSideBShortWaveRadiationFlux = true;
	}
	// ydot of side A element depends on short wave radiation flux, if computed
	if (haveSideBShortWaveRadiationFlux)
		resultInputValueReferences.push_back(std::make_pair(&m_ydot[m_statesModel->m_nElements-1], &m_results[R_FluxShortWaveRadiationB] ) );


	// add active layer heat source dependencies
	if (m_valueRefs[InputRef_ActiveLayerHeatLoads] != nullptr ) {

		IBK_ASSERT(m_statesModel->m_activeLayerIndex != NANDRAD::INVALID_ID);
		// loop through all elements of active layer
		unsigned int elemIdxStart = m_statesModel->m_materialLayerElementOffset[m_statesModel->m_activeLayerIndex];
		unsigned int elemIdxEnd = m_statesModel->m_materialLayerElementOffset[m_statesModel->m_activeLayerIndex + 1];

		for (unsigned int i = elemIdxStart; i < elemIdxEnd; ++i)
			resultInputValueReferences.push_back(std::make_pair(&m_ydot[i], m_valueRefs[InputRef_ActiveLayerHeatLoads]) );
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
		ydot[0] = m_fluxDensityHeatConductionA + m_fluxDensityShortWaveRadiationA + m_fluxDensityLongWaveRadiationA; // left BC fluxes
		for (unsigned int i=1; i<nElements; ++i) {
			ydot[i-1] -= qHeatCond[i];	// Mind: we _subtract_ flux
			ydot[i] = qHeatCond[i];		// Mind: we _set_ the positive flux
			// finally divide by element volume (volume = dx * 1m2)
			ydot[i-1] /= E[i-1].dx;
		}
		ydot[nElements-1] -= m_fluxDensityHeatConductionB + m_fluxDensityShortWaveRadiationB + m_fluxDensityLongWaveRadiationB; // right BC fluxes
		ydot[nElements-1] /= E[nElements-1].dx;

		// add active layer heat sources
		if(m_valueRefs[InputRef_ActiveLayerHeatLoads] != nullptr ) {
			// store thermal load
			double layerLoad = *m_valueRefs[InputRef_ActiveLayerHeatLoads];
			m_vectorValuedResults[VVR_ThermalLoad].dataPtr()[m_statesModel->m_activeLayerIndex] = layerLoad;

			IBK_ASSERT(m_statesModel->m_activeLayerIndex != NANDRAD::INVALID_ID);
			// loop through all elements of active layer
			unsigned int elemIdxStart = m_statesModel->m_materialLayerElementOffset[m_statesModel->m_activeLayerIndex];
			unsigned int elemIdxEnd = m_statesModel->m_materialLayerElementOffset[m_statesModel->m_activeLayerIndex + 1];

			// calculate flux density [W/m3]
			double layerLoadDensity = layerLoad/m_statesModel->m_activeLayerVolume;

			for (unsigned int i = elemIdxStart; i < elemIdxEnd; ++i)
				ydot[i] += layerLoadDensity;
		}
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
					m_results[R_FluxHeatConductionAreaSpecificA] = fluxDensity; // area specific flux [W]
					m_results[R_FluxHeatConductionA] = fluxDensity*m_area;		// total flux [W]
				}
				else {
					m_fluxDensityHeatConductionB = -fluxDensity;
					m_results[R_FluxHeatConductionAreaSpecificB] = fluxDensity; // area specific flux [W], mind sign convention
					m_results[R_FluxHeatConductionB] = fluxDensity*m_area;		// total flux [W], mind sign convention
				}
			} break;

			case NANDRAD::InterfaceHeatConduction::NUM_MT: ;// nothing to do, just to silence compiler warning
		}
	}

	// set radiation fluxes to 0 and later add optional radiant fluxes
	if (sideA) {
		m_fluxDensityShortWaveRadiationA = 0.0;
		m_fluxDensityLongWaveRadiationA = 0.0;
		m_results[R_FluxShortWaveRadiationA] = 0.0;
		m_results[R_FluxLongWaveRadiationA] = 0.0;
	}
	else {
		m_fluxDensityLongWaveRadiationB = 0.0;
		m_fluxDensityShortWaveRadiationB = 0.0;
		m_results[R_FluxShortWaveRadiationB] = 0.0;
		m_results[R_FluxLongWaveRadiationB] = 0.0;
	}

	// *** outside solar radiation boundary condition

	if (iface.m_zoneId == 0 && iface.m_solarAbsorption.m_modelType != NANDRAD::InterfaceSolarAbsorption::NUM_MT) {
		// different calculation from left or right side
		if (sideA) {
			double fluxDensity = m_statesModel->m_results[NANDRAD_MODEL::ConstructionStatesModel::R_SolarRadiationFluxA];
			m_fluxDensityShortWaveRadiationA += fluxDensity; // positive from left to right (into construction)
			m_results[R_FluxShortWaveRadiationA] += fluxDensity*m_area; // total flux [W], positive into construction
		}
		else {
			double fluxDensity = m_statesModel->m_results[NANDRAD_MODEL::ConstructionStatesModel::R_SolarRadiationFluxB];
			m_fluxDensityShortWaveRadiationB -= fluxDensity; // positive from left to right (out of construction)
			m_results[R_FluxShortWaveRadiationB] += fluxDensity*m_area; // total flux [W], positive into construction
		}
	}


	// *** outside long wave radiation boundary condition

	if (iface.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
		if (iface.m_zoneId == 0) { // ambient
			// different calculation from left or right side
			if (sideA) {
				double fluxDensity = m_statesModel->m_results[NANDRAD_MODEL::ConstructionStatesModel::R_LongWaveRadiationFluxA];
				m_fluxDensityLongWaveRadiationA += fluxDensity; // positive from left to right (into construction)
			}
			else {
				double fluxDensity = m_statesModel->m_results[NANDRAD_MODEL::ConstructionStatesModel::R_LongWaveRadiationFluxB];
				m_fluxDensityLongWaveRadiationB -= fluxDensity; // positive from left to right (into construction)
			}
		}
		else {
			// inside LW rad exchange

			if (sideA) {
				// subtract emitted LW rad (this is in W/m2)
				m_fluxDensityLongWaveRadiationA = - *m_valueRefs[InputRef_SideAEmittedLongWaveRadiation];
				// add all received LW rad (these are in W, we need to devide by OUR area)
				for (unsigned int i=0; i<m_valueRefsAbsorbedLWRadiationA.size(); ++i)
					m_fluxDensityLongWaveRadiationA += *m_valueRefsAbsorbedLWRadiationA[i] / m_area;
			}
			else {
				// same as for side A, but with inverse sign
				m_fluxDensityLongWaveRadiationB += *m_valueRefs[InputRef_SideBEmittedLongWaveRadiation];
				for (unsigned int i=0; i<m_valueRefsAbsorbedLWRadiationB.size(); ++i)
					m_fluxDensityLongWaveRadiationB -= *m_valueRefsAbsorbedLWRadiationB[i] / m_area;
			}
		}
	}

	// compute output variables
	m_results[R_FluxLongWaveRadiationA] = m_fluxDensityLongWaveRadiationA * m_area; // total flux [W], positive into construction
	m_results[R_FluxLongWaveRadiationB] = -m_fluxDensityLongWaveRadiationB * m_area; // total flux [W], positive into construction

	// *** inside solar radiation boundary condition

	// short wave radiation through windows, onto side A
	if (sideA && m_valueRefs[InputRef_SideASolarRadiationFromWindowLoads] != nullptr) {
		// we got radiation load (positive into zone and hence positive into construction surface as well)

		// retrieve total solar radiation load into the zone [W]
		double radFraction2Zone = m_statesModel->m_simPara->m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone].value;
		// compute fraction that is applied to surfaces directly (the rest goes to the room air balance) [W]
		double radLoad2AllSurfaces = (1-radFraction2Zone)* (*m_valueRefs[InputRef_SideASolarRadiationFromWindowLoads]);

		// \todo split the load up according to splitting rule

		// for area-weighted distribution we need to know the total area of all opaque surfaces to the zone connected at
		// side A

		IBK_ASSERT(m_totalAdsorptionAreaA != 0.0);
		double radLoadFraction = radLoad2AllSurfaces*m_area/m_totalAdsorptionAreaA; // in [W]
		m_results[R_FluxShortWaveRadiationA] += radLoadFraction; // this is into the construction
		m_fluxDensityShortWaveRadiationA += radLoadFraction/m_area;
	}

	// short wave radiation through windows, onto side B
	if (!sideA && m_valueRefs[InputRef_SideBSolarRadiationFromWindowLoads] != nullptr) {
		// we got radiation load (positive into zone and hence positive into construction surface as well)

		// retrieve total solar radiation load into the zone [W]
		double radFraction2Zone = m_statesModel->m_simPara->m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone].value;
		// compute fraction that is applied to surfaces directly (the rest goes to the room air balance) [W]
		double radLoad2AllSurfaces = (1-radFraction2Zone)* (*m_valueRefs[InputRef_SideBSolarRadiationFromWindowLoads]);

		// \todo split the load up according to splitting rule

		// for area-weighted distribution we need to know the total area of all opaque surfaces to the zone connected at
		// side B

		IBK_ASSERT(m_totalAdsorptionAreaB != 0.0);
		double radLoadFraction = radLoad2AllSurfaces*m_area/m_totalAdsorptionAreaB; // in [W]
		m_results[R_FluxShortWaveRadiationB] += radLoadFraction; // this is into the construction
		m_fluxDensityShortWaveRadiationB -= radLoadFraction/m_area;
	}



	// *** internal loads radiation boundary condition

	if (sideA && m_valueRefs[InputRef_SideARadiationFromEquipmentLoads] != nullptr) {
		// retrieve total solar radiation load into the zone [W]
		double internalRadiation = *m_valueRefs[InputRef_SideARadiationFromEquipmentLoads];

		// internal loads always include radiant person load into the zone [W]
		IBK_ASSERT(m_valueRefs[InputRef_SideARadiationFromPersonLoads] != nullptr);
		internalRadiation += *m_valueRefs[InputRef_SideARadiationFromPersonLoads];

		// internal loads always include radiant lighting load into the zone [W]
		IBK_ASSERT(m_valueRefs[InputRef_SideARadiationFromLightingLoads] != nullptr);
		internalRadiation += *m_valueRefs[InputRef_SideARadiationFromLightingLoads];

		// for area-weighted distribution we need to know the total area of all opaque surfaces to the zone connected at
		// side A
		IBK_ASSERT(m_totalAdsorptionAreaA != 0.0);
		double radLoadFraction = internalRadiation*m_area/m_totalAdsorptionAreaA; // in [W]
		m_results[R_FluxShortWaveRadiationA] += radLoadFraction; // this is into the construction
		m_fluxDensityShortWaveRadiationA += radLoadFraction/m_area;
	}

	if (!sideA && m_valueRefs[InputRef_SideBRadiationFromEquipmentLoads] != nullptr) {
		// retrieve total solar radiation load into the zone [W]
		double internalRadiation = *m_valueRefs[InputRef_SideBRadiationFromEquipmentLoads];

		// internal loads always include radiant person load into the zone [W]
		IBK_ASSERT(m_valueRefs[InputRef_SideBRadiationFromPersonLoads] != nullptr);
		internalRadiation += *m_valueRefs[InputRef_SideBRadiationFromPersonLoads];

		// internal loads always include radiant lighting load into the zone [W]
		IBK_ASSERT(m_valueRefs[InputRef_SideBRadiationFromLightingLoads] != nullptr);
		internalRadiation += *m_valueRefs[InputRef_SideBRadiationFromLightingLoads];

		// for area-weighted distribution we need to know the total area of all opaque surfaces to the zone connected at
		// side A
		IBK_ASSERT(m_totalAdsorptionAreaB != 0.0);
		double radLoadFraction = internalRadiation*m_area/m_totalAdsorptionAreaB; // in [W]
		m_results[R_FluxShortWaveRadiationB] += radLoadFraction; // this is into the construction
		m_fluxDensityShortWaveRadiationB -= radLoadFraction/m_area;
	}
}



} // namespace NANDRAD_MODEL

