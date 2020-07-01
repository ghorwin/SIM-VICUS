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

#ifndef ZEPPELIN_DependencyGraphH
#define ZEPPELIN_DependencyGraphH

#include <set>
#include <vector>

#include "ZEPPELIN_DependencyGroup.h" // also includes DependencyObject


#define USE_EAS_ALGORITHM
namespace ZEPPELIN {

/*! The DependencyGraph class can be used to determine order of
	evaluation of sets of interdependent objects.
	The class instance only uses pointers as references, but does
	not own them.
*/
class DependencyGraph {
public:

	/*! A vector holding groups of referenced dependency objects. */
	typedef DependencyObject::DependencySequence	ParallelObjects;

	/*! Constructor. */
	DependencyGraph(): m_emptyGroupElement(DependencyGroup::SEQUENTIAL) { }
	/*! Destructor. */
	virtual ~DependencyGraph() { clear(); }

	/*! Clusters pointers of dependency objects to object groups and passes
		these groups to the dependency graph. The calculation of interdependent
		clusters will automatically be done once objects are passed. Additionally,
		the dependencies of each object are extended with dependencies to
		the groups that cluster each dependency.
		The objects and object groups will not be owned by the DependencyGraph
		instance. Therefore, the input of object and object group vectors
		are enforced.
		\para objects vector of the dependency object pointers forming the graph
		\para objectGroups empty vector of dependency groups that will be filled

		Suppose you have the following dependency objects:
		\code
		DependencyObject A,B,C,D;
		A.dependsOn(&B);
		C.dependsOn(&B);
		A.dependsOn(&C);
		D.dependsOn(&C);

		std::list<DependencyGroup> l;
		DependencyGraph g;
		// insert objects into set of dep objects
		DependencyObject::DependencySet s;
		s.insert(A); s.insert(B); s.insert(C);
		g.setObjects(s, l);
		DependencyObject::DependencySequence &o = orderedObjects();
		std::vector<ParallelObjects> &p = orderedParallelObjects();
		// orderedObjects() returns vector with objects in the following order [B,C,A,D]
		// orderedParallelObjects() returns a vector of parallel objects [[B],[C],[A,D]]
		// with the clustered objects
		\endcode
	*/
	virtual void setObjects(DependencyObject::DependencySequence &objects,
							std::list<DependencyGroup> &objectGroups);

	/*! This vector contains the ordered list of interdependent clusters. Independent objects
		appear in their order inside the objects-set.
	*/
	const DependencyObject::DependencySequence & orderedObjects() const { return m_orderedObjects; }

	/*! This vector contains the ordered list of parallel evaluable groups. Independent objects
		are encapsulated into an internal list (of type ParallelObjects).
	*/
	const std::vector<ParallelObjects> & orderedParallelObjects() const { return m_orderedParallelObjects; }

	/*! Clears all nodes of the graph. */
	void clear();

protected:
	/*! Clusters sequential and cyclic groups and fills the objectGroups-vector.
		\para objectGroups empty vector of dependency groups that will be filled
	*/
	void clusterGraph(std::list<DependencyGroup> &objectGroups);

	/*! Performs a topological sorting of the graph and forms the m_orderedObjects
		and m_orderedParallelObjects vectors. The ordered graph only contains
		DependencyGroups from the previoulsy coposed objectGroups vector.*/
	void orderGraph();

#ifdef USE_EAS_ALGORITHM
	/*! Selects cyclic and sequential connected nodes of the graph. The algorithm 
		iteratively erases all sources and sinks from the graph. These sources and
		sinks are checked for sequential connections (only one child and one parent 
		per pair of nodes) and sorted into sequence container. The remaining graph nodes
		are assumed to include a cyclic connection. This cycle is searched for and erased
		from the graph. The described procedure is repeated until all nodes are sorted into one 
		container.
		\para cycles container including all cyclic connected nodes
		\para sequences container including all sequential connected nodes
	*/
	void findCyclesAndSequences(std::vector<DependencyObject::DependencySequence> &cycles,
							std::vector<DependencyObject::DependencySequence> &sequences);	
	
	/*! Erases all source nodes from a given graph or subgraph.
		\para sources container including source nodes
		\para graph subset of graph nodes excluding all sources
	*/
	static void popSourcesFromGraph( DependencyObject::DependencySequence &sources,
		DependencyObject::DependencySequence &graph);

	/*! Erases all sink nodes from a given graph or subgraph.
		\para sinks container including sink nodes
		\para graph subset of graph nodes excluding all sinks
	*/
	static void popSinksFromGraph( DependencyObject::DependencySequence &sinks,
		DependencyObject::DependencySequence &graph);

	/*! Erases the next cyclic connection from a graph or subgraph. The graph is assumed
		to be free of sources and sinks. Starting from the first node all forward connections 
		are passed and all connected transitively nodes sorted into a subgraph. The same 
		search is performed backward in order to filter only cyclic connections.
		\para cycle maximum subset of cyclic connected nodes that include the first graph node, 
			if no suitable cyclic connection is found an empty container is returned
		\para graph subset of graph nodes excluding the cycle nodes
	*/
	static void popNextCycleFromGraph( DependencyObject::DependencySequence &cycle,
		DependencyObject::DependencySequence &graph);

	/*! Selects all nodes that are causaly connected to the first graph node.
		\para connectedNodes subset of nodes causaly connected to the first graph node
		\para graph graph nodes for search
		\para forwardSearch causality, true if graph is searched in the direction of
		dependencies (childs), false otherwise (parent direction)
	*/
	static void findFirstConnectedNodesInGraph( DependencyObject::DependencySequence &connectedNodes,
					const DependencyObject::DependencySequence &graph, 
					bool forwardSearch = true);
#else
	/*! Recursive cycle search (starting from an arbirtary root node).
		\para depth recursion depth
		\para nodes nodes from which the cyle search continues
		\para markedGraph list of all objects that have been touched by the cyle search
		already
		\para markedBranch current path of cycle search (a branch of the graph)
	*/
	void findCycles(const unsigned int depth,
					const DependencyObject::DependencySet &nodes,
					std::vector<DependencyObject::DependencySet> &cycles,
					DependencyObject::DependencySequence &markedGraph,
					DependencyObject::DependencySequence &markedBranch);

#endif
	/*! Checkes the membership of a dependency to the current graph.
		Note that this function ignores an object that is encapsulated
		into a group.
		\para o Object whose membership is checked
	*/
	bool isObjectOfGraph(const DependencyObject &o) {
		return std::find(m_objects.begin(),m_objects.end(), &o)  != m_objects.end();
	}

	/*! Constant reference/pointer to set with pointers to dependency objects.
		All grouped DependencyObject types.
	*/
	DependencyObject::DependencySequence	m_objects;

	/*! This vector contains the ordered list of single dependency objects. */
	DependencyObject::DependencySequence	m_orderedObjects;

	/*! This vector contains a list of ordered group vectors that can be in treated in parallel. */
	std::vector<ParallelObjects>			m_orderedParallelObjects;

private:
	/*! An empty group element. */
	DependencyGroup							m_emptyGroupElement;



}; 	// class DependencyGraph

} // namespace ZEPPELIN

/*! \file ZEPPELIN_DependencyGraph.h
	\brief Contains the implementation of the dependency graph.
*/

#endif // NM_DependencyGraphH
