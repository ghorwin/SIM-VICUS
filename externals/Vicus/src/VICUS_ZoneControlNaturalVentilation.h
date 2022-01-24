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

#ifndef VICUS_ZoneControlNaturalVentilationH
#define VICUS_ZoneControlNaturalVentilationH

#include <QColor>

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Schedule.h"

namespace VICUS {

/*! Zone control parameters for natural ventilation. */
class ZoneControlNaturalVentilation : public AbstractDBElement {
public:

	/*! Parameters for ventilation control model. Can be given as constant parameters in m_para, or can be
		defined as scheduled quantities. For each of these parameters that is provided as schedule, the
		m_idSchedules[] vector contains non INVALID_ID entries.
	*/


	enum para_t {
		P_MaximumAirChangeRateComfort,				// Keyword: MaximumAirChangeRateComfort			[1/h]	' Maximum air change rate for natural ventilation to reach comfort conditions (if possible).'
		P_TemperatureAirMax,						// Keyword: TemperatureAirMax					[C]		'Upper limit for room air temperature.'
		P_TemperatureAirMin,						// Keyword: TemperatureAirMin					[C]		'Lower limit for room air temperature.'
		P_WindSpeedMax,								// Keyword: WindSpeedMax						[m/s]	'Limit for wind speed .'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor, initializes m_idSchedules with INVALID_ID */
	ZoneControlNaturalVentilation() {
		for (int i=0; i<NUM_P; ++i)
			m_idSchedules[i] = INVALID_ID;
	}

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid(const Database<Schedule> &scheduleDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Schedule ID. */
	IDType							m_idSchedules[NUM_P];					// XML:E

	/*! List of constant parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_ZoneControlNaturalVentilationH
