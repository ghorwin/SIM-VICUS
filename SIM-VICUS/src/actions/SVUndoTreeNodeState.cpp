/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVUndoTreeNodeState.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

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
										 bool on, bool exclusive) :
	m_changedStateType(t)
{
	setText( label );

	// store current node states
	const VICUS::Project & p = theProject();

	// we first search through all buildings and store the node states of all objects
	// that need changing

	// if exclusive is on, we just store all node states

	// search buildings
	for (const VICUS::Building & b : p.m_buildings) {
		if (exclusive || nodeIDs.find(b.m_id) != nodeIDs.end())
			storeState(b, m_nodeStates[b.m_id]);
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			if (exclusive || nodeIDs.find(bl.m_id) != nodeIDs.end())
				storeState(bl, m_nodeStates[bl.m_id]);
			for (const VICUS::Room & r : bl.m_rooms) {
				if (exclusive || nodeIDs.find(r.m_id) != nodeIDs.end())
					storeState(r, m_nodeStates[r.m_id]);
				for (const VICUS::Surface & s : r.m_surfaces) {
					if (exclusive || nodeIDs.find(s.m_id) != nodeIDs.end())
						storeState(s, m_nodeStates[s.m_id]);
					for (const VICUS::SubSurface & sub : s.subSurfaces()) {
						if (exclusive || nodeIDs.find(sub.m_id) != nodeIDs.end())
							storeState(sub, m_nodeStates[sub.m_id]);
					}
					storeStateChildSurface(s, nodeIDs, exclusive);
				}
			}
		}
	}

	// search plain geometry
	for (const VICUS::Surface & s : p.m_plainGeometry.m_surfaces) {
		if (exclusive || nodeIDs.find(s.m_id) != nodeIDs.end())
			storeState(s, m_nodeStates[s.m_id]);
	}

	// we have a special handling for the parent node
	if (!p.m_plainGeometry.m_surfaces.empty())
		if (exclusive || nodeIDs.find(0) != nodeIDs.end())
			storeState(p.m_plainGeometry, m_nodeStates[0]);

	// search in networks
	for (const VICUS::Network & n : p.m_geometricNetworks) {
		if (exclusive || nodeIDs.find(n.m_id) != nodeIDs.end())
			storeState(n, m_nodeStates[n.m_id]);
		// search nodes
		for (const VICUS::NetworkNode & no : n.m_nodes) {
			if (exclusive || nodeIDs.find(no.m_id) != nodeIDs.end())
				storeState(no, m_nodeStates[no.m_id]);
		}
		// search edges
		for (const VICUS::NetworkEdge & ne : n.m_edges) {
			if (exclusive || nodeIDs.find(ne.m_id) != nodeIDs.end())
				storeState(ne, m_nodeStates[ne.m_id]);
		}
	}

	for (const VICUS::Drawing & d : p.m_drawings) {
		if (exclusive || nodeIDs.find(d.m_id) != nodeIDs.end())
			storeState(d, m_nodeStates[d.m_id]);

		for (const VICUS::DrawingLayer & dl : d.m_drawingLayers) {
			if (exclusive || nodeIDs.find(dl.m_id) != nodeIDs.end())
				storeState(dl, m_nodeStates[dl.m_id]);
		}
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
		// in exclusive mode, we turn all node states opposite, for nodes *not* in nodeIDs
		if (exclusive) {
			if (nodeIDs.find(s.first) == nodeIDs.end()) {
				switch (t) {
					case VisibilityState :
						if (!on)
							s.second |= VisibilityState;
						else
							s.second &= ~VisibilityState;
					break;
					case SelectedState :
						if (!on) {
							s.second |= SelectedState;
						}
						else
							s.second &= ~SelectedState;
					break;
				}
			}
		}
	}
}


SVUndoTreeNodeState * SVUndoTreeNodeState::createUndoAction(const QString & label,
															SVUndoTreeNodeState::NodeState t,
															unsigned int nodeID, bool withChildren, bool on, bool exclusive)
{
	std::set<unsigned int> nodeIDs;
	const VICUS::Project & p = project();

	// compose set of nodeIDs - first store object that is referenced by nodeID
	nodeIDs.insert(nodeID);

	// now find the object collect child IDs
	if (withChildren) {
		const VICUS::Object * obj = p.objectById(nodeID);
		if (obj != nullptr) {
			// also store IDs of all children
			obj->collectChildIDs(nodeIDs);
		}
		// Since VICUS::PlainGeometry is not derived from VICUS::Object there is no hierarchy
		// therefore we need to handle all surfaces in an extra loop and add their IDs
		if(nodeID == 0) {
			for (const VICUS::Surface &s : project().m_plainGeometry.m_surfaces)
				nodeIDs.insert(s.m_id);
		}
	}

	// now use the regular constructor to create the undo action
	return new SVUndoTreeNodeState(label, t, nodeIDs, on, exclusive);
}


void SVUndoTreeNodeState::undo() {
	redo(); // same stuff as for redos
}

void SVUndoTreeNodeState::setStateChildSurface(std::vector<unsigned int> & modifiedIDs,
											   std::map<unsigned int, int >::const_iterator it, VICUS::Surface &s) {
	for(const VICUS::Surface &cs : s.childSurfaces()) {
		VICUS::Surface &childSurf = const_cast<VICUS::Surface&>(cs);
		if ((it = m_nodeStates.find(childSurf.m_id)) != m_nodeStates.end()) {
			setState(childSurf, it->second);
			modifiedIDs.push_back(it->first);
		}
		setStateChildSurface(modifiedIDs, it, childSurf);
	}
}

void SVUndoTreeNodeState::storeStateChildSurface(const VICUS::Surface &s, const std::set<unsigned int> &nodeIDs, bool exclusive) {
	for (const VICUS::Surface &childSurf : s.childSurfaces()) {
		if (exclusive || nodeIDs.find(childSurf.m_id) != nodeIDs.end())
			storeState(childSurf, m_nodeStates[childSurf.m_id]);
		storeStateChildSurface(childSurf, nodeIDs, exclusive);
	}
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

	// process all buildings
	for (VICUS::Building & b : p.m_buildings) {
		if ((it = m_nodeStates.find(b.m_id)) != m_nodeStates.end()) {
			setState(b, it->second);
			modifiedIDs.push_back(it->first);
		}
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			if ((it = m_nodeStates.find(bl.m_id)) != m_nodeStates.end()) {
				setState(bl, it->second);
				modifiedIDs.push_back(it->first);
			}
			for (VICUS::Room & r : bl.m_rooms) {
				if ((it = m_nodeStates.find(r.m_id)) != m_nodeStates.end()) {
					setState(r, it->second);
					modifiedIDs.push_back(it->first);
				}
				for (VICUS::Surface & s : r.m_surfaces) {
					if ((it = m_nodeStates.find(s.m_id)) != m_nodeStates.end()) {
						setState(s, it->second);
						modifiedIDs.push_back(it->first);
					}

					// Note: use of const-cast is ok here, since we do not modify polygons or anything else that
					//       changes triangulation
					for (VICUS::SubSurface & sub : const_cast<std::vector<VICUS::SubSurface> &>(s.subSurfaces())) {
						if ((it = m_nodeStates.find(sub.m_id)) != m_nodeStates.end()) {
							setState(sub, it->second);
							modifiedIDs.push_back(it->first);
						}
					}

					setStateChildSurface(modifiedIDs, it, s);
				}
			}
		}
	}

	// search in plain geometry
	for (VICUS::Surface & s : p.m_plainGeometry.m_surfaces) {
		if ((it = m_nodeStates.find(s.m_id)) != m_nodeStates.end()) {
			setState(s, it->second);
			modifiedIDs.push_back(it->first);
		}
	}

	if ((it = m_nodeStates.find(0)) != m_nodeStates.end()) {
		setState(const_cast<VICUS::PlainGeometry&>(project().m_plainGeometry), it->second);
		modifiedIDs.push_back(0);
	}

	// search in networks
	for (VICUS::Network & n : p.m_geometricNetworks) {
		if ((it = m_nodeStates.find(n.m_id)) != m_nodeStates.end()) {
			setState(n, it->second);
			modifiedIDs.push_back(it->first);
		}
		// search nodes
		for (VICUS::NetworkNode & no : n.m_nodes) {
			if ((it = m_nodeStates.find(no.m_id)) != m_nodeStates.end()) {
				setState(no, it->second);
				modifiedIDs.push_back(it->first);
			}
		}
		// search edges
		for (VICUS::NetworkEdge & ne : n.m_edges) {
			if ((it = m_nodeStates.find(ne.m_id)) != m_nodeStates.end()) {
				setState(ne, it->second);
				modifiedIDs.push_back(it->first);
			}
		}
	}

	// modify drawing state
	for (VICUS::Drawing &d : p.m_drawings) {
		if ((it = m_nodeStates.find(d.m_id)) != m_nodeStates.end()) {
			setState(d, it->second);
			modifiedIDs.push_back(it->first);
		}

		for (VICUS::DrawingLayer & dl : d.m_drawingLayers) {
			if ((it = m_nodeStates.find(dl.m_id)) != m_nodeStates.end()) {
				setState(dl, it->second);
				modifiedIDs.push_back(it->first);
			}
		}
	}

	// now swap the states
	m_nodeStates.swap(m_otherNodeStates);
	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NodeStateModified, &modInfo);
}
