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

#ifndef VICUS_RoomH
#define VICUS_RoomH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Surface.h"
#include "VICUS_Object.h"

#include <QString>

#include <IBK_Parameter.h>

namespace VICUS {

class Room : public Object {
public:

	/*! Room parameters. */
	enum para_t{
		/*! Dry density of the material. */
		P_Area,					// Keyword: Area					[m2]	'Floor area of the zone.'
		/*! Dry density of the material. */
		P_Volume,				// Keyword: Volume					[m3]	'Volume of the zone.'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	void updateParents() {
		m_children.clear();
		for (Surface & s : m_surfaces) {
			m_children.push_back(&s);
			s.m_parent = this;
			s.updateParents();
		}
	}

	/*! Creates a copy of the room object but with a new unique ID. */
	Room clone() const{
		Room r(*this); // create new room with same unique ID
		Object & o = r;
		(Object&)r = o.clone(); // assign new ID only
		return r;
	}


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int						m_id = INVALID_ID;					// XML:A:required

	QString								m_displayName;						// XML:A

	/*! Reference to assigned zone template (optional). */
	IDType								m_idZoneTemplate = INVALID_ID;		// XML:E

	/*! Stores visibility information for this surface.
		Note: keep the next line - this will cause the code generator to create serialization code
			  for the inherited m_visible variable.
	*/
	//:inherited	bool								m_visible = true;			// XML:A

	/*! Stores zone parameters.
		if area or volume is zero --> autocalulation from geometry
	*/
	IBK::Parameter						m_para[NUM_P];						// XML:E

	/*! Surfaces of the room. */
	std::vector<Surface>				m_surfaces;							// XML:E
};


} // namespace VICUS


#endif // VICUS_RoomH
