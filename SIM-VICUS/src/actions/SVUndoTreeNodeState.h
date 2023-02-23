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

#ifndef SVUndoTreeNodeStateH
#define SVUndoTreeNodeStateH

#include "SVUndoCommandBase.h"

#include <map>

#include <VICUS_Surface.h>

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
		std::vector<unsigned int>	m_nodeIDs; // unique IDs
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

    /*! Set states of child surfaces. */
    void setStateChildSurface(std::vector<unsigned int> & modifiedIDs,
                              std::map<unsigned int, int >::const_iterator it, VICUS::Surface &s);

    /*! Set states of child surfaces. */
    void storeStateChildSurface(const VICUS::Surface &s, const std::set<unsigned int> & nodeIDs, bool exclusive);

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
