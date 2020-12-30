#ifndef SVUndoAddBuildingLevelH
#define SVUndoAddBuildingLevelH

#include <VICUS_BuildingLevel.h>

#include "SVUndoCommandBase.h"

/*! Undo action for adding a new or copied building level to an existing building. */
class SVUndoAddBuildingLevel : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddBuildingLevel)
public:
	SVUndoAddBuildingLevel(const QString & label, unsigned int buildingUUID, const VICUS::BuildingLevel & addedLevel, bool topologyOnly);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added item. */
	VICUS::BuildingLevel	m_addedLevel;

	/*! If true, the change event sent is BuildingTopologyChanged, otherwise BuildingGeometryChanged. */
	bool					m_topologyOnly;

	unsigned int			m_buildingUUID;
};


#endif // SVUndoAddBuildingLevelH
