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

#ifndef VICUS_ZoneControlThermostatH
#define VICUS_ZoneControlThermostatH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of all zone control thermostats.

*/

class ZoneControlThermostat : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Tolerance heating. */
		P_ToleranceHeating,				// Keyword: ToleranceHeating			[K]			'Thermostat tolerance heating mode.'
		/*! Tolerance cooling. */
		P_ToleranceCooling,				// Keyword: ToleranceCooling			[K]			'Thermostat tolerance cooling mode.'

		NUM_P
	};

	/*! Zone control value.*/
	enum ControlValue {
		CV_AirTemperature,		// Keyword: AirTemperature					[-]		'Air temperature'
		CV_RadiantTemperature,	// Keyword: RadiantTemperature				[-]		'Radiant temperature'
		CV_OperativeTemperature,// Keyword: OperativeTemperature			[-]		'Operative temperature'
		NUM_CV
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of zone control thermostat. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of zone control thermostat. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Control value. */
	ControlValue					m_ctrlVal = NUM_CV;						// XML:E:required

	/*! Heating setpoint schedule ID. */
	unsigned int					m_heatingSetpointScheduleId = INVALID_ID;		// XML:E

	/*! Cooling setpoint schedule ID. */
	unsigned int					m_coolingSetpointScheduleId = INVALID_ID;		// XML:E

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_ZoneControlThermostatH
