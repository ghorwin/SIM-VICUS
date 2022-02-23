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

#ifndef VICUS_BuildingLevelH
#define VICUS_BuildingLevelH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Room.h"
#include "VICUS_Object.h"

#include <QString>

namespace VICUS {

class BuildingLevel : public Object {
public:
	/*! Type-info string. */
	const char * typeinfo() const override { return "BuildingLevel"; }

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	void updateParents() {
		m_children.clear();
		for (Room & s : m_rooms) {
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

	/*! The nominal elevation [m] of the floor's surface above ground. */
	double								m_elevation = 0;			// XML:E

	/*! The nominal height [m] (floor surface to ceiling) of the building level. */
	double								m_height = 2.7;				// XML:E

	/*! Vector of all rooms in a building level. */
	std::vector<Room>					m_rooms;					// XML:E

	// *** RUNTIME VARIABLES ***

	/*! Whole building level net floor area in m2.
		This area is updated whenever the pointer hierarchy is updated, that is, whenever
		anything changes in the building->building level->room tree.
	*/
	double								m_netFloorArea = -1;
};

} // namespace VICUS


#endif // VICUS_BuildingLevelH
