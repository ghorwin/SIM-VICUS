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

#include "SVUndoModifyRoom.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoModifyRoom::SVUndoModifyRoom(const QString & label,
					const VICUS::Room & r, unsigned int buildingIndex, unsigned int buildingLevelIndex,
								   unsigned int roomIndex) :
	m_room(r), m_buildingIndex(buildingIndex),
	m_buildingLevelIndex(buildingLevelIndex), m_roomIndex(roomIndex)
{
	setText( label );
}


void SVUndoModifyRoom::undo() {
	// exchange room meta data
	std::swap( theProject().m_buildings[m_buildingIndex].m_buildingLevels[m_buildingLevelIndex].m_rooms[m_roomIndex], m_room);
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	theProject().updatePointers();
}


void SVUndoModifyRoom::redo() {
	undo(); // same code as undo
}
