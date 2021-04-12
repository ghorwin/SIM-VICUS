#include "SVDatabase.h"

#include <IBK_messages.h>
#include <IBK_FormatString.h>

#include <VICUS_KeywordList.h>

#include <QtExt_Directories.h>

const unsigned int USER_ID_SPACE_START = 10000;


SVDatabase::SVDatabase() :
	m_materials(USER_ID_SPACE_START),
	m_constructions(2*USER_ID_SPACE_START),
	m_windows(3*USER_ID_SPACE_START),
	m_boundaryConditions(4*USER_ID_SPACE_START),
	m_components(5*USER_ID_SPACE_START),
	m_pipes(USER_ID_SPACE_START*100),
	m_fluids(USER_ID_SPACE_START*1001),
	m_networkComponents(USER_ID_SPACE_START*1002),
	m_EPDElements(USER_ID_SPACE_START),
	m_schedules(6*USER_ID_SPACE_START),
	m_internalLoads(7*USER_ID_SPACE_START),
	m_zoneControlThermostat(11*USER_ID_SPACE_START),
	m_zoneControlShading(13*USER_ID_SPACE_START),
	m_zoneControlVentilationNatural(12*USER_ID_SPACE_START),
	m_ventilationNatural(15*USER_ID_SPACE_START),
	m_infiltration(14*USER_ID_SPACE_START),
	m_zoneTemplates(8*USER_ID_SPACE_START)
{
}


void SVDatabase::readDatabases(DatabaseTypes t) {
	// built-in databases

	// built-in dbs are only read when no filter is applied (i.e. general initialization)
	if (t == NUM_DT) {
		IBK::Path dbDir(QtExt::Directories::databasesDir().toStdString());

		m_materials.readXML(				dbDir / "db_materials.xml", "Materials", "Material", true);
		m_constructions.readXML(			dbDir / "db_constructions.xml", "Constructions", "Construction", true);
		m_windows.readXML(					dbDir / "db_windows.xml", "Windows", "Window", true);
		m_boundaryConditions.readXML(		dbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", true);
		m_components.readXML(				dbDir / "db_components.xml", "Components", "Component", true);
		m_pipes.readXML(					dbDir / "db_pipes.xml", "NetworkPipes", "NetworkPipe", true);
		m_fluids.readXML(					dbDir / "db_fluids.xml", "NetworkFluids", "NetworkFluid", true);
		m_networkComponents.readXML(		dbDir / "db_networkComponents.xml", "NetworkComponents", "NetworkComponent", true);
		m_schedules.readXML(				dbDir / "db_schedules.xml", "Schedules", "Schedule", true);
		m_internalLoads.readXML(			dbDir / "db_internalLoads.xml", "InternalLoads", "InternalLoad", true);
		m_zoneControlThermostat.readXML(	dbDir / "db_zoneControlThermostat.xml", "ZoneControlThermostats", "ZoneControlThermostat", true);
		m_zoneControlVentilationNatural.readXML(	dbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals", "ZoneControlVentilationNatural", true);
		m_zoneControlShading.readXML(		dbDir / "db_zoneControlShading.xml", "ZoneControlShadings", "ZoneControlShading", true);
		m_infiltration.readXML(				dbDir / "db_infiltration.xml", "Infiltrations", "Infiltration", true);
		m_ventilationNatural.readXML(				dbDir / "db_ventilationNatural.xml", "VentilationNaturals", "VentilationNatural", true);
		m_zoneTemplates.readXML(			dbDir / "db_zoneTemplates.xml", "ZoneTemplates", "ZoneTemplate", true);

	}

	// user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	// now read user databases - for dialogs which request reloading of an individual user DB, the parameter
	// t indicates which database to read. By default t is NUM_DT (at program start), which means: read all user DB files.
	if (t == NUM_DT || t == DT_Materials)
		m_materials.readXML(		userDbDir / "db_materials.xml", "Materials", "Material", false);
	if (t == NUM_DT || t == DT_Constructions)
		m_constructions.readXML(	userDbDir / "db_constructions.xml", "Constructions", "Construction", false);
	if (t == NUM_DT || t == DT_Windows)
		m_windows.readXML(			userDbDir / "db_windows.xml", "Windows", "Window", false);
	if (t == NUM_DT || t == DT_BoundaryConditions)
		m_boundaryConditions.readXML(userDbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", false);
	if (t == NUM_DT || t == DT_Components)
		m_components.readXML(		userDbDir / "db_components.xml", "Components", "Component", false);
	if (t == NUM_DT || t == DT_Pipes)
		m_pipes.readXML(			userDbDir / "db_pipes.xml", "NetworkPipes", "NetworkPipe", false);
	if (t == NUM_DT || t == DT_Fluids)
		m_fluids.readXML(			userDbDir / "db_fluids.xml", "NetworkFluids", "NetworkFluid", false);
	if (t == NUM_DT || t == DT_NetworkComponents)
		m_networkComponents.readXML(userDbDir / "db_networkComponents.xml", "NetworkComponents", "NetworkComponent", false);
	if (t == NUM_DT || t == DT_Schedules)
		m_schedules.readXML(		userDbDir / "db_schedules.xml", "Schedules", "Schedule", false);
	if (t == NUM_DT || t == DT_InternalLoads)
		m_internalLoads.readXML(	userDbDir / "db_internalLoads.xml", "InternalLoads", "InternalLoad", false);
	if (t == NUM_DT || t == DT_ZoneControlThermostat)
		m_zoneControlThermostat.readXML(	userDbDir / "db_zoneControlThermostat.xml", "ZoneControlThermostats", "ZoneControlThermostat", false);
	if (t == NUM_DT || t == DT_ZoneControlNaturalVentilation)
		m_zoneControlVentilationNatural.readXML(	userDbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals", "ZoneControlVentilationNatural", false);
	if (t == NUM_DT || t == DT_ZoneControlShading)
		m_zoneControlShading.readXML(	userDbDir / "db_zoneControlShading.xml", "ZoneControlShadings", "ZoneControlShading", false);
	if (t == NUM_DT || t == DT_Infiltration)
		m_infiltration.readXML(	userDbDir / "db_infiltration.xml", "Infiltrations", "Infiltration", false);
	if (t == NUM_DT || t == DT_VentilationNatural)
		m_ventilationNatural.readXML(	userDbDir / "db_ventilationNatural.xml", "VentilationNaturals", "VentilationNatural", false);
	if (t == NUM_DT || t == DT_ZoneTemplates)
		m_zoneTemplates.readXML(	userDbDir / "db_zoneTemplates.xml", "ZoneTemplates", "ZoneTemplate", false);
}


void SVDatabase::writeDatabases() const {
	// we only write user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	m_materials.writeXML(			userDbDir / "db_materials.xml", "Materials");
	m_constructions.writeXML(		userDbDir / "db_constructions.xml", "Constructions");
	m_windows.writeXML(				userDbDir / "db_windows.xml", "Windows");
	m_boundaryConditions.writeXML(	userDbDir / "db_boundaryConditions.xml", "BoundaryConditions");
	m_components.writeXML(			userDbDir / "db_components.xml", "Components");
	m_pipes.writeXML(				userDbDir / "db_pipes.xml", "NetworkPipes");
	m_fluids.writeXML(				userDbDir / "db_fluids.xml", "NetworkFluids");
	m_networkComponents.writeXML(	userDbDir / "db_networkComponents.xml", "NetworkComponents");
	m_schedules.writeXML(			userDbDir / "db_schedules.xml", "Schedules");
	m_internalLoads.writeXML(		userDbDir / "db_internalLoads.xml", "InternalLoads");
	m_zoneControlThermostat.writeXML(userDbDir / "db_zoneControlThermostat.xml", "ZoneControlThermostats");
	m_zoneControlVentilationNatural.writeXML(userDbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals");
	m_zoneControlShading.writeXML(	userDbDir / "db_zoneControlShading.xml", "ZoneControlShadings");
	m_infiltration.writeXML(		userDbDir / "db_infiltration.xml", "Infiltrations");
	m_ventilationNatural.writeXML(	userDbDir / "db_ventilationNatural.xml", "VentilationNaturals");
	m_zoneTemplates.writeXML(		userDbDir / "db_zoneTemplates.xml", "ZoneTemplates");

}


void SVDatabase::updateEmbeddedDatabase(VICUS::Project & p) {


	// *** components ***
	std::set<const VICUS::Component *> referencedComponents;
	for (VICUS::ComponentInstance & ci : p.m_componentInstances)
		referencedComponents.insert(m_components[ci.m_componentID]); // bad/missing IDs yield a nullptr
	referencedComponents.erase(nullptr);

	p.m_embeddedDB.m_components.clear();
	for (const VICUS::Component * c : referencedComponents)
		p.m_embeddedDB.m_components.push_back(*c);


	// *** everything referenced from components: constructions, bc, ... ***

	std::set<const VICUS::Construction *> referencedConstructions;
	std::set<const VICUS::BoundaryCondition *> referencedBC;
	for (const VICUS::Component * c : referencedComponents) {
		referencedConstructions.insert(m_constructions[c->m_idConstruction]); // bad/missing IDs yield a nullptr
		referencedBC.insert(m_boundaryConditions[c->m_idSideABoundaryCondition]); // bad/missing IDs yield a nullptr
		referencedBC.insert(m_boundaryConditions[c->m_idSideBBoundaryCondition]); // bad/missing IDs yield a nullptr
	}
	referencedConstructions.erase(nullptr);
	referencedBC.erase(nullptr);

	p.m_embeddedDB.m_constructions.clear();
	for (const VICUS::Construction * c : referencedConstructions)
		p.m_embeddedDB.m_constructions.push_back(*c);
	p.m_embeddedDB.m_boundaryConditions.clear();
	for (const VICUS::BoundaryCondition * c : referencedBC)
		p.m_embeddedDB.m_boundaryConditions.push_back(*c);


	// *** materials ***
	std::set<const VICUS::Material *> referencedMaterials;
	for (const VICUS::Construction * c : referencedConstructions) {
		for (const VICUS::MaterialLayer & ml : c->m_materialLayers)
			referencedMaterials.insert(m_materials[ml.m_matId]); // bad/missing IDs yield a nullptr
	}
	referencedMaterials.erase(nullptr);

	p.m_embeddedDB.m_materials.clear();
	for (const VICUS::Material * c : referencedMaterials)
		p.m_embeddedDB.m_materials.push_back(*c);

	// *** zone templates ***
	std::set<const VICUS::ZoneTemplate *> referencedZoneTemplates;

	for(const VICUS::Building & b : p.m_buildings){
		for(const VICUS::BuildingLevel & bl : b.m_buildingLevels){
			for(const VICUS::Room &  r : bl.m_rooms){
				referencedZoneTemplates.insert(m_zoneTemplates[r.m_idZoneTemplate]);	// bad/missing IDs yield a nullptr
			}
		}
	}

	referencedZoneTemplates.erase(nullptr);

	p.m_embeddedDB.m_zoneTemplates.clear();
	for (const VICUS::ZoneTemplate * zt : referencedZoneTemplates)
		p.m_embeddedDB.m_zoneTemplates.push_back(*zt);

	// *** schedules ***
	std::set<const VICUS::Schedule *> referencedSchedule;

	// *** internal loads ***
	std::set<const VICUS::InternalLoad *> referencedInternalLoads;
	//also add all schedules
	for (const VICUS::ZoneTemplate * zt : referencedZoneTemplates) {
		for (unsigned int i=0; i<VICUS::ZoneTemplate::NUM_ST; ++i){
			const VICUS::InternalLoad *intLoad = m_internalLoads[zt->m_idReferences[i]];
			if(intLoad	!= nullptr){
				referencedInternalLoads.insert(intLoad); // bad/missing IDs yield a nullptr
				VICUS::ZoneTemplate::SubTemplateType tempType = (VICUS::ZoneTemplate::SubTemplateType)i;
				switch (tempType){
					case VICUS::ZoneTemplate::ST_IntLoadPerson:{
						const VICUS::Schedule *sched = m_schedules[intLoad->m_activityScheduleId];
						if(sched != nullptr)
							referencedSchedule.insert(sched);
						const VICUS::Schedule * sched2 = m_schedules[intLoad->m_occupancyScheduleId];
						if(sched2 != nullptr)
							referencedSchedule.insert(sched2);
					}break;
					case VICUS::ZoneTemplate::ST_IntLoadEquipment:
					break;
					case VICUS::ZoneTemplate::ST_IntLoadLighting:
					break;
					case VICUS::ZoneTemplate::ST_IntLoadOther:
					break;
					case VICUS::ZoneTemplate::ST_ControlThermostat:
					break;
					case VICUS::ZoneTemplate::NUM_ST:
					break;

				}
			}
		}
	}
	referencedInternalLoads.erase(nullptr);


	p.m_embeddedDB.m_internalLoads.clear();
	for (const VICUS::InternalLoad * il : referencedInternalLoads)
		p.m_embeddedDB.m_internalLoads.push_back(*il);
	referencedInternalLoads.erase(nullptr);


	for (const VICUS::Schedule * sched : referencedSchedule)
			referencedSchedule.insert(sched); // bad/missing IDs yield a nullptr

	p.m_embeddedDB.m_schedules.clear();
	for (const VICUS::Schedule * sched : referencedSchedule)
		p.m_embeddedDB.m_schedules.push_back(*sched);




}


