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

namespace VICUS {

class Project {
	VICUS_READWRITE
public:

	enum ViewFlags {
		VF_All,			// Keyword: All
		NUM_VF
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

	// *** FUNCTIONS ***

	/*! Function to find an element by ID. */
	template <typename T>
	T * element(std::vector<T>& vec, unsigned int id) {
		typename std::vector<T>::iterator it = std::find(vec.begin(), vec.end(), id);
		if (it == vec.end())
			return nullptr;
		else
			return &(*it);
	}

	/*! Function to find an element by ID (const-version). */
	template <typename T>
	const T * element(const std::vector<T>& vec, unsigned int id) const {
		typename std::vector<T>::const_iterator it = std::find(vec.begin(), vec.end(), id);
		if (it == vec.end())
			return nullptr;
		else
			return &(*it);
	}


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Project info tag is manually written (not by code generator). */
	NANDRAD::ProjectInfo	m_projectInfo;

	ViewSettings			m_viewSettings;		// XML:E

	std::vector<Network>	m_geomNetworks;			// XML:E

	std::vector<NANDRAD::HydraulicNetwork>	m_hydraulicNetworks;

	/*! the catalog of hydraulic components */
	std::vector<NANDRAD::HydraulicNetworkComponent>	m_hyrdaulicComponents;

	std::vector<Building>	m_buildings;		// XML:E


	// *** Database elements used in the project (normally stored in built-in and user databases)
	//     These database elements need to be merged with program databased when project is read.

	/*! Database of fluids */
	std::vector<NetworkFluid>	m_networkFluids;	// XML:E
};


} // namespace VICUS

#endif // VICUS_ProjectH
