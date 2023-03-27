#include<VICUS_Project.h>

#include "SVUndoModifyStructuralUnitRoomAssociation.h"

#include "SVProjectHandler.h"

SVUndoModifyStructuralUnitRoomAssociation::SVUndoModifyStructuralUnitRoomAssociation(const QString & label,
																					 const std::set<unsigned int> & roomIds, VICUS::StructuralUnit * unit, bool override) : m_roomIds(roomIds), m_unit(unit), m_override(override)
{
	setText( label );
}

void SVUndoModifyStructuralUnitRoomAssociation::executeSubModification(){
	for (std::pair<const unsigned int, std::set<unsigned int>> & mod : m_subModifications){
		VICUS::Project & p = theProject();
		//find structural unit
		for (const VICUS::StructuralUnit & cUnit : p.m_structuralUnits){
			if(cUnit.m_id == mod.first){
				VICUS::StructuralUnit * unit = const_cast<VICUS::StructuralUnit* >(&cUnit);
				//swap the entries
				std::swap(unit->m_roomIds, mod.second);
			}
		}
	}
}


void SVUndoModifyStructuralUnitRoomAssociation::undo() {
	m_unit->m_roomIds = m_previousRoomIds;
	// call remains identical
	executeSubModification();

	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}


void SVUndoModifyStructuralUnitRoomAssociation::redo() {
	// save previous ids for undo
	m_previousRoomIds =  m_unit->m_roomIds; // deep copy


	if(m_override){
		m_unit->m_roomIds = m_roomIds;
	} else{
		// insert all the new ids
		for(unsigned int id : m_roomIds){
			m_unit->m_roomIds.insert(id);
		}
	}


	if(!m_subModifications.empty()){
		//submodifications have already been calculated
		executeSubModification();
		return;
	}

	// check if the a room already had a structural unit
	VICUS::Project & p = theProject();


	for(const VICUS::StructuralUnit & unit : p.m_structuralUnits){
		if(unit.m_id == m_unit->m_id) {
			// current unit -> skip
			continue;
		}
		// check if the other units contain one of the added ids
		for(unsigned int roomIdsOfUnit : unit.m_roomIds){
			std::set<unsigned int> eraseIds;
			for(unsigned int id : m_roomIds){
				if(roomIdsOfUnit == id){
					//this id needs to be removed!
					eraseIds.insert(id);
				}
			}
			if(!eraseIds.empty()){
				std::set<unsigned int> newIds = unit.m_roomIds;
				for(unsigned int eId : eraseIds){
					newIds.erase(eId);
				}
				m_subModifications[unit.m_id] = newIds;
			}
		}
	}

	//execute all sub modifications
	executeSubModification();

	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);

}
