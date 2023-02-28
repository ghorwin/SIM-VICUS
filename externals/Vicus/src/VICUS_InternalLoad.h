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

#ifndef VICUS_InternalLoadH
#define VICUS_InternalLoadH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Schedule.h"

namespace VICUS {

/*! Describes the course of all Internal Loads (Person, Lighting, Equipment, Other).
*/
class InternalLoad : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Person Count. */
		P_PersonCount,						// Keyword: PersonCount						[-]			'Person count'
		/*! Person per area. */
		P_PersonPerArea,					// Keyword: PersonPerArea					[Person/m2]	'Person per area'
		/*! Area per person. */
		P_AreaPerPerson,					// Keyword: AreaPerPerson					[m2/Person]	'Area per person'
		/*! Power. */
		P_Power,							// Keyword: Power							[W]			'Power'
		/*! Power per Area. */
		P_PowerPerArea,						// Keyword: PowerPerArea					[W/m2]		'Power per area'
		/*! Moisture production rate per Area. */
		P_MoistureProductionRatePerArea,	// Keyword: MoistureProductionRatePerArea	[kg/m2s]	'Moisture production rate per area'
		/*! Convective Heat Factor. */
		P_ConvectiveHeatFactor,				// Keyword: ConvectiveHeatFactor			[---]		'Convective heat factor'
		/*! Latent Heat Factor. */
		P_LatentHeatFactor,					// Keyword: LatentHeatFactor				[---]		'Latent heat factor'
		/*! Loss Heat Factor. */
		P_LossHeatFactor,					// Keyword: LossHeatFactor					[---]		'Loss heat factor'
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
		PM_PowerPerArea,		// Keyword: PowerPerArea					[-]		'Power per area'
		PM_Power,				// Keyword: Power							[-]		'Power'
		NUM_PM
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid(const Database<Schedule> & scheduleDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;			// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;				// XML:A
	//:inherited	QColor							m_color;					// XML:A

	/*! Internal Load category. */
	Category						m_category = NUM_MC;						// XML:E:required

	/*! Person count method. */
	PersonCountMethod				m_personCountMethod=NUM_PCM;				// XML:E

	/*! Power method. */
	PowerMethod						m_powerMethod=NUM_PM;						// XML:E

	/*! Schedule ID. */
	unsigned int					m_idOccupancySchedule = INVALID_ID;			// XML:E

	/*! Only required for person */
	unsigned int					m_idActivitySchedule = INVALID_ID;			// XML:E

	/*! Only required for moisture balance enabled */
	unsigned int					m_idMoistureProductionRatePerAreaSchedule = INVALID_ID;			// XML:E

	/*! Only required for electric equipment, lights, other. */
	unsigned int					m_idPowerManagementSchedule = INVALID_ID;	// XML:E

	/*! Flag for activate the CO2 production of persons.
		The emission rate is 3.82E-8 m3/Ws
	*/
	bool							m_activateCO2Production = false;			// XML:E

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];								// XML:E
};

}
#endif // VICUS_InternalLoadH
