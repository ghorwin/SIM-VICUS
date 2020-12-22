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
	m_components(USER_ID_SPACE_START),
	m_pipes(USER_ID_SPACE_START),
	m_fluids(USER_ID_SPACE_START),
	m_EPDElements(USER_ID_SPACE_START)
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
		m_pipes.readXML(			dbDir / "db_pipes.xml", "Pipes", "Pipe", true);
		m_fluids.readXML(			dbDir / "db_fluids.xml", "Fluids", "Fluid", true);
		m_components.readXML(		dbDir / "db_components.xml", "Components", "Component", true);

	//	readXMLDB(dbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", m_dbWindowGlazingSystems, true);
	//	readXMLDB(dbDir / "db_surfaceProperties.xml", "SurfaceProperties", "SurfaceProperty", m_dbSurfaceProperty, true);
	//	readXMLDB(dbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", m_dbBoundaryCondition, true);
	//	readXML(dbDir / "db_epdElements.xml", "EPDDatasets", "EPDDataset", m_dbEPDElements, true);
	}

	// user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

	if (t == NUM_DT || t == DT_Materials)
		m_materials.readXML(		userDbDir / "db_materials.xml", "Materials", "Material", false);
	if (t == NUM_DT || t == DT_Constructions)
		m_constructions.readXML(	userDbDir / "db_constructions.xml", "Constructions", "Construction", false);
	if (t == NUM_DT || t == DT_Components)
		m_components.readXML(	userDbDir / "db_components.xml", "Components", "Component", false);
	m_windows.readXML(			userDbDir / "db_windows.xml", "Windows", "Window", false);
	m_pipes.readXML(			userDbDir / "db_pipes.xml", "Pipes", "Pipe", false);
	m_fluids.readXML(			userDbDir / "db_fluids.xml", "Fluids", "Fluid", false);

//	readXMLDB(userDbDir / "db_windowGlazingSystems.xml", "WindowGlazingSystems", "WindowGlazingSystem", m_dbWindowGlazingSystems);
//	readXMLDB(userDbDir / "db_surfaceProperties.xml", "SurfaceProperties", "SurfaceProperty", m_dbSurfaceProperty);
//	readXMLDB(userDbDir / "db_boundaryConditions.xml", "BoundaryConditions", "BoundaryCondition", m_dbBoundaryCondition);
//	readXMLDB(userDbDir / "db_epdElements.xml", "EPDDatasets", "EPDDataset", m_dbEPDElements);
}


void SVDatabase::writeDatabases() const {
	// we only write user databases

	IBK::Path userDbDir(QtExt::Directories::userDataDir().toStdString());

#if 0
	// create some dummy materials to write out
	VICUS::Material m;
	m.m_id = 100000;
	m.m_category = VICUS::Material::MC_Bricks;
	m.m_dataSource = "SimQuality";
	m.m_manufacturer = "generic";
	m.m_color = "#800000";
	m.m_notes = "en:Massiv contrete-type material used in SimQuality test cases.|de:Massives, Beton-ähnliches Material zur Verwendung in SimQuality.";
	m.m_displayName = "en:Concrete|de:Beton";
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Density, 2000);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_HeatCapacity, 1000);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Conductivity, 1.2);

	const_cast<Database*>(this)->m_materials.add(m); // const-cast needed to trick modification of DB here
#endif

#if 0
	VICUS::Construction c;
	c.m_id = 10000;
	c.m_usageType = VICUS::Construction::UT_OutsideWall;
	c.m_color = QColor("0x800000");
	c.m_notes = "blabal";
	c.m_dataSource = "IBK, generic";
	c.m_displayName = "Heavy metal wall";
	c.m_materialLayers.resize(1);
	c.m_materialLayers[0].m_matId = 10000;
	c.m_materialLayers[0].m_isActive = true;
	c.m_materialLayers[0].m_thickness.set("Thickness", 0.2, "m");
	const_cast<Database*>(this)->m_constructions.add(c); // const-cast needed to trick modification of DB here
#endif

	m_materials.writeXML(		userDbDir / "db_materials.xml", "Materials");
	m_windows.writeXML(			userDbDir / "db_windows.xml", "Windows");
	m_constructions.writeXML(	userDbDir / "db_constructions.xml", "Constructions");
	m_pipes.writeXML(			userDbDir / "db_pipes.xml", "Pipes");
	m_fluids.writeXML(			userDbDir / "db_fluids.xml", "Fluids");
	m_components.writeXML(		userDbDir / "db_components.xml", "Components");

//	writeXMLDB(userDbDir / "db_surfaceProperties.xml", "SurfaceProperties", m_dbSurfaceProperty);
//	writeXMLDB(userDbDir / "db_boundaryConditions.xml", "BoundaryConditions", m_dbBoundaryCondition);
//	writeXMLDB(userDbDir / "db_epdElements.xml", "EPDDatasets", m_dbEPDElements);
}


