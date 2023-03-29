#ifndef SVUNDOMODIFYSTRUCTURALUNITROOMASSOCIATION_H
#define SVUNDOMODIFYSTRUCTURALUNITROOMASSOCIATION_H

#include <vector>

#include "SVUndoCommandBase.h"

#include <VICUS_Room.h>
#include <VICUS_StructuralUnit.h>


class SVUndoAddStructuralUnitRoomAssociation : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyStructuralUnitRoomAssociation)
public:
	SVUndoAddStructuralUnitRoomAssociation(const QString & label,
											  const std::set<unsigned int> & roomIDs,
											  VICUS::StructuralUnit * unit);

	virtual void undo() override;
	virtual void redo() override;

private:

	void executeSubModification();

	/*! Data member to hold modified room IDs vector, these are the uniqueIDs of the rooms!. */
	std::set<unsigned int>		m_roomIds;
	std::set<unsigned int>		m_previousRoomIds;
	VICUS::StructuralUnit*		m_unit;
	std::map<unsigned int, std::set<unsigned int>> m_subModifications;

};

class SVUndoRemoveStructuralUnitRoomAssociation : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyStructuralUnitRoomAssociation)
public:
	SVUndoRemoveStructuralUnitRoomAssociation(const QString & label,
											  const std::set<unsigned int> & roomIDs);

	virtual void undo() override;
	virtual void redo() override;

private:

	void executeSubModification();

	/*! Data member to hold modified room IDs vector, these are the uniqueIDs of the rooms!. */
	std::set<unsigned int>		m_roomIds;
	std::map<VICUS::StructuralUnit *, std::set<unsigned int>> m_removedIds;

};



#endif // SVUNDOMODIFYSTRUCTURALUNITROOMASSOCIATION_H
