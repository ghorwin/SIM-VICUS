/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#include "SVDatabase.h"

#include <IBK_messages.h>
#include <IBK_FormatString.h>

#include <VICUS_KeywordList.h>

#include <QtExt_Directories.h>

#include  <QMessageBox>

#include "SVProjectHandler.h"

const unsigned int USER_ID_SPACE_START = 10000;

/*
- Materialien: 1010000-1020000
- Constructions: 1020000-1030000
- Components: 1000001-1010000
- BC: 1030000-1031000
- WindowGlazingSystems: 1031000-1035000
- Windows: 1035000-1040000
- SubSurfaceComponents: 1040000-1042000
- SurfaceHeatings: 1050000-1050500
- Schedules: 1060000-1065000
- InternalLoads: 1065000-1067500
- ZoneControlThermostat: 1067500-1070000
- ZoneControlShading: 1070000-1072500
- ZoneControlVentilation: 1072500-1075000
- ZoneIdealHeatingCooling: 1075000-1077500
- VentilationNatural: 1077500-1080000
- Infiltration: 1080000-1082500
- ZoneTemplates: 1082500-1085000
ToDo Hauke prüfen bitte Network würde ich dann bei ID-Space 1100000 anfangen lassen
- Pipes: 1100000-1102500
- Fluids: 1102500-1105000
- NetworkComponents: 1105000-1107500
- NetworkControllers: 1107500-1110000
- SubNetworks: 1110000-1112500
*/

SVDatabase::SVDatabase() :
	m_materials(1010000),
	m_constructions(1020000),
	m_windows(1035000),
	m_windowGlazingSystems(1031000),
	m_boundaryConditions(1030000),
	m_components(1000001),
	m_subSurfaceComponents(1040000),
	m_surfaceHeatings(1050000),
	m_pipes(1100000),
	m_fluids(1102500),
	m_networkComponents(1105000),
	m_networkControllers(1107500),
	m_subNetworks(1110000),
	m_supplySystems(1085000),
//	m_EPDElements(USER_ID_SPACE_START),
	m_schedules(1060000),
	m_internalLoads(1065000),
	m_zoneControlThermostat(1067500),
	m_zoneControlShading(1070000),
	m_zoneControlVentilationNatural(1072500),
	m_zoneIdealHeatingCooling(1075000),
	m_ventilationNatural(1077500),
	m_infiltration(1080000),
	m_zoneTemplates(1082500)
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
		m_windowGlazingSystems.readXML(		dbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", true);
		m_boundaryConditions.readXML(		dbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", true);
		m_components.readXML(				dbDir / "db_components.xml", "Components", "Component", true);
		m_subSurfaceComponents.readXML(		dbDir / "db_subSurfaceComponents.xml", "SubSurfaceComponents", "SubSurfaceComponent", true);
		m_surfaceHeatings.readXML(			dbDir / "db_surfaceHeatings.xml", "SurfaceHeatings", "SurfaceHeating", true);
		m_pipes.readXML(					dbDir / "db_pipes.xml", "NetworkPipes", "NetworkPipe", true);
		m_fluids.readXML(					dbDir / "db_fluids.xml", "NetworkFluids", "NetworkFluid", true);
		m_networkComponents.readXML(		dbDir / "db_networkComponents.xml", "NetworkComponents", "NetworkComponent", true);
		m_networkControllers.readXML(		dbDir / "db_networkControllers.xml", "NetworkControllers", "NetworkController", true);
		m_subNetworks.readXML		(		dbDir / "db_subNetworks.xml", "SubNetworks", "SubNetwork", true);
		m_schedules.readXML(				dbDir / "db_schedules.xml", "Schedules", "Schedule", true);
		m_internalLoads.readXML(			dbDir / "db_internalLoads.xml", "InternalLoads", "InternalLoad", true);
		m_zoneControlThermostat.readXML(	dbDir / "db_zoneControlThermostat.xml", "ZoneControlThermostats", "ZoneControlThermostat", true);
		m_zoneControlShading.readXML(		dbDir / "db_zoneControlShading.xml", "ZoneControlShadings", "ZoneControlShading", true);
		m_zoneControlVentilationNatural.readXML(	dbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals", "ZoneControlVentilationNatural", true);
		m_zoneIdealHeatingCooling.readXML(	dbDir / "db_zoneIdealHeatingCooling.xml", "ZoneIdealHeatingCoolings", "ZoneIdealHeatingCooling", true);
		m_ventilationNatural.readXML(		dbDir / "db_ventilationNatural.xml", "VentilationNaturals", "VentilationNatural", true);
		m_infiltration.readXML(				dbDir / "db_infiltration.xml", "Infiltrations", "Infiltration", true);
		m_zoneTemplates.readXML(			dbDir / "db_zoneTemplates.xml", "ZoneTemplates", "ZoneTemplate", true);
		m_supplySystems.readXML(			dbDir / "db_supplySystems.xml", "SupplySystems", "SupplySystem", true);

	}

	// user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	// now read user databases - for dialogs which request reloading of an individual user DB, the parameter
	// t indicates which database to read. By default t is NUM_DT (at program start), which means: read all user DB files.
	if (t == NUM_DT || t == DT_Materials)
		m_materials.readXML(				userDbDir / "db_materials.xml", "Materials", "Material", false);
	if (t == NUM_DT || t == DT_Constructions)
		m_constructions.readXML(			userDbDir / "db_constructions.xml", "Constructions", "Construction", false);
	if (t == NUM_DT || t == DT_Windows)
		m_windows.readXML(					userDbDir / "db_windows.xml", "Windows", "Window", false);
	if (t == NUM_DT || t == DT_WindowGlazingSystems)
		m_windowGlazingSystems.readXML(		userDbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", false);
	if (t == NUM_DT || t == DT_BoundaryConditions)
		m_boundaryConditions.readXML(		userDbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", false);
	if (t == NUM_DT || t == DT_Components)
		m_components.readXML(				userDbDir / "db_components.xml", "Components", "Component", false);
	if (t == NUM_DT || t == DT_SubSurfaceComponents)
		m_subSurfaceComponents.readXML(		userDbDir / "db_subSurfaceComponents.xml", "SubSurfaceComponents", "SubSurfaceComponent", false);
	if (t == NUM_DT || t == DT_SurfaceHeating)
		m_surfaceHeatings.readXML(			userDbDir / "db_surfaceHeatings.xml", "SurfaceHeatings", "SurfaceHeating", false);
	if (t == NUM_DT || t == DT_Pipes)
		m_pipes.readXML(					userDbDir / "db_pipes.xml", "NetworkPipes", "NetworkPipe", false);
	if (t == NUM_DT || t == DT_Fluids)
		m_fluids.readXML(					userDbDir / "db_fluids.xml", "NetworkFluids", "NetworkFluid", false);
	if (t == NUM_DT || t == DT_NetworkComponents)
		m_networkComponents.readXML(		userDbDir / "db_networkComponents.xml", "NetworkComponents", "NetworkComponent", false);
	if (t == NUM_DT || t == DT_NetworkControllers)
		m_networkControllers.readXML(		userDbDir / "db_networkControllers.xml", "NetworkControllers", "NetworkController", false);
	if (t == NUM_DT || t == DT_SubNetworks)
		m_subNetworks.readXML(				userDbDir / "db_subNetworks.xml", "SubNetworks", "SubNetwork", false);
	if (t == NUM_DT || t == DT_SupplySystems)
		m_supplySystems.readXML(				userDbDir / "db_supplySystems.xml", "SupplySystems", "SupplySystem", false);
	if (t == NUM_DT || t == DT_Schedules)
		m_schedules.readXML(				userDbDir / "db_schedules.xml", "Schedules", "Schedule", false);
	if (t == NUM_DT || t == DT_InternalLoads)
		m_internalLoads.readXML(			userDbDir / "db_internalLoads.xml", "InternalLoads", "InternalLoad", false);
	if (t == NUM_DT || t == DT_ZoneControlThermostat)
		m_zoneControlThermostat.readXML(	userDbDir / "db_zoneControlThermostat.xml", "ZoneControlThermostats", "ZoneControlThermostat", false);
	if (t == NUM_DT || t == DT_ZoneControlShading)
		m_zoneControlShading.readXML(		userDbDir / "db_zoneControlShading.xml", "ZoneControlShadings", "ZoneControlShading", false);
	if (t == NUM_DT || t == DT_ZoneControlNaturalVentilation)
		m_zoneControlVentilationNatural.readXML(	userDbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals", "ZoneControlVentilationNatural", false);
	if (t == NUM_DT || t == DT_ZoneIdealHeatingCooling)
		m_zoneIdealHeatingCooling.readXML(	userDbDir / "db_zoneIdealHeatingCooling.xml", "ZoneIdealHeatingCoolings", "ZoneIdealHeatingCooling", false);
	if (t == NUM_DT || t == DT_VentilationNatural)
		m_ventilationNatural.readXML(		userDbDir / "db_ventilationNatural.xml", "VentilationNaturals", "VentilationNatural", false);
	if (t == NUM_DT || t == DT_Infiltration)
		m_infiltration.readXML(				userDbDir / "db_infiltration.xml", "Infiltrations", "Infiltration", false);
	if (t == NUM_DT || t == DT_ZoneTemplates)
		m_zoneTemplates.readXML(			userDbDir / "db_zoneTemplates.xml", "ZoneTemplates", "ZoneTemplate", false);
}


void SVDatabase::writeDatabases() {
	// we only write user databases

	// At this point the project is closed and we must not have any local DB elements anylonger.
	// There is no easy way to test this.

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	m_materials.writeXML(			userDbDir / "db_materials.xml", "Materials");
	m_constructions.writeXML(		userDbDir / "db_constructions.xml", "Constructions");
	m_windows.writeXML(				userDbDir / "db_windows.xml", "Windows");
	m_windowGlazingSystems.writeXML(userDbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems");
	m_boundaryConditions.writeXML(	userDbDir / "db_boundaryConditions.xml", "BoundaryConditions");
	m_components.writeXML(			userDbDir / "db_components.xml", "Components");
	m_subSurfaceComponents.writeXML(userDbDir / "db_subSurfaceComponents.xml", "SubSurfaceComponents");
	m_surfaceHeatings.writeXML(		userDbDir / "db_surfaceHeatings.xml", "SurfaceHeatings");
	m_pipes.writeXML(				userDbDir / "db_pipes.xml", "NetworkPipes");
	m_fluids.writeXML(				userDbDir / "db_fluids.xml", "NetworkFluids");
	m_networkComponents.writeXML(	userDbDir / "db_networkComponents.xml", "NetworkComponents");
	m_networkControllers.writeXML(	userDbDir / "db_networkControllers.xml", "NetworkControllers");
	m_subNetworks.writeXML		(	userDbDir / "db_subNetworks.xml", "SubNetworks");
	m_schedules.writeXML(			userDbDir / "db_schedules.xml", "Schedules");
	m_internalLoads.writeXML(		userDbDir / "db_internalLoads.xml", "InternalLoads");
	m_zoneControlThermostat.writeXML(userDbDir / "db_zoneControlThermostat.xml", "ZoneControlThermostats");
	m_zoneControlShading.writeXML(	userDbDir / "db_zoneControlShading.xml", "ZoneControlShadings");
	m_zoneControlVentilationNatural.writeXML(userDbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals");
	m_zoneIdealHeatingCooling.writeXML(	userDbDir / "db_zoneIdealHeatingCooling.xml", "ZoneIdealHeatingCoolings");
	m_ventilationNatural.writeXML(	userDbDir / "db_ventilationNatural.xml", "VentilationNaturals");
	m_infiltration.writeXML(		userDbDir / "db_infiltration.xml", "Infiltrations");
	m_zoneTemplates.writeXML(		userDbDir / "db_zoneTemplates.xml", "ZoneTemplates");
	m_supplySystems.writeXML(		userDbDir / "db_supplySystems.xml", "SupplySystems");
}


void SVDatabase::mergeDatabases(const SVDatabase & db) {
	// process all databases and import not yet existing elements
	m_materials.import(db.m_materials);
	m_constructions.import(db.m_constructions);
	m_windows.import(db.m_windows);
	m_windowGlazingSystems.import(db.m_windowGlazingSystems);
	m_boundaryConditions.import(db.m_boundaryConditions);
	m_components.import(db.m_components);
	m_subSurfaceComponents.import(db.m_subSurfaceComponents);
	m_surfaceHeatings.import(db.m_surfaceHeatings);
	m_pipes.import(db.m_pipes);
	m_fluids.import(db.m_fluids);
	m_networkComponents.import(db.m_networkComponents);
	m_networkControllers.import(db.m_networkControllers);
	m_subNetworks.import(db.m_subNetworks);
	m_supplySystems.import(db.m_supplySystems);
	m_schedules.import(db.m_schedules);
	m_internalLoads.import(db.m_internalLoads);
	m_zoneControlThermostat.import(db.m_zoneControlThermostat);
	m_zoneControlShading.import(db.m_zoneControlShading);
	m_zoneControlVentilationNatural.import(db.m_zoneControlVentilationNatural);
	m_zoneIdealHeatingCooling.import(db.m_zoneIdealHeatingCooling);
	m_ventilationNatural.import(db.m_ventilationNatural);
	m_infiltration.import(db.m_infiltration);
	m_zoneTemplates.import(db.m_zoneTemplates);

}


// Local sort operator, so sort AbstractDBElement vectors by m_id member
struct SortByID : public std::binary_function<VICUS::AbstractDBElement, VICUS::AbstractDBElement, bool> {
	bool operator()(VICUS::AbstractDBElement & x, VICUS::AbstractDBElement & y) const {
		return x.m_id < y.m_id;
	}

};


// Local utility function used to transfer DB elements that are marked as "referenced" or "local" into a sequential vector
template <typename T>
void storeVector(std::vector<T> & vec, const VICUS::Database<T> & src) {
	// clear target vector
	vec.clear();
	// store objects of correct type but skip nullptr
	for (typename std::map<unsigned int, T>::const_iterator it = src.begin(); it != src.end(); ++it) {
		if (it->second.m_isReferenced || it->second.m_local)
			vec.push_back(it->second);
	}
	std::sort(vec.begin(), vec.end(), SortByID());
}


/*! Local utility function. Stores pointers to all DB Elements which are local in the container */
template <typename T>
void collectLocalElements(const VICUS::Database<T> & db, std::set<const T*> & container) {
	for (auto it=db.begin(); it!=db.end(); ++it){
		if (it->second.m_local)
			container.insert(&it->second);
	}
}




void SVDatabase::updateEmbeddedDatabase(VICUS::Project & p) {

	// collect all database elements that are referenced from project or from other DB elements
	updateReferencedElements(p); // now all reference elements have m_isReferenced = true

	// transfer now only those DB elements that are marked as referenced or marked as local
	// i.e. we transfer all elements which are
	// - currently used in the project or
	// - which are currently not used but not stored in the user DB
	storeVector(p.m_embeddedDB.m_materials, m_materials);
	storeVector(p.m_embeddedDB.m_constructions, m_constructions);
	storeVector(p.m_embeddedDB.m_windows, m_windows);
	storeVector(p.m_embeddedDB.m_windowGlazingSystems, m_windowGlazingSystems);
	storeVector(p.m_embeddedDB.m_boundaryConditions, m_boundaryConditions);
	storeVector(p.m_embeddedDB.m_components, m_components);
	storeVector(p.m_embeddedDB.m_subSurfaceComponents, m_subSurfaceComponents);
	storeVector(p.m_embeddedDB.m_surfaceHeatings, m_surfaceHeatings);
	storeVector(p.m_embeddedDB.m_pipes, m_pipes);
	storeVector(p.m_embeddedDB.m_fluids, m_fluids);
	storeVector(p.m_embeddedDB.m_networkComponents, m_networkComponents);
	storeVector(p.m_embeddedDB.m_networkControllers, m_networkControllers);
	storeVector(p.m_embeddedDB.m_subNetworks, m_subNetworks);
	storeVector(p.m_embeddedDB.m_supplySystems, m_supplySystems);
	storeVector(p.m_embeddedDB.m_schedules, m_schedules);
	storeVector(p.m_embeddedDB.m_internalLoads, m_internalLoads);
	storeVector(p.m_embeddedDB.m_zoneControlThermostats, m_zoneControlThermostat);
	storeVector(p.m_embeddedDB.m_zoneControlShading, m_zoneControlShading);
	storeVector(p.m_embeddedDB.m_zoneControlVentilationNatural, m_zoneControlVentilationNatural);
	storeVector(p.m_embeddedDB.m_zoneIdealHeatingCooling, m_zoneIdealHeatingCooling);
	storeVector(p.m_embeddedDB.m_ventilationNatural, m_ventilationNatural);
	storeVector(p.m_embeddedDB.m_infiltration, m_infiltration);
	storeVector(p.m_embeddedDB.m_zoneTemplates, m_zoneTemplates);
}


void SVDatabase::updateReferencedElements(const VICUS::Project &p) {

	// set all elements referenced-property to false
	for (auto it=m_materials.begin(); it!=m_materials.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_constructions.begin(); it!=m_constructions.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_windows.begin(); it!=m_windows.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_windowGlazingSystems.begin(); it!=m_windowGlazingSystems.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_boundaryConditions.begin(); it!=m_boundaryConditions.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_components.begin(); it!=m_components.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_subSurfaceComponents.begin(); it!=m_subSurfaceComponents.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_surfaceHeatings.begin(); it!=m_surfaceHeatings.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_pipes.begin(); it!=m_pipes.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_fluids.begin(); it!=m_fluids.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_subNetworks.begin(); it!=m_subNetworks.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_networkComponents.begin(); it!=m_networkComponents.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_networkControllers.begin(); it!=m_networkControllers.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_schedules.begin(); it!=m_schedules.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_internalLoads.begin(); it!=m_internalLoads.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_zoneControlThermostat.begin(); it!=m_zoneControlThermostat.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_zoneControlShading.begin(); it!=m_zoneControlShading.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_zoneControlVentilationNatural.begin(); it!=m_zoneControlVentilationNatural.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_zoneIdealHeatingCooling.begin(); it!=m_zoneIdealHeatingCooling.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_ventilationNatural.begin(); it!=m_ventilationNatural.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_infiltration.begin(); it!=m_infiltration.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_zoneTemplates.begin(); it!=m_zoneTemplates.end(); ++it)
		it->second.m_isReferenced = false;
	for (auto it=m_supplySystems.begin(); it!=m_supplySystems.end(); ++it)
		it->second.m_isReferenced = false;


	// Now collect all directly referenced elements from project

	std::set<const VICUS::AbstractDBElement*> referencedElements;

	// -> Buildings
	// Components and surface heating
	for (const VICUS::ComponentInstance & ci : p.m_componentInstances) {
		referencedElements.insert(m_components[ci.m_idComponent]);
		referencedElements.insert(m_surfaceHeatings[ci.m_idSurfaceHeating]);
		referencedElements.insert(m_supplySystems[ci.m_idSupplySystem]);
	}

	// SubSurfaceComponent
	for (const VICUS::SubSurfaceComponentInstance & ci : p.m_subSurfaceComponentInstances)
		referencedElements.insert(m_subSurfaceComponents[ci.m_idSubSurfaceComponent]);

	// ZoneTemplate
	for (const VICUS::Building & b : p.m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room &  r : bl.m_rooms)
				referencedElements.insert(m_zoneTemplates[r.m_idZoneTemplate]);

	// -> Networks
	for (const VICUS::Network &net: p.m_geometricNetworks){
		// fluids
		referencedElements.insert(m_fluids[net.m_idFluid]);
		// pipes
		for (const VICUS::NetworkEdge &edge: net.m_edges)
			referencedElements.insert(m_pipes[edge.m_idPipe]);
		// sub networks
		for (const VICUS::NetworkNode &node: net.m_nodes)
			referencedElements.insert(m_subNetworks[node.m_idSubNetwork]);
	}

	// Update all children
	updateElementChildren();

	// Now collect all children elements of referenced elements
	for (const VICUS::AbstractDBElement * el: referencedElements){
		if (el != nullptr)
			el->collectChildren(referencedElements);
	}

	// And finally remember all referenced elements
	for (const VICUS::AbstractDBElement * el: referencedElements){
		if (el != nullptr)
			el->m_isReferenced = true;
	}
}


void SVDatabase::updateElementChildren() {

	// In this function, only DB element interdependencies are searched for and
	// marked in m_isReferenced members.
	// However, top-level elements that are referenced from project objects are not yet
	// marked. This is done in function updateReferencedElements() which internally calls this
	// function first.

	// clear all children, parent relations
	m_materials.clearChildren();
	m_constructions.clearChildren();
	m_windows.clearChildren();
	m_windowGlazingSystems.clearChildren();
	m_boundaryConditions.clearChildren();
	m_components.clearChildren();
	m_subSurfaceComponents.clearChildren();
	m_surfaceHeatings.clearChildren();
	m_pipes.clearChildren();
	m_fluids.clearChildren();
	m_networkComponents.clearChildren();
	m_networkControllers.clearChildren();
	m_subNetworks.clearChildren();
	m_schedules.clearChildren();
	m_internalLoads.clearChildren();
	m_zoneControlThermostat.clearChildren();
	m_zoneControlShading.clearChildren();
	m_zoneControlVentilationNatural.clearChildren();
	m_zoneIdealHeatingCooling.clearChildren();
	m_ventilationNatural.clearChildren();
	m_infiltration.clearChildren();
	m_zoneTemplates.clearChildren();
	m_supplySystems.clearChildren();


	// Now set all children relations

	// *** Buildings ***

	// referenced from constructions
	for (auto it = m_constructions.begin(); it != m_constructions.end(); ++it) {
		VICUS::Construction &c = it->second;
		for (const VICUS::MaterialLayer & ml : c.m_materialLayers)
			c.m_childrenRefs.insert(m_materials[ml.m_idMaterial]);
	}
	// referenced from window (frame+divider)
	for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
		VICUS::Window &c = it->second;
		c.m_childrenRefs.insert(m_materials[c.m_frame.m_idMaterial]);
		c.m_childrenRefs.insert(m_materials[c.m_divider.m_idMaterial]);
	}

	// referenced from components
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		VICUS::Component &c = it->second;

		// constructions
		c.m_childrenRefs.insert(m_constructions[c.m_idConstruction]);

		// BCs
		VICUS::BoundaryCondition *bcA = m_boundaryConditions[c.m_idSideABoundaryCondition];
		if (bcA != nullptr) {
			c.m_childrenRefs.insert(bcA);
			// schedules
			if(bcA->m_heatConduction.m_otherZoneType == VICUS::InterfaceHeatConduction::OZ_Scheduled)
				bcA->m_childrenRefs.insert(m_schedules[bcA->m_heatConduction.m_idSchedule]);
		}

		VICUS::BoundaryCondition *bcB = m_boundaryConditions[c.m_idSideBBoundaryCondition];
		c.m_childrenRefs.insert(bcB);
		if (bcB != nullptr) {
			// schedules
			if(bcB->m_heatConduction.m_otherZoneType == VICUS::InterfaceHeatConduction::OZ_Scheduled)
				bcB->m_childrenRefs.insert(m_schedules[bcB->m_heatConduction.m_idSchedule]);
		}
	}

	// referenced from SubSurfaceComponentInstance
	for (auto it = m_subSurfaceComponents.begin(); it != m_subSurfaceComponents.end(); ++it) {
		VICUS::SubSurfaceComponent &c = it->second;

		// BCs
		c.m_childrenRefs.insert(m_boundaryConditions[c.m_idSideABoundaryCondition]);
		c.m_childrenRefs.insert(m_boundaryConditions[c.m_idSideBBoundaryCondition]);

		// windows
		VICUS::Window * w = m_windows[c.m_idWindow];
		if (w != nullptr) {
			c.m_childrenRefs.insert(w);
			w->m_childrenRefs.insert(m_windowGlazingSystems[w->m_idGlazingSystem]);
		}
	}

	// referenced from SurfaceHeatings
	for (auto it = m_surfaceHeatings.begin(); it != m_surfaceHeatings.end(); ++it) {
		VICUS::SurfaceHeating &sh = it->second;
		sh.m_childrenRefs.insert(m_pipes[sh.m_idPipe]);
	}

	// referenced from zone templates
	for (auto it = m_zoneTemplates.begin(); it != m_zoneTemplates.end(); ++it) {
		VICUS::ZoneTemplate &zt = it->second;

		for (unsigned int i=0; i<VICUS::ZoneTemplate::NUM_ST; ++i) {
			IDType idType = zt.m_idReferences[i];
			VICUS::InternalLoad *intLoad = m_internalLoads[idType];
			VICUS::ZoneControlThermostat * thermo = m_zoneControlThermostat[idType];
			VICUS::ZoneIdealHeatingCooling * idealHeatCool = m_zoneIdealHeatingCooling[idType];
			VICUS::Infiltration *inf = m_infiltration[idType];
			VICUS::VentilationNatural *ventiNat = m_ventilationNatural[idType];
			VICUS::ZoneControlShading *ctrlShad = m_zoneControlShading[idType];
			if (intLoad	!= nullptr) {
				zt.m_childrenRefs.insert(intLoad);
				VICUS::ZoneTemplate::SubTemplateType tempType = (VICUS::ZoneTemplate::SubTemplateType)i;
				switch (tempType) {
					case VICUS::ZoneTemplate::ST_IntLoadPerson: {
						intLoad->m_childrenRefs.insert(m_schedules[intLoad->m_idActivitySchedule]);
						intLoad->m_childrenRefs.insert(m_schedules[intLoad->m_idOccupancySchedule]);
						VICUS::Schedule *moistSchedule = m_schedules[intLoad->m_idMoistureProductionRatePerAreaSchedule];
						// moisture production rate is optional
						if(moistSchedule != nullptr)
							intLoad->m_childrenRefs.insert(moistSchedule);
					}
					break;
					case VICUS::ZoneTemplate::ST_IntLoadEquipment:
					case VICUS::ZoneTemplate::ST_IntLoadLighting:
					case VICUS::ZoneTemplate::ST_IntLoadOther:
						intLoad->m_childrenRefs.insert(m_schedules[intLoad->m_idPowerManagementSchedule]);
					break;
					case VICUS::ZoneTemplate::ST_ControlShading:
					case VICUS::ZoneTemplate::ST_ControlThermostat:
					case VICUS::ZoneTemplate::ST_ControlVentilationNatural:
					case VICUS::ZoneTemplate::ST_Infiltration:
					case VICUS::ZoneTemplate::ST_VentilationNatural:
					case VICUS::ZoneTemplate::ST_IdealHeatingCooling:
					case VICUS::ZoneTemplate::NUM_ST:
					break;
				}
			}
			else if(thermo != nullptr) {
				zt.m_childrenRefs.insert(thermo);
				thermo->m_childrenRefs.insert(m_schedules[thermo->m_idHeatingSetpointSchedule]);
				thermo->m_childrenRefs.insert(m_schedules[thermo->m_idCoolingSetpointSchedule]);
			}
			else if (inf != nullptr) {
				zt.m_childrenRefs.insert(inf);
			}
			else if (ventiNat != nullptr) {
				zt.m_childrenRefs.insert(ventiNat);
				ventiNat->m_childrenRefs.insert(m_schedules[ventiNat->m_idSchedule]);
			}
			else if(idealHeatCool != nullptr)
				zt.m_childrenRefs.insert(idealHeatCool);
			else if(ctrlShad != nullptr)
				zt.m_childrenRefs.insert(ctrlShad);
		}
	}

	// referenced from supply systems
	for (auto it=m_supplySystems.begin(); it!=m_supplySystems.end(); ++it) {
		VICUS::SupplySystem &sup = it->second;
		sup.m_childrenRefs.insert(m_subNetworks[it->second.m_idSubNetwork]);
	}


	// *** Networks elements ***

	// referenced from  sub networks
	for (auto it=m_subNetworks.begin(); it!=m_subNetworks.end(); ++it) {

		VICUS::SubNetwork &sub = it->second;

		for (const VICUS::NetworkElement &elem: sub.m_elements){
			// network controller
			VICUS::NetworkController * ctr = m_networkControllers[elem.m_controlElementId];
			if (ctr != nullptr)
				sub.m_childrenRefs.insert(ctr);

			// network components
			VICUS::NetworkComponent * comp = m_networkComponents[elem.m_componentId];
			if (comp != nullptr) {
				sub.m_childrenRefs.insert(comp);

				// schedules
				for (unsigned int i: comp->m_scheduleIds){
					VICUS::Schedule * sched = m_schedules[i];
					if (sched != nullptr)
						comp->m_childrenRefs.insert(sched);
				}

				// pipes
				VICUS::NetworkPipe * pipe = m_pipes[comp->m_pipePropertiesId];
				if (pipe != nullptr)
					comp->m_childrenRefs.insert(pipe);
			}
		}
	}
}



// local search function to identify duplicates in DBs
template <typename T>
void findDublicates(const VICUS::Database<T> & db, std::vector<SVDatabase::DuplicateInfo> & dupInfos) {
	std::set<unsigned int> duplicateIDs; // stores all IDs of all already found duplicates
	// process all database elements
	for (typename std::map<unsigned int, T>::const_iterator it = db.begin(); it != db.end(); ++it) {
		// skip elements already marked as duplicates
		if (duplicateIDs.find(it->first) != duplicateIDs.end() ) continue;

		// process all other elements
		for (typename std::map<unsigned int, T>::const_iterator it2 = db.begin(); it2 != db.end(); ++it2) {
			// skip ourselves
			if (it->first == it2->first) continue;
			// skip elements already marked as duplicates
			if (duplicateIDs.find(it2->first) != duplicateIDs.end() ) continue;

			// are we sufficiently similar
			VICUS::AbstractDBElement::ComparisonResult compRes = it->second.equal(&it2->second);
			if (compRes != VICUS::AbstractDBElement::Different) {
				SVDatabase::DuplicateInfo info;
				info.m_idFirst = it->first;
				info.m_idSecond = it2->first;
				info.m_identical = (compRes == VICUS::AbstractDBElement::Equal);
				dupInfos.push_back(info);
				duplicateIDs.insert(it->first);
				duplicateIDs.insert(it2->first);
				break;
			}
		}
	}
}



void SVDatabase::determineDuplicates(std::vector<std::vector<SVDatabase::DuplicateInfo> > & duplicatePairs) const {
	duplicatePairs.resize(NUM_DT);

	// process all databases and search for duplicates
	findDublicates(m_materials, duplicatePairs[DT_Materials]);
	findDublicates(m_constructions, duplicatePairs[DT_Constructions]);
	findDublicates(m_windows, duplicatePairs[DT_Windows]);
	findDublicates(m_windowGlazingSystems, duplicatePairs[DT_WindowGlazingSystems]);
	findDublicates(m_boundaryConditions, duplicatePairs[DT_BoundaryConditions]);
	findDublicates(m_components, duplicatePairs[DT_Components]);
	findDublicates(m_subSurfaceComponents, duplicatePairs[DT_SubSurfaceComponents]);
	findDublicates(m_surfaceHeatings, duplicatePairs[DT_SurfaceHeating]);
	findDublicates(m_pipes, duplicatePairs[DT_Pipes]);
	findDublicates(m_fluids, duplicatePairs[DT_Fluids]);
	findDublicates(m_networkComponents, duplicatePairs[DT_NetworkComponents]);
	findDublicates(m_networkControllers, duplicatePairs[DT_NetworkControllers]);
	findDublicates(m_subNetworks, duplicatePairs[DT_SubNetworks]);
	findDublicates(m_supplySystems, duplicatePairs[DT_SupplySystems]);
	findDublicates(m_schedules, duplicatePairs[DT_Schedules]);
	findDublicates(m_internalLoads, duplicatePairs[DT_InternalLoads]);
	findDublicates(m_zoneControlThermostat, duplicatePairs[DT_ZoneControlThermostat]);
	findDublicates(m_zoneControlShading, duplicatePairs[DT_ZoneControlShading]);
	findDublicates(m_zoneControlVentilationNatural, duplicatePairs[DT_ZoneControlNaturalVentilation]);
	findDublicates(m_zoneIdealHeatingCooling, duplicatePairs[DT_ZoneIdealHeatingCooling]);
	findDublicates(m_ventilationNatural, duplicatePairs[DT_VentilationNatural]);
	findDublicates(m_infiltration, duplicatePairs[DT_Infiltration]);
	findDublicates(m_zoneTemplates, duplicatePairs[DT_ZoneTemplates]);
}


// local convenience function to substitute IDs and mark the respective DB as modified
template <typename T>
void replaceID(unsigned int oldID, unsigned int newId, unsigned int & idVar, T & db) {
	if (idVar == oldID) {
		idVar = newId;
		db.m_modified = true;
	}
}


void SVDatabase::removeDBElement(SVDatabase::DatabaseTypes dbType, unsigned int elementID, unsigned int replacementElementID) {
	// depending on database type, we need to replace references to the element on other dbs as well
	// Note: if we have a project, also replace directly referenced DB elements in project
	//       - Components
	//       - SubSurfaceComponents
	//       - ZoneTemplate
	//       - SurfaceHeating
	//       - NetworkPipes
	//       - NetworkFluid
	//       - NetworkComponent
	//       - SubNetwork

	switch (dbType) {

		case DT_Materials : {
			// materials are used in construction layers, window frame and divider
			// replace mat ID everywhere
			for (const auto & p : m_constructions) {
				VICUS::Construction & con = const_cast<VICUS::Construction &>(p.second); // const-cast is ok here
				for (VICUS::MaterialLayer & ml : con.m_materialLayers)
					replaceID(elementID, replacementElementID, ml.m_idMaterial, m_constructions);
			}
			for (const auto & p : m_windows) {
				VICUS::Window & w = const_cast<VICUS::Window&>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, w.m_frame.m_idMaterial, m_windows);
				replaceID(elementID, replacementElementID, w.m_divider.m_idMaterial, m_windows);
			}
			// finally remove material
			m_materials.remove(elementID);
			m_materials.m_modified = true;
		} break;

		case DT_Constructions : {
			for (const auto & p : m_components) {
				VICUS::Component & c = const_cast<VICUS::Component &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idConstruction, m_components);
			}
			for (const auto & p : m_subSurfaceComponents) {
				VICUS::SubSurfaceComponent & c = const_cast<VICUS::SubSurfaceComponent &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idConstruction, m_subSurfaceComponents);
			}
			m_constructions.remove(elementID);
			m_constructions.m_modified = true;
		} break;

		case SVDatabase::DT_Windows: {
			for (const auto & p : m_subSurfaceComponents) {
				VICUS::SubSurfaceComponent & c = const_cast<VICUS::SubSurfaceComponent &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idWindow, m_subSurfaceComponents);
			}
			m_windows.remove(elementID);
			m_windows.m_modified = true;
		} break;

		case SVDatabase::DT_WindowGlazingSystems: {
			for (const auto & p : m_windows) {
				VICUS::Window & c = const_cast<VICUS::Window &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idGlazingSystem, m_windows);
			}
			m_windowGlazingSystems.remove(elementID);
			m_windowGlazingSystems.m_modified = true;
		} break;

		case SVDatabase::DT_BoundaryConditions: {
			for (const auto & p : m_components) {
				VICUS::Component & c = const_cast<VICUS::Component &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idSideABoundaryCondition, m_components);
				replaceID(elementID, replacementElementID, c.m_idSideBBoundaryCondition, m_components);
			}
			for (const auto & p : m_subSurfaceComponents) {
				VICUS::SubSurfaceComponent & c = const_cast<VICUS::SubSurfaceComponent &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idSideABoundaryCondition, m_subSurfaceComponents);
				replaceID(elementID, replacementElementID, c.m_idSideBBoundaryCondition, m_subSurfaceComponents);
			}
			m_boundaryConditions.remove(elementID);
			m_boundaryConditions.m_modified = true;
		} break;

		case SVDatabase::DT_Components: {
			// components are referenced from project
			if (SVProjectHandler::instance().isValid()) {
				for (const auto & p : project().m_componentInstances) {
					VICUS::ComponentInstance & c = const_cast<VICUS::ComponentInstance &>(p); // const-cast is ok here
					if (c.m_idComponent == elementID)
						c.m_idComponent = replacementElementID;
				}
			}
			m_components.remove(elementID);
			m_components.m_modified = true;
		} break;

		case SVDatabase::DT_SubSurfaceComponents: {
			// components are referenced from project
			if (SVProjectHandler::instance().isValid()) {
				for (const auto & p : project().m_subSurfaceComponentInstances) {
					VICUS::SubSurfaceComponentInstance & c = const_cast<VICUS::SubSurfaceComponentInstance &>(p); // const-cast is ok here
					if (c.m_idSubSurfaceComponent == elementID)
						c.m_idSubSurfaceComponent = replacementElementID;
				}
			}
			m_subSurfaceComponents.remove(elementID);
			m_subSurfaceComponents.m_modified = true;
		} break;

		case SVDatabase::DT_SurfaceHeating: {
			// referenced from project
			if (SVProjectHandler::instance().isValid()) {
				for (const auto & p : project().m_componentInstances) {
					VICUS::ComponentInstance & c = const_cast<VICUS::ComponentInstance &>(p); // const-cast is ok here
					if (c.m_idSurfaceHeating == elementID)
						c.m_idSurfaceHeating = replacementElementID;
				}
			}
			m_surfaceHeatings.remove(elementID);
			m_surfaceHeatings.m_modified = true;
		} break;

		case SVDatabase::DT_Pipes: {
			// referenced from project
			if (SVProjectHandler::instance().isValid()) {
				for (const auto & p : project().m_geometricNetworks) {
					VICUS::Network & net = const_cast<VICUS::Network &>(p); // const-cast is ok here
					for (unsigned int & np : net.m_availablePipes)
						if (np == elementID)
							np = replacementElementID;
					for (VICUS::NetworkEdge & ne : net.m_edges)
						if (ne.m_idPipe == elementID)
							ne.m_idPipe = replacementElementID;
				}
			}
			for (const auto & p : m_networkComponents) {
				VICUS::NetworkComponent & c = const_cast<VICUS::NetworkComponent &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_pipePropertiesId, m_networkComponents);
			}
			for (const auto & p : m_surfaceHeatings) {
				VICUS::SurfaceHeating & c = const_cast<VICUS::SurfaceHeating &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idPipe, m_surfaceHeatings);
			}

			m_pipes.remove(elementID);
			m_pipes.m_modified = true;
		} break;

		case SVDatabase::DT_Fluids: {
			if (SVProjectHandler::instance().isValid()) {
				for (const auto & p : project().m_geometricNetworks) {
					VICUS::Network & net = const_cast<VICUS::Network &>(p); // const-cast is ok here
					replaceID(elementID, replacementElementID, net.m_idFluid, m_fluids);
				}
			}
			m_fluids.remove(elementID);
			m_fluids.m_modified = true;
		} break;

		case SVDatabase::DT_NetworkComponents: {
			for (const auto & p : m_subNetworks) {
				VICUS::SubNetwork & s = const_cast<VICUS::SubNetwork &>(p.second); // const-cast is ok here
				for (VICUS::NetworkElement & el: s.m_elements)
					replaceID(elementID, replacementElementID, el.m_componentId, m_subNetworks);
			}
			m_networkComponents.remove(elementID);
			m_networkComponents.m_modified = true;
		} break;

		case SVDatabase::DT_NetworkControllers: {
			for (const auto & p : m_subNetworks) {
				VICUS::SubNetwork & s = const_cast<VICUS::SubNetwork &>(p.second); // const-cast is ok here
				for (VICUS::NetworkElement & el: s.m_elements)
					replaceID(elementID, replacementElementID, el.m_controlElementId, m_subNetworks);
			}
			m_networkControllers.remove(elementID);
			m_networkControllers.m_modified = true;
		} break;

		case SVDatabase::DT_SubNetworks: {
			if (SVProjectHandler::instance().isValid()) {
				for (const auto & p : project().m_geometricNetworks) {
					VICUS::Network & net = const_cast<VICUS::Network &>(p); // const-cast is ok here
					for (VICUS::NetworkNode & no: net.m_nodes){
						if (no.m_idSubNetwork == elementID)
							no.m_idSubNetwork = replacementElementID;
					}
				}
			}
			m_subNetworks.remove(elementID);
			m_subNetworks.m_modified = true;
		} break;

		case SVDatabase::DT_SupplySystems: {
			for (const auto & p : m_supplySystems) {
				VICUS::SupplySystem & c = const_cast<VICUS::SupplySystem &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_id, m_supplySystems);
			}
			m_supplySystems.remove(elementID);
			m_supplySystems.m_modified = true;
		} break;


		case SVDatabase::DT_Schedules:
			for (const auto & p : m_internalLoads) {
				VICUS::InternalLoad & c = const_cast<VICUS::InternalLoad &>(p.second); // const-cast is ok here
				// might be any of the following four
				replaceID(elementID, replacementElementID, c.m_idActivitySchedule, m_internalLoads);
				replaceID(elementID, replacementElementID, c.m_idMoistureProductionRatePerAreaSchedule, m_internalLoads);
				replaceID(elementID, replacementElementID, c.m_idOccupancySchedule, m_internalLoads);
				replaceID(elementID, replacementElementID, c.m_idPowerManagementSchedule, m_internalLoads);
			}
			for (const auto & p : m_ventilationNatural) {
				VICUS::VentilationNatural & c = const_cast<VICUS::VentilationNatural &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idSchedule, m_ventilationNatural);
			}
			for (const auto & p : m_zoneControlThermostat) {
				VICUS::ZoneControlThermostat & c = const_cast<VICUS::ZoneControlThermostat &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idCoolingSetpointSchedule, m_zoneControlThermostat);
				replaceID(elementID, replacementElementID, c.m_idHeatingSetpointSchedule, m_zoneControlThermostat);
			}
			for (const auto & p : m_zoneControlVentilationNatural) {
				VICUS::ZoneControlNaturalVentilation & c = const_cast<VICUS::ZoneControlNaturalVentilation &>(p.second); // const-cast is ok here
				for (int j=0; j<VICUS::ZoneControlNaturalVentilation::NUM_P; ++j)
					replaceID(elementID, replacementElementID, c.m_idSchedules[j], m_zoneControlVentilationNatural);
			}
			for (const auto & p : m_networkComponents) {
				VICUS::NetworkComponent & c = const_cast<VICUS::NetworkComponent &>(p.second); // const-cast is ok here
				for (unsigned int & id: c.m_scheduleIds)
					replaceID(elementID, replacementElementID, id, m_networkComponents);
			}

			m_schedules.remove(elementID);
			m_schedules.m_modified = true;
		break;

		case SVDatabase::DT_InternalLoads: {
			for (const auto & p : m_zoneTemplates) {
				VICUS::ZoneTemplate & c = const_cast<VICUS::ZoneTemplate &>(p.second); // const-cast is ok here
				// might be any of the following four
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson], m_zoneTemplates);
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_IntLoadLighting], m_zoneTemplates);
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment], m_zoneTemplates);
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_IntLoadOther], m_zoneTemplates);
			}
			m_internalLoads.remove(elementID);
			m_internalLoads.m_modified = true;
		} break;

		case SVDatabase::DT_ZoneControlThermostat: {
			for (const auto & p : m_zoneTemplates) {
				VICUS::ZoneTemplate & c = const_cast<VICUS::ZoneTemplate &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_ControlThermostat], m_zoneTemplates);
			}
			m_zoneControlThermostat.remove(elementID);
			m_zoneControlThermostat.m_modified = true;
		} break;

		case SVDatabase::DT_ZoneControlShading:  {
			for (const auto & p : m_zoneTemplates) {
				VICUS::ZoneTemplate & c = const_cast<VICUS::ZoneTemplate &>(p.second); // const-cast is ok here
				// TODO Dirk : add as soon as available
				//replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_], m_zoneTemplates);
			}
			m_zoneControlShading.remove(elementID);
			m_zoneControlShading.m_modified = true;
		} break;

		case SVDatabase::DT_ZoneControlNaturalVentilation:  {
			for (const auto & p : m_zoneTemplates) {
				VICUS::ZoneTemplate & c = const_cast<VICUS::ZoneTemplate &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_ControlVentilationNatural], m_zoneTemplates);
			}
			m_zoneControlVentilationNatural.remove(elementID);
			m_zoneControlVentilationNatural.m_modified = true;
		} break;

		case SVDatabase::DT_ZoneIdealHeatingCooling:  {
			for (const auto & p : m_zoneTemplates) {
				VICUS::ZoneTemplate & c = const_cast<VICUS::ZoneTemplate &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_IdealHeatingCooling], m_zoneTemplates);
			}
			m_zoneIdealHeatingCooling.remove(elementID);
			m_zoneIdealHeatingCooling.m_modified = true;
		} break;

		case SVDatabase::DT_VentilationNatural:   {
			for (const auto & p : m_zoneTemplates) {
				VICUS::ZoneTemplate & c = const_cast<VICUS::ZoneTemplate &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_VentilationNatural], m_zoneTemplates);
			}
			m_ventilationNatural.remove(elementID);
			m_ventilationNatural.m_modified = true;
		} break;

		case SVDatabase::DT_Infiltration:   {
			for (const auto & p : m_zoneTemplates) {
				VICUS::ZoneTemplate & c = const_cast<VICUS::ZoneTemplate &>(p.second); // const-cast is ok here
				replaceID(elementID, replacementElementID, c.m_idReferences[VICUS::ZoneTemplate::ST_Infiltration], m_zoneTemplates);
			}
			m_infiltration.remove(elementID);
			m_infiltration.m_modified = true;
		} break;

		case SVDatabase::DT_ZoneTemplates:
			// components are referenced from project
			if (SVProjectHandler::instance().isValid()) {
				for (const VICUS::Building & b : project().m_buildings)
					for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
						for (const VICUS::Room & r : bl.m_rooms) {
							VICUS::Room & c = const_cast<VICUS::Room &>(r); // const-cast is ok here
							if (c.m_idZoneTemplate == elementID)
								c.m_idZoneTemplate = replacementElementID;
						}
			}
			m_zoneTemplates.remove(elementID);
			m_zoneTemplates.m_modified = true;
		break;

		case SVDatabase::NUM_DT: ; // just to make compiler happy
	}

}


void SVDatabase::removeLocalElements() {
	m_materials.removeLocalElements();
	m_constructions.removeLocalElements();
	m_windows.removeLocalElements();
	m_windowGlazingSystems.removeLocalElements();
	m_boundaryConditions.removeLocalElements();
	m_components.removeLocalElements();
	m_subSurfaceComponents.removeLocalElements();
	m_surfaceHeatings.removeLocalElements();
	m_pipes.removeLocalElements();
	m_fluids.removeLocalElements();
	m_networkComponents.removeLocalElements();
	m_networkControllers.removeLocalElements();
	m_subNetworks.removeLocalElements();
	m_schedules.removeLocalElements();
	m_internalLoads.removeLocalElements();
	m_zoneControlThermostat.removeLocalElements();
	m_zoneControlShading.removeLocalElements();
	m_zoneControlVentilationNatural.removeLocalElements();
	m_zoneIdealHeatingCooling.removeLocalElements();
	m_ventilationNatural.removeLocalElements();
	m_infiltration.removeLocalElements();
	m_zoneTemplates.removeLocalElements();
}


void SVDatabase::removeNotReferencedLocalElements(SVDatabase::DatabaseTypes dbType, const VICUS::Project &p) {
	updateReferencedElements(p);

	switch (dbType) {
		case DT_Materials:
			m_materials.removeNotReferencedLocalElements(); break;
		case DT_Constructions:
			m_constructions.removeNotReferencedLocalElements(); break;
		case DT_Windows:
			m_windows.removeNotReferencedLocalElements(); break;
		case DT_WindowGlazingSystems:
			m_windowGlazingSystems.removeNotReferencedLocalElements(); break;
		case DT_BoundaryConditions:
			m_boundaryConditions.removeNotReferencedLocalElements(); break;
		case DT_Components:
			m_components.removeNotReferencedLocalElements(); break;
		case DT_SubSurfaceComponents:
			m_subSurfaceComponents.removeNotReferencedLocalElements(); break;
		case DT_SurfaceHeating:
			m_surfaceHeatings.removeNotReferencedLocalElements(); break;
		case DT_Pipes:
			m_pipes.removeNotReferencedLocalElements(); break;
		case DT_Fluids:
			m_fluids.removeNotReferencedLocalElements(); break;
		case DT_SubNetworks:
			m_subNetworks.removeNotReferencedLocalElements(); break;
		case DT_SupplySystems:
			m_supplySystems.removeNotReferencedLocalElements(); break;
		case DT_NetworkControllers:
			m_networkControllers.removeNotReferencedLocalElements();  break;
		case DT_NetworkComponents:
			m_networkComponents.removeNotReferencedLocalElements(); break;
		case DT_Schedules:
			m_schedules.removeNotReferencedLocalElements(); break;
		case DT_InternalLoads:
			m_internalLoads.removeNotReferencedLocalElements(); break;
		case DT_ZoneControlThermostat:
			m_zoneControlThermostat.removeNotReferencedLocalElements(); break;
		case DT_ZoneControlShading:
			m_zoneControlShading.removeNotReferencedLocalElements(); break;
		case DT_ZoneControlNaturalVentilation:
			m_zoneControlVentilationNatural.removeNotReferencedLocalElements(); break;
		case DT_ZoneIdealHeatingCooling:
			m_zoneIdealHeatingCooling.removeNotReferencedLocalElements(); break;
		case DT_VentilationNatural:
			m_ventilationNatural.removeNotReferencedLocalElements(); break;
		case DT_Infiltration:
			m_infiltration.removeNotReferencedLocalElements(); break;
		case DT_ZoneTemplates:
			m_zoneTemplates.removeNotReferencedLocalElements(); break;
		case NUM_DT:
			break;
	}
}


const VICUS::AbstractDBElement * SVDatabase::lookupSubTemplate(VICUS::ZoneTemplate::SubTemplateType st, const IDType idReferenceArray[]) const {
	IBK_ASSERT(st < VICUS::ZoneTemplate::NUM_ST);
	unsigned int id = idReferenceArray[st];
	if (id == VICUS::INVALID_ID)
		return nullptr;

	switch (st) {
		case VICUS::ZoneTemplate::ST_IntLoadPerson:
		case VICUS::ZoneTemplate::ST_IntLoadEquipment:
		case VICUS::ZoneTemplate::ST_IntLoadLighting:
		case VICUS::ZoneTemplate::ST_IntLoadOther:					return m_internalLoads[id];
		case VICUS::ZoneTemplate::ST_ControlThermostat:				return m_zoneControlThermostat[id];
		case VICUS::ZoneTemplate::ST_ControlShading:				return m_zoneControlShading[id];
		case VICUS::ZoneTemplate::ST_ControlVentilationNatural:		return m_zoneControlVentilationNatural[id];
		case VICUS::ZoneTemplate::ST_Infiltration:					return m_infiltration[id];
		case VICUS::ZoneTemplate::ST_VentilationNatural:			return m_ventilationNatural[id];
		case VICUS::ZoneTemplate::ST_IdealHeatingCooling:			return m_zoneIdealHeatingCooling[id];
		case VICUS::ZoneTemplate::NUM_ST:							; // just to make compiler happy
	}

	return nullptr;
}


void SVDatabase::findLocalChildren(DatabaseTypes dbType, unsigned int id,
								   std::set<VICUS::AbstractDBElement *> &localChildren) {

	updateElementChildren();

	Q_ASSERT(id!=VICUS::INVALID_ID);

	switch (dbType) {
		case DT_Materials:
			Q_ASSERT(m_materials[id] != nullptr);
			m_materials[id]->collectLocalChildren(localChildren); break;
		case DT_Constructions:
			Q_ASSERT(m_constructions[id] != nullptr);
			m_constructions[id]->collectLocalChildren(localChildren); break;
		case DT_Windows:
			Q_ASSERT(m_windows[id] != nullptr);
			m_windows[id]->collectLocalChildren(localChildren); break;
		case DT_WindowGlazingSystems:
			Q_ASSERT(m_windowGlazingSystems[id] != nullptr);
			m_windowGlazingSystems[id]->collectLocalChildren(localChildren); break;
		case DT_BoundaryConditions:
			Q_ASSERT(m_boundaryConditions[id] != nullptr);
			m_boundaryConditions[id]->collectLocalChildren(localChildren); break;
		case DT_Components:
			Q_ASSERT(m_components[id] != nullptr);
			m_components[id]->collectLocalChildren(localChildren); break;
		case DT_SubSurfaceComponents:
			Q_ASSERT(m_subSurfaceComponents[id] != nullptr);
			m_subSurfaceComponents[id]->collectLocalChildren(localChildren); break;
		case DT_SurfaceHeating:
			Q_ASSERT(m_surfaceHeatings[id] != nullptr);
			m_surfaceHeatings[id]->collectLocalChildren(localChildren); break;
		case DT_Pipes:
			Q_ASSERT(m_pipes[id] != nullptr);
			m_pipes[id]->collectLocalChildren(localChildren); break;
		case DT_Fluids:
			Q_ASSERT(m_fluids[id] != nullptr);
			m_fluids[id]->collectLocalChildren(localChildren); break;
		case DT_SubNetworks:
			Q_ASSERT(m_subNetworks[id] != nullptr);
			m_subNetworks[id]->collectLocalChildren(localChildren); break;
		case DT_SupplySystems:
			Q_ASSERT(m_supplySystems[id] != nullptr);
			m_supplySystems[id]->collectLocalChildren(localChildren); break;
		case DT_NetworkControllers:
			Q_ASSERT(m_networkControllers[id] != nullptr);
			m_networkControllers[id]->collectLocalChildren(localChildren);  break;
		case DT_NetworkComponents:
			Q_ASSERT(m_networkComponents[id] != nullptr);
			m_networkComponents[id]->collectLocalChildren(localChildren); break;
		case DT_Schedules:
			Q_ASSERT(m_schedules[id] != nullptr);
			m_schedules[id]->collectLocalChildren(localChildren); break;
		case DT_InternalLoads:
			Q_ASSERT(m_internalLoads[id] != nullptr);
			m_internalLoads[id]->collectLocalChildren(localChildren); break;
		case DT_ZoneControlThermostat:
			Q_ASSERT(m_zoneControlThermostat[id] != nullptr);
			m_zoneControlThermostat[id]->collectLocalChildren(localChildren); break;
		case DT_ZoneControlShading:
			Q_ASSERT(m_zoneControlShading[id] != nullptr);
			m_zoneControlShading[id]->collectLocalChildren(localChildren); break;
		case DT_ZoneControlNaturalVentilation:
			Q_ASSERT(m_zoneControlVentilationNatural[id] != nullptr);
			m_zoneControlVentilationNatural[id]->collectLocalChildren(localChildren); break;
		case DT_ZoneIdealHeatingCooling:
			Q_ASSERT(m_zoneIdealHeatingCooling[id] != nullptr);
			m_zoneIdealHeatingCooling[id]->collectLocalChildren(localChildren); break;
		case DT_VentilationNatural:
			Q_ASSERT(m_ventilationNatural[id] != nullptr);
			m_ventilationNatural[id]->collectLocalChildren(localChildren); break;
		case DT_Infiltration:
			Q_ASSERT(m_infiltration[id] != nullptr);
			m_infiltration[id]->collectLocalChildren(localChildren); break;
		case DT_ZoneTemplates:
			Q_ASSERT(m_zoneTemplates[id] != nullptr);
			m_zoneTemplates[id]->collectLocalChildren(localChildren); break;
		case NUM_DT:
			break;
	}
}



