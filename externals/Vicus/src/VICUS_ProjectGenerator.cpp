#include "VICUS_Project.h"

#include <NANDRAD_Project.h>
#include <IBK_algorithm.h>

namespace VICUS {

inline QString MultiLangString2QString(const IBK::MultiLanguageString & mls) {
	return QString::fromStdString(mls.string(IBK::MultiLanguageString::m_language, "en"));
}


class ModelGeneratorBase {
public:
	ModelGeneratorBase(const VICUS::Project * pro) :
		m_project(pro)
	{}

	const AbstractDBElement * findZoneSubTemplate(const Room * r, VICUS::ZoneTemplate::SubTemplateType subType) {
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

		const AbstractDBElement * subTemplate = nullptr;
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
				subTemplate = Project::element(m_project->m_embeddedDB.m_infiltration, zoneTemplate->m_idReferences[subType]);
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

	// All definition lists below have the same size and share the same index
	// i.e. m_internalLoadObjects[3] references m_objLists[3] and
	//      m_schedules[3] corresponds also to m_objLists[3]

	std::vector<NANDRAD::InternalLoadsModel>		m_internalLoadObjects;
	std::vector<NANDRAD::ObjectList>				m_objLists;
	std::vector<std::string>						m_objListNames;

	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_objLists
	std::vector< std::vector<NANDRAD::Schedule> >	m_schedules;
};

class VentilationModelGenerator : public ModelGeneratorBase {
public:
	VentilationModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, QStringList & errorStack, const Project *pro);

	// All definition lists below have the same size and share the same index

	std::vector<NANDRAD::NaturalVentilationModel>	m_natVentObjects;
	std::vector<NANDRAD::ObjectList>				m_objLists;
	std::vector<std::string>						m_objListNames;

	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_objLists
	std::vector< std::vector<NANDRAD::Schedule> >	m_schedules;
};


void Project::generateBuildingProjectDataNeu(NANDRAD::Project & p, QStringList & errorStack) const {

	// First mandatory input data checks.
	// We rely on unique IDs being used in the VICUS model
	// Hence, we first check for IDs and inform user about duplicate ID.

	// *** Schedules ***

	// check for unique IDs
	std::set<unsigned int> scheduleIdSet;
	try {
		checkForUniqueIDs(m_embeddedDB.m_schedules, scheduleIdSet);
	} catch (...) {
		errorStack.append(tr("Duplicate schedule IDs found."));
		return;
	}
	for (const Schedule & sched : m_embeddedDB.m_schedules) {
		if (!sched.isValid())
			errorStack.append(tr("Schedule #%1 '%2'.").arg(sched.m_id).arg(MultiLangString2QString(sched.m_displayName)));
	}
	if (!errorStack.isEmpty())	return;


	// *** Zones ***

	std::set<unsigned int> idSet;
	std::vector<const VICUS::Room *> zones;
	generateNandradZones(zones, idSet, p, errorStack);
	if (!errorStack.isEmpty())	return;


	// *** Models based on zone templates ***

	// process all zones
	InternalLoadsModelGenerator internalLoads(this);
	VentilationModelGenerator ventilation(this);
	for (const VICUS::Room * r : zones) {
		internalLoads.generate(r, errorStack);
//		ventilation.generate(r, errorStack);
//		idealHeating.generate(r, errorStack);
	}
	if (!errorStack.isEmpty())	return;


	// transfer data to project

	// *** InternalLoads ***
	p.m_models.m_internalLoadsModels = internalLoads.m_internalLoadObjects;
	p.m_objectLists.insert(p.m_objectLists.end(), internalLoads.m_objLists.begin(), internalLoads.m_objLists.end());
	for (unsigned int i=0; i<internalLoads.m_schedules.size(); ++i)
		p.m_schedules.m_scheduleGroups[internalLoads.m_objLists[i].m_name] = internalLoads.m_schedules[i];

	// *** ...

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
			r->m_para[VICUS::Room::P_Area].checkedValue("Area", "m2", "m2", 0, false, std::numeric_limits<double>::max(), true, nullptr);
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
		intLoadPerson = dynamic_cast<const InternalLoad*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_IntLoadPerson));
		intLoadEquipment = dynamic_cast<const InternalLoad*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_IntLoadEquipment));
		intLoadLighting = dynamic_cast<const InternalLoad*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_IntLoadLighting));
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

	// we only need an all day schedule if we miss at least on of the schedules
	if (!dc.m_values.m_values.empty())
		scheds.push_back(allDays);

	VICUS::Database<Schedule> scheduleDB(0);
	// Schedule DB ID test has been done already
	scheduleDB.setData(m_project->m_embeddedDB.m_schedules);

	// initialize load fractions with 0
	double personRadiationFraction = 0;
	double equipmentRadiationFraction = 0;
	double lightingRadiationFraction = 0;

	// for each given VICUS schedule, generate a set of schedules and insert them in vector scheds (unless already existing, in which case these are extended)

	// *** Person loads (category = IC_Person)***

	if (intLoadPerson != nullptr) {
		if (!intLoadPerson->isValid(scheduleDB)) {
			errorStack.append(qApp->tr("Invalid parameters in internal loads model #%1 '%2'").arg(intLoadPerson->m_id)
							  .arg(MultiLangString2QString(intLoadPerson->m_displayName)) );
		}
		else {
			IBK_ASSERT(intLoadPerson->m_category == InternalLoad::IC_Person);

			// TODO Dirk, other fractions!
			personRadiationFraction = 1-intLoadPerson->m_para[InternalLoad::P_ConvectiveHeatFactor].value;

			// - depending on parametrization option (m_personCountMethod) compute W/m2
			double personPerArea = 0;
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

			// - retrieve schedules referenced via m_occupancyScheduleId and m_activityScheduleId
			//   valid schedule references have been checked in isValid() already
			const Schedule * activitySchedule = scheduleDB[intLoadPerson->m_activityScheduleId];
			const Schedule * occupancySchedule = scheduleDB[intLoadPerson->m_occupancyScheduleId];
			// - multiply all three values for each sample in each schedule -> VICUS::Schedule with time series of time points and values for different day types
			VICUS::Schedule combinedSchedule = activitySchedule->multiply(*occupancySchedule);
			combinedSchedule = combinedSchedule.multiply(personPerArea);

			// - convert and insert VICUS::Schedule to NANDRAD schedule group
			combinedSchedule.insertIntoNandradSchedulegroup(personSchedName, scheds);
		}
	}


	// *** Equipment loads (category = IC_ElectricEquiment )***

	if (intLoadEquipment != nullptr) {
		if (!intLoadEquipment->isValid(scheduleDB)) {
			errorStack.append(qApp->tr("Invalid parameters in internal loads model #%1 '%2'").arg(intLoadEquipment->m_id)
							  .arg(MultiLangString2QString(intLoadEquipment->m_displayName)) );
		}
		else {
			IBK_ASSERT(intLoadEquipment->m_category == InternalLoad::IC_ElectricEquiment);

			equipmentRadiationFraction = 1-intLoadEquipment->m_para[InternalLoad::P_ConvectiveHeatFactor].value;

			// - depending on parametrization option (m_personCountMethod) compute W/m2
			double powerPerArea = 0;
			switch (intLoadEquipment->m_powerMethod) {
				case VICUS::InternalLoad::PM_PowerPerArea:
					powerPerArea = intLoadEquipment->m_para[InternalLoad::P_PowerPerArea].value;
				break;
				case VICUS::InternalLoad::PM_Power:
					powerPerArea = intLoadEquipment->m_para[InternalLoad::P_Power].value/r->m_para[VICUS::Room::P_Area].value;
				break;
				case VICUS::InternalLoad::NUM_PM: ; // just to make compiler happy
			}

			// - retrieve schedules referenced via
			//   valid schedule references have been checked in isValid() already
			const Schedule * powerManagementSchedule = scheduleDB[intLoadEquipment->m_powerManagementScheduleId];
			// - multiply all three values for each sample in each schedule -> VICUS::Schedule with time series of time points and values for different day types
			VICUS::Schedule combinedSchedule = powerManagementSchedule->multiply(powerPerArea);

			// - convert and insert VICUS::Schedule to NANDRAD schedule group
			combinedSchedule.insertIntoNandradSchedulegroup(equipmentSchedName, scheds);
		}
	}


	// *** Lighting loads (category = IC_Lighting )***

	if (intLoadLighting != nullptr) {
		if (!intLoadLighting->isValid(scheduleDB)) {
			errorStack.append(qApp->tr("Invalid parameters in internal loads model #%1 '%2'").arg(intLoadLighting->m_id)
							  .arg(MultiLangString2QString(intLoadLighting->m_displayName)) );
		}
		else {
			IBK_ASSERT(intLoadLighting->m_category == InternalLoad::IC_Lighting);

			lightingRadiationFraction = 1-intLoadLighting->m_para[InternalLoad::P_ConvectiveHeatFactor].value;

			// - depending on parametrization option (m_personCountMethod) compute W/m2
			double powerPerArea = 0;
			switch (intLoadLighting->m_powerMethod) {
				case VICUS::InternalLoad::PM_PowerPerArea:
					powerPerArea = intLoadLighting->m_para[InternalLoad::P_PowerPerArea].value;
				break;
				case VICUS::InternalLoad::PM_Power:
					powerPerArea = intLoadLighting->m_para[InternalLoad::P_Power].value/r->m_para[VICUS::Room::P_Area].value;
				break;
				case VICUS::InternalLoad::NUM_PM: ; // just to make compiler happy
			}

			// - retrieve schedules referenced via
			//   valid schedule references have been checked in isValid() already
			const Schedule * powerManagementSchedule = scheduleDB[intLoadLighting->m_powerManagementScheduleId];
			// - multiply all three values for each sample in each schedule -> VICUS::Schedule with time series of time points and values for different day types
			VICUS::Schedule combinedSchedule = powerManagementSchedule->multiply(powerPerArea);

			// - convert and insert VICUS::Schedule to NANDRAD schedule group
			combinedSchedule.insertIntoNandradSchedulegroup(lightingSchedName, scheds);
		}
	}


	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;

	// now we have a valid schedule group, yet without object list name

	// generate NANDRAD::InternalLoads object
	NANDRAD::InternalLoadsModel internalLoadsModel;
	internalLoadsModel.m_id = Project::uniqueId(m_internalLoadObjects);
	const VICUS::ZoneTemplate * zt = Project::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);
	internalLoadsModel.m_displayName = zt->m_displayName.string();
	internalLoadsModel.m_modelType = NANDRAD::InternalLoadsModel::MT_Scheduled;

	NANDRAD::KeywordList::setParameter(internalLoadsModel.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_PersonRadiationFraction, personRadiationFraction);
	NANDRAD::KeywordList::setParameter(internalLoadsModel.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_EquipmentRadiationFraction, equipmentRadiationFraction);
	NANDRAD::KeywordList::setParameter(internalLoadsModel.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_LightingRadiationFraction, lightingRadiationFraction);

	// Now we check if we have already an internal loads model object with exactly the same parameters (except ID and name).
	// Then, we also compare the matching schedule (the internal loads model object and corresponding schedule have same ID).
	// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
	// otherwise we add load and schedule definitions and generate a new object list.

	for (unsigned int i=0; i<m_internalLoadObjects.size(); ++i) {
		if (m_internalLoadObjects[i].equal(internalLoadsModel) &&
			NANDRAD::Schedules::equalSchedules(m_schedules[i], scheds) )
		{
			// insert our zone ID in object list
			m_objLists[i].m_filterID.m_ids.insert(r->m_id);
		}
		else {
			// append definitions and create new object list
			NANDRAD::ObjectList ol;
			ol.m_name = IBK::pick_name("InternalLoads-" + zt->m_displayName.string(), m_objListNames.begin(), m_objListNames.end());
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ol.m_filterID.m_ids.insert(r->m_id);

			// set object list in new definition
			internalLoadsModel.m_zoneObjectList = ol.m_name;

			// add all definitions
			m_internalLoadObjects.push_back(internalLoadsModel);
			m_schedules.push_back(scheds);
			m_objLists.push_back(ol);
			m_objListNames.push_back(ol.m_name);
		}
	}
}

void VentilationModelGenerator::generate(const Room *r, QStringList &errorStack, const VICUS::Project * pro) {
	FUNCID("VentilationModelGenerator::generate");

	// check if we have a zone template with id to infiltration or ventilation

	// InternalLoad holds data for sub templates ST_IntLoadPerson, ST_IntLoadEquipment or ST_IntLoadLighting
	// we check if at least one of those is defined
	const Infiltration						* infiltration = nullptr;
	const VentilationNatural				* ventilation  = nullptr;
	const ZoneControlNaturalVentilation		* ctrlVentilation = nullptr;
	const VICUS::ZoneTemplate * zoneTemplate = Project::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);

	unsigned int idSubTempInf = INVALID_ID;
	unsigned int idSubTempVent = INVALID_ID;

	VICUS::Database<Schedule> scheduleDB(0);
	// Schedule DB ID test has been done already
	scheduleDB.setData(m_project->m_embeddedDB.m_schedules);

	try {
		if (zoneTemplate == nullptr)
			throw IBK::Exception( qApp->tr("Invalid zone template ID #%1 referenced from zone #%2 '%3'.").arg(r->m_idZoneTemplate)
			  .arg(r->m_id).arg(r->m_displayName).toStdString(), FUNC_ID);

		// check if subType exists
		unsigned int subTempId = zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_VentilationNatural];
		idSubTempVent = subTempId;
		if (subTempId != INVALID_ID){
			ventilation = Project::element(pro->m_embeddedDB.m_ventilationNatural, subTempId);
			if (ventilation == nullptr || !ventilation->isValid(scheduleDB))
				throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
									  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_VentilationNatural])
									  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	try {
		unsigned int subTempId = zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_Infiltration];
		idSubTempInf = subTempId;
		if (subTempId != INVALID_ID){
			infiltration = Project::element(pro->m_embeddedDB.m_infiltration, subTempId);
			if (infiltration == nullptr || !infiltration->isValid())
				throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
									  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_Infiltration])
									  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	try {
		unsigned int subTempId = zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_ControlNaturalVentilation];
		if (subTempId != INVALID_ID){
			ctrlVentilation = Project::element(pro->m_embeddedDB.m_zoneControlVentilationNatural, subTempId);
			if (ctrlVentilation == nullptr || !ctrlVentilation->isValid())
				throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
									  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_Infiltration])
									  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}
		//infiltration = findZoneSubTemplate<Infiltration>(r, VICUS::ZoneTemplate::ST_Infiltration);
		//ventilation  = findZoneSubTemplate<VentilationNatural>(r, VICUS::ZoneTemplate::ST_VentilationNatural);


	// no model defined?
	if (infiltration == nullptr && ventilation == nullptr )
		return;

	//which system we have?
	//1. only infiltration
	//2. only ventilation
	//3. infiltration and ventilation
	enum VentiType{
		V_Infiltration,
		V_Ventilation,
		V_InfAndVenti,
	};
	bool isInf = idSubTempInf != VICUS::INVALID_ID;
	bool isVenti = idSubTempVent != VICUS::INVALID_ID;
	VentiType ventiType;
	if(isInf && !isVenti)				ventiType = V_Infiltration;
	else if(!isInf && isVenti)			ventiType = V_Ventilation;
	else if(isInf && isVenti)			ventiType = V_InfAndVenti;

	NANDRAD::NaturalVentilationModel natVentMod;
	natVentMod.m_id = Project::uniqueId(m_natVentObjects);
	natVentMod.m_displayName = zoneTemplate->m_displayName.string();
	//TODO id and display name

	//if we have a controlling than add control parameter
	if(ctrlVentilation != nullptr){
		//set all control values
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MaximumEnviromentAirTemperatureACRLimit,
										   ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MaximumRoomAirTemperatureACRLimit,
										   ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MinimumEnviromentAirTemperatureACRLimit,
										   ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MinimumRoomAirTemperatureACRLimit,
										   ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_DeltaTemperatureACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_TemperatureDifference].get_value("K"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_WindSpeedACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_WindSpeedMax].get_value("m/s"));
	}

	switch(ventiType){
		case VentiType::V_Infiltration:{
			natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Constant;
			switch(infiltration->m_airChangeType){
				case VICUS::Infiltration::AC_normal:{
					NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
													   NANDRAD::NaturalVentilationModel::P_VentilationRate,
													   infiltration->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h"));
				}break;
				case VICUS::Infiltration::AC_n50:{
					double val = infiltration->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h");
					val *= infiltration->m_para[VICUS::Infiltration::P_ShieldingCoefficient].get_value("-");
					NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
													   NANDRAD::NaturalVentilationModel::P_VentilationRate,
													   val);
				}break;
				case VICUS::Infiltration::NUM_AC:	//only for compiler
				break;
			}
		}
		break;
		case VentiType::V_Ventilation:{
			if(ctrlVentilation == nullptr)
				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Scheduled;
			else{

				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR;
				NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
												   NANDRAD::NaturalVentilationModel::P_VentilationRate,
												   0);
			}
		}
		break;
		case VentiType::V_InfAndVenti:{
			if(ctrlVentilation == nullptr)
				//create in schedule a infiltration + ventilation
				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Scheduled;
			else{
				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR;
				switch(infiltration->m_airChangeType){
					case VICUS::Infiltration::AC_normal:{
						NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
														   NANDRAD::NaturalVentilationModel::P_VentilationRate,
														   infiltration->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h"));
					}break;
					case VICUS::Infiltration::AC_n50:{
						double val = infiltration->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h");
						val *= infiltration->m_para[VICUS::Infiltration::P_ShieldingCoefficient].get_value("-");
						NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
														   NANDRAD::NaturalVentilationModel::P_VentilationRate,
														   val);
					}break;
					case VICUS::Infiltration::NUM_AC:	//only for compiler
					break;
				}
			}
		}
		break;
	}

	// *** schedules ***
	// schedule generation:
	//
	// 1. create basic schedule (name?)
	std::vector<NANDRAD::Schedule> scheds;
	VICUS::Schedule combinedSchedule;
	if(ventiType != V_Infiltration){
		const Schedule * ventSched = scheduleDB[ventilation->m_scheduleId];
		combinedSchedule = ventSched->multiply(ventilation->m_para[VentilationNatural::P_AirChangeRate].value);
		if(!(ventiType == V_Ventilation || (ventiType == V_InfAndVenti && ctrlVentilation != nullptr))){
			double infVal = infiltration->m_para[Infiltration::P_AirChangeRate].value;
			if(infiltration->m_airChangeType == Infiltration::AC_n50)
				infVal *= infiltration->m_para[Infiltration::P_ShieldingCoefficient].value;
			combinedSchedule.add(infVal);
		}
		std::string schedName =  (std::string)NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t",
											NANDRAD::NaturalVentilationModel::P_VentilationRate) + "Schedule [1/h]";
		combinedSchedule.insertIntoNandradSchedulegroup(schedName, scheds);
	}

	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;

	// now we have a valid schedule group, yet without object list name

	// Now we check if we have already an natural ventilation model object with exactly the same parameters (except ID and name).
	// Then, we also compare the matching schedule (the natural ventilation model object and corresponding schedule have same ID).
	// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
	// otherwise we add value and schedule definitions and generate a new object list.

	for (unsigned int i=0; i<m_natVentObjects.size(); ++i) {
		if (m_natVentObjects[i].equal(natVentMod) &&
			NANDRAD::Schedules::equalSchedules(m_schedules[i], scheds) )
		{
			// insert our zone ID in object list
			m_objLists[i].m_filterID.m_ids.insert(r->m_id);
		}
		else {
			// append definitions and create new object list
			NANDRAD::ObjectList ol;
			ol.m_name = IBK::pick_name("Ventilation-" + zoneTemplate->m_displayName.string(), m_objListNames.begin(), m_objListNames.end());
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ol.m_filterID.m_ids.insert(r->m_id);

			// set object list in new definition
			natVentMod.m_zoneObjectList = ol.m_name;

			// add all definitions
			m_natVentObjects.push_back(natVentMod);
			m_schedules.push_back(scheds);
			m_objLists.push_back(ol);
			m_objListNames.push_back(ol.m_name);
		}
	}
}

} // namespace VICUS

