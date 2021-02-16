#ifndef SVUndoModifyBuildingTopologyH
#define SVUndoModifyBuildingTopologyH

#include <VICUS_Building.h>
#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of the building topology, i.e. building levels or rooms are moved around (but not deleted/added).
	Notification type BuildingTopologyChanged is used.
*/
class SVUndoModifyBuildingTopology : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyBuildingTopology)
public:
	/*! Replaces building entity at given index in buildings vector. */
	SVUndoModifyBuildingTopology(const QString & label, const std::vector<VICUS::Building> & buildings);

	virtual void undo();
	virtual void redo();

private:
	/*! Data member to hold modified buildings vector. */
	std::vector<VICUS::Building> m_buildings;
};

#endif // SVUndoModifyBuildingTopologyH
