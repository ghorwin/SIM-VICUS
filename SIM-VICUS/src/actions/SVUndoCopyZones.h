#ifndef SVUndoCopyZonesH
#define SVUndoCopyZonesH

#include <VICUS_BuildingLevel.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Undo action for adding a vector with copied zones/rooms to an existing building level. */
class SVUndoCopyZone : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoCopyZone)
public:
	SVUndoCopyZone(const QString & label, unsigned int buildingLevelUUID,
				  const std::vector<VICUS::Room> copiedRooms,
				  const std::vector<VICUS::ComponentInstance> * componentInstances = nullptr);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added item. */
	std::vector<VICUS::Room>				m_copiedRooms;

	unsigned int							m_buildingLevelUUID;

	/*! If not empty, this vector contains component instances that are created alongside the room's surfaces. */
	std::vector<VICUS::ComponentInstance>	m_componentInstances;
};


#endif // SVUndoCopyZonesH
