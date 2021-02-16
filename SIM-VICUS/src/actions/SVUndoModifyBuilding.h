#ifndef SVUndoModifyBuildingH
#define SVUndoModifyBuildingH

#include <VICUS_Building.h>
#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of a building object only. */
class SVUndoModifyBuilding : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyBuilding)
public:
	/*! Replaces building entity at given index in buildings vector. */
	SVUndoModifyBuilding(const QString & label, const VICUS::Building & b, unsigned int buildingIndex, bool withoutLevels);

	virtual void undo();
	virtual void redo();

private:
	VICUS::Building m_building;
	unsigned int	m_buildingIndex;
	/*! If true, the building levels vector member variable won't be modified (saves memory).
		Also, the notification type BuildingTopologyChanged is used when levels are not modified, which
		speeds up UI updates.
	*/
	bool			m_withoutLevels;
};

#endif // SVUndoModifyBuildingH
