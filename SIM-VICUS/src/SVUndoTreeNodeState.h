#ifndef SVUndoTreeNodeStateH
#define SVUndoTreeNodeStateH

#include "SVUndoCommandBase.h"
#include "SVProjectHandler.h"

/*! Undo action for change of state of a node entity in the navigation tree view.
	State can be selection or visibility. State can be changed for several entities at a time.
*/
class SVUndoTreeNodeState : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoTreeNodeState)
public:
	/*! Data type of the modification info object. */
	class ModifiedNodes : public ModificationInfo {
	public:
		std::vector<unsigned int> nodeIDs;
	};

	enum NodeState {
		VisibilityState = 0x01,
		SelectedState   = 0x02
	};

	/*! Constructor, takes set of node IDs to change at the same time. */
	SVUndoTreeNodeState(const QString & label, NodeState t, const std::set<unsigned int> & nodeIDs, bool on);

	/*! Factory function, takes single node ID and flag to also change all children and grand-children. */
	static SVUndoTreeNodeState * createUndoAction(const QString & label, NodeState t, unsigned int nodeID, bool withChildren, bool on);

	virtual void undo();
	virtual void redo();

private:

	/*! key = unique object id, value = bitmask of NodeState values */
	std::map<unsigned int, unsigned int >	m_nodeStates;
	std::map<unsigned int, unsigned int >	m_otherNodeStates;
};

#endif // SVUndoTreeNodeStateH
