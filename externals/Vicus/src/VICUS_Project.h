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

#ifndef VICUS_ProjectH
#define VICUS_ProjectH

#include <QCoreApplication> // for tr functions

#include <vector>

#include <IBK_Path.h>

#include <NANDRAD_ProjectInfo.h>
#include <NANDRAD_Location.h>
#include <NANDRAD_SolverParameter.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Interface.h>
#include <NANDRAD_FMIDescription.h>
#include <NANDRAD_Schedules.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Network.h"
#include "VICUS_Building.h"
#include "VICUS_ViewSettings.h"
#include "VICUS_Outputs.h"
#include "VICUS_LcaSettings.h"
#include "VICUS_LccSettings.h"
#include "VICUS_ComponentInstance.h"
#include "VICUS_SubSurfaceComponentInstance.h"
#include "VICUS_EmbeddedDatabase.h"
#include "VICUS_PlainGeometry.h"
#include "VICUS_Drawing.h"


namespace IBK {
	class NotificationHandler;
}

namespace VICUS {

/*! The project data structure for the SIM-VICUS user interface.
*/
class Project {
	Q_DECLARE_TR_FUNCTIONS(Project)

	/*! Private read-write functions. */
	VICUS_READWRITE
public:

	/*! Bitmasks for different selection groups used in selectObjects(). */
	enum SelectionGroups {
		SG_Building			= 0x001,
		SG_Network			= 0x002,
		SG_Obstacle			= 0x004,
		SG_Drawing			= 0x008,
		SG_All				= SG_Building | SG_Network | SG_Obstacle | SG_Drawing,
		NUM_SG
	};

	/*! Sensor types for dynamic shading control. */
	enum SenorType {
		ST_Horizontal,
		ST_North,
		ST_East,
		ST_South,
		ST_West,
		NUM_ST
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Constructor, creates dummy data. */
	Project();

	/*! Generate default copy constructor. */
	Project(const Project &) = default;

	/*! Parses only the header of the XML file.
		This function is supposed to be fast, yet not a complete XML parser.
		\param filename  The full path to the project file.
	*/
	void parseHeader(const IBK::Path & filename);

	/*! Reads the project data from an XML file.
		\param filename  The full path to the project file.
	*/
	void readXML(const IBK::Path & filename);

	/*! Reads the additional drawing data from an XML file.
		\param filename  The full path to the drawing file.
	*/
	void readDrawingXML(const IBK::Path & filename);

	/*! Reads the project data from an text which contains XML.
		\param projectText  Text with VICUS project.
	*/
	void readImportedXML(const QString & projectText, IBK::NotificationHandler *notifyer);

	/*! Actual read function, called from both variants of readXML(). */
	void readXMLDocument(TiXmlElement * rootElement);

	/*! Writes the project file to an XML file.
		\param filename  The full path to the project file.
	*/
	void writeXML(const IBK::Path & filename) const;

	void writeDrawingXML(const IBK::Path & filename) const;

	/*! Reads the placeholder section into m_placeholders map. */
	void readDirectoryPlaceholdersXML(const TiXmlElement * element);

	/*! Writes the section with directory place holders. */
	void writeDirectoryPlaceholdersXML(TiXmlElement * parent) const;

	/*! Call this function whenever project data has changed that depends on
		objects linked through pointers (building hierarchies, networks etc.).
		Checks interrelated definitions/references for validity.
		Throws IBK::Exceptions in case of errors.
		Call this function after reading a project and during debugging after
		project modifications to ensure data consistency.
	*/
	void updatePointers();

	/*! Adds child surfaces to pointers of project. */
	void addChildSurface(const VICUS::Surface &s);

	/*! Searches through all objects and determines the largest object ID (not unique ID!) used for buildings, buildingLevels, rooms, surface,
		subsurfaces, networks, etc and returns the next ID to be used for new data elements. For example, if IDs 10, 11, 14 have been used already,
		the function returns 15.
		\note Expects that updatePointers() has been called beforehand (i.e. project is synced)
	*/
	unsigned int nextUnusedID() const;

	/*! Searches through all objects in project structure for the object with the object ID.
		\return Returns nullptr, when object cannot be found.
	*/
	VICUS::Object * objectById(unsigned int id);

	/*! Searches through all objects in project structure for the object with the object ID.
		Const version of the above function.
		\return Returns nullptr, when object cannot be found.
	*/
	const VICUS::Object * objectById(unsigned int id) const {
		return const_cast<VICUS::Project*>(this)->objectById(id);
	}

	/*! Tries to find a room with given ID. A convenience function that uses objectById() internally. */
	VICUS::Room * roomByID(unsigned int roomID) {
		return dynamic_cast<VICUS::Room *>(objectById(roomID));
	}

	/*! Const-version of the function above. */
	const VICUS::Room * roomByID(unsigned int roomID) const {
		return dynamic_cast<const VICUS::Room *>(objectById(roomID));
	}

	/*! Tries to find a surface with given ID. A convenience function that uses objectById() internally. */
	VICUS::Surface * surfaceByID(unsigned int surfaceID) {
		return dynamic_cast<VICUS::Surface *>(objectById(surfaceID));
	}

	/*! Const-version of the function above. */
	const VICUS::Surface * surfaceByID(unsigned int surfaceID) const {
		return dynamic_cast<const VICUS::Surface *>(objectById(surfaceID));
	}

	/*! Tries to find a sub-surface with given ID. A convenience function that uses objectById() internally. */
	VICUS::SubSurface * subSurfaceByID(unsigned int surfID) {
		return dynamic_cast<VICUS::SubSurface *>(objectById(surfID));
	}

	/*! Const-version of the function above. */
	const VICUS::SubSurface * subSurfaceByID(unsigned int surfID) const {
		return dynamic_cast<const VICUS::SubSurface *>(objectById(surfID));
	}

	/*! Selects objects and return set with pointers according to additional filters.
		\param selectedObjs Here the pointers to selected objects are returned.
		\param sg			Selection group, that the object belongs to.
		\param takeSelected	If true, only objects with "selected" property enabled are taken. If false,
			selection property is ignored.
		\param takeVisible	If true, only objects with "visible" property enabled are taken. If false,
			visible property is ignored.
	*/
	void selectObjects(std::set<const Object *> &selectedObjs, SelectionGroups sg,
					   bool takeSelected,
					   bool takeVisible) const;

	/*! Select all child surfaces of surface. */
	void selectChildSurfaces(std::set<const Object*> &selectedObjs, const VICUS::Surface & s,
							 bool takeSelected, bool takeVisible) const;

	/*! This function collects the pointers to all selected sub surfaces.
		This is a convenience function which essentially does the same as selectObjects, but
		only returns visible and selected objects of type SubSurface.
		\returns Returns true if any sub surface is selected (same as subSurfaces.size() > 0).
	*/
	bool selectedSubSurfaces(std::vector<const SubSurface*> & subSurfaces, const VICUS::Project::SelectionGroups &sg) const;

	/*! This function collects the pointers to all selected surfaces.
		This is a convenience function which essentially does the same as selectObjects, but
		only returns visible and selected objects of type Surface.
		\returns Returns true if any surface is selected (same as surfaces.size() > 0).
	*/
	bool selectedSurfaces(std::vector<const Surface*> & surfaces, const VICUS::Project::SelectionGroups &sg) const;

	/*! This function collects the pointers to all selected rooms.
		This is a convenience function which essentially does the same as selectObjects, but
		only returns visible and selected objects of type Room.
		Type of wich should be returned can be set with
		\returns Returns true if any room is selected (same as rooms.size() > 0).
	*/
	bool selectedRooms(std::vector<const Room*> & rooms) const;

	QString newUniqueBuildingName(const QString & baseName) const;
	QString newUniqueBuildingLevelName(const QString & baseName) const;
	QString newUniqueRoomName(const QString & baseName) const;
	QString newUniqueSurfaceName(const QString & baseName) const;
	QString newUniqueSubSurfaceName(const QString & baseName) const;

	// *** PROJECT CONVERSION RELATED FUNCTIONS ***

	/*! Converts VICUS data structure into NANDRAD project file.
		This function throws an error message in case the conversion failed.
		\param p The NANDRAD project to be populated.
	*/
	void generateNandradProject(NANDRAD::Project & p, QStringList & errorStack, const std::string & nandradProjectPath) const;
//	void generateBuildingProjectData(NANDRAD::Project & p) const;
	void generateNetworkProjectData(NANDRAD::Project & p, QStringList & errorStack, const std::string & nandradProjectPath, unsigned int networkId) const;

	void generateHeatLoadExport();

	// *** STATIC FUNCTIONS ***

	/*! This function computes the global bounding box of all selected surfaces and the center point in global coordinates.
		\returns Returns the dimensions of the bounding box and its center point in argument 'center' in global coordinates.
	*/
	static IBKMK::Vector3D boundingBox(const std::vector<const Drawing *> &drawings,
									   const std::vector<const Surface *> &surfaces,
									   const std::vector<const SubSurface *> &subsurfaces,
									   IBKMK::Vector3D &center,
									   bool transformPoints = true);

	/*! This function computes the global bounding box of all selected edges & nodes and the center point in global coordinates.
		\returns Returns the dimensions of the bounding box and its center point in argument 'center' in global coordinates.
	*/
	static IBKMK::Vector3D boundingBox(const std::vector<const Drawing *> & drawings,
									   const std::vector<const NetworkEdge*> &edges,
									   const std::vector<const NetworkNode*> &nodes,
									   IBKMK::Vector3D &center);

	/*! This function computes the bounding box of all selected surfaces and the center point in LOCAL coordinates of
		the provided local coordinate system.
		\returns Returns the dimensions of the bounding box and its center point in argument 'center' in local coordinates.
	*/
	static IBKMK::Vector3D boundingBox(std::vector<const Drawing *> & drawings,
									   std::vector<const VICUS::Surface*> &surfaces,
									   std::vector<const VICUS::SubSurface*> &subsurfaces,
									   IBKMK::Vector3D &center,
									   const IBKMK::Vector3D &offset = IBKMK::Vector3D(0,0,0),
									   const IBKMK::Vector3D &xAxis = IBKMK::Vector3D(1,0,0),
									   const IBKMK::Vector3D &yAxis = IBKMK::Vector3D(0,1,0),
									   const IBKMK::Vector3D &zAxis = IBKMK::Vector3D(0,0,1));

	/*! Attempts to create new surface-surface connections based on the current selection.
		Newly created component instances are stored in vector newComponentInstances alongside
		original component instances.
		Existing component instances that reference now connected surfaces are removed. The
		component is selected based on existing component instances.
	*/
	static bool connectSurfaces(double maxDist, double maxAngle, const std::set<const VICUS::Surface*> & selectedSurfaces,
								std::vector<VICUS::ComponentInstance> & newComponentInstances);


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Project info tag is manually written (not by code generator), to be located directly under root-node
		and not inside Project node.
	*/
	NANDRAD::ProjectInfo								m_projectInfo;

	/*! Solver parameters for NANDRAD. */
	NANDRAD::SolverParameter							m_solverParameter;			// XML:E
	/*! Simulation parameters for NANDRAD. */
	NANDRAD::SimulationParameter						m_simulationParameter;		// XML:E
	/*! Location and climate settings for NANDRAD. */
	NANDRAD::Location									m_location;					// XML:E

	/*! Output definitions. */
	VICUS::Outputs										m_outputs;					// XML:E

	ViewSettings										m_viewSettings;				// XML:E

	LcaSettings											m_lcaSettings;				// XML:E

	LccSettings											m_lccSettings;				// XML:E

	std::vector<Network>								m_geometricNetworks;		// XML:E

	std::vector<Building>								m_buildings;	 			// XML:E

	/*! All components actually placed in the geometry.
		This vector is outside buildings, so that two building parts can be connected with
		a component.
	*/
	std::vector<ComponentInstance>						m_componentInstances;				// XML:E
	std::vector<SubSurfaceComponentInstance>			m_subSurfaceComponentInstances;		// XML:E

	/*! Vector with plain (dumb) geometry. */
	PlainGeometry										m_plainGeometry;			// XML:E

	std::vector<Drawing>								m_drawings;


	/*! Path placeholder mappings used to substitute placeholders for database and user databases.
		These placeholders are read from the path placeholders section of the project file and hold
		a full directory path for a given placeholder name. You can compose a placeholder yourself:
		\code
		m_placeholders["Database"] = "/path/to/databases";
		// and then use this to resolve a relative path defined in the project file as
		string relative_path = "${Database}/db_materials.xml";
		\endcode
	*/
	std::map< std::string, IBK::Path >					m_placeholders;


	/*! Holds the database elements referenced in the project. These are a copy of db elements
		in the built-in and user-database and stored for project exchange between computers.
	*/
	EmbeddedDatabase									m_embeddedDB;				// XML:E


	/*! Definitions for exporting an FMU from the model. */
	NANDRAD::FMIDescription								m_fmiDescription;			// XML:E

	/*! Contains a file name for a IFC model in case of import from IFC. */
	IBK::Path											m_ifcFilePath;				// XML:E

	/*! Contains a file name for a drawing file. */
	IBK::Path											m_drawingFilePath;			// XML:E


	/*! Mapping element holds the room data for later export. */
	struct RoomMapping {
		unsigned int							m_idBuildingVicus;
		unsigned int							m_idBuildingLevelVicus;
		unsigned int							m_idRoomVicus;
		unsigned int							m_idRoomNandrad;
		unsigned int							m_idZoneTemplateVicus;
		std::string								m_nameBuildingVicus;
		std::string								m_nameBuildingLevelVicus;
		std::string								m_nameRoomVicus;
		std::string								m_nameRoomNandrad;
		std::string								m_zonetemplateName;
		double									m_floorArea = 0;		// in m2
		double									m_volume = 0;			// in m3
	};



private:
	// Functions below are implemented in VICUS_ProjectGenerator.cpp

	void generateBuildingProjectData(const QString &modelName,
									 NANDRAD::Project & p, QStringList & errorStack,
									 std::map<unsigned int, unsigned int> &surfaceIdsVicusToNandrad,
									 std::vector<RoomMapping> &roomMappings,  std::map<unsigned int, unsigned int> componentInstanceMapping)const;

	void generateNandradZones(std::vector<const VICUS::Room *> & zones, std::set<unsigned int> & idSet,
							  NANDRAD::Project & p, QStringList & errorStack,
							  std::vector<RoomMapping> &mappings)const;

	/*! Adds a vicus schedule to nandrad project. */
	void addVicusScheduleToNandradProject(const VICUS::Schedule &schedVic, const std::string &scheduleQuantityName,
									 NANDRAD::Schedules & schedules, const std::string &objListName)const;

	/*! If available, reads a shading factor file and write the corresponding NANDRAD shading factors file using NANDRAD id's.
		\param surfaceIdsVicusToNandrad Maps vicus IDs to NANDRAD ids, like nandradID = surfaceIdsVicusToNandrad[vicusID];
		\param projectFilePath Full path to NANDRAD project file (used to generated path to shading factor file).
		\param shadingFactorFilePath If function returns successfully, this path contains the full file path to the generated shading factors file.
		\return Returns true on success, false on error. In case of error an appropriate error message has been written to log.
	*/
	bool generateShadingFactorsFile(const std::map<unsigned int, unsigned int> & surfaceIdsVicusToNandrad,
									const IBK::Path &projectFilePath, IBK::Path & shadingFactorFilePath) const;

	/*! Export mapping table for VICUS and NANDRAD room ids and names.
		Also export zone template ids and names.
	*/
	bool exportMappingTable(const IBK::Path & filepath, const std::vector<RoomMapping> &mappings,
							bool addFloorAreaAndVolume = false) const;

	/*! Export for each room VICUS and NANDRAD name, floor area [m2] and volume [m3] */
	bool exportAreaAndVolume();

	/*! Adds the given object to the m_objectPtr map but first checks if its ID is already in the map.
		If there is an ID conflict, the function throws an IBK::Exception.
	*/
	void addAndCheckForUniqueness(VICUS::Object* o);

	/*! Adds view factors to the nandrad project.
		If there are conflicts Exceptions are thrown.
	*/
	void addViewFactorsToNandradZones(NANDRAD::Project & p, const std::vector<Project::RoomMapping> &roomMappings, const std::map<unsigned int, unsigned int> &componentInstanceMapping,
									  const std::map<unsigned int, unsigned int> &subSurfaceMapping, QStringList & errorStack) const;

	/*! Cached unique-ID -> object ptr map. Greatly speeds up objectByID() and any other lookup functions.
		This map is updated in updatePointers().
	*/
	std::map<unsigned int, VICUS::Object*>		m_objectPtr;

};



} // namespace VICUS

#endif // VICUS_ProjectH
