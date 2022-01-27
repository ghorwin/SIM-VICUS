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

#include "SVUndoModifyRoomZoneTemplateAssociation.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoModifyRoomZoneTemplateAssociation::SVUndoModifyRoomZoneTemplateAssociation(const QString & label,
			const std::vector<unsigned int> & roomIDs, unsigned int zoneTemplateID) :
	m_roomIDs(roomIDs)
{
	setText( label );

	// populate vector with new zone template ID
	m_zoneTemplateIDs = std::vector<unsigned int>(m_roomIDs.size(), zoneTemplateID);
}


void SVUndoModifyRoomZoneTemplateAssociation::undo() {
	// exchange zone template IDs
	VICUS::Project & p = theProject();

	for (VICUS::Building & b : p.m_buildings) {
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (VICUS::Room & r : bl.m_rooms) {
				// check if room is in the set of modified IDs, and if so, determine room index in id vector
				unsigned int idx = 0;
				for (; idx < m_roomIDs.size(); ++idx)
					if (m_roomIDs[idx] == r.m_id)
						break;

				// it is possible that the current room is not selected for modification
				// in this case we just skip it
				if (idx == m_roomIDs.size())
					continue;

				// swap current template ID and template ID in vector
				std::swap(m_zoneTemplateIDs[idx], r.m_idZoneTemplate);
			}
		}
	}
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}


void SVUndoModifyRoomZoneTemplateAssociation::redo() {
	undo(); // same code as undo
}
