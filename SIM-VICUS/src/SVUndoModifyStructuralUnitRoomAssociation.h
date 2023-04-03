#ifndef SVUndoModifyStructuralUnitRoomAssociationH
#define SVUndoModifyStructuralUnitRoomAssociationH

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

	/*! In case the a rooms has another structural unit, its initial unit need to be removed, this functions handles this part. */
	void executeSubModification();

	/*! Data member to hold modified room IDs vector, these are the uniqueIDs of the rooms!. */
	std::set<unsigned int>		m_roomIds;

	/*! Save the previous room ids for undo action. */
	std::set<unsigned int>		m_previousRoomIds;

	/*! The relevant structural unit. */
	VICUS::StructuralUnit*		m_unit;

	/*! In case the a rooms has another structural unit, its initial unit need to be removed, these modifications will be saved here. */
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

	/*! Data member to hold modified room IDs vector, these are the uniqueIDs of the rooms!. */
	std::set<unsigned int>		m_roomIds;
	/*! Removed ids will be saved with the according unit for the undo action. */
	std::map<VICUS::StructuralUnit *, std::set<unsigned int>> m_removedIds;

};



#endif // SVUndoModifyStructuralUnitRoomAssociationH
