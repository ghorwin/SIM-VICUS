#ifndef SVUNDODELETESTRUCTURALUNIT_H
#define SVUNDODELETESTRUCTURALUNIT_H

#include <VICUS_StructuralUnit.h>

#include "SVUndoCommandBase.h"

class SVUndoDeleteStructuralUnit : public SVUndoCommandBase {
public:
	SVUndoDeleteStructuralUnit(const QString & label, const VICUS::StructuralUnit & deletedUnit);
	virtual void undo() override;
	virtual void redo() override;

private:

	/*! Cache for deleted structural unit. */
	VICUS::StructuralUnit		m_deletedUnit;

	/*! Position of the structural unit to be deleted */
	unsigned int m_position;

};

#endif // SVUNDODELETESTRUCTURALUNIT_H
