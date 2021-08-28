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

#ifndef VICUS_EPDDatasetH
#define VICUS_EPDDatasetH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <QString>
#include <QColor>

#include <IBK_Parameter.h>

namespace VICUS {

/*! TODO wait for implementation */
class EPDDataset : public AbstractDBElement {
public:
	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Mass of material in dependence to the area. */
		P_BasisWeight,				//Keyword: BasisWeight				[kg/m2]	'mass of material in dependence of the area.'
		/*! Dry density of the material. */
		P_Density,					// Keyword: Density					[kg/m3]	'Dry density of the material.'
		/*! Global Warming Potential. */
		P_GWP,						// Keyword: GWP						[kg]	'Global Warming Potential.'
		/*! Depletion potential of the stratospheric ozone layer . */
		P_ODP,						// Keyword: ODP						[kg]	'Depletion potential of the stratospheric ozone layer.'
		/*! Photochemical Ozone Creation Potential . */
		P_POCP,						// Keyword: POCP					[kg]	'Photochemical Ozone Creation Potential.'
		/*! Acidification potential . */
		P_AP,						// Keyword: AP						[kg]	'Acidification potential.'
		/*! Eutrophication potential. */
		P_EP,						// Keyword: EP						[kg]	'Eutrophication potential.'
		/*! Total use of non-renewable primary energy resources. */
		P_PENRT,					// Keyword: PENRT					[W/mK]	'Total use of non-renewable primary energy resources.'
		/*! Total use of renewable primary energy resources . */
		P_PERT,						// Keyword: PERT					[W/mK]	'Total use of renewable primary energy resources .'

		NUM_P
	};

		//the most frequently used categories
	enum Category{
		C_A1,
		C_A2,
		C_A1_A2,
		C_A3,
		C_A1_A3,
		C_B6,
		C_C2,
		C_C2_3,
		C_C2_C4,
		C_C3,
		C_C3_C4,
		C_C4,
		C_D,
		NUM_C
	};

	enum Mode {
		M_Generic,
		M_Specific,
		M_Average,
		M_Representative,
		M_Template,
		NUM_M
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE

	/*! checks the parameters referenceunit, referencequantity, the categories and the different values of the EPDs
		to see if the EPDDatest exists already and has the latest version.
	*/
	bool behavesLike(const EPDDataset &other) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! UUID of material. */
	QString							m_uuid;									// XML:A

	/*! Notes. */
	QString							m_notes;								// XML:E

	/*! Manufacturer. */
	QString							m_manufacturer;							// XML:E

	/*! Data source. */
	QString							m_dataSource;							// XML:E

	/*! Expire date for valid epd dataset. */
	QString							m_expireDate;							// XML:E

	/*! Reference unit. */
	QString							m_referenceUnit;						// XML:E

	/*! Reference quantity. */
	double							m_referenceQuantity;					// XML:E

	/*! Sub type element. */
	Mode							m_subtype = NUM_M;						// XML:E

	/*! Category type A1, A2, ... */
	Category						m_category = NUM_C;						// XML:E

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E




};

}
#endif // VICUS_EPDDatasetH
