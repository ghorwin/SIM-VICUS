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

#ifndef VICUS_ComponentH
#define VICUS_ComponentH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Material.h"
#include "VICUS_Construction.h"
#include "VICUS_BoundaryCondition.h"

#include <IBK_MultiLanguageString.h>

#include <QString>
#include <QColor>
#include <vector>

namespace VICUS {

class Component : public AbstractDBElement {
public:

	/*! Component types. */
	enum ComponentType {
		CT_OutsideWall,				// Keyword: OutsideWall				'Outside wall construction'
		CT_OutsideWallToGround,		// Keyword: OutsideWallToGround		'Outside wall construction in contact with ground'
		CT_InsideWall,				// Keyword: InsideWall				'Interior construction'
		CT_FloorToCellar,			// Keyword: FloorToCellar			'Floor to basement'
		CT_FloorToAir,				// Keyword: FloorToAir				'Floor in contact with air'
		CT_FloorToGround,			// Keyword: FloorToGround			'Floor in contact with ground'
		CT_Ceiling,					// Keyword: Ceiling					'Ceiling construction'
		CT_SlopedRoof,				// Keyword: SlopedRoof				'Sloped roof construction'
		CT_FlatRoof,				// Keyword: FlatRoof				'Flat roof construction'
		CT_ColdRoof,				// Keyword: ColdRoof				'Flat roof construction (to heated/insulated space)'
		CT_WarmRoof,				// Keyword: WarmRoof				'Flat roof construction (to cold/ventilated space)'
		CT_Miscellaneous,			// Keyword: Miscellaneous			'Some other component type'
		NUM_CT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<Material> & materials,
				 const VICUS::Database<Construction> & constructions,
				 const VICUS::Database<BoundaryCondition> & bcs,
				 const VICUS::Database<Schedule> & scheduleDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;			// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;				// XML:A
	//:inherited	QColor							m_color;					// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;									// XML:E

	/*! Manufacturer. */
	IBK::MultiLanguageString		m_manufacturer;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;								// XML:E

	/*! Component type. */
	ComponentType					m_type = CT_Miscellaneous;					// XML:E:required

	/*! Opaque construction ID. */
	unsigned int					m_idConstruction = INVALID_ID;				// XML:E

	/*! Boundary condition ID for Side A (usually outside). */
	unsigned int					m_idSideABoundaryCondition = INVALID_ID;	// XML:E

	/*! Boundary condition ID for Side B (usually inside). */
	unsigned int					m_idSideBBoundaryCondition = INVALID_ID;	// XML:E

	/*! Acoustic boundary condition ID for Side A (usually outside). */
	unsigned int					m_idSideAAcousticBoundaryCondition = INVALID_ID;	// XML:E

	/*! Acoustic boundary condition ID for Side B (usually inside). */
	unsigned int					m_idSideBAcousticBoundaryCondition = INVALID_ID;	// XML:E

	/*! Surface property ID.
		TODO Dirk, kann das weg? nein
	*/
	unsigned int					m_idSurfaceProperty = INVALID_ID;			// XML:E

	/*! Identifies the layer of the construction (counting from side A, from index 0), that shall be used
		for floor heating/cooling.
	*/
	unsigned int					m_activeLayerIndex = INVALID_ID;			// XML:E

};

} // namespace VICUS


#endif // VICUS_ComponentH
