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

#ifndef ZEPPELIN_DependencyGroupH
#define ZEPPELIN_DependencyGroupH

#include <algorithm>
#include <vector>
#include <list>

#include "ZEPPELIN_DependencyObject.h"

namespace ZEPPELIN {

/*! Groups several dependency objects (without owning them) and identifies these
	as cyclic or as sequential.
*/
class DependencyGroup : public DependencyObject {
public:

	/*! Different types of groups. */
	enum Type {
		/*! The objects in this group refer each other and create a cycle. */
		CYCLIC,
		/*! There exists a sequence in which all objects can be evaluated. */
		SEQUENTIAL
	};

	/*! Constructor, takes the type of the group. */
	DependencyGroup(const Type type) :
		m_type(type)
	{
	}

	/*! Return of the group type. */
	Type type() const {
		return m_type;
	}

	/*! Return of the group objects. */
	const DependencyObject::DependencySequence &depObjects() const {
		return m_depObjects;
	}

	/*! Function creating Euler routes and euler pathes interpreting current group as directed subgraph.
	We accept node with different inlet and outlet connections in a cycle. In this case,
	we create invalid connections that will be destroyed later. At the end, a path through all
	edges of the graph (including some necessary backward connections) will be returned.
	*/
	void createEulerPathesInDirectedSubGraph(std::vector<std::list<const DependencyObject*> > &eulerPathes);

	/*! Function creating Euler routes and euler pathes interpreting current group as undirected subgraph.
	We accept node with uneven connections in a cycle. In this case,
	we create a invalid connection that will be destroyed later. At the end, a path through all
	edges of the graph (including some necessary backward connections) will be returned.
	*/
	void createEulerPathesInUndirectedSubGraph(std::vector<std::list<const DependencyObject*> > &eulerPathes);

	/*! Fills set with dependent models. */
	virtual const DependencyObject::DependencySequence & dependencies() const;
	/*! Fills set with dependent models. */
	virtual const DependencyObject::DependencySequence & dependencyObjects() const;
	/*! Fills set with dependent models. */
	virtual const DependencyObject::DependencySequence &  parents() const;
	/*! Replaces the dependency sequence. */
	virtual void set(const DependencyObject::DependencySequence & depObjectVector);
	/*! Inserts an object into the group. */
	virtual void insert(DependencyObject* o);
	/*! Erases an object from the group and updates dependencies afterwards. */
	virtual void erase(DependencyObject* o);
	/*! Checks if current group shares dependencies with other group. */
	virtual bool hasIntersection(const DependencyGroup* group) const;
	/*! Merges two groups and stores the result inside current object. */
	virtual void merge(const DependencyGroup* group);
	/*! Intersects two groups and stores the result inside current object. */
	virtual void intersect(const DependencyGroup* group);
	/*! Calculates the complement between current group and a second group, stores the result inside current object. */
	virtual void complement(const DependencyGroup* group);


private:
	/*! Defines whether the dependency objects in this class are independent or cyclic. */
	Type									m_type;

	/*! Holds the dependency objects. */
	DependencyObject::DependencySequence	m_depObjects;

}; // DependencyGroup


} // namespace ZEPPELIN

#endif // ZEPPELIN_DependencyGroupH
