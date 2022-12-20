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

#ifndef VICUS_MaterialH
#define VICUS_MaterialH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_EpdCategorySet.h"

#include <QString>
#include <QColor>
#include <QCoreApplication>

#include <IBK_Parameter.h>

#include <NANDRAD_LinearSplineParameter.h>
#include <NANDRAD_Material.h>

namespace VICUS {

class Material : public AbstractDBElement {
	Q_DECLARE_TR_FUNCTIONS(Material)
public:

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Dry density of the material. */
		P_Density,					// Keyword: Density					[kg/m3]	'Dry density of the material.'
		/*! Specific heat capacity of the material. */
		P_HeatCapacity,				// Keyword: HeatCapacity			[J/kgK]	'Specific heat capacity of the material.'
		/*! Thermal conductivity of the dry material. */
		P_Conductivity,				// Keyword: Conductivity			[W/mK]	'Thermal conductivity of the dry material.'

		//hygrothermal parameters
		/*! Thermal conductivity of the dry material. */
		P_Mu,						// Keyword: Mu						[-]		'Vapor diffusion resistance factor.'
		/*! Moisture content at 80% relative humidity. */
		P_W80,						// Keyword: W80						[kg/m3]	'Water content in relation to 80% humidity.'
		/*! Moisture content at saturation. */
		P_Wsat,						// Keyword: Wsat					[kg/m3]	'Water content at saturation.'

		NUM_P
	};

	/*! Material categories.*/
	enum Category {
		//opaque categories
		MC_Coating,					// Keyword: Coating
		MC_Plaster,					// Keyword: Plaster
		MC_Bricks,					// Keyword: Bricks
		MC_NaturalStones,			// Keyword: NaturalStones
		MC_Cementitious,			// Keyword: Cementitious
		MC_Insulations,				// Keyword: Insulations
		MC_BuildingBoards,			// Keyword: BuildingBoards
		MC_Woodbased,				// Keyword: Woodbased
		MC_NaturalMaterials,		// Keyword: NaturalMaterials
		MC_Soils,					// Keyword: Soils
		MC_CladdingSystems,			// Keyword: CladdingSystems
		MC_Foils,					// Keyword: Foils
		MC_Miscellaneous,			// Keyword: Miscellaneous
		NUM_MC
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default c'tor. */
	Material() {}

	/*! Initializing constructor.
		\param id
		\param name Display name.
		\param conductivity Thermal conductivity in W/mK
		\param density Bulk density in kg/m3
		\param specHeatCapa Specific heat capacity in J/kgK
	*/
	Material( unsigned int id, const IBK::MultiLanguageString &name, double conductivity, double density, double specHeatCapa);

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Composes a NANDRAD material from the VICUS data structure. */
	NANDRAD::Material toNandrad() const;

	/*! Checks if all parameters are valid. */
	bool isValid(bool hygrothermalCalculation = false) const;

	/*! Comparison operator. */
	ComparisonResult equal(const AbstractDBElement * other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Manufacturer. */
	IBK::MultiLanguageString		m_manufacturer;							// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Material category. */
	Category						m_category = MC_Miscellaneous;			// XML:E:required

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E

	/*! Holds all Data for combined EPDs for Material. */
	EpdCategorySet					m_epdCategorySet;						// XML:E

};

} // namespace VICUS


#endif // VICUS_MaterialH
