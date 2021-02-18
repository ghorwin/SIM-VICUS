#include "SVDatabase.h"

#include <IBK_messages.h>
#include <IBK_FormatString.h>

#include <VICUS_KeywordList.h>

#include <QtExt_Directories.h>

const unsigned int USER_ID_SPACE_START = 100000;


SVDatabase::SVDatabase() :
	m_materials(USER_ID_SPACE_START),
	m_constructions(USER_ID_SPACE_START),
	m_windows(USER_ID_SPACE_START),
	m_boundaryConditions(USER_ID_SPACE_START),
	m_components(USER_ID_SPACE_START),
	m_pipes(USER_ID_SPACE_START),
	m_fluids(USER_ID_SPACE_START),
	m_networkComponents(USER_ID_SPACE_START),
	m_EPDElements(USER_ID_SPACE_START),
	m_schedules(USER_ID_SPACE_START)
{
}


void SVDatabase::readDatabases(DatabaseTypes t) {
	// built-in databases

	// built-in dbs are only read when no filter is applied (i.e. general initialization)
	if (t == NUM_DT) {
		IBK::Path dbDir(QtExt::Directories::databasesDir().toStdString());

		m_materials.readXML(		dbDir / "db_materials.xml", "Materials", "Material", true);
		m_constructions.readXML(	dbDir / "db_constructions.xml", "Constructions", "Construction", true);
		m_windows.readXML(			dbDir / "db_windows.xml", "Windows", "Window", true);
		m_boundaryConditions.readXML(dbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", true);
		m_components.readXML(		dbDir / "db_components.xml", "Components", "Component", true);
		m_pipes.readXML(			dbDir / "db_pipes.xml", "NetworkPipes", "NetworkPipe", true);
		m_fluids.readXML(			dbDir / "db_fluids.xml", "NetworkFluids", "NetworkFluid", true);
		m_networkComponents.readXML(dbDir / "db_networkComponents.xml", "NetworkComponents", "NetworkComponent", true);
		m_schedules.readXML(		dbDir / "db_schedules.xml", "Schedules", "Schedule", true);

	//	readXML(dbDir / "db_epdElements.xml", "EPDDatasets", "EPDDataset", m_dbEPDElements, true);
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

//	readXMLDB(userDbDir / "db_epdElements.xml", "EPDDatasets", "EPDDataset", m_dbEPDElements);
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
	m_networkComponents.writeXML(	userDbDir / "db_schedules.xml", "Schedules");

//	writeXMLDB(userDbDir / "db_epdElements.xml", "EPDDatasets", m_dbEPDElements);
}


