/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef VICUS_ProjectH
#define VICUS_ProjectH

#include <QCoreApplication> // for tr functions

#include <vector>

#include <IBK_Path.h>

#include <NANDRAD_Project.h>
#include <NANDRAD_SolverParameter.h>
#include <NANDRAD_FMIDescription.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Network.h"
#include "VICUS_Building.h"
#include "VICUS_ViewSettings.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_Outputs.h"
#include "VICUS_ComponentInstance.h"
#include "VICUS_SubSurfaceComponentInstance.h"
#include "VICUS_EmbeddedDatabase.h"


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
		SG_All				= SG_Building | SG_Network | SG_Obstacle,
		NUM_SG
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

	/*! Writes the project file to an XML file.
		\param filename  The full path to the project file.
	*/
	void writeXML(const IBK::Path & filename) const;

	/*! Reads the placeholder section into m_placeholders map. */
	void readDirectoryPlaceholdersXML(const TiXmlElement * element);

	/*! Writes the section with directory place holders. */
	void writeDirectoryPlaceholdersXML(TiXmlElement * parent) const;

	/*! Removes un-referenced/un-needed data structures. */
	void clean();

	/*! Call this function whenever project data has changed that depends on
		objects linked through pointers (building hierarchies, networks etc.).
	*/
	void updatePointers();

	/*! Searches through all unique id-objects in project structure for the uniqueID.
		Throws an exception, if no object with this unique ID can be found.
	*/
	const VICUS::Object * objectById(unsigned int uniqueID) const;

	/*! Searches through all buildings and tries to find a surface with given surface ID (this
		is not the uniqueID, but the persistant id from the data model.
	*/
	VICUS::Surface * surfaceByID(unsigned int surfaceID);

	/*! Searches through all buildings and tries to find a subsurface with given subsurface ID (this
		is not the uniqueID, but the persistant id from the data model).
	*/
	VICUS::SubSurface * subSurfaceByID(unsigned int surfID);

	/*! Selects objects and returns set with pointers according to additional filters.
		\param selectedObjs Here the pointers to selected objects are returned.
		\param sg			Selection group, that the object belongs to.
		\param takeSelected	If true, only objects with "selected" property enabled are taken. If false,
			selection property is ignored.
	*/
	void selectedBuildingObjects(std::set<const Object *> &selectedObjs, Object *obj) const;

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



	// *** PROJECT CONVERSION RELATED FUNCTIONS ***

	/*! This exception class extends IBK::Exception by additional members
		needed to adjust the start-simulation-dialog, for example, to focus problematic inputs.
	*/
	class ConversionError : public IBK::Exception { // NO KEYWORDS
	public:
		enum Errortypes {
			ET_MissingClimate,
			ET_InvalidID,
			ET_MismatchingSurfaceArea,
			ET_MissingParentZone,
			ET_NotValid
		};

		ConversionError(Errortypes errorType, const std::string & errmsg) :
			IBK::Exception(errmsg, "ConversionError"), m_errorType(errorType) {}
		ConversionError(Errortypes errorType, const IBK::FormatString & errmsg) :
			IBK::Exception(errmsg, "ConversionError"), m_errorType(errorType) {}
		ConversionError(Errortypes errorType, const QString & errmsg) :
			IBK::Exception(errmsg.toStdString(), "ConversionError"), m_errorType(errorType) {}
		ConversionError(const ConversionError&) = default;
		virtual ~ConversionError() override;

		Errortypes m_errorType;
	};

	/*! Converts VICUS data structure into NANDRAD project file.
		This function throws an error message in case the conversion failed.
		\param p The NANDRAD project to be populated.
	*/
	void generateNandradProject(NANDRAD::Project & p) const;
	void generateBuildingProjectData(NANDRAD::Project & p) const;
	void generateNetworkProjectData(NANDRAD::Project & p) const;
	NANDRAD::Interface generateInterface(const VICUS::ComponentInstance & ci, unsigned int bcID,
										 std::vector<unsigned int> &allModelIds,
										 std::map<unsigned int, unsigned int> &vicusToNandradIds,
										 unsigned int & interfaceID, bool takeASide = true) const;

	// *** STATIC FUNCTIONS ***

	/*! This function computes the bounding box of all selected surfaces and the center point.
		\returns Returns the dimensions of the bounding box and its center point in argument 'center'.
	*/
	static IBKMK::Vector3D boundingBox(std::vector<const Surface*> &surfaces, IBKMK::Vector3D &center);

	/*! Function to find an element by ID. */
	template <typename T>
	static T * element(std::vector<T>& vec, unsigned int id) {
		typename std::vector<T>::iterator it = std::find(vec.begin(), vec.end(), id);
		if (it == vec.end())
			return nullptr;
		else
			return &(*it);
	}

	/*! Checks if an object with m_id matching the searched id exists in the vector. */
	template <typename T>
	static bool contains(const std::vector<T> & vec, unsigned int id) {
		for (auto & t : vec)
			if (t->m_id == id)
				return true;
		return false;
	}


	/*! Function to find an element by ID (const-version). */
	template <typename T>
	static const T * element(const std::vector<T>& vec, unsigned int id) {
		typename std::vector<T>::const_iterator it = std::find(vec.begin(), vec.end(), id);
		if (it == vec.end())
			return nullptr;
		else
			return &(*it);
	}

	/*! Function to generate unique ID (const-version). */
	template <typename T>
	static unsigned int uniqueId(const std::vector<T>& vec) {
		for (unsigned id=1; id<std::numeric_limits<unsigned>::max(); ++id){
			if (std::find(vec.begin(), vec.end(), id) == vec.end())
				return id;
		}
		return 999999; // just to make compiler happy, we will find an unused ID in the loop above
	}

	/*! Function to generate unique ID. First check predefined id. And the Id to the container. */
	template <typename T>
	static unsigned int uniqueIdWithPredef(std::vector<T>& vec, unsigned int newId) {
		if(std::find(vec.begin(), vec.end(), newId) == vec.end()){
			vec.push_back(newId);
			return newId;
		}
		unsigned id=uniqueId(vec);
		vec.push_back(id);
		return id;
	}

	/*! Function to generate unique ID. First check predefined id. Add the Id to the container.  */
	template <typename T>
	static unsigned int uniqueIdWithPredef(std::vector<T>& vec, unsigned int id, std::map<T,T> mapOldToNewId){

		if(mapOldToNewId.find(id) == mapOldToNewId.end())
			mapOldToNewId[id] = uniqueIdWithPredef(vec, id);
		return mapOldToNewId[id];
	}

	/*! Function to generate a unique ID that is larger than all the other IDs used.
		This is useful if a series of objects with newly generated IDs shall be added to a container.
	*/
	template <typename T>
	static unsigned int largestUniqueId(const std::vector<T>& vec) {
		unsigned int largest = 0;
		for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it)
			largest = std::max(largest, it->m_id);
		return largest+1; // Mind: plus one, to get past the largest _existing_ ID
	}



	/*! Generates a new unique name in format "basename" or "basename [<nr>]" with increasing numbers until
		the name no longer exists in set existingNames.
	*/
	static QString uniqueName(const QString & baseName, const std::set<QString> & existingNames) {
		// generate new unique object/surface name
		unsigned int count = 1;
		QString name = baseName;
		for (;;) {
			// process all surfaces and check if we have already a new surface with our current name
			if (existingNames.find(name) == existingNames.end())
				break;
			name = QString("%1 [%2]").arg(baseName).arg(++count);
		}
		return name;
	}

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

	std::vector<Network>								m_geometricNetworks;		// XML:E

	std::vector<Building>								m_buildings;				// XML:E

	/*! All components actually placed in the geometry.
		This vector is outside buildings, so that two building parts can be connected with
		a component.
	*/
	std::vector<ComponentInstance>						m_componentInstances;				// XML:E
	std::vector<SubSurfaceComponentInstance>			m_subSurfaceComponentInstances;		// XML:E

	/*! Vector with plain (dumb) geometry. */
	std::vector<Surface>								m_plainGeometry;			// XML:E

	/*! Path placeholder mappings used to substitute placeholders for database and user databases.
		These placeholders are read from the path placeholders section of the project file and hold
		a full directory path for a given placeholder name. You can compose a placeholder yourself:
		\code
		m_placeholders["Database"] = "/path/to/databases";
		// and then use this to resolve a relative path defined in the project file as
		string relative_path = "${Database}/db_materials.xml";
		\endcode
	*/
	std::map< std::string, IBK::Path >	m_placeholders;


	/*! Holds the database elements referenced in the project. These are a copy of db elements
		in the built-in and user-database and stored for project exchange between computers.
	*/
	EmbeddedDatabase									m_embeddedDB;				// XML:E


	/*! Definitions for exporting an FMU from the model. */
	NANDRAD::FMIDescription								m_fmiDescription;			// XML:E

private:
	/*! Return room name by id.
		TODO Coding style beachten!
	*/
	std::string getRoomNameById(unsigned int id) const;



};


} // namespace VICUS

#endif // VICUS_ProjectH
