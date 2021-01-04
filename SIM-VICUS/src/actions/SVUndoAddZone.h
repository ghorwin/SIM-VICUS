#ifndef SVUndoAddZoneH
#define SVUndoAddZoneH

#include <VICUS_BuildingLevel.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Undo action for adding a new or copied zone/room to an existing building level. */
class SVUndoAddZone : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddZone)
public:
	SVUndoAddZone(const QString & label, unsigned int buildingLevelUUID,
				  const VICUS::Room & addedRoom, bool topologyOnly,
				  const std::vector<VICUS::ComponentInstance> * componentInstances = nullptr);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added item. */
	VICUS::Room				m_addedRoom;

	/*! If true, the change event sent is BuildingTopologyChanged, otherwise BuildingGeometryChanged. */
	bool					m_topologyOnly;

	unsigned int			m_buildingLevelUUID;

	/*! If not empty, this vector contains component instances that are created alongside the room's surfaces. */
	std::vector<VICUS::ComponentInstance>	m_componentInstances;
};


#endif // SVUndoAddZoneH
