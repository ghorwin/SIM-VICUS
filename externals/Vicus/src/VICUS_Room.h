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
#include "VICUS_StructuralUnit.h"

#include <QString>

#include <IBK_Parameter.h>

namespace VICUS {

/*! Stores all data for a room. */
class Room : public Object {
public:
	/*! Type-info string. */
	const char * typeinfo() const override { return "Room"; }

	/*! Room parameters.
		These parameters are either automatically computed when zone is created/modified, or manually
		entered/adjusted by user.
	*/
	enum para_t {
		/*! Floor usable area of the zone, used for zone load definition. */
		P_Area,					// Keyword: Area					[m2]	'Floor usable area of the zone'
		/*! Geometrical violume of the zone, used for air exchange calulation + energy balance. */
		P_Volume,				// Keyword: Volume					[m3]	'Volume of the zone'
		/*! */
		P_HeatCapacity,			// Keyword: HeatCapacity			[J/K]	'Extra heat capacity'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Calculates the floor area of the room. */
	void calculateFloorArea();

	/*! Calculates the volume of the room. Room must have a closed shell. */
	void calculateVolume();

	void updateParents() {
		m_children.clear();
		for (Surface & s : m_surfaces) {
			m_children.push_back(&s);
			s.m_parent = this;
			s.updateParents();
		}
	}


	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int		m_id = INVALID_ID;			// XML:A:required
	//:inherited	QString				m_displayName;				// XML:A
	//:inherited	bool				m_visible = true;			// XML:A
	//:inherited	std::string			m_ifcGUID;					// XML:A

	/*! Reference to assigned zone template (optional). */
	IDType								m_idZoneTemplate			= INVALID_ID;	// XML:E
	/*! Reference to assigned acoustic template (optional). */
	IDType								m_idAcousticTemplate		= INVALID_ID;	// XML:E
	/*! Reference to assigned sound protection (optional). */
	IDType								m_idSoundProtectionTemplate = INVALID_ID;	// XML:E
	/*! Reference to assigned acoustsic building type ID. */
	IDType								m_idAcousticBuildingType	= INVALID_ID;	// XML:E

	/*! Zone parameters. */
	IBK::Parameter						m_para[NUM_P];						// XML:E

	/*! Surfaces of the room. */
	std::vector<Surface>				m_surfaces;							// XML:E

	// *** RUNTIME VARIABLES ***

	/*! Whole room net floor area in m2.
		This area is updated either from user-defined area in m_para or automatically computed from
		zone geometry. If the latter is not possible, user must enter a floor area.
	*/
	double								m_netFloorArea = -1;

	/*! Whole room volume in m3.
		This volume is updated either from user-defined volume in m_para or automatically computed from
		zone geometry. If the latter is not possible, user must enter a valid volume.
	*/
	double								m_volume = -1;

    // These pointers are updated in VICUS::Project::updatePointers() and can be used
    // to quicky travers the data model.

    VICUS::StructuralUnit *				m_structuralUnit = nullptr;

};


} // namespace VICUS


#endif // VICUS_RoomH
