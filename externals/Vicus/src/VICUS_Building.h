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

#ifndef VICUS_BuildingH
#define VICUS_BuildingH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_BuildingLevel.h"
#include "VICUS_Object.h"

#include <QString>

#include <vector>

namespace VICUS {

/*! Represents the building level node in the data hierarchy. */
class Building : public Object {
public:
	/*! Type-info string. */
	const char * typeinfo() const override { return "Building"; }

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	void updateParents() {
		m_children.clear();
		for (BuildingLevel & s : m_buildingLevels) {
			s.m_parent = this;
			m_children.push_back(&s);
			s.updateParents();
		}
	}

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int		m_id = INVALID_ID;			// XML:A:required
	//:inherited	QString				m_displayName;				// XML:A
	//:inherited	bool				m_visible = true;			// XML:A
	//:inherited	std::string			m_ifcGUID;					// XML:A

	/*! Vector of building levels. */
	std::vector<BuildingLevel>			m_buildingLevels;			// XML:E


	// *** RUNTIME VARIABLES ***

	/*! Whole building net floor area in m2.
		This area is updated whenever the pointer hierarchy is updated, that is, whenever
		anything changes in the building->building level->room tree.
	*/
	double								m_netFloorArea = -1;
};

} // namespace VICUS


#endif // VICUS_BuildingH
