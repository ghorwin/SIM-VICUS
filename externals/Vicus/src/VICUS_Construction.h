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

#ifndef VICUS_ConstructionH
#define VICUS_ConstructionH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_MaterialLayer.h"

#include <QString>
#include <QColor>

#include <vector>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>

#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Material.h"

namespace VICUS {

class Construction : public AbstractDBElement {
public:
	enum UsageType {
		UT_OutsideWall,				// Keyword: OutsideWall				'Outside wall construction'
		UT_OutsideWallToGround,		// Keyword: OutsideWallToGround		'Outside wall construction in contact with ground'
		UT_InsideWall,				// Keyword: InsideWall				'Interior construction'
		UT_FloorToCellar,			// Keyword: FloorToCellar			'Floor to basement'
		UT_FloorToGround,			// Keyword: FloorToGround			'Floor in contact with ground'
		UT_Ceiling,					// Keyword: Ceiling					'Ceiling construction'
		UT_SlopedRoof,				// Keyword: SlopedRoof				'Sloped roof construction'
		UT_FlatRoof,				// Keyword: FlatRoof				'Flat roof construction'
		NUM_UT						// Keyword: ---						'Miscellaneous'
	};

	enum InsulationKind {
		IK_NotInsulated,			// Keyword: NotInsulated			'Not insulated'
		IK_InsideInsulation,		// Keyword: InsideInsulation		'Inside insulated'
		IK_CoreInsulation,			// Keyword: CoreInsulation			'Core insulation'
		IK_OutsideInsulation,		// Keyword: OutsideInsulation		'Outside insulated'
		NUM_IK						// Keyword: ---						'Not selected'
	};

	enum MaterialKind {
		MK_BrickMasonry,			// Keyword: BrickMasonry			'Brick masonry'
		MK_NaturalStoneMasonry,		// Keyword: NaturalStoneMasonry		'Natural stones'
		MK_Concrete,				// Keyword: Concrete				'Concrete'
		MK_Wood,					// Keyword: Wood					'Wood'
		MK_FrameWork,				// Keyword: FrameWork				'Frame construction'
		MK_Loam,					// Keyword: Loam					'Loam'
		NUM_MK						// Keyword: ---						'Not selected'
	};

	enum para_t{
		P_ImpactSoundValue,		// Keyword: ImpactSoundValue	[-]	'Impact sound value'
		P_AirSoundResistanceValue,	// Keyword: AirSoundResistanceValue	[-]	'Air sound resistance value'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Material> & materials) const;

	/*! Computes the u-value. */
	bool calculateUValue(double & UValue, const VICUS::Database<Material> & materials, double ri, double re) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A

	/*! The usage type (classification property). */
	UsageType						m_usageType = NUM_UT;					// XML:E
	/*! The type of insulation used (classification property). */
	InsulationKind					m_insulationKind = NUM_IK;				// XML:E
	/*! The main/load bearing material (classification property). */
	MaterialKind					m_materialKind = NUM_MK;				// XML:E

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Acoustic attributes. */
	IBK::Parameter					m_acousticPara[NUM_P];					// XML:E

	/*! The individual material layers. */
	std::vector<MaterialLayer>		m_materialLayers;						// XML:E
};

} // namespace VICUS


#endif // VICUS_ConstructionH
