#ifndef SVUNDOMODIFYSTRUCTURALUNITROOMASSOCIATION_H
#define SVUNDOMODIFYSTRUCTURALUNITROOMASSOCIATION_H

#include <vector>

#include "SVUndoCommandBase.h"

#include <VICUS_Room.h>
#include <VICUS_StructuralUnit.h>


class SVUndoModifyStructuralUnitRoomAssociation : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyStructuralUnitRoomAssociation)
public:
	SVUndoModifyStructuralUnitRoomAssociation(const QString & label,
											  const std::set<unsigned int> & roomIDs,
											  VICUS::StructuralUnit * unit,
											  bool override = false);

	virtual void undo();
	virtual void redo();

private:

	void executeSubModification();

	/*! Data member to hold modified room IDs vector, these are the uniqueIDs of the rooms!. */
	std::set<unsigned int>		m_roomIds;
	std::set<unsigned int>		m_previousRoomIds;
	VICUS::StructuralUnit		* m_unit;
	bool						m_override;
	std::map<unsigned int, std::set<unsigned int>> m_subModifications;

};

#endif // SVUNDOMODIFYSTRUCTURALUNITROOMASSOCIATION_H
