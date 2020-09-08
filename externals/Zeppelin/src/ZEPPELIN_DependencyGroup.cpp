/*	The Zeppelin graph algorithm library.
Copyright(c) 2010-2017, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A.Paepcke		<anne.paepcke - [at] - tu - dresden.de>
All rights reserved.

This library is free software; you can redistribute it and / or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
Lesser General Public License for more details.
*/

#include "ZEPPELIN_DependencyGroup.h"

#include <cassert>
#include <iterator>

namespace ZEPPELIN {

const DependencyObject::DependencySequence & DependencyGroup::dependencies() const {
	return m_childs;
}


const DependencyObject::DependencySequence & DependencyGroup::dependencyObjects() const {
	return m_depObjects;
}

const DependencyObject::DependencySequence & DependencyGroup::parents() const {
	return m_parents;
}


void DependencyGroup::createEulerPathesInDirectedSubGraph(std::vector<std::list<const DependencyObject*> > &eulerPathes) {

	// create connections
	const DependencyObject::DependencySequence &objects = m_depObjects;

	// ignore empty groups
	if (objects.empty())
		return;

	switch (m_type) {
		case ZEPPELIN::DependencyGroup::SEQUENTIAL: {

			std::list<const ZEPPELIN::DependencyObject*> eulerPath;
			// for a sequential group register all nodes in their appearance
			for (unsigned int o = 0; o < objects.size(); ++o) {
				// register all backward connections
				const DependencyObject* node = objects[o];
				eulerPath.push_back(node);
			}
			eulerPathes.push_back(eulerPath);
		}
		break;
		case ZEPPELIN::DependencyGroup::CYCLIC: {
			// store a list of dummy nodes in order to create an euler cycle
			//std::list<ZEPPELIN::DependencyObject> dummyObjects;

			struct ObjectWithIndex : public ZEPPELIN::DependencyObject {
				ObjectWithIndex(const ZEPPELIN::DependencyObject &object) :
					ZEPPELIN::DependencyObject(object) {
					m_index = 0;
				}

				unsigned int m_index;
			};

			// copy of objects careting an euler graph
			std::list<ObjectWithIndex> eulerGraph;

			// create an euler path including additional elements and excudig siources and sinks
			for (unsigned int i = 0; i < m_depObjects.size(); ++i) {
				// create a copy
				ObjectWithIndex eulerGraphObject = *m_depObjects[i];
				eulerGraphObject.m_index = i;
				// clear connections of current oject
				eulerGraphObject.clear();
				// add to euler graph
				eulerGraph.push_back(eulerGraphObject);
			}

			// list of objects with missing forward or backward connections
			// if more than one connection is missing, we register object several times
			std::vector<ObjectWithIndex*> objectsWithMissingBackwardConnections;
			std::vector<ObjectWithIndex*> objectsWithMissingForwardConnections;

			// create connection of graph
			std::list<ObjectWithIndex>::iterator it = eulerGraph.begin();
			for (unsigned int i = 0; i < m_depObjects.size(); ++i, ++it) {
				const ZEPPELIN::DependencyObject *object = m_depObjects[i];
				// get all connections
				std::set<const ZEPPELIN::DependencyObject*> backwardConnections;
				std::set<const ZEPPELIN::DependencyObject*> forwardConnections;
				for (unsigned int j = 0; j < object->dependencies().size(); ++j) {
					const ZEPPELIN::DependencyObject* depObject = object->dependencies()[j];
					// we only consider objects of the current group
					std::vector<ZEPPELIN::DependencyObject*>::iterator fIt =
						std::find(m_depObjects.begin(), m_depObjects.end(),
							depObject);

					if (fIt != m_depObjects.end()) {
						backwardConnections.insert(depObject);
						// register dependency
						int pos = std::distance(m_depObjects.begin(), fIt);
						// find object in list
						std::list<ObjectWithIndex>::iterator eulerDepObject =
							eulerGraph.begin();
						// and register a dependency to an indexed object
						std::advance(eulerDepObject, pos);
						it->dependsOn(*eulerDepObject);
					}
				}
				for (unsigned int j = 0; j < object->parents().size(); ++j) {
					const ZEPPELIN::DependencyObject* parObject = object->parents()[j];
					// we only consider objects of the current group
					if (std::find(m_depObjects.begin(), m_depObjects.end(), parObject)
						!= m_depObjects.end()) {
						forwardConnections.insert(parObject);
					}
				}
				// update parents
				it->updateParents();

				// in a euler graph the number of forward connections must equal the
				// number of backward connections
				// otherwise we create invalid connections
				if (forwardConnections.size() > backwardConnections.size()) {
					for (unsigned int j = (unsigned int)backwardConnections.size();
						j < (unsigned int)forwardConnections.size(); ++j)
						objectsWithMissingBackwardConnections.push_back(&(*it));
				}
				if (backwardConnections.size() > forwardConnections.size()) {
					for(unsigned int j = (unsigned int) forwardConnections.size();
						j < (unsigned int)backwardConnections.size(); ++j)
						objectsWithMissingForwardConnections.push_back(&(*it));
				}
			}

			// subgraph must include the same number of missing forward and backward connections
			assert(objectsWithMissingBackwardConnections.size() ==
				objectsWithMissingForwardConnections.size());

			// connect pairs of nodes
			for (unsigned int i = 0; i < objectsWithMissingBackwardConnections.size(); ++i) {
				// create a copy
				ObjectWithIndex *object = objectsWithMissingBackwardConnections[i];
				ObjectWithIndex *nextObject = objectsWithMissingForwardConnections[i];

				// add to euler cycle
				ObjectWithIndex dummyGraphObject = ZEPPELIN::DependencyObject();
				dummyGraphObject.m_index = (unsigned int)m_depObjects.size();
				eulerGraph.push_back(dummyGraphObject);

				// create a dummy connector
				//dummyObjects.push_back(ZEPPELIN::DependencyObject());
				ZEPPELIN::DependencyObject &dummyNode = eulerGraph.back();
				// connect in all directions
				object->dependsOn(dummyNode);
				dummyNode.dependsOn(*object);
				nextObject->dependsOn(dummyNode);
				dummyNode.dependsOn(*nextObject);

				// update parents
				object->updateParents();
				nextObject->updateParents();
				dummyNode.updateParents();
			}

			// register all pairs of connections
			std::set < std::pair<const ZEPPELIN::DependencyObject*,
				const ZEPPELIN::DependencyObject*> > registeredConnections;

			// create a local euler graph
			std::list<const ObjectWithIndex*> eulerCycle;

			// store all checked nodes
			std::set<const ObjectWithIndex*> startNodes;
			// set first node to source
			eulerCycle.push_back(&(*eulerGraph.begin()));
			std::list<const ObjectWithIndex*>::iterator startNode =
				eulerCycle.begin();

			// if all connections of all nodes are registered we continue in global loop
			while (startNode != eulerCycle.end()) {

				// startNode was visited already
				if (startNodes.find(*startNode) != startNodes.end()) {
					++startNode;
					continue;
				}

				const ObjectWithIndex *node = *startNode;
				// find all forward connection
				const DependencyObject::DependencySequence &parObjects = node->parents();
				// isolated node
				if (parObjects.empty()) {
					++startNode;
					continue;
				}

				// set iterator for inserting position inside list
				std::list<const ObjectWithIndex*>::iterator insertPos =
					startNode;
				++insertPos;

				const ObjectWithIndex* nextNode = nullptr;

				std::list<const ObjectWithIndex*> newPath;

				while (nextNode != *startNode) {
					// find all forward connection
					const DependencyObject::DependencySequence &parObjects = node->parents();
					// register object node
					unsigned int p = 0;

					for (; p < parObjects.size(); ++p) {
						nextNode = dynamic_cast<const ObjectWithIndex*>(parObjects[p]);

						// check if node fulfills conditions
						std::pair<const ZEPPELIN::DependencyObject*,
							const ZEPPELIN::DependencyObject*>  nextConnection =
							std::make_pair(node, nextNode);
						// connection was already visited
						if (registeredConnections.find(nextConnection) != registeredConnections.end())
							continue;
						// register connection in local container
						registeredConnections.insert(nextConnection);
						// add node
						newPath.push_back(nextNode);
						// set a new node
						node = nextNode;
						break;
					}

					// no path found from current node
					if (newPath.empty()) {
						// register start nodes
						startNodes.insert(*startNode);
						++startNode;
						break;
					}

					// no valid node found at the end of a new path
					assert(p != (unsigned int)parObjects.size());
				}

				// add new list of nodes
				if (!newPath.empty())
					eulerCycle.insert(insertPos, newPath.begin(), newPath.end());
			}

			std::list<const ZEPPELIN::DependencyObject *> eulerPath;

			// copy into new pathes ignoring dummy nodes
			for (std::list<const ObjectWithIndex*>::const_iterator
				it = eulerCycle.begin(); it != eulerCycle.end(); ++it) {

				unsigned int index = (*it)->m_index;
				// cut cycle at invalid nodes
				if (index >= (unsigned int)m_depObjects.size()) {
					if (!eulerPath.empty())
						eulerPathes.push_back(eulerPath);
					// prepare for next section
					eulerPath.clear();
					continue;
				}
				// add suitable object to graph
				eulerPath.push_back(m_depObjects[index]);
			}

			// add last element
			if (!eulerPath.empty())
				eulerPathes.push_back(eulerPath);
		}
		break;
	}
}


void DependencyGroup::createEulerPathesInUndirectedSubGraph(std::vector<std::list<const DependencyObject*> > &eulerPathes) {

	// create connections
	const DependencyObject::DependencySequence &objects = m_depObjects;

	// ignore empty groups
	if (objects.empty())
		return;

	switch (m_type) {
	case ZEPPELIN::DependencyGroup::SEQUENTIAL: {

		std::list<const ZEPPELIN::DependencyObject*> eulerPath;
		// for a sequential group register all nodes in their appearance
		for (unsigned int o = 0; o < objects.size(); ++o) {
			// register all backward connections
			const DependencyObject* node = objects[o];
			eulerPath.push_back(node);
		}
		eulerPathes.push_back(eulerPath);
	}
	break;
	case ZEPPELIN::DependencyGroup::CYCLIC: {
		// store a list of dummy nodes in order to create an euler cycle
		//std::list<ZEPPELIN::DependencyObject> dummyObjects;

		struct ObjectWithIndex : public ZEPPELIN::DependencyObject {
			ObjectWithIndex(const ZEPPELIN::DependencyObject &object) :
				ZEPPELIN::DependencyObject(object) {
				m_index = 0;
			}

			unsigned int m_index;
		};

		// copy of objects careting an euler graph
		std::list<ObjectWithIndex> eulerGraph;

		// create an euler path including additional elements and excudig siources and sinks
		for (unsigned int i = 0; i < m_depObjects.size(); ++i) {
			// create a copy
			ObjectWithIndex eulerGraphObject = *m_depObjects[i];
			eulerGraphObject.m_index = i;
			// clear connections of current oject
			eulerGraphObject.clear();
			// add to euler graph
			eulerGraph.push_back(eulerGraphObject);
		}

		// list of uneven objects
		std::vector<ObjectWithIndex*> objectsWithUnevenConnections;

		// create connection of graph
		std::list<ObjectWithIndex>::iterator it = eulerGraph.begin();
		for (unsigned int i = 0; i < m_depObjects.size(); ++i, ++it) {
			const ZEPPELIN::DependencyObject *object = m_depObjects[i];
			// get all connections
			std::set<const ZEPPELIN::DependencyObject*> connections;
			for (unsigned int j = 0; j < object->dependencies().size(); ++j) {
				const ZEPPELIN::DependencyObject* depObject = object->dependencies()[j];
				// we only consider objects of the current group
				std::vector<ZEPPELIN::DependencyObject*>::iterator fIt =
					std::find(m_depObjects.begin(), m_depObjects.end(),
						depObject);

				if (fIt != m_depObjects.end()) {
					connections.insert(depObject);
					// register dependency
					int pos = std::distance(m_depObjects.begin(), fIt);
					// find object in list
					std::list<ObjectWithIndex>::iterator eulerDepObject =
						eulerGraph.begin();
					// and register a dependency to an indexed object
					std::advance(eulerDepObject, pos);
					it->dependsOn(*eulerDepObject);
				}
			}
			// symmetric treatment of paranet objects
			for (unsigned int j = 0; j < object->parents().size(); ++j) {
				const ZEPPELIN::DependencyObject* parObject = object->parents()[j];
				// we only consider objects of the current group
				std::vector<ZEPPELIN::DependencyObject*>::iterator fIt =
					std::find(m_depObjects.begin(), m_depObjects.end(),
						parObject);

				// we only consider objects of the current group
				if (fIt != m_depObjects.end()) {
					connections.insert(parObject);
					int pos = std::distance(m_depObjects.begin(), fIt);
					// find object in list
					std::list<ObjectWithIndex>::iterator eulerDepObject =
						eulerGraph.begin();
					// and register a dependency to an indexed object
					std::advance(eulerDepObject, pos);
					it->dependsOn(*eulerDepObject);
				}
			}
			// update parents
			it->updateParents();

			// register object with uneven connections
			if ((unsigned int)connections.size() % 2 != 0)
				objectsWithUnevenConnections.push_back(&(*it));
		}

		// only an even number of objects with uneven connections is accepted
		assert((unsigned int)objectsWithUnevenConnections.size() % 2 == 0);

		// connect pairs of nodes
		for (unsigned int i = 0; i < objectsWithUnevenConnections.size() / 2; ++i) {
			// create a copy
			ObjectWithIndex *object = objectsWithUnevenConnections[2 * i];
			ObjectWithIndex *nextObject = objectsWithUnevenConnections[2 * i + 1];

			// add to euler cycle
			ObjectWithIndex dummyGraphObject = ZEPPELIN::DependencyObject();
			dummyGraphObject.m_index = (unsigned int)m_depObjects.size();
			eulerGraph.push_back(dummyGraphObject);

			// create a dummy connector
			//dummyObjects.push_back(ZEPPELIN::DependencyObject());
			ZEPPELIN::DependencyObject &dummyNode = eulerGraph.back();
			// connect in all directions
			object->dependsOn(dummyNode);
			dummyNode.dependsOn(*object);
			nextObject->dependsOn(dummyNode);
			dummyNode.dependsOn(*nextObject);

			// update parents
			object->updateParents();
			nextObject->updateParents();
			dummyNode.updateParents();
		}

		// register all pairs of connections
		std::set < std::pair<const ZEPPELIN::DependencyObject*,
			const ZEPPELIN::DependencyObject*> > registeredConnections;

		// create a local euler graph
		std::list<const ObjectWithIndex*> eulerCycle;

		// store all checked nodes
		std::set<const ObjectWithIndex*> startNodes;
		// set first node to source
		eulerCycle.push_back(&(*eulerGraph.begin()));
		std::list<const ObjectWithIndex*>::iterator startNode =
			eulerCycle.begin();

		// if all connections of all nodes are registered we continue in global loop
		while (startNode != eulerCycle.end()) {

			// startNode was visited already
			if (startNodes.find(*startNode) != startNodes.end()) {
				++startNode;
				continue;
			}

			const ObjectWithIndex *node = *startNode;
			// find all forward connection
			const DependencyObject::DependencySequence &parObjects = node->parents();
			// isolated node
			if (parObjects.empty()) {
				// register start nodes
				startNodes.insert(*startNode);
				++startNode;
				continue;
			}

			// set iterator for inserting position inside list
			std::list<const ObjectWithIndex*>::iterator insertPos =
				startNode;
			++insertPos;

			const ObjectWithIndex* nextNode = nullptr;

			std::list<const ObjectWithIndex*> newPath;

			while (nextNode != *startNode) {
				// find all forward connection
				const DependencyObject::DependencySequence &parObjects = node->parents();
				// register object node
				unsigned int p = 0;

				for (; p < parObjects.size(); ++p) {
					nextNode = dynamic_cast<const ObjectWithIndex*>(parObjects[p]);

					// check if node fulfills conditions
					std::pair<const ZEPPELIN::DependencyObject*,
						const ZEPPELIN::DependencyObject*>  nextConnection =
						std::make_pair(node, nextNode);
					// connection was already visited
					if (registeredConnections.find(nextConnection) != registeredConnections.end())
						continue;
					// register connection in local container
					registeredConnections.insert(nextConnection);
					// register inverse connection in local container
					std::pair<const ZEPPELIN::DependencyObject*,
						const ZEPPELIN::DependencyObject*> inverseConnection =
						std::make_pair(nextNode, node);
					registeredConnections.insert(inverseConnection);
					// add node
					newPath.push_back(nextNode);
					// set a new node
					node = nextNode;
					break;
				}

				// no path found from current node
				if (newPath.empty()) {
					++startNode;
					break;
				}

				// no valid node found at the end of a new path
				assert(p != (unsigned int)parObjects.size());
			}

			// add new list of nodes
			if (!newPath.empty())
				eulerCycle.insert(insertPos, newPath.begin(), newPath.end());
		}

		std::list<const ZEPPELIN::DependencyObject *> eulerPath;

		// copy into new pathes ignoring dummy nodes
		for (std::list<const ObjectWithIndex*>::const_iterator
			it = eulerCycle.begin(); it != eulerCycle.end(); ++it) {

			unsigned int index = (*it)->m_index;
			// cut cycle at invalid nodes
			if (index >= (unsigned int)m_depObjects.size()) {
				if (!eulerPath.empty())
					eulerPathes.push_back(eulerPath);
				// prepare for next section
				eulerPath.clear();
				continue;
			}
			// add suitable object to graph
			eulerPath.push_back(m_depObjects[index]);
		}

		// add last element
		if (!eulerPath.empty())
			eulerPathes.push_back(eulerPath);
	}
	break;
	default: break;
	}
}


void DependencyGroup::set(const DependencyObject::DependencySequence & depObjectVector) {
	m_childs.clear();
	m_depObjects.clear();
	m_parents.clear();

	for (DependencyObject::DependencySequence::const_iterator it = depObjectVector.begin(); it != depObjectVector.end(); ++it)
		insert(*it);
}

void DependencyGroup::insert(DependencyObject* o) {
	// insert all member into current group
	DependencyObject::DependencySequence::iterator it = std::find(m_depObjects.begin(), m_depObjects.end(), o);
	// object is already inside the group
	if(it != m_depObjects.end())
		return;

	// object is a group itself
	DependencyGroup *group = dynamic_cast<DependencyGroup *>(o);
	// resove groups before inserting the elements
	if(group != nullptr)
	{
		// insert all group elements: note that we are only allowed to use
		// the internal insert function
		m_depObjects.insert(m_depObjects.end(), group->m_depObjects.begin(), group->m_depObjects.end());

		// add all dependencies for the group
		for (unsigned int i = 0; i < group->dependencies().size(); ++i) {
			if (std::find(m_childs.begin(), m_childs.end(), group->dependencies()[i]) ==
				m_childs.end())
				m_childs.push_back(group->dependencies()[i]); // store in global vector
		}

		// remove all object deps that are group members itself
		for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
			it != m_depObjects.end(); ++it)
		{
			DependencyObject::DependencySequence::iterator delIt =
				std::find(m_childs.begin(), m_childs.end(), *it);
			if (delIt != m_childs.end())
				m_childs.erase(delIt);
		}
		// erase the group element itself
		DependencyObject::DependencySequence::iterator globDelIt =
			std::find(m_childs.begin(), m_childs.end(), o);
		if (globDelIt != m_childs.end())
			m_childs.erase(globDelIt);

		// add all parents for the group
		for (unsigned int i = 0; i < group->parents().size(); ++i) {
			if (std::find(m_parents.begin(), m_parents.end(), group->parents()[i]) ==
				m_parents.end())
				m_parents.push_back(group->parents()[i]); // store in global set
		}
		// remove all object deps that are group members itself
		for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
			it != m_depObjects.end(); ++it)
		{
			DependencyObject::DependencySequence::iterator delIt =
				std::find(m_parents.begin(), m_parents.end(), *it);
			if (delIt != m_parents.end())
				m_parents.erase(delIt);
		}
		// erase the group element itself
		globDelIt =	std::find(m_parents.begin(), m_parents.end(), o);
		if (globDelIt != m_parents.end())
			m_parents.erase(globDelIt);
	}
	else {
		m_depObjects.insert(m_depObjects.end(),o);

		// update all dependencies
		for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
			it != m_depObjects.end(); ++it)
		{
			// get dependencies of current dependency object
			const DependencyObject::DependencySequence objDeps = (*it)->dependencies();
			for (unsigned int i = 0; i < objDeps.size(); ++i) {
				if (std::find(m_childs.begin(), m_childs.end(), objDeps[i]) ==
					m_childs.end())
					m_childs.push_back(objDeps[i]); // store in global set
			}
		}
		// remove all object deps that are group members itself
		for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
			it != m_depObjects.end(); ++it)
		{
			DependencyObject::DependencySequence::iterator delIt =
				std::find(m_childs.begin(), m_childs.end(), *it);
			if (delIt != m_childs.end())
				m_childs.erase(delIt);
		}

		for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
			it != m_depObjects.end(); ++it)
		{
			// get parents of current dependency object
			DependencyObject::DependencySequence parents = (*it)->parents();

			for (unsigned int i = 0; i < parents.size(); ++i) {
				if (std::find(m_parents.begin(), m_parents.end(), parents[i]) ==
					m_parents.end())
				m_parents.push_back(parents[i]); // store in global vector
			}
		}
		// remove all object deps that are group members itself
		for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
			it != m_depObjects.end(); ++it)
		{
			DependencyObject::DependencySequence::iterator delIt =
				std::find(m_parents.begin(), m_parents.end(), *it);
			if (delIt != m_parents.end())
				m_parents.erase(delIt);
		}
	}
}


void DependencyGroup::erase(DependencyObject* o) {
	// erase member from current group
	DependencyObject::DependencySequence::iterator it = std::find(m_depObjects.begin(), m_depObjects.end(), o);
	// object is not member of dependend object vector
	if(it == m_depObjects.end())
		return;
	m_depObjects.erase(it);

	// update all dependencies
	m_childs.clear();
	for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
		it != m_depObjects.end(); ++it)
	{
		// get dependencies of current dependency object
		const DependencyObject::DependencySequence objDeps = (*it)->dependencies();
		for (unsigned int i = 0; i < objDeps.size(); ++i) {
			if (std::find(m_childs.begin(), m_childs.end(), objDeps[i]) ==
				m_childs.end())
				m_childs.push_back(objDeps[i]); // store in global set
		}
	}
	// remove all object deps that are group members itself
	for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
		it != m_depObjects.end(); ++it)
	{
		DependencyObject::DependencySequence::iterator delIt =
			std::find(m_childs.begin(), m_childs.end(), *it);
		if (delIt != m_childs.end())
			m_childs.erase(delIt);
	}

	m_parents.clear();
	for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
		it != m_depObjects.end(); ++it)
	{
		// get parents of current dependency object
		DependencyObject::DependencySequence parents = (*it)->parents();
		for (unsigned int i = 0; i < parents.size(); ++i) {
			if (std::find(m_parents.begin(), m_parents.end(), parents[i]) ==
				m_parents.end())
				m_parents.push_back(parents[i]); // store in global set
		}
	}
	// remove all object deps that are group members itself
	for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin();
		it != m_depObjects.end(); ++it)
	{
		DependencyObject::DependencySequence::iterator delIt =
			std::find(m_parents.begin(), m_parents.end(), *it);
		if (delIt != m_parents.end())
			m_parents.erase(delIt);
	}
}


bool DependencyGroup::hasIntersection(const DependencyGroup* group) const  {

	for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin(); it != m_depObjects.end(); ++it) {
		// find group member inside other group
		DependencyObject::DependencySequence::const_iterator object =
			std::find(group->m_depObjects.begin(),group->m_depObjects.end(),*it);
		if (object != group->m_depObjects.end())
			return true;
	}
	return false;
}


void DependencyGroup::merge(const DependencyGroup* group) {
	// insert all group members into current object
	for(DependencyObject::DependencySequence::const_iterator it = group->m_depObjects.begin(); it != group->m_depObjects.end(); ++it) {
		insert(*it);
	}
}


void DependencyGroup::intersect(const DependencyGroup* group) {
	// cut all members that are not inside both groups
	for (DependencyObject::DependencySequence::const_iterator it = m_depObjects.begin(); it != m_depObjects.end(); ++it) {
		// find group member inside other group
		DependencyObject::DependencySequence::const_iterator object =
			std::find(group->m_depObjects.begin(),group->m_depObjects.end(),*it);
		// not found: erase member
		if (object == group->m_depObjects.end())
			erase(*it);
	}
}


void DependencyGroup::complement(const DependencyGroup* group) {
	// insert all group members into current object
	for (DependencyObject::DependencySequence::const_iterator it = group->m_depObjects.begin(); it != group->m_depObjects.end(); ++it) {
		erase(*it);
	}
}


} // namespace ZEPPELIN
