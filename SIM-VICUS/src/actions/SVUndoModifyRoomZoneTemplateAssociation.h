#ifndef SVUndoModifyRoomZoneTemplateAssociationH
#define SVUndoModifyRoomZoneTemplateAssociationH

#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of the zone template IDs in rooms.
*/
class SVUndoModifyRoomZoneTemplateAssociation : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyRoomZoneTemplateAssociation)
public:
	/*! Replaces zone template IDs in all rooms identified by their unique IDs. */
	SVUndoModifyRoomZoneTemplateAssociation(const QString & label,
											const std::vector<unsigned int> & roomIDs,
											unsigned int zoneTemplateID);

	virtual void undo();
	virtual void redo();

private:
	/*! Data member to hold modified room IDs vector, these are the uniqueIDs of the rooms!. */
	std::vector<unsigned int> m_roomIDs;
	std::vector<unsigned int> m_zoneTemplateIDs;
};

#endif // SVUndoModifyRoomZoneTemplateAssociationH
