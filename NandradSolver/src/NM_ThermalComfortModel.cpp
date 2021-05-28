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

#include "NM_ThermalComfortModel.h"

#include <IBK_Exception.h>
#include <IBK_physics.h>

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>
#include <NANDRAD_ConstructionInstance.h>

#include "NM_ConstructionStatesModel.h"
#include "NM_RoomStatesModel.h"
#include "NM_WindowModel.h"

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {


void ThermalComfortModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	QuantityDescription result;
	result.m_constant = true;
	result.m_description = NANDRAD_MODEL::KeywordList::Description("ThermalComfortModel::Results", R_OperativeTemperature);
	result.m_name = NANDRAD_MODEL::KeywordList::Keyword("ThermalComfortModel::Results", R_OperativeTemperature);
	result.m_unit = NANDRAD_MODEL::KeywordList::Unit("ThermalComfortModel::Results", R_OperativeTemperature);

	resDesc.push_back(result);
}


const double * ThermalComfortModel::resultValueRef(const InputReference & quantity) const {
	if (quantity.m_name.m_name == NANDRAD_MODEL::KeywordList::Keyword("ThermalComfortModel::Results", R_OperativeTemperature))
		return &m_operativeTemperature;
	else
		return nullptr;
}


void ThermalComfortModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// search all models for construction models that have an interface to this zone
	// and collect data to create input references
	for (AbstractModel * model : models) {

		// *** surface temperature from walls ***
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE) {
			// we need a construction states model here
			ConstructionStatesModel* conMod = dynamic_cast<ConstructionStatesModel*>(model);
			if (conMod == nullptr) continue;

			// check if either interface references us

			// side A
			if (conMod->interfaceAZoneID() == m_id) {
				SurfaceInputData r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name = "SurfaceTemperatureA";
				r.m_netArea = conMod->construction()->m_netHeatTransferArea;
				m_surfaceRefData.push_back(r);
			}

			// check if either interface references us
			if (conMod->interfaceBZoneID() == m_id) {
				SurfaceInputData r;
				r.m_id = conMod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
				r.m_name = "SurfaceTemperatureB";
				r.m_netArea = conMod->construction()->m_netHeatTransferArea;
				m_surfaceRefData.push_back(r);
			}
		}

		// *** surface temperatures from windows ***
		if (model->referenceType() == NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT) {
			// we need a window model here
			WindowModel* mod = dynamic_cast<WindowModel*>(model);
			if (mod == nullptr) continue;

			// check if either interface references us

			// side A
			if (mod->interfaceAZoneID() == m_id) {
				SurfaceInputData r;
				r.m_id = mod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
				r.m_name = "SurfaceTemperatureA";
				r.m_netArea = mod->windowModel()->m_area;
				m_surfaceRefData.push_back(r);
			}

			// check if either interface references us
			if (mod->interfaceBZoneID() == m_id) {
				SurfaceInputData r;
				r.m_id = mod->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
				r.m_name = "SurfaceTemperatureB";
				r.m_netArea = mod->windowModel()->m_area;
				m_surfaceRefData.push_back(r);
			}
		}

	} // model object loop
}


void ThermalComfortModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// compose input references for collected surfaces
	for (const SurfaceInputData & r : m_surfaceRefData) {
		InputReference ref;
		ref.m_id = r.m_id;
		ref.m_referenceType = r.m_referenceType;
		ref.m_name.m_name = r.m_name;
		inputRefs.push_back(ref);
	}

	// last input ref is the zone's air temperature
	InputReference r;
	r.m_id = m_id; // our ID is also the zone ID
	r.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	r.m_name.m_name = "AirTemperature";
	inputRefs.push_back(r);
}


void ThermalComfortModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	// simply store and check value references
	IBK_ASSERT(resultValueRefs.size() == m_surfaceRefData.size() + 1);
	for (unsigned int i=0; i<m_surfaceRefData.size(); ++i)
		m_surfaceRefData[i].m_valueRef = resultValueRefs[i];
	m_zoneAirTemp = resultValueRefs[resultValueRefs.size()-1];
}


void ThermalComfortModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// operative temperature depends on all input variables
	for (unsigned int i=0; i<m_surfaceRefData.size(); ++i) {
		// dependency on room air temperature of corresponding zone
		resultInputValueReferences.push_back(std::make_pair(&m_operativeTemperature, m_surfaceRefData[i].m_valueRef) );
	}
	// also add dependency on zone air temperature
	resultInputValueReferences.push_back(std::make_pair(&m_operativeTemperature, m_zoneAirTemp) );
}


int ThermalComfortModel::update() {

	// compute radiant temperature as area-weighted mean temperature
	// add room air temperature
	double area = 0;
	double areaTemperatureSum = 0;

	for (const SurfaceInputData & r : m_surfaceRefData) {
		area += r.m_netArea;
		areaTemperatureSum += *r.m_valueRef * r.m_netArea;
	}

	if (area != 0.0) {
		double radiantTemperature = areaTemperatureSum/area;
		m_operativeTemperature = 0.5*(radiantTemperature + *m_zoneAirTemp);
	}
	else {
		// no surfaces in zone? may happen in artificial test projects
		m_operativeTemperature = *m_zoneAirTemp;
	}

	return 0; // signal success
}


} // namespace NANDRAD_MODEL
