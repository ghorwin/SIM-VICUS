#ifndef SVUndoAddStructuralUnitH
#define SVUndoAddStructuralUnitH

#include <VICUS_StructuralUnit.h>

#include "SVUndoCommandBase.h"

class SVUndoAddStructuralUnit: public SVUndoCommandBase {
		Q_DECLARE_TR_FUNCTIONS(SVUndoAddStructuralUnit)
public:
	SVUndoAddStructuralUnit(const QString & label, const VICUS::StructuralUnit & addedUnit);
	virtual void undo() override;
	virtual void redo() override;

private:

	/*! Cache for added structural unit. */
	VICUS::StructuralUnit		m_addedUnit;

};

#endif // SVUndoAddStructuralUnitH
