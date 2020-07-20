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

#include "ZEPPELIN_DependencyGraph.h"

#include <stdexcept>
#include <assert.h>
#include <iterator>
#include <iostream>

namespace ZEPPELIN {

// Dummy implementation of destructor to avoid v-table warning
DependencyObject::~DependencyObject() {
}


void DependencyGraph::setObjects(DependencyObject::DependencySequence & objects,
								 std::list<DependencyGroup> &objectGroups)
{
	//const char * const FUNC_ID = "[DependencyGraph::setObjects]";
	clear();

	// copy object list
	m_objects.reserve(objects.size());
	std::copy(objects.begin(), objects.end(), std::back_inserter(m_objects));
	// fill graph with cycles
	clusterGraph(objectGroups);
	// cut all sinks and set m_ordered objects
	orderGraph();
}


#ifdef USE_EAS_ALGORITHM

void DependencyGraph::clusterGraph(std::list<DependencyGroup> &objectGroups) {

	/********************* Identify sequential and cyclic groups ***************************************/

	// identify all sequeneces and cycles
	std::vector<DependencyObject::DependencySequence> sequences, cycles;
	findCyclesAndSequences(cycles, sequences);

	for(unsigned int i = 0; i < sequences.size(); ++i) {
		DependencyGroup group(DependencyGroup::SEQUENTIAL);
		// add objects to group
		DependencyObject::DependencySequence &sequence = sequences[i];

		for(unsigned int j = 0; j < sequence.size(); ++j) {
			group.insert(sequence[j]);
		}
		objectGroups.push_back(group);
	}
	for(unsigned int i = 0; i < cycles.size(); ++i) {
		DependencyGroup group(DependencyGroup::CYCLIC);
		// add objects to group
		DependencyObject::DependencySequence &cycle = cycles[i];

		for(unsigned int j = 0; j < cycle.size(); ++j) {
			group.insert(cycle[j]);
		}
		objectGroups.push_back(group);
	}
	// recompose the graph vector as a vector of dependency groups
	m_objects.clear();
	for (std::list<DependencyGroup>::iterator groupIt =
		objectGroups.begin(); groupIt != objectGroups.end() ; ++groupIt) {
		m_objects.push_back(&(*groupIt));
	}
	// update all dependencies:
	// simply add a group as a dependency to all objects that depend
	// on one group member
	// we start with the last iterator position inside group vector
	for (std::list<DependencyGroup>::iterator groupIt =
		objectGroups.begin(); groupIt != objectGroups.end() ; ++groupIt) {
		// sequential nodes are depObject-eleemnts of the groups
		for (DependencyObject::DependencySequence::const_iterator nodeIt =
			groupIt->depObjects().begin();  nodeIt != groupIt->depObjects().end();
			++ nodeIt)
		{
			// manipulate dependency object dependencies
			for (DependencyObject::DependencySequence::iterator parent
				= m_objects.begin(); parent != m_objects.end(); ++parent)
			{
				// no parent node
				if (std::find((*parent)->dependencies().begin(), (*parent)->dependencies().end(), *nodeIt)
					== (*parent)->dependencies().end())
					continue;
				// add group as a new dependency
				(*parent)->dependsOn(*groupIt);
			}
		}
		// update parent connections
		groupIt->updateParents();
	}
}


void DependencyGraph::findCyclesAndSequences(std::vector<DependencyObject::DependencySequence> &cycles,
							std::vector<DependencyObject::DependencySequence> &sequences)
{
	DependencyObject::DependencySequence remainingGraph = m_objects;
	// create a graph from all objects
	// loop until graph is empty
	while(!remainingGraph.empty()) {

		DependencyObject::DependencySequence sources, sinks;
		// store graph size to ensure whetehr iot changed or not
		unsigned int graphSize = 10000;
		// remove all sequential depdnencies
		while(graphSize > remainingGraph.size() ) {
			// overwrite size storage
			graphSize = remainingGraph.size();
			// remove next sources and sinks
			popSourcesFromGraph(sources, remainingGraph);
			popSinksFromGraph(sinks, remainingGraph);
		}

		std::set<DependencyObject *> registeredSources;
		// create sequences from all sources: start with first sources
		for(unsigned int i = 0; i < sources.size(); ++i) {
			DependencyObject *source = sources[i];
			// skip already registered sources
			if(registeredSources.find(source) != registeredSources.end() )
				continue;

			sequences.push_back(DependencyObject::DependencySequence());
			// create a single element sequence
			DependencyObject::DependencySequence &sequence = sequences.back();
			sequence.push_back(source);
			// store source as registered
			registeredSources.insert(source);

			// isolated source
			if(source->dependencies().empty()) {
				continue;
			}
			// go to next dependency
			while(source->dependencies().size() == 1) {
				source = *source->dependencies().begin();
				// only accept single connections
				if(source->parents().size() != 1)
					break;
				// if source has only one parent it must be found by the source/sink search
				// therefore it cannot be part of the remaining grapg any longer (if
				// there are no programming errors)
				assert(std::find(remainingGraph.begin(), remainingGraph.end(), source)
					== remainingGraph.end() );
				// add to container
				sequence.push_back(source);
				// store source as registered
				registeredSources.insert(source);
			}
			// reverse sequence
			std::reverse(sequence.begin(), sequence.end());
		}
		// and sinks
		std::set<DependencyObject *> registeredSinks;
		// create sequences from all sources: start with first sources
		for(unsigned int i = 0; i < sinks.size(); ++i) {
			DependencyObject *sink = sinks[i];
			// skip already registered sinks
			if(registeredSinks.find(sink) != registeredSinks.end() )
				continue;
			// skip already registered sources
			if(registeredSources.find(sink) != registeredSources.end() )
				continue;

			sequences.push_back(DependencyObject::DependencySequence());
			// create a single element sequence
			DependencyObject::DependencySequence &sequence = sequences.back();
			sequence.push_back(sink);
			// store source as registered
			registeredSinks.insert(sink);

			// isolated sink
			if(sink->parents().empty()) {
				continue;
			}
			// go to next dependency
			while(sink->parents().size() == 1) {
				sink = *sink->parents().begin();
				// only accept single connections
				if(sink->dependencies().size() != 1)
					break;
				// if sink has only one child it must be found by the source/sink search
				// therefore it cannot be part of the remaining graph any longer (if
				// there are no programming errors)
				assert(std::find(remainingGraph.begin(), remainingGraph.end(), sink)
					== remainingGraph.end() );
				// we dont assume that current sink is inside sinks container - we allow
				// a sink also to be a source and to be selected by the sources container

				// add sink to seqeunce container
				sequence.insert(sequence.begin(), sink);
				// store source as registered
				registeredSinks.insert(sink);
			}
			// reverse sequence
			std::reverse(sequence.begin(), sequence.end());
		}

		// no cycles
		if(remainingGraph.empty() )
			break;

		DependencyObject::DependencySequence cyclicObjects;
		// remove the next cycle we find
		popNextCycleFromGraph(cyclicObjects, remainingGraph);
		// there must! be a cycle
		assert(!cyclicObjects.empty());
		cycles.push_back(cyclicObjects);
	}
	assert(remainingGraph.empty());
}

void DependencyGraph::popSourcesFromGraph( DependencyObject::DependencySequence &sources,
		DependencyObject::DependencySequence &graph)
{
	// erase all sinks and sources
	for(unsigned int i = 0; i < graph.size(); ++i) {

		DependencyObject *node = graph[i];
		// count all dependencies that are part of the graph
		unsigned int pars = 0;
		for(DependencyObject::DependencySequence::const_iterator nodeIt =
			node->parents().begin(); nodeIt != node->parents().end();
			++nodeIt) {
			// parent of onself
			if(*nodeIt == node)
				continue;
			// skip nodes that are not part of the current graph
			if (std::find(graph.begin(), graph.end(), *nodeIt)
				== graph.end()) continue;
			// otherwise count
			++pars;
			break;
		}
		// a source
		if(pars == 0) {
			sources.push_back(node);
		}
	}
	// erase sources and sinks from graph
	for(DependencyObject::DependencySequence::iterator nodeIt =
		sources.begin(); nodeIt != sources.end();
		++nodeIt) {

		DependencyObject::DependencySequence::iterator
			nodeInGraphIt = std::find(graph.begin(), graph.end(), *nodeIt);
		if(nodeInGraphIt != graph.end() )
			graph.erase(nodeInGraphIt);
	}
}



void DependencyGraph::popSinksFromGraph(DependencyObject::DependencySequence &sinks,
		DependencyObject::DependencySequence &graph)
{
	// erase all sinks and sources
	for(unsigned int i = 0; i < graph.size(); ++i) {

		DependencyObject *node = graph[i];
		// count all dependencies that are part of the graph
		unsigned int deps = 0;
		for(DependencyObject::DependencySequence::const_iterator nodeIt =
			node->dependencies().begin(); nodeIt != node->dependencies().end();
			++nodeIt) {
			// dependency of onself
			if(*nodeIt == node)
				continue;
			// skip nodes that are not part of the current graph
			if (std::find(graph.begin(), graph.end(), *nodeIt)
				== graph.end()) continue;
			// otherwise count
			++deps;
			break;
		}
		// a sink
		if(deps == 0) {
			sinks.push_back(node);
		}
	}
	// erase sinks from graph
	for(DependencyObject::DependencySequence::iterator nodeIt =
		sinks.begin(); nodeIt != sinks.end();
		++nodeIt) {

		DependencyObject::DependencySequence::iterator
			nodeInGraphIt = std::find(graph.begin(), graph.end(), *nodeIt);
		if(nodeInGraphIt != graph.end() )
			graph.erase(nodeInGraphIt);
	}
}


void DependencyGraph::popNextCycleFromGraph( DependencyObject::DependencySequence &cycle,
		DependencyObject::DependencySequence &graph)
{
	// cyclic dependencies cannot be removed as source or sink
	// fidn first cycle beginning at the first reimaning graph element
	// start with forward search
	DependencyObject::DependencySequence connectedObjects;
	findFirstConnectedNodesInGraph(connectedObjects, graph);
	// now fidn all objects backward to ensure they really are cyclic
	findFirstConnectedNodesInGraph(cycle, connectedObjects, false);
	// erase fromm remaning graph
	for(unsigned int c = 0; c < cycle.size(); ++c) {

		DependencyObject::DependencySequence::iterator
			nodeInGraphIt = std::find(graph.begin(), graph.end(),
			cycle[c]);
		assert(nodeInGraphIt != graph.end() );
		graph.erase(nodeInGraphIt);
	}
}

void DependencyGraph::findFirstConnectedNodesInGraph(DependencyObject::DependencySequence &connectedNodes,
					const DependencyObject::DependencySequence &graph,
					bool forwardSearch)
{
	if(!connectedNodes.empty())
		connectedNodes.clear();

	// no graph element
	if(graph.empty())
		return;

	// add the first node to the connected graph
	connectedNodes.push_back(graph[0]);

	unsigned int lastPos = 0;
	// loop until no changes are registered
	while(lastPos < connectedNodes.size()) {
		// set node to next element
		DependencyObject *node = connectedNodes[lastPos];

		DependencyObject::DependencySequence nextNodes;

		if(forwardSearch)
			nextNodes = node->dependencies();
		else // backward search
			nextNodes = node->parents();

		// add all dependend objects
		for(DependencyObject::DependencySequence::iterator nodeIt =
			nextNodes.begin(); nodeIt != nextNodes.end();
			++nodeIt) {
			// skip nodes that are not part of the current graph
			if (std::find(graph.begin(), graph.end(), *nodeIt)
				== graph.end()) continue;

			// skip nodes that registered already
			if (std::find(connectedNodes.begin(), connectedNodes.end(), *nodeIt)
				!= connectedNodes.end()) continue;

			// recursively add eleemnts to connected graph
			// add element to graph
			connectedNodes.push_back(*nodeIt);
		}
		// update counter
		++lastPos;
	}
}


void DependencyGraph::orderGraph() {

	// create worker set and clear target vector
	m_orderedObjects.clear();

	// objectsLeft contains all unsorted graph nodes
	DependencyObject::DependencySequence remainingGraph = m_objects;

	while (!remainingGraph.empty()) {
		// we store all found sinks in objCluster
		DependencyObject::DependencySequence sinks;
		// pop all sinks from reomaing graph
		popSinksFromGraph(sinks, remainingGraph);
		assert(!sinks.empty()); // didn't find any sinks, error in group initialisation

		// store all sinks inside a new ParallelObjects object
		m_orderedParallelObjects.push_back(ParallelObjects());
		ParallelObjects &parallelObjects = m_orderedParallelObjects.back();

		for (unsigned int i = 0; i < sinks.size(); ++i)
		{
			// insert found sinks one after another to vector
			m_orderedObjects.push_back(sinks[i]);
			// insert found sinks into parallel group set:
			// note: all sinks found in one iteration can be treated in parallel
			parallelObjects.push_back(sinks[i]);
		}
	}
}

#else

void DependencyGraph::clusterGraph(std::list<DependencyGroup> &objectGroups) {

	/********************* Identify sequential groups ***************************************/

	// storage of already checked nodes
	DependencyObject::DependencySequence markedGraph;

	// find all sequences and compose sequential groups
	for (DependencyObject::DependencySequence::iterator rootNode = m_objects.begin();
		rootNode != m_objects.end(); ++rootNode)
	{
		// object is a group already
		if (dynamic_cast<DependencyGroup*> (*rootNode )  != NULL)
			continue;	// go to next element

		// skip elements already passed the inside another path, O(n)
		if (std::find(markedGraph.begin(), markedGraph.end(), *rootNode) != markedGraph.end() )
			continue;	// go to next element

		// find start and end position of a sequence
		std::vector<DependencyObject*> sequence;
		DependencyObject* firstSequentialNode = *rootNode;
		// find first element of a sequence
		// conditions: must have exactly one paranet
		//             parent must have exactly one child
		//             object must be part of this graph, O(n) search
		while (firstSequentialNode->parents().size() == 1 &&
			(*firstSequentialNode->parents().begin())->dependencies().size() == 1 &&
			isObjectOfGraph(**firstSequentialNode->parents().begin()) )
		{
			firstSequentialNode = *firstSequentialNode->parents().begin();
		}

		sequence.push_back(firstSequentialNode);
		DependencyObject* sequentialNode = firstSequentialNode;
		bool isCycle = false;
		// now fill all elements of the current sequence
		while (sequentialNode->dependencies().size() == 1 &&
			(*sequentialNode->dependencies().begin())->parents().size() == 1 &&
			(firstSequentialNode->parents().empty() || isObjectOfGraph(**firstSequentialNode->parents().begin() ) ) )
		{
			sequentialNode = *sequentialNode->dependencies().begin();
			// do not accept cycles
			if(sequentialNode == firstSequentialNode) {
				isCycle = true;
				break;
			}
			// add to stack
			sequence.push_back(sequentialNode);
		}
		// special case : first and last node are part of a cycle
		if(sequentialNode->dependencies().find(firstSequentialNode)
				!= sequentialNode->dependencies().end()) {
			isCycle = true;
		}

		// we have finished and scompose the group object
		objectGroups.push_back(DependencyGroup(DependencyGroup::SEQUENTIAL));
		DependencyGroup &group = objectGroups.back();
		// sequences
		if(!isCycle) {
			for (unsigned int i = 0; i < sequence.size(); ++i)
				group.insert(sequence[i]);
		}
		// cycle elements are treated as single sequences
		else {
			// we have minimum one sequence (first element)
			group.insert(sequence[0]);
			for (unsigned int i = 1; i < sequence.size(); ++i)
			{
				objectGroups.push_back(DependencyGroup(DependencyGroup::SEQUENTIAL));
				DependencyGroup &newGroup = objectGroups.back();
				newGroup.insert(sequence[i]);
			}
		}

		// update the graph of objects that are touched already
		for (unsigned int i = 0; i < sequence.size(); ++i) {
			// add to marked graph
			if (std::find(markedGraph.begin(), markedGraph.end(), sequence[i]) == markedGraph.end() )
				markedGraph.push_back(sequence[i]);
		}
	}

	// recompose the graph vector as a vector of dependency groups
	m_objects.clear();

	std::list<DependencyGroup>::iterator group;
	for (group = objectGroups.begin(); group != objectGroups.end(); ++group)
		m_objects.push_back(&(*group));
	// update all dependencies:
	// simply add a group as a dependency to all objects that depend
	// on one group member
	for (group = objectGroups.begin(); group != objectGroups.end(); ++group) {
		// sequential nodes are depObject-eleemnts of the groups
		for (DependencyObject::DependencySequence::const_iterator sequentialNode =
			group->depObjects().begin();  sequentialNode != group->depObjects().end();
			++ sequentialNode)
		{
			// manipulate dependency object dependencies
			for (DependencyObject::DependencySequence::iterator parent = m_objects.begin(); parent != m_objects.end(); ++parent) {
				// no parent node
				if ((*parent)->dependencies().find(*sequentialNode) == (*parent)->dependencies().end())
					continue;
				// add group as a new dependency
				(*parent)->dependsOn(*group);
			}
		}
		// update parent connections
		group->updateParents();
	}

	/********************* Identify cyclic groups ***************************************/

	// store the iterator position for the start of cyclic groups
	unsigned int iteratorCyclicGroupsPos = objectGroups.size();
	// clear alraedy checked ndoe
	markedGraph.clear();

	// compose cyclic groups
	for (DependencyObject::DependencySequence::iterator rootNode = m_objects.begin();
		rootNode != m_objects.end(); ++rootNode)
	{
		// first element is member of a cyclic group already
		DependencyGroup *rootGroup = dynamic_cast<DependencyGroup*> (*rootNode);
		if (rootGroup != NULL && rootGroup->type() == DependencyGroup::CYCLIC)
			continue;	// go to next element

		// skip elements already passed the inside another path
		if (std::find(markedGraph.begin(), markedGraph.end(), *rootNode) != markedGraph.end() )
			continue;	// go to next element

		// create containers for cycles search
		std::vector<DependencyObject::DependencySet> cycles;
		DependencyObject::DependencySequence markedBranch;
		markedBranch.push_back(*rootNode);
		markedGraph.push_back(*rootNode);
		// find all cycles from the current element
		findCycles(0, (*rootNode)->dependencies(), cycles, markedGraph, markedBranch);
		// continue if no cycles are found
		if (cycles.empty())
			continue;	// go to next element

		// merge all cycles: start from the first and check the following
		for (unsigned int i = 0; i < cycles.size(); ++i) {
			DependencyObject::DependencySet &cycleToMerge = cycles[i];
			bool merged = false;
			// merge into following cycles if necessary
			for (unsigned int j = i + 1; j < cycles.size(); ++j) {
				DependencyObject::DependencySet::const_iterator cycleNode = cycleToMerge.begin();
				for (;cycleNode != cycleToMerge.end(); ++cycleNode) {
					// we have an intersection of both cycles
					if (cycles[j].find(*cycleNode) != cycles[j].end() )
						break;
				}
				// cycles do not intersect
				if (cycleNode == cycleToMerge.end())
					continue;
				// merge first cycle into the following one
				cycles[j].insert(cycleToMerge.begin(), cycleToMerge.end() );
				// set signal
				merged = true;
			}
			// if merged delete cycle
			if (merged)
				cycleToMerge.clear();
		}

		// delete all empty cycles
		unsigned int cycleIdx = 0;
		while (cycleIdx < cycles.size() ) {
			std::vector<DependencyObject::DependencySet>::iterator it = cycles.begin();
			std::advance(it, cycleIdx);
			// delete cycle and continue inside loop
			if (it->empty()) {
				cycles.erase(it);
				continue;
			}
			// go to next element
			++cycleIdx;
		}

		for (unsigned int i = 0; i < cycles.size(); ++i) {
			// compose cyclic groups
			objectGroups.push_back(DependencyGroup(DependencyGroup::CYCLIC));
			DependencyGroup &group = objectGroups.back();
			// group all objects of the first cycle
			for (DependencyObject::DependencySet::iterator node = cycles[i].begin(); node != cycles[i].end(); ++node)
				group.insert(*node);

			// substitute the first objects by the new group and update
			// the marked graph object
			for (DependencyObject::DependencySet::iterator node = cycles[i].begin(); node != cycles[i].end(); ++node) {
				std::vector<DependencyObject*>::iterator nodeInGraph =
					std::find(m_objects.begin(), m_objects.end(), *node);
				// how to signal a non necessary group?=
				*nodeInGraph = &m_emptyGroupElement;
				// delete from marked graph
				DependencyObject::DependencySequence::iterator markedIt
						= std::find(markedGraph.begin(), markedGraph.end(), *node);
				if (markedIt != markedGraph.end() )
					markedGraph.erase(markedIt);
			}
		}
	}

	// delete all empty groups
	unsigned int nodeIdx = 0;
	while (nodeIdx < m_objects.size()) {
		// set iterator to the next possible position
		std::vector<DependencyObject *>::iterator node = m_objects.begin();
		std::advance(node,nodeIdx);

		DependencyGroup *group = dynamic_cast<DependencyGroup *>(*node);
		// object is of type group: programmer error
		assert(group != NULL);
		// empty groups are deleted
		if (*group == m_emptyGroupElement ) {
			// erase all objects that are not of type group from graph
			m_objects.erase(node);
			continue;
		}
		++nodeIdx;
	}

	group = objectGroups.begin();
	std::advance(group, iteratorCyclicGroupsPos);
	for (; group != objectGroups.end(); ++group)
		m_objects.push_back(&(*group));
	// update all dependencies:
	// simply add a group as a dependency to all objects that depend
	// on one group member
	// we start with the last iterator position inside group vector
	group = objectGroups.begin();
	std::advance(group, iteratorCyclicGroupsPos);
	for ( ; group != objectGroups.end(); ++group) {
		// sequential nodes are depObject-eleemnts of the groups
		for (DependencyObject::DependencySequence::const_iterator cyclicNode =
			group->depObjects().begin();  cyclicNode != group->depObjects().end();
			++ cyclicNode)
		{
			// manipulate dependency object dependencies
			for (DependencyObject::DependencySequence::iterator parent
				= m_objects.begin(); parent != m_objects.end(); ++parent)
			{
				// no parent node
				if ((*parent)->dependencies().find(*cyclicNode) == (*parent)->dependencies().end())
					continue;
				// add group as a new dependency
				(*parent)->dependsOn(*group);
			}
		}
		// update parent connections
		group->updateParents();
	}
}


void DependencyGraph::findCycles(const unsigned int depth,
								  const DependencyObject::DependencySet &nodes,
								  std::vector<DependencyObject::DependencySet> &cycles,
								  DependencyObject::DependencySequence &markedGraph,
								  DependencyObject::DependencySequence &markedBranch)
{
	unsigned int d = depth;
	unsigned int  rootNodeIdx = 0;
	// we merged object and node
	// and the iterator element may not longer exist as dependency
	// therefore, store the last valid iterator position
	for ( ; rootNodeIdx < nodes.size(); ++rootNodeIdx) {
		DependencyObject::DependencySet::const_iterator node = nodes.begin();
		// shift to next valid iterator position
		std::advance(node, rootNodeIdx);
		// is node really part of the current graph?
		// If not: continue
		if (!isObjectOfGraph(**node)) continue;

		// element is already inside branch: cycle found
		DependencyObject::DependencySequence::iterator markedNode =
			std::find(markedBranch.begin(), markedBranch.end(),*node);

		// cycle found: avoid cycles of element size 1
		DependencyObject::DependencySequence::iterator markedBranchLast = markedBranch.begin();
		std::advance(markedBranchLast, markedBranch.size() -1);
		if (markedNode != markedBranch.end() && markedBranch.size() > 1 && markedNode != markedBranchLast ) {

			// add a new cycle
			cycles.push_back(DependencyObject::DependencySet());
			DependencyObject::DependencySet &cycle = cycles.back();
			// now insert all members of the cycle
			DependencyObject::DependencySequence::iterator cycleNode = markedNode;
			while( cycleNode != markedBranch.end()) {
				// merge node to cycle
				cycle.insert(*cycleNode);
				// add dependencies to all parents
				//for(DependencyObject::DependencySequence::iterator depNode = markedNode;
				//	depNode != markedBranch.end(); ++depNode) {
				//	if(depNode == cycleNode)
				//		continue;
				if(markedNode != cycleNode) {
					for(DependencyObject::DependencySet::iterator parNode
						= (*cycleNode)->parents().begin();
						parNode != (*cycleNode)->parents().end(); ++parNode) {
							// ski equal nodes
							if(*parNode == *markedNode)
								continue;
							(*parNode)->dependsOn(**markedNode);
					}
					// update parent connections
					(*markedNode)->updateParents();
				}

				++cycleNode;
			}
			// set node to its parent
			assert(!(*node)->parents().empty());
			// in the current case do not call the routine again
		}
		else {
			// skip elements that are part of an already passed path
			if (std::find(markedGraph.begin(), markedGraph.end(), *node) != markedGraph.end())
				continue;
			// recursive call of routine
			markedGraph.push_back(*node);
			markedBranch.push_back(*node);
			findCycles(++d, (*node)->dependencies(), cycles, markedGraph, markedBranch);
		}
		// after call add element to marked graph and remove from branches
		if(markedBranch.size() > 0 && markedBranch.back() == *node) {
			markedBranch.pop_back();
		}
	}
	// else: go further
}


void DependencyGraph::orderGraph() {

	// create worker set and clear target vector
	m_orderedObjects.clear();

	// objectsLeft contains all unsorted graph nodes
	DependencyObject::DependencySet objectsLeft;
	for(unsigned int i = 0; i < m_objects.size(); ++i)
		objectsLeft.insert(m_objects[i]);

	while (!objectsLeft.empty()) {
		// we store all found sinks in objCluster
		std::set<DependencyObject*> objCluster;
		// search for sinks
		for (DependencyObject::DependencySet::const_iterator object = objectsLeft.begin();
			object != objectsLeft.end(); ++object)
		{
			// test if it is a sink, meaning it does not have dependencies in the objectsLeft vector
			// anylonger
			DependencyObject::DependencySet deps;
			deps = (*object)->dependencies();

			// if there is not any other object that uses this object, we have a sink
			DependencyObject::DependencySet::const_iterator dep = deps.begin();
			for (;dep != deps.end(); ++dep)
			{
				// dependency matches a group member -> dependency found, object is not a sink
				if(objectsLeft.find(*dep) != objectsLeft.end())  break;
			}
			// if we checked all myDeps and didn't find any, we have a new sink
			if (dep == deps.end())
				objCluster.insert(*object);
		}
		assert(!objCluster.empty()); // didn't find any sinks, error in group initialisation

		// store all sinks inside a new ParallelObjects object
		m_orderedParallelObjects.push_back(ParallelObjects());
		ParallelObjects &parallelObjects = m_orderedParallelObjects.back();

		for (std::set<DependencyObject*>::const_iterator object = objCluster.begin();
			object != objCluster.end(); ++object)
		{
			// insert found sinks one after another to vector
			m_orderedObjects.push_back(*object);
			// insert found sinks into parallel group set:
			// note: all sinks found in one iteration can be treated in parallel
			parallelObjects.push_back(*object);
			// and remove from set with remaining objects
			objectsLeft.erase(*object);
		}
	}
}

#endif

void DependencyGraph::clear() {
	m_orderedObjects.clear();
	m_orderedParallelObjects.clear();
	m_objects.clear();
}

} // namespace ZEPPELIN
