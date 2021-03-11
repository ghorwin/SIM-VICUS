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
	/*! Which state of the object has been modified? */
	enum NodeState {
		VisibilityState = 0x01,
		SelectedState   = 0x02
	};

	/*! Data type of the modification info object. */
	class ModifiedNodes : public ModificationInfo {
	public:
		NodeState					m_changedStateType;
		std::vector<unsigned int>	m_nodeIDs;
	};


	/*! Constructor, takes set of node IDs to change excusively at the same time. If exclusive is true,
		all the others or changed into the opposite.
		So, you may select a set of surfaces and set the node state "VisibilityState" to on, then all these will be set to
		on, while all others not in the nodeIDs set will be hidden.
	*/
	SVUndoTreeNodeState(const QString & label, NodeState t, const std::set<unsigned int> & nodeIDs, bool on, bool exclusive=false);

	/*! Factory function, takes single node ID and flag to also change all children and grand-children. */
	static SVUndoTreeNodeState * createUndoAction(const QString & label, NodeState t, unsigned int nodeID, bool withChildren, bool on);

	virtual void undo();
	virtual void redo();

private:
	/*! Remembers what state has been changed.
		This information is passed on along with the modification info, so that clients
		can choose what to update.
	*/
	NodeState								m_changedStateType;

	/*! key = unique object id, value = bitmask of NodeState values */
	std::map<unsigned int, int >			m_nodeStates;
	std::map<unsigned int, int >			m_otherNodeStates;
};

#endif // SVUndoTreeNodeStateH
