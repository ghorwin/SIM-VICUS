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

#ifndef VICUS_EpdDatasetH
#define VICUS_EpdDatasetH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_EpdModuleDataset.h"
#include "VICUS_LcaSettings.h"

#include <QString>
#include <QColor>

#include <IBK_Parameter.h>

namespace VICUS {

/*! TODO wait for implementation */
class EpdDataset : public AbstractDBElement {
public:

	enum Type {
		T_Generic,							// Keyword: Generic
		T_Specific,							// Keyword: Specific
		T_Average,							// Keyword: Average
		T_Representative,					// Keyword: Representative
		T_Template,							// Keyword: Template
		NUM_T
	};

	/*! All Categoruies. */
	enum Category {
		C_CategoryA,						// Keyword: A		'Production'
		C_CategoryB,						// Keyword: B		'Usage'
		C_CategoryC,						// Keyword: C		'Disposal'
		C_CategoryD,						// Keyword: D		'Deposit'
		NUM_C
	};

	// *** PUBLIC MEMBER FUNCTIONS ***
	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Returns an EPD with all Parameters scaled by the defined factor. */
	EpdDataset scaleByFactor(const double &factor) const;

	/*! checks the parameters reference-unit, reference-quantity, the categories and the different values of the EPDs
		to see if the EPDDatest exists already and has the latest version.
	*/
	bool behavesLike(const EpdDataset &other) const;

	/*! Return summed total Epd Data by Category. */
	EpdModuleDataset calcTotalEpdByCategory(const Category &cat, const LcaSettings &settings) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	/*! Defines Operator + .*/
	EpdDataset operator+(const EpdDataset& epd);

	/*! Defines Operator += .*/
	void operator+=(const EpdDataset& epd);

	/*! Returns whether a needed category defined by LCA Settings such as BNB (A1,A2,A3, ...) is valid. */
	bool isCategoryDefined(const LcaSettings &settings, const Category &cat) const;

	/*! Returns expanded all combined EPD Category Datasets into unique Datasets.
		Before: EpdCategoryDataset: A1,A2,A3 -> Data
		After:	EpdCategoryDataset:	A1 -> Data
									A2 -> Data
									A3 -> Data
	*/
	std::vector<EpdModuleDataset> expandCategoryDatasets() const;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! UUID of material. */
	QString							m_uuid;									// XML:A

	/*! Category. */
	IBK::MultiLanguageString		m_category;								// XML:A

	/*! Notes. */
	QString							m_notes;								// XML:E

	/*! Manufacturer. */
	QString							m_manufacturer;							// XML:E

	/*! Data source. */
	QString							m_dataSource;							// XML:E

	/*! Expire date for valid epd dataset. */
	QString							m_expireYear;							// XML:E

	/*! Reference unit. */
	IBK::Unit						m_referenceUnit;						// XML:E

	/*! Reference quantity. */
	double							m_referenceQuantity;					// XML:E

	/*! Sub type element. */
	Type							m_type = NUM_T;							// XML:E:required

	/*! String with all modules defined, separated by Comma. */
	QString							m_modules;								// XML:E:required

	/*! Vector with all category specific datasats. */
	std::vector<EpdModuleDataset>	m_epdModuleDataset;					// XML:E
};

}
#endif // VICUS_EpdDatasetH
