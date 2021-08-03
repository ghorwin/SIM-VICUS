/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef VICUS_ObjectH
#define VICUS_ObjectH

#include <vector>
#include <set>

#include <QString>

#include "VICUS_Constants.h"

namespace VICUS {

/*! Base class for all objects that have a parent and are shown in the
	navigation tree widget.

	Also, each object has a unique ID that is used throughout the user interface to
	identify it.

	Basically, whenever such an object is newly created, it will get a new unique ID.

	Note: whenever you add/remove an object in the item hierarchy, make sure to call updateParents() in top level node.
*/
class Object {
public:
	/*! Standard C'tor. */
	Object() : m_uniqueID(++LAST_ID) {}
	/*! Default copy constructor. */
	Object(Object const&) = default;
	/*! D'tor. */
	virtual ~Object();

	unsigned int uniqueID() const { return m_uniqueID; }

	/*! Recursively searches through data hierarchy and returns pointer to object matching the given unique ID.
		\return Returns pointer to wanted object or nullptr, if it couldn't be found.
	*/
	Object * findChild(unsigned int uID);
	/*! Same as function above, const version. */
	const Object * findChild(unsigned int uID) const;

	/*! Recursively selected all unique IDs of children. */
	void collectChildIDs(std::set<unsigned int> & nodeContainer) const;

	/*! Takes an old object and returns a new object with a new unique ID, but same m_parent and m_children. */
	Object clone() const {
		Object o(*this);
		o.m_uniqueID = ++LAST_ID;
		return o;
	}


	/*! Parent pointer, do not modify. */
	Object	*m_parent = nullptr;

	/*! Stores visibility information for this surface. */
	bool								m_selected = false;
	/*! Stores visibility information for this surface (serialized manually in derived classes). */
	bool								m_visible = true;

	/*! Persistant ID of object (not the unique ID, may not be unique in DB model, must be handled
		appropriately in error handling).
	*/
	unsigned int						m_id = INVALID_ID;

	/*! The descriptive name of the object. */
	QString								m_displayName;

protected:
	/*! List of all children. */
	std::vector<Object *>				m_children;

private:
	unsigned int						m_uniqueID;

	/*! Unique ID marker, incremented in each constructor call. */
	static unsigned int LAST_ID;
};

} // namespace VICUS

#endif // VICUS_ObjectH
