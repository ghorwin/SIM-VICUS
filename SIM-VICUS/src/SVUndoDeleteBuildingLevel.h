#ifndef SVUndoDeleteBuildingLevelH
#define SVUndoDeleteBuildingLevelH

#include "SVUndoCommandBase.h"

#include <VICUS_BuildingLevel.h>

class SVUndoDeleteBuildingLevel: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteBuildingLevel)
public:
	SVUndoDeleteBuildingLevel(const QString & label, unsigned int buildingIndex, unsigned int buildingLevelIndex);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for deleted building level. */
	VICUS::BuildingLevel	m_deletedLevel;

	/*! Index of building in project's building vector to be modified. */
	unsigned int	m_buildingIndex;

	/*! Index of level to be removed. */
	unsigned int	m_levelIndex;
};

#endif // SVUndoDeleteBuildingLevelH
