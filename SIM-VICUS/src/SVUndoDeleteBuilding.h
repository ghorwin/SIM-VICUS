#ifndef SVUndoDeleteBuildingH
#define SVUndoDeleteBuildingH

#include "SVUndoCommandBase.h"

#include <VICUS_Building.h>

class SVUndoDeleteBuilding: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteBuilding)
public:
	SVUndoDeleteBuilding(const QString & label, unsigned int buildingIndex);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for deleted building. */
	VICUS::Building	m_deletedBuilding;

	/*! Index of building in project's building vector to be removed. */
	unsigned int	m_buildingIndex;

};

#endif // SVUndoDeleteBuildingH
