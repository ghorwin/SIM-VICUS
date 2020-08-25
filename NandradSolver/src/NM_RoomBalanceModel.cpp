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

#include "NM_RoomBalanceModel.h"

#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"
#include "NM_ConstructionBalanceModel.h"
#include "NM_NaturalVentilationModel.h"


namespace NANDRAD_MODEL {

void RoomBalanceModel::setup( const NANDRAD::SimulationParameter &simPara) {
	// copy all object pointers
	m_simPara     = &simPara;

	// results depend on calculation mode
	m_moistureBalanceEnabled = simPara.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled();
	if (m_moistureBalanceEnabled) {
		m_results.resize(NUM_R);
		// resize ydot vector - two balance equations
		m_ydot.resize(2);
	}
	else {
		// resize results vector
		m_results.resize(2);

		// resize ydot vector - one balance equation
		m_ydot.resize(1);
	}
}


void RoomBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	int varCount = 2;
	if (m_simPara->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled()) {
		varCount = NUM_R; // more variables for hygrothermal calculation
	}

	/// \todo what about CO2 ???

	for (int i=0; i<varCount; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("RoomBalanceModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("RoomBalanceModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("RoomBalanceModel::Results", i);

		resDesc.push_back(result);
	}
}


void RoomBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	// first seach in m_results vector
	res.clear();
	// fill with all results and vector valued results

	for (unsigned int i = 0; i < m_results.size(); ++i) {
		res.push_back(&m_results[i]);
	}
}


const double * RoomBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// search inside keyword list result quantities
	// Note: index in m_results corresponds to enumeration values in enum 'Results'
	const char * const category = "RoomBalanceModel::Results";

	if (quantityName.m_name == "ydot") {
		return &m_ydot[0];
	}
	else if (KeywordList::CategoryExists(category) && KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}
	else
		return nullptr;
}


int RoomBalanceModel::priorityOfModelEvaluation() const {
	// room balance model is evaluated one step before outputs
	return AbstractStateDependency::priorityOffsetTail+5;
}


void RoomBalanceModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// we create batches of input references for all input quantities that we require in the room model

	// WARNING: the order in which input refs are added to the vector is important. Function setInputValueRefs()
	//          relies on that!

	// search all models for construction models that have an interface to this zone
	for (AbstractModel * model : models) {

		// *** heat conduction from walls ***
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE) {
			// this might now be any model generated from a zone block, so first
			// convert to
			ConstructionBalanceModel* conMod = dynamic_cast<ConstructionBalanceModel*>(model);
			if (conMod == nullptr) continue;

			// check if either interface references us
			if (conMod->interfaceAZoneID() == m_id) {
				// create input reference to this zone
				InputReference r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name.m_name = "FluxHeatConductionA";
				m_heatCondValueRefs.push_back(nullptr);
				m_inputRefs.push_back(r);
			}
			// check if either interface references us
			if (conMod->interfaceBZoneID() == m_id) {
				// create input reference to this zone
				InputReference r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name.m_name = "FluxHeatConductionB";
				m_heatCondValueRefs.push_back(nullptr);
				m_inputRefs.push_back(r);
			}
		}
		// create input references for zone-specific inputs (optional)
		else if (model->referenceType() == NANDRAD::ModelInputReference::MRT_ZONE) {
			// *** natural ventilation model ***

			NaturalVentilationModel * natVentModel = dynamic_cast<NaturalVentilationModel *>(model);
			if (natVentModel != nullptr) {
				++m_infiltrationModelCount;
				InputReference r;
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
				r.m_name.m_name = "InfiltrationHeatFlux";
				r.m_name.m_index = (int)m_id; // select result for us (our zone id)
				r.m_required = false;
				m_inputRefs.push_back(r);
			}

		}
	} // model object loop

}


void RoomBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void RoomBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	FUNCID(RoomBalanceModel::setInputValueRefs);
	// copy value references as ordered by input references
	std::vector<const double *>::const_iterator it = resultValueRefs.begin();

	for (unsigned int i=0; i<m_heatCondValueRefs.size(); ++i, ++it)
		m_heatCondValueRefs[i] = *it;

	// infiltration heat flux
	for (unsigned int i=0; i<m_infiltrationModelCount; ++i, ++it) {
		if (*it == nullptr)	continue; // nullptr = no flux = skip
		// check against duplicate definition
		if (m_infiltrationValueRef != nullptr)
			throw IBK::Exception(IBK::FormatString("Duplicate infiltration heat flux result generated by different models "
												   "for zone id=%1.").arg(m_id), FUNC_ID);
		m_infiltrationValueRef = *it;
	}

	/// \todo other input fluxes that we sum up

	// check that we have all
	IBK_ASSERT(it == resultValueRefs.end());
}


void RoomBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_moistureBalanceEnabled) {
		/// \todo hygrothermal implementation
	}
	else {
		// for each computed quantity indicate which variables are needed for computation

		// add all heat conduction flux vars as input references for the summation heat flux
		for (const double * heatCondVars : m_heatCondValueRefs)
			resultInputValueReferences.push_back(std::make_pair(&m_results[R_ConstructionHeatConductionLoad], heatCondVars));
		// the room energy balance now depends on this heat conduction flux sum
		resultInputValueReferences.push_back(std::make_pair(&m_ydot[0], &m_results[R_ConstructionHeatConductionLoad]));

		/// \todo other variables with influence on room energy balance (see update())
	}
}




int RoomBalanceModel::update() {

	double sumQHeatCondToWalls = 0.0; // sum of heat fluxes in [W] positive from wall surfaces to room
	for (const double ** flux = m_heatCondValueRefs.data(), **fluxEnd = flux + m_heatCondValueRefs.size(); flux != fluxEnd; ++flux)
		sumQHeatCondToWalls -= **flux;

	// store results
	m_results[R_ConstructionHeatConductionLoad] = sumQHeatCondToWalls;

	double SumQdot = sumQHeatCondToWalls;

	// store the sum of all loads
	m_results[R_CompleteThermalLoad] = SumQdot;
	// solve the balance: ydot = sum loads in [W] (no need to devide by volume since conserved quantity is energy of room air in Joule)
	m_ydot[0] = SumQdot;
	// signal success
	return 0;
}


int RoomBalanceModel::ydot(double* ydot) {
	// copy values to ydot
	ydot[0] = m_ydot[0];
	if (m_ydot.size() > 1) {
		ydot[1] = m_ydot[1];
	}
	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL

