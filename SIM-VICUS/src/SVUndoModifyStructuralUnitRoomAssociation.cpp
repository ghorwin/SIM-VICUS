#include<VICUS_Project.h>

#include "SVUndoModifyStructuralUnitRoomAssociation.h"

#include "SVProjectHandler.h"

// for adding assosiations

SVUndoAddStructuralUnitRoomAssociation::SVUndoAddStructuralUnitRoomAssociation(const QString & label,
																					 const std::set<unsigned int> & roomIds, VICUS::StructuralUnit * unit) : m_roomIds(roomIds), m_unit(unit)
{
	setText( label );
}

void SVUndoAddStructuralUnitRoomAssociation::executeSubModification(){
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


void SVUndoAddStructuralUnitRoomAssociation::undo() {
	m_unit->m_roomIds = m_previousRoomIds;
	// call remains identical
	executeSubModification();

	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}


void SVUndoAddStructuralUnitRoomAssociation::redo() {
	// save previous ids for undo
	m_previousRoomIds =  m_unit->m_roomIds; // deep copy

	// insert all the new ids
	for(unsigned int id : m_roomIds){
		m_unit->m_roomIds.insert(id);
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
		std::set<unsigned int> eraseIds;
		// check if the other units contain one of the added ids
		for(unsigned int roomIdsOfUnit : unit.m_roomIds){

			for(unsigned int id : m_roomIds){
				if(roomIdsOfUnit == id){
					//this id needs to be removed!
					eraseIds.insert(id);
				}
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

	//execute all sub modifications
	executeSubModification();

	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);

}

// for removing assosiations

SVUndoRemoveStructuralUnitRoomAssociation::SVUndoRemoveStructuralUnitRoomAssociation(const QString & label,
																					 const std::set<unsigned int> & roomIds) : m_roomIds(roomIds)
{
	setText( label );
}


void SVUndoRemoveStructuralUnitRoomAssociation::undo() {
	// for each map entry insert the ids again
	for(std::pair<VICUS::StructuralUnit* const, std::set<unsigned int>> entry : m_removedIds){
		for( unsigned int id : entry.second){
			entry.first->m_roomIds.insert(id);
		}
	}

	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);
}


void SVUndoRemoveStructuralUnitRoomAssociation::redo() {

	for(unsigned int id : m_roomIds){
		// for each room id, the structural unit will be searched and then it will be removed
		bool found = false;
		for(VICUS::StructuralUnit & unit : theProject().m_structuralUnits){
			for(unsigned int unitRoomId : unit.m_roomIds){
				if(id == unitRoomId){
					found = true;
					// found the unit with an Id to be removed
					unit.m_roomIds.erase(id);
					// save which ids are getting removed for undo
					m_removedIds[&unit].insert(id);
					break;
				}
			}
			if(found) break;
		}
	}
	SVProjectHandler::instance().setModified(SVProjectHandler::StructuralUnitsModified);

}




