#include "VICUS_Project.h"

#include <NANDRAD_Project.h>

namespace VICUS {


class ModelGeneratorBase {
public:
	ModelGeneratorBase(const VICUS::Project * pro) :
		m_project(pro)
	{}

	template <typename T>
	const T * findZoneSubTemplate(const Room * r, VICUS::ZoneTemplate::SubTemplateType subType) {
		FUNCID(ModelGeneratorBase::findZoneSubTemplate);
		if (r->m_idZoneTemplate == VICUS::INVALID_ID)
			return nullptr;
		// lookup zone template

		const VICUS::ZoneTemplate * zoneTemplate = Project::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);
		if (zoneTemplate == nullptr)
			throw IBK::Exception( qApp->tr("Invalid zone template ID #%1 referenced from zone #%2 '%3'.").arg(r->m_idZoneTemplate)
			  .arg(r->m_id).arg(r->m_displayName).toStdString(), FUNC_ID);

		// check if subType exists
		if (zoneTemplate->m_idReferences[subType] == INVALID_ID)
			return nullptr;

		const T * subTemplate = nullptr;
		switch (subType) {
			case VICUS::ZoneTemplate::ST_IntLoadPerson:
			case VICUS::ZoneTemplate::ST_IntLoadEquipment:
			case VICUS::ZoneTemplate::ST_IntLoadLighting:
				subTemplate = Project::element(m_project->m_embeddedDB.m_internalLoads, zoneTemplate->m_idReferences[subType]);
			break;

			case VICUS::ZoneTemplate::ST_IntLoadOther:
			break;
			case VICUS::ZoneTemplate::ST_ControlThermostat:
			break;
			case VICUS::ZoneTemplate::ST_ControlNaturalVentilation:
			break;
			case VICUS::ZoneTemplate::ST_Infiltration:
			break;
			case VICUS::ZoneTemplate::ST_VentilationNatural:
			break;
			case VICUS::ZoneTemplate::ST_IdealHeatingCooling:
			break;
			case VICUS::ZoneTemplate::NUM_ST:
			break;
		}

		if (subTemplate == nullptr)
			throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
								  .arg(zoneTemplate->m_idReferences[subType])
								  .arg(zoneTemplate->m_id).arg(QString::fromStdString(zoneTemplate->m_displayName.string())).toStdString(), FUNC_ID);
		return subTemplate;
	}

	const VICUS::Project * m_project;
};


class InternalLoadsModelGenerator : public ModelGeneratorBase {
public:
	InternalLoadsModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, QStringList & errorStack);


	QStringList errorStack;

	std::vector<NANDRAD::InternalLoadsModel>	m_defs;
	std::vector<NANDRAD::ObjectList>			m_objLists;
	std::vector<NANDRAD::Schedule>				m_schedules;

};



void Project::generateBuildingProjectDataNeu(NANDRAD::Project & p, QStringList & errorStack) const {

	// mandatory input data checks

	// we rely on unique IDs being used in the VICUS model
	// hence, we first check for IDs and inform user about duplicate ID

	std::set<unsigned int> idSet;

	// *** Zones ***

	std::vector<const VICUS::Room *> zones;
	generateNandradZones(zones, idSet, p, errorStack);
	if (!errorStack.isEmpty())	return;


	// *** Models based on zone templates ***

	// process all zones
	InternalLoadsModelGenerator internalLoads(this);
	for (const VICUS::Room * r : zones) {
		internalLoads.generate(r, errorStack);
	}

}


void Project::generateNandradZones(std::vector<const VICUS::Room *> & zones,
								   std::set<unsigned int> & idSet,
								   NANDRAD::Project & p, QStringList & errorStack) const
{
	// collect a list of zone references for further processing

	for (const VICUS::Building & b : m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room & r : bl.m_rooms)
				zones.push_back(&r);

	// check for unqiue zone IDs
	try {
		checkForUniqueIDs(zones, idSet);
	} catch (...) {
		errorStack.append(tr("Duplicate room IDs found."));
		return;
	}

	// process all zones and generate NANDRAD::Zone objects
	for (const VICUS::Room * r : zones) {
		// first create a NANDRAD zone for the room
		NANDRAD::Zone z;
		z.m_id = r->m_id;
		z.m_displayName = r->m_displayName.toStdString();

		// Note: in the code below we expect the parameter's base units to be the same as the default unit for the
		//       populated parameters

		bool isZoneOk = true;
		try {
			r->m_para[VICUS::Room::P_Area].checkedValue("Area", "m2", "m2", 0, true, std::numeric_limits<double>::max(), true, nullptr);
		} catch (...) {
			errorStack.append(tr("Zone #%1 '%2' does not have a valid area defined.").arg(r->m_id).arg(r->m_displayName));
			isZoneOk = false;
		}
		try {
			r->m_para[VICUS::Room::P_Volume].checkedValue("Volume", "m3", "m3", 0, true, std::numeric_limits<double>::max(), true, nullptr);
		} catch (...) {
			errorStack.append(tr("Zone #%1 '%2' does not have a valid volume defined.").arg(r->m_id).arg(r->m_displayName));
			isZoneOk = false;
		}

		if (!isZoneOk)
			continue;

		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, r->m_para[VICUS::Room::P_Area].value);
		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, r->m_para[VICUS::Room::P_Volume].value);

		// for now, zones are always active
		z.m_type = NANDRAD::Zone::ZT_Active;
		// finally append zone
		p.m_zones.push_back(z);
	}
}


void InternalLoadsModelGenerator::generate(const Room * r, QStringList & errorStack) {
	// check if we have a zone template with id to internal loads

	// InternalLoad holds data for sub templates ST_IntLoadPerson, ST_IntLoadEquipment or ST_IntLoadLighting
	// we check if at least one of those is defined
	const InternalLoad * intLoad = nullptr;
	try {

		intLoad = findZoneSubTemplate<InternalLoad>(r, VICUS::ZoneTemplate::ST_IntLoadPerson);
		if (intLoad == nullptr)
			intLoad = findZoneSubTemplate<InternalLoad>(r, VICUS::ZoneTemplate::ST_IntLoadEquipment);
		if (intLoad == nullptr)
			intLoad = findZoneSubTemplate<InternalLoad>(r, VICUS::ZoneTemplate::ST_IntLoadLighting);
	}
	catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	// no internal loads defined?
	if (intLoad == nullptr)
		return;

	// generate schedules

	// generate NANDRAD::InternalLoads object
	// generate object lists

}

} // namespace VICUS

