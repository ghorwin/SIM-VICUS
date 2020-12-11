#ifndef VICUS_EPDDatasetH
#define VICUS_EPDDatasetH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_EPDCategroySet.h"

#include <QString>
#include <QColor>
#include <QDate>

#include <IBK_Parameter.h>

namespace VICUS {

class EPDDataset : public AbstractDBElement {
public:
	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Dry density of the material. */
		P_Density,					// Keyword: Density					[kg/m3]	'Dry density of the material.'
		/*! Global Warming Potential. */
		P_GWP,						// Keyword: GWP						[W/mK]	'Global Warming Potential.'
		/*! Depletion potential of the stratospheric ozone layer . */
		P_ODP,						// Keyword: ODP						[W/mK]	'Depletion potential of the stratospheric ozone layer.'
		/*! Photochemical Ozone Creation Potential . */
		P_POCP,						// Keyword: POCP					[W/mK]	'Photochemical Ozone Creation Potential.'
		/*! Acidification potential . */
		P_AP,						// Keyword: AP						[W/mK]	'Acidification potential.'
		/*! Eutrophication potential. */
		P_EP,						// Keyword: EP						[W/mK]	'Eutrophication potential.'
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

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of lca material. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! UUID of material. */
	QString							m_uuid;									// XML:A

	/*! Display name of material. */
	QString							m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

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
