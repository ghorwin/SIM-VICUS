#include "VICUS_Project.h"

#include <NANDRAD_Project.h>

namespace VICUS {

inline QString MultiLangString2QString(const IBK::MultiLanguageString & mls) {
	return QString::fromStdString(mls.string(IBK::MultiLanguageString::m_language, "en"));
}


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
								  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
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


	// *** Schedules ***

	// check for unique IDs
	// TODO


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
	const InternalLoad * intLoadPerson = nullptr;
	const InternalLoad * intLoadEquipment = nullptr;
	const InternalLoad * intLoadLighting = nullptr;
	try {
		intLoadPerson = findZoneSubTemplate<InternalLoad>(r, VICUS::ZoneTemplate::ST_IntLoadPerson);
		intLoadEquipment = findZoneSubTemplate<InternalLoad>(r, VICUS::ZoneTemplate::ST_IntLoadEquipment);
		intLoadLighting = findZoneSubTemplate<InternalLoad>(r, VICUS::ZoneTemplate::ST_IntLoadLighting);
	}
	catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	// no internal loads defined?
	if (intLoadPerson == nullptr && intLoadEquipment == nullptr && intLoadLighting == nullptr)
		return;

	// generate schedules

	std::string personSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_PersonHeatLoadPerArea) + "Schedule [W/m2]";
	std::string equipmentSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_EquipmentHeatLoadPerArea) + "Schedule [W/m2]";
	std::string lightingSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_LightingHeatLoadPerArea) + "Schedule [W/m2]";

	// schedule generation:
	//
	// 1. create basic schedule (name?)
	std::vector<NANDRAD::Schedule> scheds; // contains all schedules for this internal loads parametrization

	// 2. initialize with "AllDays" DailyCycle
	NANDRAD::Schedule allDays;
	allDays.m_type = NANDRAD::Schedule::ST_ALLDAYS;
	NANDRAD::DailyCycle dc;
	dc.m_interpolation = NANDRAD::DailyCycle::IT_Constant;
	dc.m_timePoints.push_back(0);
	if (intLoadPerson == nullptr)		dc.m_values.m_values[personSchedName].push_back(0);
	if (intLoadEquipment == nullptr)	dc.m_values.m_values[equipmentSchedName].push_back(0);
	if (intLoadLighting == nullptr)		dc.m_values.m_values[lightingSchedName].push_back(0);
	allDays.m_dailyCycles.push_back( dc );

	scheds.push_back(allDays);

	// for each given VICUS schedule, generate a set of schedules and insert them in vector scheds (unless already existing, in which case these are extended)

	// Person loads (category = IC_Person)
	VICUS::Database<Schedule> scheduleDB(0);
	// Schedule DB ID test has been done already
	scheduleDB.setData(m_project->m_embeddedDB.m_schedules);
	if (intLoadPerson != nullptr) {
		if (!intLoadPerson->isValid(scheduleDB)) {
			errorStack.append(qApp->tr("Invalid parameters in internal loads model #%1 '%2'").arg(intLoadPerson->m_id)
							  .arg(MultiLangString2QString(intLoadPerson->m_displayName)) );
		}
		else {
			IBK_ASSERT(intLoadPerson->m_category == InternalLoad::IC_Person);

			// - depending on parametrization option (m_personCountMethod) compute W/m2
			double personPerArea;
			switch (intLoadPerson->m_personCountMethod) {
				case VICUS::InternalLoad::PCM_PersonPerArea:
					personPerArea = intLoadPerson->m_para[InternalLoad::P_PersonPerArea].value;
				break;
				case VICUS::InternalLoad::PCM_AreaPerPerson:
					personPerArea = 1./intLoadPerson->m_para[InternalLoad::P_AreaPerPerson].value;
				break;
				case VICUS::InternalLoad::PCM_PersonCount:
					personPerArea = intLoadPerson->m_para[InternalLoad::P_PersonCount].value/r->m_para[VICUS::Room::P_Area].value;
				break;
				case VICUS::InternalLoad::NUM_PCM: ; // just to make compiler happy
			}

			// - retrieve schedules referenced via m_occupancyScheduleId and m_activityScheduleId -> incl. error checks
			const Schedule * activitySchedule = scheduleDB[intLoadPerson->m_activityScheduleId];
			const Schedule * occupancySchedule = scheduleDB[intLoadPerson->m_occupancyScheduleId];
			// - multiply all three values for each sample in each schedule -> VICUS::Schedule with time series of time points and values for different day types
			VICUS::Schedule combinedSchedule = activitySchedule->multiply(*occupancySchedule);
			combinedSchedule = combinedSchedule.multiply(personPerArea);

			// - convert VICUS::Schedule to NANDRAD Schedule


			// - insert into scheds !
		}
	}


	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;

	// generate NANDRAD::InternalLoads object
	// generate object lists

}

} // namespace VICUS

