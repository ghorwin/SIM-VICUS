#include "SVUndoModifyStructuralUnit.h"

#include <VICUS_Project.h>
#include "SVProjectHandler.h"

SVUndoModifyStructuralUnit::SVUndoModifyStructuralUnit(const QString & label, const VICUS::StructuralUnit & modifiedUnit):m_modifiedUnit(modifiedUnit)
{
setText(label);
}

void SVUndoModifyStructuralUnit::undo() {
	redo();
}


void SVUndoModifyStructuralUnit::redo() {
	// find the unit that will be modified
	for(VICUS::StructuralUnit & unit : theProject().m_structuralUnits){
		if(unit.m_id == m_modifiedUnit.m_id){
			std::swap(unit,m_modifiedUnit);
			return;
		}
	}
	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}
