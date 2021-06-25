/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "VICUS_NetworkComponent.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

namespace VICUS {


bool NetworkComponent::isValid(const Database<Schedule> &scheduleDB) const {

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType = NANDRAD::HydraulicNetworkComponent::ModelType (m_modelType);

	std::vector<unsigned int> paraVec = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	for (unsigned int i: paraVec){
		try {
			NANDRAD::HydraulicNetworkComponent::checkModelParameter(m_para[i], i);
		} catch (IBK::Exception) {
			return false;
		}
	}

	// check if given schedules really exist
	std::vector<std::string> reqSchedules = NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(nandradModelType);
	std::vector<std::string> exSchedules;
	for (unsigned int id: m_scheduleIds){
		const Schedule *sched = scheduleDB[id];
		if (sched == nullptr)
			return false;
		exSchedules.push_back(sched->m_displayName.string());
	}

	// check if required schedules are given
	for (const std::string &reqSchedule: reqSchedules){
		if (std::find(exSchedules.begin(), exSchedules.end(), reqSchedule) == exSchedules.end())
			return false;
	}

	return true;
}


AbstractDBElement::ComparisonResult NetworkComponent::equal(const AbstractDBElement *other) const {

	const NetworkComponent * otherNetComp = dynamic_cast<const NetworkComponent*>(other);
	if (otherNetComp == nullptr)
		return Different;

	// check id
	if (m_id != otherNetComp->m_id)
		return Different;

	//check parameters
	for(unsigned int i=0; i<NANDRAD::HydraulicNetworkComponent::NUM_P; ++i){
		if(m_para[i] != otherNetComp->m_para[i])
			return Different;
	}
	if(m_modelType != otherNetComp->m_modelType)
		return Different;

	// check data table
	if (m_polynomCoefficients != otherNetComp->m_polynomCoefficients)
		return Different;

	//check meta data
	if(m_displayName != otherNetComp->m_displayName ||
			m_color != otherNetComp->m_color ||
			m_dataSource != otherNetComp->m_dataSource ||
			m_manufacturer != otherNetComp->m_manufacturer ||
			m_notes != otherNetComp->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
