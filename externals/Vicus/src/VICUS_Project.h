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

	// *** PUBLIC MEMBER VARIABLES ***

	ViewSettings			m_viewSettings;		// XML:E

	NANDRAD::Project		m_nandradData;

	std::vector<Network>	m_networks;

	std::vector<Building>	m_buildings;		// XML:E

	std::vector<NetworkFluid>	m_networkFluidDB;	// XML:E
	std::vector<NetworkPipe>	m_networkPipeDB;	// XML:E
};


} // namespace VICUS

#endif // VICUS_ProjectH
