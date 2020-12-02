#ifndef SVUndoAddBuildingH
#define SVUndoAddBuildingH

#include <VICUS_Building.h>

#include "SVUndoCommandBase.h"

class SVUndoAddBuilding : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddBuilding)
public:
	SVUndoAddBuilding(const QString & label, const VICUS::Building & addedBuilding);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added building. */
	VICUS::Building	m_addedBuilding;
};


#endif // SVUndoAddBuildingH
