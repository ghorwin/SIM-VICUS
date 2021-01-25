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

#include <vector>

#include <IBK_Path.h>

#include <NANDRAD_Project.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Network.h"
#include "VICUS_Building.h"
#include "VICUS_ViewSettings.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_Outputs.h"
#include "VICUS_ComponentInstance.h"


#include <NANDRAD_SolverParameter.h>

namespace VICUS {

class Project {
	VICUS_READWRITE
public:

	/*! Bitmasks for different selection groups used in selectObjects(). */
	enum SelectionGroups {
		SG_Building			= 0x001,
		SG_Network			= 0x002,
		SG_All				= SG_Building | SG_Network,
		NUM_SG
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Constructor, creates dummy data. */
	Project();

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

	/*! This function checks all surfaces in the project if they are selected or not.
		\returns Returns true, if any surface is selected, and if so, stores the arithmetic average of all
				 surface vertexes in variable centerPoint.
		\todo Stephan, remove and use selectObjects() with subsequent bounding box calculation instead.
	*/
	bool haveSelectedSurfaces(IBKMK::Vector3D & centerPoint) const;

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
	bool selectedSurfaces(std::vector<const Surface*> & surfaces) const;

	/*! This function returns the Bounding Box of all selected surfaces
		\returns Returns true, if any surface is selected
	*/
	bool boundingBoxofSelectedSurfaces(IBKMK::Vector3D & boundingbox) const;

	// *** FUNCTIONS ***

	/*! Function to find an element by ID. */
	template <typename T>
	static T * element(std::vector<T>& vec, unsigned int id) {
		typename std::vector<T>::iterator it = std::find(vec.begin(), vec.end(), id);
		if (it == vec.end())
			return nullptr;
		else
			return &(*it);
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
	std::vector<ComponentInstance>						m_componentInstances;		// XML:E

	/*! Vector with plain (dumb) geometry. */
	std::vector<Surface>								m_plainGeometry;			// XML:E


	// *** Database elements used in the project (normally stored in built-in and user databases)
	//     These database elements need to be merged with program databased when project is read.

	/*! Database of fluids */
	std::vector<NetworkFluid>							m_networkFluids;			// XML:E

};


} // namespace VICUS

#endif // VICUS_ProjectH
