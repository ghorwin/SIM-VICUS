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

	Also, each object has a unique ID m_id that is used throughout the user interface and
	data model to identify it.

	Note: whenever you add/remove an object in the item hierarchy, make sure to call updateParents() in top level node.
*/
class Object {
public:
	/*! Standard C'tor. */
	Object() = default;
	/*! Default copy constructor. */
	Object(Object const&) = default;
	/*! D'tor. */
	virtual ~Object();

	/*! Recursively searches through data hierarchy and returns pointer to object matching the given ID.
		\return Returns pointer to wanted object or nullptr, if it couldn't be found.
	*/
	Object * findChild(unsigned int ID);
	/*! Same as function above, const version. */
	const Object * findChild(unsigned int ID) const;

	/*! Recursively selected all unique IDs of children, includes the object's ID itself as well.
		\note IDs are _added_ to the container which is not cleared initially!
	*/
	void collectChildIDs(std::set<unsigned int> & nodeContainer) const;

	/*! Returns a short descriptive string usable for error messages that identifies the object type. */
	virtual const char * typeinfo() const = 0;

	/*! Returns a descriptive string with object type, ID and optionally displayname that helps identifying this object. */
	QString info() const;

	/*! Parent pointer, do not modify. */
	Object	*m_parent = nullptr;

	/*! Persistant ID of object (not the unique ID, may not be unique in DB model, must be handled
		appropriately in error handling).
	*/
	unsigned int						m_id = INVALID_ID;

	/*! Stores visibility information for this surface. */
	bool								m_selected = false;
	/*! Stores visibility information for this surface (serialized manually in derived classes). */
	bool								m_visible = true;

	/*! The descriptive name of the object. */
	QString								m_displayName;

	/*! Contains a GUID to a IFC object in case of import from IFC.*/
	std::string							m_ifcGUID;

protected:
	/*! List of all children. */
	std::vector<Object *>				m_children;
};

} // namespace VICUS

#endif // VICUS_ObjectH
