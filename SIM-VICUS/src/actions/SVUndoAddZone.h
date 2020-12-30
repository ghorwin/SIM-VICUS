#ifndef SVUndoAddZoneH
#define SVUndoAddZoneH

#include <VICUS_BuildingLevel.h>

#include "SVUndoCommandBase.h"

/*! Undo action for adding a new or copied zone/room to an existing building level. */
class SVUndoAddZone : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddZone)
public:
	SVUndoAddZone(const QString & label, unsigned int buildingLevelUUID, const VICUS::Room & addedRoom, bool topologyOnly);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added item. */
	VICUS::Room				m_addedRoom;

	/*! If true, the change event sent is BuildingTopologyChanged, otherwise BuildingGeometryChanged. */
	bool					m_topologyOnly;

	unsigned int			m_buildingLevelUUID;
};


#endif // SVUndoAddZoneH
