/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef VICUS_InternalLoadH
#define VICUS_InternalLoadH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of all Internal Loads (Person, Lighting, Equipment, Other).

*/

class InternalLoad : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Person Count. */
		///TODO Einheit "Pers" muss aufgenommen werden
		P_PersonCount,					// Keyword: PersonCount				[-]		'Person Count.'
		/*! Person per area. */
		///TODO Einheit "Pers/m2" muss aufgenommen werden
		P_PersonPerArea,				// Keyword: PersonPerArea			[m2]	'Person per area.'
		/*! Area per person. */
		///TODO Einheit "m2/Pers muss aufgenommen werden
		P_AreaPerPerson,				// Keyword: AreaPerPerson			[m2]	'Area per person.'
		/*! Power. */
		P_Power,						// Keyword: Power					[W]		'Power.'
		/*! Power per Area. */
		P_PowerPerArea,					// Keyword: PowerPerArea			[W/m2]	'Power per area.'
		/*! Convective Heat Factor. */
		P_ConvectiveHeatFactor,			// Keyword: ConvectiveHeatFactor	[---]	'Convective Heat Factor.'
		/*! Latent Heat Factor. */
		P_LatentHeatFactor,				// Keyword: LatentHeatFactor		[---]	'Latent Heat Factor.'
		/*! Loss Heat Factor. */
		P_LossHeatFactor,				// Keyword: LossHeatFactor			[---]	'Loss Heat Factor.'

		NUM_P
	};

	/*! Internal load categories.*/
	enum Category {
		IC_Person,				// Keyword: Person							[-]		'Person'
		IC_ElectricEquiment,	// Keyword: ElectricEquiment				[-]		'ElectricEquiment'
		IC_Lighting,			// Keyword: Lighting						[-]		'Lighting'
		IC_Other,				// Keyword: Other							[-]		'Other'
		NUM_MC
	};

	/*! The description is used to identify the unit in the gui for person. */
	enum PersonCountMethod{
		PCM_PersonPerArea,		// Keyword: PersonPerArea					[-]		'Person per m2'
		PCM_AreaPerPerson,		// Keyword: AreaPerPerson					[-]		'm2 per Person'
		PCM_PersonCount,		// Keyword: PersonCount						[-]		'Person count'
		NUM_PCM
	};

	/*! The description is used to identify the unit in the gui for all other equipment (electric, lights, other, ...). */
	enum PowerMethod{
		PM_PowerPerArea,		// Keyword: PowerPerArea					[-]		'Power'
		PM_Power,				// Keyword: Power							[-]		'Power'
		NUM_PM
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of Intenal Load. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of Intenal Load. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Internal Load category. */
	Category						m_category = NUM_MC;					// XML:E:required

	/*! Person count method. */
	PersonCountMethod				m_personCountMethod=NUM_PCM;			// XML:E

	/*! Power method. */
	PowerMethod						m_powerMethod=NUM_PM;					// XML:E

	/*! Schedule ID. */
	unsigned int					m_occupancyScheduleId = INVALID_ID;		// XML:E

	/*! Only required for person*/
	unsigned int					m_activityScheduleId = INVALID_ID;		// XML:E

	/*! Only required for electric equipment, lights, other. */
	unsigned int					m_powerManagementScheduleId = INVALID_ID;		// XML:E

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_InternalLoadH
