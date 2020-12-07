#include "SVUndoTreeNodeState.h"

template<typename T>
void storeState(const T & obj, int & bitmask) {
	bitmask = 0;
	if (obj.m_visible)
		bitmask |= SVUndoTreeNodeState::VisibilityState;
	if (obj.m_selected)
		bitmask |= SVUndoTreeNodeState::SelectedState;
}

template<typename T>
void setState(T & obj, const int & bitmask) {
	obj.m_visible = bitmask & SVUndoTreeNodeState::VisibilityState;
	obj.m_selected = bitmask & SVUndoTreeNodeState::SelectedState;
}


SVUndoTreeNodeState::SVUndoTreeNodeState(const QString & label,
										 NodeState t,
										 const std::set<unsigned int> & nodeIDs,
										 bool on) :
	m_changedStateType(t)
{
	setText( label );

	// store current node states
	const VICUS::Project & p = theProject();

	// search buildings
	for (const VICUS::Building & b : p.m_buildings) {
		if (nodeIDs.find(b.uniqueID()) != nodeIDs.end())
			storeState(b, m_nodeStates[b.uniqueID()]);
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			if (nodeIDs.find(bl.uniqueID()) != nodeIDs.end())
				storeState(bl, m_nodeStates[bl.uniqueID()]);
			for (const VICUS::Room & r : bl.m_rooms) {
				if (nodeIDs.find(r.uniqueID()) != nodeIDs.end())
					storeState(r, m_nodeStates[r.uniqueID()]);
				for (const VICUS::Surface & s : r.m_surfaces) {
					if (nodeIDs.find(s.uniqueID()) != nodeIDs.end())
						storeState(s, m_nodeStates[s.uniqueID()]);
				}
			}
		}
	}

	// search plain geometry
	for (const VICUS::Surface & s : p.m_plainGeometry) {
		if (nodeIDs.find(s.uniqueID()) != nodeIDs.end())
			storeState(s, m_nodeStates[s.uniqueID()]);
	}

	m_otherNodeStates = m_nodeStates;
	// now set the "new" node states
	for (auto & s : m_nodeStates) {
		switch (t) {
			case VisibilityState :
				if (on)
					s.second |= VisibilityState;
				else
					s.second &= ~VisibilityState;
			break;
			case SelectedState :
				if (on) {
					s.second |= SelectedState;
				}
				else
					s.second &= ~SelectedState;
			break;
		}
	}
}

SVUndoTreeNodeState * SVUndoTreeNodeState::createUndoAction(const QString & label,
															SVUndoTreeNodeState::NodeState t,
															unsigned int nodeID, bool withChildren, bool on)
{
	std::set<unsigned int> nodeIDs;
	const VICUS::Project & p = project();

	// compose set of nodeIDs - first store object that is referenced by nodeID
	nodeIDs.insert(nodeID);

	// now look in buildings and collect child IDs
	if (withChildren) {
		for (const VICUS::Building & b : p.m_buildings) {
			const VICUS::Object * obj = b.findChild(nodeID);
			if (obj != nullptr) {
					// also store IDs of all children
					obj->collectChildIDs(nodeIDs);
				break;
			}
		}
	}

	// now use the regular constructor to create the undo action
	return new SVUndoTreeNodeState(label, t, nodeIDs, on);
}


void SVUndoTreeNodeState::undo() {
	redo(); // same stuff as for redos
}


void SVUndoTreeNodeState::redo() {
	// get a copy of the project
	VICUS::Project & p = theProject();
	ModifiedNodes modInfo;
	modInfo.m_changedStateType = m_changedStateType;
	std::vector<unsigned int> & modifiedIDs = modInfo.m_nodeIDs;
	modifiedIDs.reserve(m_nodeStates.size());
	// process all entities in the entire data structure

	// we set the values in m_nodeStates in the project
	std::map<unsigned int, int >::const_iterator it;

	// search in buildings
	for (VICUS::Building & b : p.m_buildings) {
		if ((it = m_nodeStates.find(b.uniqueID())) != m_nodeStates.end()) {
			setState(b, it->second);
			modifiedIDs.push_back(it->first);
		}
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			if ((it = m_nodeStates.find(bl.uniqueID())) != m_nodeStates.end()) {
				setState(bl, it->second);
				modifiedIDs.push_back(it->first);
			}
			for (VICUS::Room & r : bl.m_rooms) {
				if ((it = m_nodeStates.find(r.uniqueID())) != m_nodeStates.end()) {
					setState(r, it->second);
					modifiedIDs.push_back(it->first);
				}
				for (VICUS::Surface & s : r.m_surfaces) {
					if ((it = m_nodeStates.find(s.uniqueID())) != m_nodeStates.end()) {
						setState(s, it->second);
						modifiedIDs.push_back(it->first);
					}
				}
			}
		}
	}

	// search in plain geometry
	for (VICUS::Surface & s : p.m_plainGeometry) {
		if ((it = m_nodeStates.find(s.uniqueID())) != m_nodeStates.end()) {
			setState(s, it->second);
			modifiedIDs.push_back(it->first);
		}
	}

	// now swap the states
	m_nodeStates.swap(m_otherNodeStates);
	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NodeStateModified, &modInfo);
}
