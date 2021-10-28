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

#include "NM_NetworkInterfaceAdapterModel.h"

#include <IBK_Exception.h>

#include <NANDRAD_NetworkInterfaceAdapterModel.h>
#include <NANDRAD_ObjectList.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {

void NetworkInterfaceAdapterModel::setup(const NANDRAD::NetworkInterfaceAdapterModel & modelData) {
	m_modelData = &modelData;
	m_results.resize(NUM_R);
}


void NetworkInterfaceAdapterModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	for (int i=0; i<NUM_R; ++i) {
		QuantityDescription result;
		result.m_constant = true;
		result.m_description = NANDRAD_MODEL::KeywordList::Description("NetworkInterfaceAdapterModel::Results", i);
		result.m_name = NANDRAD_MODEL::KeywordList::Keyword("NetworkInterfaceAdapterModel::Results", i);
		result.m_unit = NANDRAD_MODEL::KeywordList::Unit("NetworkInterfaceAdapterModel::Results", i);
		resDesc.push_back(result);
	}
}


const double * NetworkInterfaceAdapterModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	const char * const category = "NetworkInterfaceAdapterModel::Results";
	if (KeywordList::KeywordExists(category, quantityName.m_name)) {
		int resIdx = KeywordList::Enumeration(category, quantityName.m_name);
		return &m_results[(unsigned int)resIdx];
	}
	else {
		return nullptr;
	}
}


void NetworkInterfaceAdapterModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// we need to construct 3 input references
	// - heat load from ThermalLoadSummationModel
	// - mass flux from schedule
	// - supply temperature from schedule

	InputReference ref;
	ref.m_id = m_modelData->m_summationModelId;
	ref.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
	ref.m_name.m_name = "TotalHeatLoad";
	ref.m_required = true;
	inputRefs.push_back(ref);

	// the scheduled parameters must be provided with the same ID as ourselfs (our own parameters)
	ref.m_id = m_id;
	ref.m_name.m_name = "SupplyTemperatureSchedule";
	inputRefs.push_back(ref);

	ref.m_name.m_name = "MassFluxSchedule";
	inputRefs.push_back(ref);
}


void NetworkInterfaceAdapterModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	IBK_ASSERT(resultValueRefs.size() ==  3);
	m_valueRefs = resultValueRefs; // Note: we set all our input refs as mandatory, so we can rely on getting valid pointers
}


void NetworkInterfaceAdapterModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	resultInputValueReferences.push_back(std::make_pair(&m_results[R_ReturnTemperature], m_valueRefs[0]));
	resultInputValueReferences.push_back(std::make_pair(&m_results[R_ReturnTemperature], m_valueRefs[1]));
	resultInputValueReferences.push_back(std::make_pair(&m_results[R_ReturnTemperature], m_valueRefs[2]));
}


int NetworkInterfaceAdapterModel::update() {
	// get input parameters
	double heatLoad = *m_valueRefs[0]; // W
	double supplyTemp = *m_valueRefs[1]; // K
	double massFlux = *m_valueRefs[2]; // kg/s

	// J/kgK
	double heatCapacity = m_modelData->m_fluidHeatCapacity.value;

	if (massFlux <= 0)
		m_results[R_ReturnTemperature] = supplyTemp;
	else {
		// J/(s * kg/s * J/kgK) = J/kg*kgK/J = K
		double returnTemperature = supplyTemp - heatLoad/(massFlux*heatCapacity);
		m_results[R_ReturnTemperature] = returnTemperature;
	}
	return 0; // signal success
}

} // namespace NANDRAD_MODEL
