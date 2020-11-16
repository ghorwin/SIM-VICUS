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

#include "NM_RoomRadiationLoadsModel.h"

#include "NM_ConstructionBalanceModel.h"
#include "NM_WindowModel.h"
#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {

void RoomRadiationLoadsModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	QuantityDescription result;
	result.m_constant = false;
	result.m_description = NANDRAD_MODEL::KeywordList::Description("RoomRadiationLoadsModel::Results", R_WindowSolarRadiationFluxSum);
	result.m_name = NANDRAD_MODEL::KeywordList::Keyword("RoomRadiationLoadsModel::Results", R_WindowSolarRadiationFluxSum);
	result.m_unit = NANDRAD_MODEL::KeywordList::Unit("RoomRadiationLoadsModel::Results", R_WindowSolarRadiationFluxSum);

	resDesc.push_back(result);
}


const double * RoomRadiationLoadsModel::resultValueRef(const QuantityName & quantityName) const {
	if (quantityName.m_name == NANDRAD_MODEL::KeywordList::Keyword("RoomRadiationLoadsModel::Results", R_WindowSolarRadiationFluxSum))
		return &m_result;
	else
		return nullptr;
}


void NANDRAD_MODEL::RoomRadiationLoadsModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// WARNING: We only add input references whose values are summed up - order does not matter.

	// search all models for construction models that have an interface to this zone
	for (AbstractModel * model : models) {

		// *** solar radiation loads through windows ***
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT) {
			// we need a window model here
			WindowModel* mod = dynamic_cast<WindowModel*>(model);
			if (mod == nullptr) continue;

			// check if either interface references us

			// side A
			if (mod->interfaceAZoneID() == m_id) {
				InputReference r;
				r.m_id = mod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
				r.m_name.m_name = "FluxShortWaveRadiationA";
				m_windowSolarRadiationLoadsRefs.push_back(nullptr);
				m_inputRefs.push_back(r);
			}

			// check if either interface references us
			if (mod->interfaceBZoneID() == m_id) {
				InputReference r;
				r.m_id = mod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
				r.m_name.m_name = "FluxShortWaveRadiationB";
				m_windowSolarRadiationLoadsRefs.push_back(nullptr);
				m_inputRefs.push_back(r);
			}
		}

	} // model object loop
}


void RoomRadiationLoadsModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void RoomRadiationLoadsModel::setInputValueRefs(const std::vector<QuantityDescription> &,
												const std::vector<const double *> & resultValueRefs)
{
	// copy value references
	m_windowSolarRadiationLoadsRefs = resultValueRefs;
}


void RoomRadiationLoadsModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// our result variable depends on all summed up solar radiation loads
	for (const double * solarLoadVars : m_windowSolarRadiationLoadsRefs)
		resultInputValueReferences.push_back(std::make_pair(&m_result, solarLoadVars));
}


int RoomRadiationLoadsModel::update() {
	double sumQSolarRadWindowsToWalls = 0.0; // sum of heat fluxes in [W] positive from windows to room

	// \todo adjust factor
	const double fraction = 1.0;

	// Mind sign convention: flux is positive if "into window surface". Since we need to know the load as imposed onto the room,
	// we change the sign.
	for (const double ** flux = m_windowSolarRadiationLoadsRefs.data(), **fluxEnd = flux + m_windowSolarRadiationLoadsRefs.size(); flux != fluxEnd; ++flux)
		sumQSolarRadWindowsToWalls -= **flux * fraction;

	m_result = sumQSolarRadWindowsToWalls;
	return 0; // all ok
}


} // namespace NANDRAD_MODEL


