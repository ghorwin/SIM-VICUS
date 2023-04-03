#ifndef SVUNDOMODIFYSTRUCTURALUNIT_H
#define SVUNDOMODIFYSTRUCTURALUNIT_H

#include <VICUS_StructuralUnit.h>

#include "SVUndoCommandBase.h"


class SVUndoModifyStructuralUnit : public SVUndoCommandBase {
public:
	SVUndoModifyStructuralUnit(const QString & label, const VICUS::StructuralUnit & modifiedUnit);
	virtual void undo() override;
	virtual void redo() override;

private:

	/*! Cache for modified structural unit. */
	VICUS::StructuralUnit		m_modifiedUnit;

};



#endif // SVUNDOMODIFYSTRUCTURALUNIT_H