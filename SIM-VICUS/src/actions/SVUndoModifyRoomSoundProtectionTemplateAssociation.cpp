#include "SVUndoModifyRoomSoundProtectionTemplateAssociation.h"

#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoModifyRoomSoundProtectionTemplateAssociation::SVUndoModifyRoomSoundProtectionTemplateAssociation(const QString & label,
																									   const std::vector<unsigned int> & roomIDs,
																									   unsigned int soundProtectionTemplateID,
																									   unsigned int buildingTypeId)
	: m_roomIDs(roomIDs)
{
	setText( label );
	// populate vector with new zone template ID
	m_soundProtectionTemplateIDs	= std::vector<unsigned int>(m_roomIDs.size(), soundProtectionTemplateID);
	m_buildingTypeIDs				= std::vector<unsigned int>(m_roomIDs.size(), buildingTypeId);

}


void SVUndoModifyRoomSoundProtectionTemplateAssociation::undo() {
	// exchange acoustic template IDs
	VICUS::Project & p = theProject();

	Data d;
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
				std::swap(m_soundProtectionTemplateIDs[idx],	r.m_idSoundProtectionTemplate);
				std::swap(m_buildingTypeIDs[idx],				r.m_idAcousticBuildingType);

				// now store also the information, that the room has been updated
				const VICUS::Object *obj = dynamic_cast<VICUS::Object*>(&r);
				Q_ASSERT(obj != nullptr);
				d.m_objects.push_back(obj);
			}
		}
	}
	theProject().updatePointers();
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged, &d);
}


void SVUndoModifyRoomSoundProtectionTemplateAssociation::redo() {
	undo(); // same code as undo
}




