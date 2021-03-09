#include "SVUndoModifyRoomZoneTemplateAssociation.h"
#include "SVProjectHandler.h"

#include <VICUS_Building.h>

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
					if (m_roomIDs[idx] == r.uniqueID())
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

	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}


void SVUndoModifyRoomZoneTemplateAssociation::redo() {
	undo(); // same code as undo
}
