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

#include "NM_ConstructionStatesModel.h"

#include <algorithm>

#include <IBK_messages.h>
#include <IBK_physics.h>

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_ConstructionType.h>
#include <NANDRAD_SolverParameter.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Material.h>

#include "NM_Loads.h"

#include "NM_KeywordList.h"

#include <CCM_Defines.h>

#define CONSTANT_EXTRAPOLATION

namespace NANDRAD_MODEL {


/*! A grid generation utility class.
	Written by Andreas Nicolai for THERAKLES from IBK/TU Dresden.
*/
class Mesh {
public:
	/*! Defines mesh type. */
	enum MeshType {
		Uniform,
		TanHSingle,
		TanHDouble
	};

	/*! Constructor, used to define a mesh type and its properties. */
	Mesh(const MeshType type, const double stretch = 1, const double ratio=1);

	/*! Populates the vector ds_vec with normalized element width based on the current
		mesh settings. The sum in vector ds_vec will be always 1. If a single-sided
		grid is used, element 0 of the vector will have the smallest size. */
	void generate(unsigned int n, std::vector<double> & ds_vec);

	/*! Generates a grid between coordinates x1 and x2 and stores the new element widths
		in vector dx_vec, and the element's center coordinates in vector x_vec. */
	void generate(const unsigned int n, const double x1, const double x2,
		std::vector<double> & dx_vec, std::vector<double> & x_vec);

	/*! Type of mesh.
		\sa MeshType. */
	MeshType t;

	/*! Mesh stretch factor/density (larger values - coarser grid). */
	double d;

	/*! Ratio between boundary element widths dx_0 / dx_{n-1}. */
	double r;
};


void ConstructionStatesModel::setup(const NANDRAD::ConstructionInstance & con,
									const NANDRAD::SimulationParameter & simPara,
									const NANDRAD::SolverParameter & solverPara,
									Loads & loads)
{
	FUNCID(ConstructionStatesModel::setup);

	// cache pointers to input data structures
	m_con = &con;
	m_simPara = &simPara;
	m_solverPara = &solverPara;
	m_loads = &loads;

	m_moistureBalanceEnabled = m_simPara->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled();

	// we initialize only those variables needed for state calculation:
	// - grid and grid-element to layer association
	// - precalculate variables needed for decomposition and flux calculation
	// - initial conditions/states

	// *** grid generation

	generateGrid();
	IBK::IBK_Message(IBK::FormatString("Construction is discretized with %1 elements.\n")
					 .arg(m_elements.size()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);


	// BC sanity checking has already been done during construction setup, so we can rely on valid/existing
	// parameters here

	bool haveRadiationBCA = m_con->m_interfaceA.m_zoneId == 0 && ( m_con->m_interfaceA.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT ||
			m_con->m_interfaceA.m_solarAbsorption.m_modelType != NANDRAD::InterfaceSolarAbsorption::NUM_MT );
	bool haveRadiationBCB = m_con->m_interfaceB.m_zoneId == 0 && ( m_con->m_interfaceB.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT ||
			m_con->m_interfaceB.m_solarAbsorption.m_modelType != NANDRAD::InterfaceSolarAbsorption::NUM_MT );
	// if we have radiation data, register surface here
	if (haveRadiationBCA || haveRadiationBCB)
		loads.addSurface(m_con->m_id,
				m_con->m_para[NANDRAD::ConstructionInstance::P_Orientation].value/DEG2RAD,
				m_con->m_para[NANDRAD::ConstructionInstance::P_Inclination].value/DEG2RAD);


	// store active layer
	m_activeLayerIndex = m_con->m_constructionType->m_activeLayerIndex;
	// calculate active layer volume
	if (m_activeLayerIndex != NANDRAD::INVALID_ID)
		m_activeLayerVolume = con.m_netHeatTransferArea * con.m_constructionType->m_materialLayers[m_activeLayerIndex].m_thickness;

	// *** storage member initialization

	m_y.resize(nPrimaryStateResults());

	m_rhoce.resize(m_nElements);

	m_fluxes_q.resize(m_nElements+1);
	m_rTInv.resize(m_nElements+1);
	// precalculate capacities and resistances
	for (unsigned int i=0; i<m_nElements; ++i) {
		const Element & E = m_elements[i];
		// dry bulk density [kg/m3]
		double rho = E.mat->m_para[NANDRAD::Material::P_Density].value;
		// specific heat capacity [J/kgK]
		double ce = E.mat->m_para[NANDRAD::Material::P_HeatCapacity].value;
		m_rhoce[i] = rho*ce;
	}
	// cache thermal resistances/U-values
	for (unsigned int i=1; i<m_nElements; ++i) {
		// thermal conductivity for dry material in [W/mK] - left
		const Element & E_left = m_elements[i-1];
		const Element & E_right = m_elements[i];
		double lambda_left = E_left.mat->m_para[NANDRAD::Material::P_Conductivity].value;
		double lambda_right = E_right.mat->m_para[NANDRAD::Material::P_Conductivity].value;
		// harmonic average of thermal conductivities is the inverse of the thermal resistance between
		// the centers of the elements
		double lambda_mean = 2/(E_left.dx / lambda_left
									 + E_right.dx / lambda_right); // 1/ (m / W/mK) = 1/m2K/W  = W/m2K -> U-value
		m_rTInv[i] = lambda_mean;
	}
	// Note: m_rTInv[0] and m_rTInv[m_nElements] are not used and remain uninitialized.


	// *** now resize the memory cache for results

	unsigned int skalarResultCount = R_LongWaveRadiationFluxB+1;
	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal code
	}
	m_results.resize(skalarResultCount);
	m_vectorValuedResults.resize(1);
	m_vectorValuedResults[VVR_ElementTemperature] = VectorValuedQuantity(nPrimaryStateResults(), 0);

	for (auto it=m_con->m_interfaceA.m_connectedInterfaces.begin(); it!=m_con->m_interfaceA.m_connectedInterfaces.end(); ++it)
		m_emittedLongWaveRadiationA[it->first] = 0;
	for (auto it=m_con->m_interfaceB.m_connectedInterfaces.begin(); it!=m_con->m_interfaceB.m_connectedInterfaces.end(); ++it)
		m_emittedLongWaveRadiationB[it->first] = 0;
}


void ConstructionStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	int skalarResultCount = R_LongWaveRadiationFluxB+1;
	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal implementation
//		varCount = 2; // more variables for hygrothermal calculation
	}

	for (int i=0; i<skalarResultCount; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("ConstructionStatesModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("ConstructionStatesModel::Results", i);

		resDesc.push_back(result);
	}

	// add vector valued quantities
	QuantityDescription res;
	res.m_constant = true;
	res.m_description = NANDRAD_MODEL::KeywordList::Description("ConstructionStatesModel::VectorValuedResults", VVR_ElementTemperature);
	res.m_name = NANDRAD_MODEL::KeywordList::Keyword("ConstructionStatesModel::VectorValuedResults", VVR_ElementTemperature);
	res.m_unit = NANDRAD_MODEL::KeywordList::Unit("ConstructionStatesModel::VectorValuedResults", VVR_ElementTemperature);
	// this is a vector-valued quantity with as many elements as material layers
	// and the temperatures returned are actually mean temperatures of the individual elements of the material layer
	res.resize(m_con->m_constructionType->m_materialLayers.size());
	resDesc.push_back(res);

	// in the case of an actiev construction add active temparture layer
	if (m_con->m_constructionType->m_activeLayerIndex != NANDRAD::INVALID_ID) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = "Temperature of the active material layer";
		result.m_name = "ActiveLayerTemperature";
		result.m_unit = "C";
		resDesc.push_back(result);
	}

	// emmited inner long wave radiation of surface A
	if (m_con->m_interfaceA.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT &&
		m_con->m_interfaceA.m_zoneId != 0) {
		res.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		res.m_id = m_id; // id of emmitting instance (ourself)
		res.m_constant = true;
		res.m_name = "EmittedLongWaveRadiation";
		res.m_unit = "W";
		const std::map<unsigned int, const NANDRAD::Interface*>	&connectedInterfacesA = m_con->m_interfaceA.m_connectedInterfaces;
		for (auto it = connectedInterfacesA.begin(); it!=connectedInterfacesA.end(); ++it)
			res.m_indexKeys.push_back(it->first); // id of targeted instance (the absorbing construction instance)
		res.m_indexKeyType = VectorValuedQuantityIndex::IK_ModelID;
		resDesc.push_back(res);
	}

	// emmited long wave radiation of surface B
	if (m_con->m_interfaceB.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT &&
		m_con->m_interfaceB.m_zoneId != 0) {
		res.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		res.m_id = m_id; // id of emmitting instance (ourself)
		res.m_constant = true;
		res.m_name = "EmittedLongWaveRadiation";
		res.m_unit = "W";
		const std::map<unsigned int, const NANDRAD::Interface*>	&connectedInterfacesB = m_con->m_interfaceB.m_connectedInterfaces;
		for (auto it = connectedInterfacesB.begin(); it!=connectedInterfacesB.end(); ++it)
			res.m_indexKeys.push_back(it->first); // id of targeted instance (the absorbing construction instance)
		res.m_indexKeyType = VectorValuedQuantityIndex::IK_ModelID;
		resDesc.push_back(res);
	}
}


const double * ConstructionStatesModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "ConstructionStatesModel::Results";

	if (quantityName.m_name == "y") {
		return &m_y[0];
	}
	else if (quantityName.m_name == "ActiveLayerTemperature") {
		// no active layer
		if (m_activeLayerIndex == NANDRAD::INVALID_ID)
			return nullptr;
		return &m_activeLayerMeanTemperature;
	}
	else if (KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}

	// search for vector-valued results and also check for valid indexes
	// a vector valued result
	const char * const categoryVectors = "ConstructionStatesModel::VectorValuedResults";

	if (KeywordList::KeywordExists(categoryVectors, quantityName.m_name)) {
		unsigned int resIdx = (unsigned int)KeywordList::Enumeration(categoryVectors, quantityName.m_name);
		const VectorValuedQuantity & vecResults  = m_vectorValuedResults[resIdx]; // reading improvement
		// no index is given (requesting entire vector?)
		if (quantityName.m_index == -1) {
			// return access to the first vector element
			return &vecResults.data()[0];
		}
		// index definition
		else {
			return &vecResults[(size_t)quantityName.m_index];
		}
		// Note: function may throw an IBK::Exception if the requested index is out of range
	}

	// emitted long wave radiation to construction instance with given target id
	if (quantityName.m_name == "EmittedLongWaveRadiation") {
		if ( m_emittedLongWaveRadiationA.find((unsigned int)quantityName.m_index) != m_emittedLongWaveRadiationA.end() )
			return &m_emittedLongWaveRadiationA.at((unsigned int)quantity.m_name.m_index);
		else if ( m_emittedLongWaveRadiationB.find((unsigned int)quantity.m_name.m_index) != m_emittedLongWaveRadiationB.end() )
			return &m_emittedLongWaveRadiationB.at((unsigned int)quantity.m_name.m_index);
		else
			IBK_ASSERT(false); // index not found: programming error
	}

	return nullptr; // quantity not found
}


unsigned int ConstructionStatesModel::nPrimaryStateResults() const {
	if (!m_moistureBalanceEnabled) {
		// *** thermal transport ***
		return m_nElements;
	}
	// *** hygrothermal transport ***
	else {
		return m_nElements*2;
	}
}


void ConstructionStatesModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal
	}
	else {
		// we add dependencies of all scalar vars and all vector valued vars to the states vector m_y

		// first scalar quantities

		// surface temperatures depend on states in their elements
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_SurfaceTemperatureA], &m_y[0]) );
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_SurfaceTemperatureB], &m_y[m_nElements-1]) );
#ifndef CONSTANT_EXTRAPOLATION
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_SurfaceTemperatureA], &m_y[1]) );
		resultInputValueReferences.push_back(std::make_pair(&m_results[R_SurfaceTemperatureB], &m_y[m_nElements-2]) );
#endif

		// now vector-valued quantities
		// temperatures in all elements depend on their respective energy densities
		for (unsigned int i=0; i<m_nElements; ++i)
			resultInputValueReferences.push_back(std::make_pair(&m_vectorValuedResults[VVR_ElementTemperature].data()[i], &m_y[i]) );

		// mean active layer temperature depends on all discretization element temperatures of this active layer
		if (m_activeLayerIndex != NANDRAD::INVALID_ID) {

			// loop through all elements of active layer
			unsigned int elemIdxStart = m_materialLayerElementOffset[m_activeLayerIndex];
			unsigned int elemIdxEnd = m_materialLayerElementOffset[m_activeLayerIndex + 1];

			for (unsigned int i = elemIdxStart; i < elemIdxEnd; ++i)
				resultInputValueReferences.push_back(std::make_pair(&m_activeLayerMeanTemperature, &m_vectorValuedResults[VVR_ElementTemperature].data()[i] ) );
		}
	}
}


unsigned int ConstructionStatesModel::interfaceAZoneID() const {
	return m_con->interfaceAZoneID();
}


unsigned int ConstructionStatesModel::interfaceBZoneID() const {
	return m_con->interfaceBZoneID();
}


void ConstructionStatesModel::
yInitial(double * y) const {
	// retrieve initial temperature, which has already been checked for valid values
	double T_initial = m_simPara->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].value;
	for (unsigned i=0; i<m_nElements; ++i) {
		// energy density
		y[i] = m_rhoce[i]*T_initial;
	}
}


// helper define to get raw pointers from vector storage members
#define DOUBLE_PTR(x) &x[0]

int ConstructionStatesModel::update(const double * y) {
	// here we compute all temperatures from conserved quantities (i.e. energy densities) and
	// also compute all thermal fluxes across elements

	if (!m_moistureBalanceEnabled) {

		/// \todo switch between different loop kernels when PCM materials are in the construction

		// decomposition algorithm for thermal balances only
		// this is a speeded up version for thermal-only calculations
		// does decomposition and internal flux calculation in one

		double * states_u = DOUBLE_PTR(m_y); // in thermal calculation, m_y holds all energy densities [J/m3]
		double * states_T = m_vectorValuedResults[VVR_ElementTemperature].dataPtr();

		double * vec_q = DOUBLE_PTR(m_fluxes_q);

		double * rhoce = DOUBLE_PTR(m_rhoce);
		double * rT_inv = DOUBLE_PTR(m_rTInv);

		double u, T, T_last;

		*states_u = u = *y;
		// temperature in [K]
		*states_T = T_last = u / (*rhoce);

		double * states_uLast = states_u + m_nElements;

		// fast loop kernel for constructions without PCM materials
		// this loop kernel is much much faster (fits into CPU cache) than a more
		// general case with if-clauses for each element as in the defined-out block below
		while(++states_u != states_uLast) {
			*states_u = u = *(++y);

			// temperature in [K]
			*(++states_T) = T = u / (*(++rhoce));

			// compute heat conduction flux across element centers
			*(++vec_q) = *(++rT_inv) * (T_last-T);

			// update last element's values
			T_last = T;
		}
	}
	// *** hygrothermal transport ***
	else {
		/// \todo hygrothermal
	}

	// compute surface temperatures

	double * states_T = m_vectorValuedResults[VVR_ElementTemperature].dataPtr();
	// Special treatment: constant interpolation for only two elements, i.e. single-layer constructions
	// without discretization
	if (m_elements.size() == 2) {
		m_TsA = states_T[0];
		m_TsB = states_T[1];
	}
	else {
#ifdef CONSTANT_EXTRAPOLATION
		m_TsA = states_T[0];
		m_TsB = states_T[m_nElements-1];
#else
		// linear extrapolation of temperature
		m_TsA = (1.0 + m_elements[0].wR) * states_T[0] - m_elements[0].wR * states_T[1];
		m_TsB = (1.0 + m_elements[m_nElements-1].wL) * states_T[m_nElements-1]
				- m_elements[m_nElements-1].wL * states_T[m_nElements-2];
#endif
	}
	// store surface temperatures also in result values for outputs
	m_results[R_SurfaceTemperatureA] = m_TsA;
	m_results[R_SurfaceTemperatureB] = m_TsB;

#if 0
	// the code below is the slow "very easy to read" code and only kept for documentation purposes
	for (unsigned int i=0; i<m_nElements; ++i) {
		const Element & E = m_elements[i];
		// dry bulk density [kg/m3]
		double rho = E.mat->m_para[NANDRAD::Material::MP_DENSITY].value;
		// specific heat capacity [J/kgK]
		double ce = E.mat->m_para[NANDRAD::Material::MP_HEAT_CAPACITY].value;

		// *** thermal transport ***
		if (!m_moistureBalanceEnabled) {
			// *** primary state variable (extensive quantity) ***
			double u = y[i];
			// energy density [J/m3]
			m_statesU[i]		= u;
			// temperature in [K]
			m_statesT[i]		= u / (rho*ce);
			// thermal conductivity for dry material in [W/mK]
			m_statesLambda[i]	= E.mat->m_para[NANDRAD::Material::MP_CONDUCTIVITY].value;
		}

		// *** hygrothermal transport ***
		else {
			/// \todo hygrothermal
		}
	}
#endif


	// compute active layer temperature
	if (m_activeLayerIndex != NANDRAD::INVALID_ID) {
		// loop through all elements of active layer
		unsigned int elemIdxStart = m_materialLayerElementOffset[m_activeLayerIndex];
		unsigned int elemIdxEnd = m_materialLayerElementOffset[m_activeLayerIndex + 1];

		m_activeLayerMeanTemperature = 0;
		for (unsigned int i = elemIdxStart; i < elemIdxEnd; ++i)
			m_activeLayerMeanTemperature += m_vectorValuedResults[VVR_ElementTemperature].dataPtr()[i] * m_elements[i].dx;
		m_activeLayerMeanTemperature /= m_con->m_constructionType->m_materialLayers[m_activeLayerIndex].m_thickness;
	}

	// *** Boundary Conditions ***

	// left side (A)
	if (m_con->m_interfaceA.m_id != NANDRAD::INVALID_ID) {

		// now also compute solar radiation boundary conditions, if outside interface has any
		if (m_con->m_interfaceA.m_zoneId == 0 && m_con->m_interfaceA.m_solarAbsorption.m_modelType != NANDRAD::InterfaceSolarAbsorption::NUM_MT) {
			// get nominal radiation fluxes across surface of this construction
			double qRadDir, qRadDiff, incidenceAngle;
			double qRadGlobal = m_loads->qSWRad(m_con->m_id, qRadDir, qRadDiff, incidenceAngle);
			// store adsorbed flux
			m_results[R_SolarRadiationFluxA] = m_con->m_interfaceA.m_solarAbsorption.radFlux(qRadGlobal);
		}

		// long wave radiation balance with the ambient
		if (m_con->m_interfaceA.m_zoneId == 0 && m_con->m_interfaceA.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
			double eps = m_con->m_interfaceA.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
			double incomingLWRadiation = m_loads->qLWRad(m_con->m_id);
			double TsA2 = m_TsA * m_TsA;
			double radiationBalance = eps * (incomingLWRadiation - IBK::BOLTZMANN * TsA2 * TsA2);
			// store absorbed flux
			m_results[R_LongWaveRadiationFluxA] = radiationBalance;
		}

		// emitted long wave radiation to other construction instance
		if (m_con->m_interfaceA.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
			IBK_ASSERT(m_con->m_interfaceA.m_viewFactors.size() == m_con->m_interfaceA.m_connectedInterfaces.size());
			const double &sourceEps = m_con->m_interfaceA.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
			for (auto it=m_con->m_interfaceA.m_viewFactors.begin(); it!=m_con->m_interfaceA.m_viewFactors.end(); ++it) {
				const double &TsA2 = m_TsA * m_TsA;
				const double &viewFactor = it->second;
				unsigned int targetId = it->first; // id of targeted construction instance
				const double &targetEps = m_con->m_interfaceA.m_connectedInterfaces.at(targetId)->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
				const double &area = m_con->m_para[NANDRAD::ConstructionInstance::P_Area].value;
				m_emittedLongWaveRadiationA[targetId] = area * viewFactor * sourceEps * targetEps * IBK::BOLTZMANN * TsA2 * TsA2;
			}
		}
	}

	// right side (B)
	if (m_con->m_interfaceB.m_id != NANDRAD::INVALID_ID) {

		// now also compute solar radiation boundary conditions, if outside interface has any
		if (m_con->m_interfaceB.m_zoneId == 0 && m_con->m_interfaceB.m_solarAbsorption.m_modelType != NANDRAD::InterfaceSolarAbsorption::NUM_MT) {
			// get nominal radiation fluxes across surface of this construction
			double qRadDir, qRadDiff, incidenceAngle;
			double qRadGlobal = m_loads->qSWRad(m_con->m_id, qRadDir, qRadDiff, incidenceAngle);
			// store adsorbed flux
			m_results[R_SolarRadiationFluxB] = m_con->m_interfaceB.m_solarAbsorption.radFlux(qRadGlobal);
		}

		// long wave radiation
		if (m_con->m_interfaceB.m_zoneId == 0 && m_con->m_interfaceB.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
			double eps = m_con->m_interfaceB.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
			double incomingLWRadiation = m_loads->qLWRad(m_con->m_id);
			double TsB2 = m_TsB * m_TsB;
			double radiationBalance = eps * (incomingLWRadiation - IBK::BOLTZMANN * TsB2 * TsB2);
			// store absorbed flux
			m_results[R_LongWaveRadiationFluxB] = radiationBalance;
		}

		// emitted long wave radiation to other construction instance
		if (m_con->m_interfaceB.m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
			IBK_ASSERT(m_con->m_interfaceB.m_viewFactors.size() == m_con->m_interfaceB.m_connectedInterfaces.size());
			const double &sourceEps = m_con->m_interfaceB.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
			for (auto it=m_con->m_interfaceB.m_viewFactors.begin(); it!=m_con->m_interfaceB.m_viewFactors.end(); ++it) {
				const double &TsB2 = m_TsB * m_TsB;
				const double &viewFactor = it->second;
				unsigned int targetId = it->first; // id of targeted construction instance
				const double &targetEps = m_con->m_interfaceB.m_connectedInterfaces.at(targetId)->m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
				const double &area = m_con->m_para[NANDRAD::ConstructionInstance::P_Area].value;
				m_emittedLongWaveRadiationB[targetId] = area * viewFactor * sourceEps * targetEps * IBK::BOLTZMANN * TsB2 * TsB2;
			}
		}
	}

	return 0; // signal success
}


// *** private member functions

void ConstructionStatesModel::generateGrid() {
	FUNCID(ConstructionStatesModel::generateGrid);

	// content checks of construction type and construction instance have been done already

	// get pointer to construction type
	const NANDRAD::ConstructionType * conType = m_con->m_constructionType;
	// number of material layers
	size_t nLayers = conType->m_materialLayers.size();

	// Resize layer offsets
	m_materialLayerElementOffset.resize(nLayers,0);

	// for either cases (density == 0 and all others):
	// - discretize all material layers into several elements
	// vector containing all element widths
	std::vector<double> dx_vec;
	// vector containing all element midpoints
	std::vector<double> x_vec;
	// vector containing all element-specific material pointers
	std::vector<const NANDRAD::Material *> mat_vec;
	// layer width
	double dLayer;
	// current total material width
	double x = 0;

	// retrieve parameters regulating grid generation
	double stretch = m_solverPara->m_para[NANDRAD::SolverParameter::P_DiscStretchFactor].value;
	double minDX = m_solverPara->m_para[NANDRAD::SolverParameter::P_DiscMinDx].value;
	unsigned int maxElementsPerLayer = m_solverPara->m_intPara[NANDRAD::SolverParameter::IP_DiscMaxElementsPerLayer].toUInt(true);
	if (maxElementsPerLayer < 3) {
		IBK::IBK_Message(IBK::FormatString("Instead of setting max. elements per layer to less than 3, you may want to use a stretch factor of 0 to disable grid generation."),
						 IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
	}

	// for about thin layers
	for (unsigned int i=0; i<nLayers; ++i) {
		if (conType->m_materialLayers[i].m_thickness < 0.001)
			IBK::IBK_Message(IBK::FormatString("Thickness of layer #%1 is %2 m, which is so small that it will not affect heat transfer/storage capacity. To improve numerical performance, you should remove such small layers."),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
	}

	// case: density is set to 0
	// internal layers are used as elements directly
	// boundary layers are split into 2 elements
	if (stretch == 0.0) {
		// loop over all material layers
		for (unsigned int i=0; i<nLayers; ++i) {
			// update element offset
			m_materialLayerElementOffset[i] = dx_vec.size();
			// material layer width, valid thickness has been tested already
			dLayer = conType->m_materialLayers[i].m_thickness;
			// boundary material layers are split into 2 equal sizes elements
			if (i == 0 || i == nLayers - 1) {
				dx_vec.push_back(dLayer/2);
				dx_vec.push_back(dLayer/2);
				x_vec.push_back(x + dLayer/4);
				x_vec.push_back(x + dLayer*3.0/4);
				mat_vec.push_back(conType->m_materialLayers[i].m_material);
				mat_vec.push_back(conType->m_materialLayers[i].m_material);
			}
			else {
				// internal material layers are used as elements directly
				dx_vec.push_back(dLayer);
				x_vec.push_back(x + dLayer/2);
				mat_vec.push_back(conType->m_materialLayers[i].m_material);
			}
			// update current material width
			x += dLayer;
		}
	}

	// Case: m_density == 1, attempt equidistant discretization with approximately
	// element width as set by DiscMinDx parameter
	else {
		if (stretch == 1.0) {
			// loop over all material layers
			for (unsigned int i=0; i<nLayers; ++i) {
				// update element offset
				m_materialLayerElementOffset[i] = dx_vec.size();
				// material layer width, valid thickness has been tested already
				dLayer = conType->m_materialLayers[i].m_thickness;
				// calculate number of discretized elements for current material layer
				unsigned int n_x = (unsigned int)std::ceil(dLayer / minDX);
				// compute the size of the last element
				double x_last = dLayer - (n_x - 1) * minDX;
				double dx_new;
				// if the last element gets very small, neglect this element
				if (x_last < minDX)
					n_x -= 1;
				// final element width
				dx_new = dLayer / n_x;
				// save elements in dx_vec and x_vec, and update current material width
				for (unsigned int e=0; e<n_x; ++e){
					dx_vec.push_back(dx_new);
					x_vec.push_back(x + dx_new/2);
					mat_vec.push_back(conType->m_materialLayers[i].m_material);
					x += dx_new;
				}
			}
		}
		// all other cases: use stretching function to get variable discretization grid
		else {
			Mesh grid(Mesh::TanHDouble, stretch); // double-sided grid

			// determine required min dx by enforcing at least 3 elements per layer
			double rmin_dx = 1;
			for (unsigned int i=0; i<nLayers; ++i) {
				rmin_dx = std::min(rmin_dx, conType->m_materialLayers[i].m_thickness/3.0); // three elements per layer
			}
			// TODO : adjust min_dx or skip layer if very thin

			std::vector<double> xElem;
			std::vector<double> dxElem;
			// loop over all layers
			for (unsigned int i=0; i<nLayers; ++i) {
				// store offset of element number for the current layer
				// m_materialLayerElementOffset[i] = xElem.size();
				m_materialLayerElementOffset[i] = dx_vec.size();

				unsigned int n = 2;	// start with 2 elements per layer
				dLayer = conType->m_materialLayers[i].m_thickness;
				// repeatedly refine grid for this layer until our minimum element width at the boundary is no longer exceeded
				do {
					++n;
					grid.generate(n, x, x + dLayer, dxElem, xElem);
					if (dxElem[0] <= 1.1*minDX) break;
				} while (n < maxElementsPerLayer); // do not go beyond maximum element count
				if (n >= maxElementsPerLayer) {
					Mesh grid2(grid);
					do {
						grid2.d *= 1.2;
						grid2.generate(n, x, x + dLayer, dxElem, xElem);
					} while (dxElem[0] > 1.1*minDX && grid.d < 100); // do not go beyond maximum element count
					IBK::IBK_Message( IBK::FormatString("Maximum number of elements per layer (%1) is reached in material "
														"layer #%2 (d=%3m), stretch factor increased to %4, resulting in %5 elements")
									  .arg(maxElementsPerLayer)
									  .arg(i+1).arg(dLayer).arg(grid2.d)
									  .arg(n), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				}
				// insert into into global discretization vector
				x_vec.insert(x_vec.end(), xElem.begin(), xElem.end() );
				dx_vec.insert(dx_vec.end(), dxElem.begin(), dxElem.end() );
				// append material pointers
				for (unsigned int j=0; j<n; ++j)
					mat_vec.push_back(conType->m_materialLayers[i].m_material);
				x += dLayer;
			}
		}
	}

	// total number of discretized elements
	m_nElements = dx_vec.size();
	IBK_ASSERT(mat_vec.size() == m_nElements);
	// total number of unkowns
	if (m_moistureBalanceEnabled)
		m_n = m_nElements*2;
	else
		m_n = m_nElements;
	// total construction width
	m_constructionWidth = x;

	// add number of elements so that we can easily get the interval of elements corresponding to the active layer
	m_materialLayerElementOffset.push_back(m_nElements);

	// compute weight factors for coefficient averaging and store material pointer
	for (unsigned int i=0; i<m_nElements; ++i) {
		double wL, wR;
		// left weight factors
		if (i == 0)
			// left boundary element
			wL = 1;
		else
			// internal elements
			wL = dx_vec[i]/(dx_vec[i-1] + dx_vec[i]);
		// right weight factors
		if (i == m_nElements-1)
			// right boundary element
			wR = 1;
		else
			// internal elements
			wR = dx_vec[i]/(dx_vec[i] + dx_vec[i+1]);
		// add to element vector
		m_elements.push_back(Element(i, x_vec[i], dx_vec[i], wL, wR, mat_vec[i]));
	}
}






// Mesh implementation

Mesh::Mesh(MeshType type, double stretch, double ratio)
	: t(type), d(stretch), r(ratio)
{
}

void Mesh::generate(unsigned int n, std::vector<double> & ds_vec) {
	double s = 0;
	ds_vec.resize(n);
	for (unsigned int i=1; i<n; ++i) {
		double xi = double(i)/n;
		double s_next = 0; // only to silence compiler warning; s_next is set in all switch cases below
		switch (t) {
			case Uniform :
				s_next = xi;
				break;

			case TanHSingle :
				s_next = 1 + tanh(d*(xi-1))/tanh(d);
				break;

			case TanHDouble :
			{
				double A = r*r;
				double u = 0.5*(1 + tanh(d*(xi-0.5))/tanh(d/2));
				s_next = u/(A + (1-A)*u);
			} break;
		}
		ds_vec[i-1] = s_next-s;
		s = s_next;
	}
	// compute last element such, that sum(ds) = 1
	ds_vec[n-1] = 1 - s;
}

void Mesh::generate(const unsigned int n, const double x1, const double x2,
	std::vector<double> & dx_vec, std::vector<double> & x_vec)
{
	generate(n, dx_vec); 	// dx_vec now holds normalized element widths
	x_vec.resize(n);
	double L = x2 - x1; 	// can be negative, if working left to right
	double fabsL = fabs(L);
	double x = x1;
	for (unsigned int i=0; i<n; ++i) {
		double next_x = x + L*dx_vec[i];
		x_vec[i] = 0.5*(x + next_x);
		dx_vec[i] *= fabsL; // scale element widths, all widths are always positive
		x = next_x;
	}
	// if L is negative, we reverse the vector
	if (L < 0) {
		std::reverse(x_vec.begin(), x_vec.end());
		std::reverse(dx_vec.begin(), dx_vec.end());
	}
	// The element's center coordinates are always sorted increasing.
	// with dx_vec[0] = 2*(x_vec[0]-x1)
}


} // namespace NANDRAD_MODEL

