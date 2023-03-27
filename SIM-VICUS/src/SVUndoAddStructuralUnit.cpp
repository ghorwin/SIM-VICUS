#include "SVUndoAddStructuralUnit.h"

#include <VICUS_Project.h>
#include "SVProjectHandler.h"

SVUndoAddStructuralUnit::SVUndoAddStructuralUnit(const QString & label, const VICUS::StructuralUnit & addedUnit) :
	m_addedUnit(addedUnit)
{
	setText( label );
}


void SVUndoAddStructuralUnit::undo() {

	// remove last building
	Q_ASSERT(!theProject().m_buildings.empty());

	theProject().m_structuralUnits.pop_back();
	theProject().updatePointers();
	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}


void SVUndoAddStructuralUnit::redo() {
	// append building
	theProject().m_structuralUnits.push_back(m_addedUnit);
	theProject().updatePointers();
	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}
