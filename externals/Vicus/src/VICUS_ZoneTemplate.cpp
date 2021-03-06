#include "VICUS_ZoneTemplate.h"



namespace VICUS {

bool ZoneTemplate::isValid() const {

	if( m_idIntLoadPerson != INVALID_ID){
		///TODO Dirk->Andreas wie prüfe ich denn hier dieses Untertemplate?
	}

	return true;
}


unsigned int ZoneTemplate::subTemplateCount() const {

	unsigned int count = 0;
	if (m_idIntLoadPerson != INVALID_ID) ++count;
	if (m_idIntLoadElectricEquipment != INVALID_ID) ++count;
	if (m_idIntLoadLighting != INVALID_ID) ++count;
	if (m_idIntLoadOther != INVALID_ID) ++count;
	if (m_idControlThermostat != INVALID_ID) ++count;
	if (m_idControlShading != INVALID_ID) ++count;
	if (m_idNaturalVentilation != INVALID_ID) ++count;
	if (m_idMechanicalVentilation != INVALID_ID) ++count;
	if (m_idInfiltration != INVALID_ID) ++count;
	return count;

}



} // namespace VICUS
