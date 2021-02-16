#ifndef SVUndoModifyBuildingLevelH
#define SVUndoModifyBuildingLevelH

#include <VICUS_BuildingLevel.h>
#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of the building level data only. */
class SVUndoModifyBuildingLevel : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyBuildingLevel)
public:
	/*! Replaces building entity at given index in buildings vector. */
	SVUndoModifyBuildingLevel(const QString & label, const VICUS::BuildingLevel & bl, unsigned int buildingIndex,
							  unsigned int buildingLevelIndex, bool withoutRooms);

	virtual void undo();
	virtual void redo();

private:
	VICUS::BuildingLevel	m_buildingLevel;
	unsigned int			m_buildingIndex;
	unsigned int			m_buildingLevelIndex;
	/*! If true, the rooms vector member variable won't be modified (saves memory).
		Also, the notification type BuildingTopologyChanged is used when levels are not modified, which
		speeds up UI updates.
	*/
	bool					m_withoutRooms;
};

#endif // SVUndoModifyBuildingLevelH
