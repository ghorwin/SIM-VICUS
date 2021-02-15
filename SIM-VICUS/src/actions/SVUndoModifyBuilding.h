#ifndef SVUndoModifyBuildingH
#define SVUndoModifyBuildingH

#include <VICUS_Building.h>
#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of the building meta data only. */
class SVUndoModifyBuilding : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyBuilding)
public:
	/*! Replaces building entity at given index in buildings vector. */
	SVUndoModifyBuilding(const QString & label, const VICUS::Building & b, unsigned int buildingIndex);

	virtual void undo();
	virtual void redo();

private:
	VICUS::Building m_building;
	unsigned int	m_buildingIndex;
};

#endif // SVUndoModifyBuildingH
