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

#include "VICUS_ZoneTemplate.h"



namespace VICUS {

ZoneTemplate::ZoneTemplate() {
	for (int i = 0; i<NUM_ST; ++i)
		m_idReferences[i] = INVALID_ID;
}


bool ZoneTemplate::isValid(const Database<InternalLoad> & intLoadDB,
						   const Database<ZoneControlThermostat> & thermostatDB,
						   const Database<Schedule> &schedulesDB,
						   const Database<Infiltration> & infiltraionDB,
						   const Database<VentilationNatural> &ventilationDB,
						   const Database<ZoneIdealHeatingCooling> &idealHeatingCoolingDB,
						   const Database<ZoneControlShading> &ctrlShadingDB,
						   const Database<ZoneControlNaturalVentilation> &ctrlNatVentDB) const
{

	if (m_id == INVALID_ID) {
		m_errorMsg = "ID of zone template is invalid.";
		return false;
	}

	// test all referenced sub-template types
	unsigned int count = 0;
	for (unsigned int i=0; i<NUM_ST; ++i) {
		unsigned int id = m_idReferences[i];
		if (id == INVALID_ID)
			continue;

		++count;

		switch ((SubTemplateType)i) {
			case ZoneTemplate::ST_IntLoadPerson:
			case ZoneTemplate::ST_IntLoadLighting:
			case ZoneTemplate::ST_IntLoadOther:
			case ZoneTemplate::ST_IntLoadEquipment: {
				const InternalLoad *intLoad = intLoadDB[id];
				if (intLoad == nullptr) {
					m_errorMsg = "Internal load with ID '" + std::to_string(id) + "' does not exist.";
					return false;
				}
				if (!intLoad->isValid(schedulesDB)) {
					m_errorMsg = IBK::FormatString("Internal load '%1' is invalid.").arg(intLoad->m_displayName).str();
					return false;
				}
			}
			break;

			case ZoneTemplate::ST_ControlThermostat: {
				const ZoneControlThermostat *thermo = thermostatDB[id];
				if (thermo == nullptr) {
					IBK::FormatString("Zone control thermostat with ID '%1' does not exist.").arg(id).str();
					return false;
				}
				if (!thermo->isValid(schedulesDB)) {
					m_errorMsg = IBK::FormatString("Zone control thermostat '%1' is invalid.").arg(thermo->m_displayName).str();
					return false;
				}
			}
			break;

			case ZoneTemplate::ST_Infiltration: {
				const Infiltration *inf = infiltraionDB[id];
				if (inf == nullptr) {
					IBK::FormatString("Infiltration model with ID '%1' does not exist.").arg(id).str();
					return false;
				}
				if (!inf->isValid()) {
					m_errorMsg = IBK::FormatString("Infiltration model '%1' is invalid.").arg(inf->m_displayName).str();
					return false;
				}
			}
			break;

			case ZoneTemplate::ST_VentilationNatural: {
				const VentilationNatural *venti = ventilationDB[id];
				if (venti == nullptr) {
					IBK::FormatString("Natural ventilation model with ID '%1' does not exist.").arg(id).str();
					return false;
				}
				if (!venti->isValid(schedulesDB)) {
					m_errorMsg = IBK::FormatString("Natural ventilation model '%1' is invalid.").arg(venti->m_displayName).str();
					return false;
				}
			}
			break;

			case ZoneTemplate::ST_IdealHeatingCooling: {
				const ZoneIdealHeatingCooling *ideal = idealHeatingCoolingDB[id];
				if (ideal == nullptr) {
					IBK::FormatString("Ideal heating model with ID '%1' does not exist.").arg(id).str();
					return false;
				}
				if (!ideal->isValid()) {
					m_errorMsg = IBK::FormatString("Ideal heating model  '%1' is invalid.").arg(ideal->m_displayName).str();
					return false;
				}
				// ideal heating/cooling always requires a valid thermostat
				const ZoneControlThermostat *thermo = thermostatDB[m_idReferences[ST_ControlThermostat]];
				if (thermo == nullptr) {
					IBK::FormatString("Zone control thermostat with ID '%1' does not exist.").arg(m_idReferences[ST_ControlThermostat]).str();
					return false;
				}
				if (!thermo->isValid(schedulesDB)) {
					m_errorMsg = IBK::FormatString("Zone control thermostat '%1' is invalid.").arg(thermo->m_displayName).str();
					return false;
				}
			}
			break;

			case ZoneTemplate::ST_ControlVentilationNatural: {
				const ZoneControlNaturalVentilation *ctrl = ctrlNatVentDB[id];
				if(ctrl == nullptr) {
					IBK::FormatString("Zone natural ventilation control model with ID '%1' does not exist.").arg(id).str();
					return false;
				}
				if (!ctrl->isValid(schedulesDB)) {
					m_errorMsg = IBK::FormatString("Zone natural ventilation control model  '%1' is invalid.").arg(ctrl->m_displayName).str();
					return false;
				}
				// Zone natural ventilation control always requires a valid ventilation model
				const VentilationNatural *vent = ventilationDB[m_idReferences[ST_VentilationNatural]];
				if (vent == nullptr) {
					IBK::FormatString("Ventilation model with ID '%1' does not exist.").arg(m_idReferences[ST_ControlThermostat]).str();
					return false;
				}
				if (!vent->isValid(schedulesDB)) {
					m_errorMsg = IBK::FormatString("Ventilation '%1' is invalid.").arg(vent->m_displayName).str();
					return false;
				}
			}
			break;

			case ZoneTemplate::ST_ControlShading: {
				const ZoneControlShading *ctrl = ctrlShadingDB[id];
				if(ctrl == nullptr) {
					IBK::FormatString("Shading control model with ID '%1' does not exist.").arg(id).str();
					return false;
				}
				if (!ctrl->isValid()) {
					m_errorMsg = IBK::FormatString("Shading control model  '%1' is invalid.").arg(ctrl->m_displayName).str();
					return false;
				}
			}
			break;

			case ZoneTemplate::NUM_ST: ; // just to make compiler happy
		}
	}

	return true;
}


unsigned int ZoneTemplate::subTemplateCount() const {

	unsigned int count = 0;
	for (int i=0; i<NUM_ST; ++i)
		if (m_idReferences[i] != VICUS::INVALID_ID) ++count;
	return count;

}


ZoneTemplate::SubTemplateType ZoneTemplate::usedReference(unsigned int index) const {
	int count = 0;
	int i=0;
	// Example  : index = 0 and we have the first id set
	//            loop 1:   i = 0, count = 1 -> return i=0
	// Example 2: index = 1 and we have the first and third id set:
	//            loop 1:   i = 0, count = 1
	//            loop 2:   i = 1, count = 1
	//            loop 3:   i = 2, count = 2  -> return i=1
	for (; i<NUM_ST; ++i) {
		// increase count for each used id reference
		if (m_idReferences[i] != VICUS::INVALID_ID)
			++count;
		if (count > (int)index)
			break;
	}
	return (ZoneTemplate::SubTemplateType)i; // if index > number of used references, we return NUM_ST here
}


AbstractDBElement::ComparisonResult ZoneTemplate::equal(const AbstractDBElement *other) const {
	const ZoneTemplate * otherEPD = dynamic_cast<const ZoneTemplate*>(other);
	if (otherEPD == nullptr)
		return Different;

	// check parameters
	for (unsigned int i=0; i<NUM_ST; ++i){
		if (m_idReferences[i] != otherEPD->m_idReferences[i])
			return Different;
	}

	// check meta data
	if (m_displayName != otherEPD->m_displayName ||
		m_dataSource != otherEPD->m_dataSource ||
		m_notes != otherEPD->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}



} // namespace VICUS
