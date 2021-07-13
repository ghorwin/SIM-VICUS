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

const unsigned int USER_ID_SPACE_START = 10000;


SVDatabase::SVDatabase() :
	m_materials(USER_ID_SPACE_START),
	m_constructions(2*USER_ID_SPACE_START),
	m_windows(3*USER_ID_SPACE_START),
	m_windowGlazingSystems(30*USER_ID_SPACE_START),
	m_boundaryConditions(4*USER_ID_SPACE_START),
	m_components(5*USER_ID_SPACE_START),
	m_subSurfaceComponents(16*USER_ID_SPACE_START),
	m_surfaceHeatings(35*USER_ID_SPACE_START),
	m_pipes(USER_ID_SPACE_START*100),
	m_fluids(USER_ID_SPACE_START*1001),
	m_networkComponents(USER_ID_SPACE_START*1002),
	m_networkControllers(USER_ID_SPACE_START*1003),
	m_subNetworks(USER_ID_SPACE_START*1004),
	m_EPDElements(USER_ID_SPACE_START),
	m_schedules(6*USER_ID_SPACE_START),
	m_internalLoads(7*USER_ID_SPACE_START),
	m_zoneControlThermostat(11*USER_ID_SPACE_START),
	m_zoneControlShading(13*USER_ID_SPACE_START),
	m_zoneControlVentilationNatural(12*USER_ID_SPACE_START),
	m_zoneIdealHeatingCooling(20*USER_ID_SPACE_START),
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
		m_zoneControlVentilationNatural.readXML(	dbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals", "ZoneControlVentilationNatural", true);
		m_zoneControlShading.readXML(		dbDir / "db_zoneControlShading.xml", "ZoneControlShadings", "ZoneControlShading", true);
		m_zoneIdealHeatingCooling.readXML(	dbDir / "db_zoneIdealHeatingCooling.xml", "ZoneIdealHeatingCoolings", "ZoneIdealHeatingCooling", true);
		m_infiltration.readXML(				dbDir / "db_infiltration.xml", "Infiltrations", "Infiltration", true);
		m_ventilationNatural.readXML(		dbDir / "db_ventilationNatural.xml", "VentilationNaturals", "VentilationNatural", true);
		m_zoneTemplates.readXML(			dbDir / "db_zoneTemplates.xml", "ZoneTemplates", "ZoneTemplate", true);

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
	if (t == NUM_DT || t == DT_Schedules)
		m_schedules.readXML(				userDbDir / "db_schedules.xml", "Schedules", "Schedule", false);
	if (t == NUM_DT || t == DT_InternalLoads)
		m_internalLoads.readXML(			userDbDir / "db_internalLoads.xml", "InternalLoads", "InternalLoad", false);
	if (t == NUM_DT || t == DT_ZoneControlThermostat)
		m_zoneControlThermostat.readXML(	userDbDir / "db_zoneControlThermostat.xml", "ZoneControlThermostats", "ZoneControlThermostat", false);
	if (t == NUM_DT || t == DT_ZoneControlNaturalVentilation)
		m_zoneControlVentilationNatural.readXML(	userDbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals", "ZoneControlVentilationNatural", false);
	if (t == NUM_DT || t == DT_ZoneControlShading)
		m_zoneControlShading.readXML(		userDbDir / "db_zoneControlShading.xml", "ZoneControlShadings", "ZoneControlShading", false);
	if (t == NUM_DT || t == DT_ZoneIdealHeatingCooling)
		m_zoneIdealHeatingCooling.readXML(	userDbDir / "db_zoneIdealHeatingCooling.xml", "ZoneIdealHeatingCoolings", "ZoneIdealHeatingCooling", false);
	if (t == NUM_DT || t == DT_Infiltration)
		m_infiltration.readXML(				userDbDir / "db_infiltration.xml", "Infiltrations", "Infiltration", false);
	if (t == NUM_DT || t == DT_VentilationNatural)
		m_ventilationNatural.readXML(		userDbDir / "db_ventilationNatural.xml", "VentilationNaturals", "VentilationNatural", false);
	if (t == NUM_DT || t == DT_ZoneTemplates)
		m_zoneTemplates.readXML(			userDbDir / "db_zoneTemplates.xml", "ZoneTemplates", "ZoneTemplate", false);
}


void SVDatabase::writeDatabases() const {
	// we only write user databases

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
	m_zoneControlVentilationNatural.writeXML(userDbDir / "db_zoneControlVentilationNatural.xml", "ZoneControlVentilationNaturals");
	m_zoneControlShading.writeXML(	userDbDir / "db_zoneControlShading.xml", "ZoneControlShadings");
	m_zoneIdealHeatingCooling.writeXML(	userDbDir / "db_zoneIdealHeatingCooling.xml", "ZoneIdealHeatingCoolings");
	m_infiltration.writeXML(		userDbDir / "db_infiltration.xml", "Infiltrations");
	m_ventilationNatural.writeXML(	userDbDir / "db_ventilationNatural.xml", "VentilationNaturals");
	m_zoneTemplates.writeXML(		userDbDir / "db_zoneTemplates.xml", "ZoneTemplates");

}


template <typename T>
void storeVector(std::vector<T> & vec, const std::set<const T*> & container) {
	// clear target vector
	vec.clear();
	// store objects but skip nullptr
	for (const T * c : container)
		if (c != nullptr) vec.push_back(*c);
}

void SVDatabase::updateEmbeddedDatabase(VICUS::Project & p) {

	// create sets for objects that are referenced from other objects
	std::set<const VICUS::Material *>				referencedMaterials;
	std::set<const VICUS::Construction *>			referencedConstructions;
	std::set<const VICUS::Window *>					referencedWindows;
	std::set<const VICUS::WindowGlazingSystem *>	referencedGlazingSystems;
	std::set<const VICUS::BoundaryCondition *>		referencedBC;
	std::set<const VICUS::Component *>				referencedComponents;
	std::set<const VICUS::SubSurfaceComponent *>	referencedSubSurfaceComponents;
	std::set<const VICUS::SurfaceHeating *>			referencedSurfaceHeatings;
	std::set<const VICUS::Schedule *>				referencedSchedule;
	std::set<const VICUS::InternalLoad *>			referencedInternalLoads;
	std::set<const VICUS::ZoneControlThermostat *>	referencedThermostats;
	std::set<const VICUS::ZoneIdealHeatingCooling *> referencedIdealHeatCool;
	std::set<const VICUS::VentilationNatural *>		referencedVentilation;
	std::set<const VICUS::Infiltration *>			referencedInfiltration;
	std::set<const VICUS::ZoneTemplate *>			referencedZoneTemplates;

	std::set<const VICUS::NetworkPipe*>				referencedNetworkPipes;
	std::set<const VICUS::NetworkComponent *>		referencedNetworkComponents;
	std::set<const VICUS::NetworkController *>		referencedNetworkControllers;
	std::set<const VICUS::SubNetwork *>				referencedSubNetworks;
	std::set<const VICUS::NetworkFluid *>			referencedNetworkFluids;


	// we first collect all objects that are not referenced themselves
	// then, we collect objects that are referenced from an already collected object
	// this is continued until we have collected all objects that are used somewhere in the
	// project


	// *** components ***

	for (VICUS::ComponentInstance & ci : p.m_componentInstances) {
		const VICUS::Component * c = m_components[ci.m_componentID]; // bad/missing IDs yield a nullptr
		if (c == nullptr) continue;

		referencedComponents.insert(c);

		// *** everything referenced from components: constructions, bc, ... ***

		referencedConstructions.insert(m_constructions[c->m_idConstruction]); // bad/missing IDs yield a nullptr
		referencedBC.insert(m_boundaryConditions[c->m_idSideABoundaryCondition]); // bad/missing IDs yield a nullptr
		referencedBC.insert(m_boundaryConditions[c->m_idSideBBoundaryCondition]); // bad/missing IDs yield a nullptr

		// do we have surface heating referenced?
		referencedSurfaceHeatings.insert(m_surfaceHeatings[ci.m_surfaceHeatingID]);
	}

	// *** sub-surface components ***
	for (VICUS::SubSurfaceComponentInstance & ci : p.m_subSurfaceComponentInstances) {
		const VICUS::SubSurfaceComponent * c = m_subSurfaceComponents[ci.m_subSurfaceComponentID]; // bad/missing IDs yield a nullptr
		if (c == nullptr) continue;

		referencedSubSurfaceComponents.insert(c);

		// collect referenced windows
		const VICUS::Window * w = m_windows[c->m_idWindow];
		if (w == nullptr) continue;

		referencedWindows.insert(w);
		referencedGlazingSystems.insert(m_windowGlazingSystems[w->m_idGlazingSystem]); // bad/missing IDs yield a nullptr
		referencedBC.insert(m_boundaryConditions[c->m_idSideABoundaryCondition]); // bad/missing IDs yield a nullptr
		referencedBC.insert(m_boundaryConditions[c->m_idSideBBoundaryCondition]); // bad/missing IDs yield a nullptr
	}


	// *** zone templates ***

	for (const VICUS::Building & b : p.m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room &  r : bl.m_rooms)
				referencedZoneTemplates.insert(m_zoneTemplates[r.m_idZoneTemplate]);	// bad/missing IDs yield a nullptr
	referencedZoneTemplates.erase(nullptr);

	// *** materials ***
	//
	// referenced from constructions and window (frame+divider)

	for (const VICUS::Construction * c : referencedConstructions) {
		for (const VICUS::MaterialLayer & ml : c->m_materialLayers)
			referencedMaterials.insert(m_materials[ml.m_matId]); // bad/missing IDs yield a nullptr
	}
	for (const VICUS::Window * c : referencedWindows) {
		referencedMaterials.insert(m_materials[c->m_frame.m_idMaterial]); // bad/missing IDs yield a nullptr
		referencedMaterials.insert(m_materials[c->m_divider.m_idMaterial]); // bad/missing IDs yield a nullptr
	}


	// *** everything that is referenced from zone templates

	// TODO Dirk, Review and check for completeness!

	for (const VICUS::ZoneTemplate * zt : referencedZoneTemplates) {
		for (unsigned int i=0; i<VICUS::ZoneTemplate::NUM_ST; ++i){
			IDType idType = zt->m_idReferences[i];
			const VICUS::InternalLoad *intLoad = m_internalLoads[idType];
			const VICUS::ZoneControlThermostat * thermo = m_zoneControlThermostat[idType];
			const VICUS::ZoneIdealHeatingCooling * idealHeatCool = m_zoneIdealHeatingCooling[idType];
			const VICUS::Infiltration *inf = m_infiltration[idType];
			const VICUS::VentilationNatural *ventiNat = m_ventilationNatural[idType];
			if (intLoad	!= nullptr){
				referencedInternalLoads.insert(intLoad);
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
					case VICUS::ZoneTemplate::ST_IntLoadLighting:
					case VICUS::ZoneTemplate::ST_IntLoadOther:{
						///TODO Katja
						const VICUS::Schedule *sched = m_schedules[intLoad->m_powerManagementScheduleId];
						if(sched != nullptr)
							referencedSchedule.insert(sched);
					}	break;
					case VICUS::ZoneTemplate::NUM_ST:
					break;

				}
			}
			else if(thermo != nullptr){
				referencedThermostats.insert(thermo);

				const VICUS::Schedule *sched = m_schedules[thermo->m_heatingSetpointScheduleId];
				if(sched != nullptr)
					referencedSchedule.insert(sched);
				const VICUS::Schedule * sched2 = m_schedules[thermo->m_coolingSetpointScheduleId];
				if(sched2 != nullptr)
					referencedSchedule.insert(sched2);
			}
			else if (inf != nullptr){
				referencedInfiltration.insert(inf);
			}
			else if (ventiNat != nullptr){
				referencedVentilation.insert(ventiNat);
				const VICUS::Schedule *sched = m_schedules[ventiNat->m_scheduleId];
				if(sched != nullptr)
					referencedSchedule.insert(sched);
			}
			else if(idealHeatCool != nullptr)
				referencedIdealHeatCool.insert(idealHeatCool);
		}
	}


	// iterate through networks
	for (const VICUS::Network &net: p.m_geometricNetworks){

		// fluids
		const VICUS::NetworkFluid * f = m_fluids[net.m_fluidID];
		if (f != nullptr)
			referencedNetworkFluids.insert(f);

		// pipes
		for (const VICUS::NetworkEdge &edge: net.m_edges){
			const VICUS::NetworkPipe * p = m_pipes[edge.m_pipeId];
			if (p != nullptr)
				referencedNetworkPipes.insert(p);
		}

		// sub networks
		for (const VICUS::NetworkNode &node: net.m_nodes){
			const VICUS::SubNetwork * sub = m_subNetworks[node.m_subNetworkId];
			if (sub != nullptr)
				referencedSubNetworks.insert(sub);
		}
	}

	// iterate through collected sub networks
	for (const VICUS::SubNetwork *sub: referencedSubNetworks){
		for (const NANDRAD::HydraulicNetworkElement &el: sub->m_elements){

		// network components
		const VICUS::NetworkComponent * comp = m_networkComponents[el.m_componentId];
		if (comp != nullptr)
			referencedNetworkComponents.insert(comp);

		// network controllers
		const VICUS::NetworkController * ctrl = m_networkControllers[el.m_controlElementId];
		if (ctrl != nullptr)
			referencedNetworkControllers.insert(ctrl);
		}
	}

	// iterate through components
	for (const VICUS::NetworkComponent *comp: referencedNetworkComponents){

		// schedules
		for (unsigned int i: comp->m_scheduleIds){
			const VICUS::Schedule *sched = m_schedules[i];
			if (sched != nullptr)
				referencedSchedule.insert(sched);
		}
	}

	// *** transfer collected objects to project's embedded database ***

	storeVector(p.m_embeddedDB.m_materials, referencedMaterials);
	storeVector(p.m_embeddedDB.m_constructions, referencedConstructions);
	storeVector(p.m_embeddedDB.m_windows, referencedWindows);
	storeVector(p.m_embeddedDB.m_windowGlazingSystems, referencedGlazingSystems);
	storeVector(p.m_embeddedDB.m_boundaryConditions, referencedBC);
	storeVector(p.m_embeddedDB.m_components, referencedComponents);
	storeVector(p.m_embeddedDB.m_subSurfaceComponents, referencedSubSurfaceComponents);

	storeVector(p.m_embeddedDB.m_networkComponents, referencedNetworkComponents);
	storeVector(p.m_embeddedDB.m_fluids, referencedNetworkFluids);
	storeVector(p.m_embeddedDB.m_pipes, referencedNetworkPipes);
	storeVector(p.m_embeddedDB.m_networkControllers, referencedNetworkControllers);
	storeVector(p.m_embeddedDB.m_subNetworks, referencedSubNetworks);


	// TODO m_EPDElements

	storeVector(p.m_embeddedDB.m_schedules, referencedSchedule);
	storeVector(p.m_embeddedDB.m_internalLoads, referencedInternalLoads);
	storeVector(p.m_embeddedDB.m_zoneControlThermostats, referencedThermostats);


	// TODO m_zoneControlShadings

	storeVector(p.m_embeddedDB.m_zoneIdealHeatingCooling, referencedIdealHeatCool);

	// TODO m_zoneControlVentilationNaturals

	storeVector(p.m_embeddedDB.m_ventilationNatural, referencedVentilation);
	storeVector(p.m_embeddedDB.m_infiltration, referencedInfiltration);
	storeVector(p.m_embeddedDB.m_zoneTemplates, referencedZoneTemplates);
	storeVector(p.m_embeddedDB.m_surfaceHeatings, referencedSurfaceHeatings);

}


