#include "SVUndoDeleteStructuralUnit.h"

#include <VICUS_Project.h>
#include "SVProjectHandler.h"

SVUndoDeleteStructuralUnit::SVUndoDeleteStructuralUnit(const QString & label, const VICUS::StructuralUnit & deletedUnit) : m_deletedUnit(deletedUnit)
{
	setText(label);

	// find index
	for(unsigned int i = 0; i < theProject().m_structuralUnits.size(); i++){
		if(theProject().m_structuralUnits[i].m_id == deletedUnit.m_id){
			m_position = i;
			break;
		}
	}
}

void SVUndoDeleteStructuralUnit::undo() {
	theProject().m_structuralUnits.insert(theProject().m_structuralUnits.begin() + m_position, m_deletedUnit);
	theProject().updatePointers();

	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}


void SVUndoDeleteStructuralUnit::redo() {
	// delete from structural units vector
	theProject().m_structuralUnits.erase(theProject().m_structuralUnits.begin() + m_position);
	theProject().updatePointers();

	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}
