#ifndef VICUS_ZoneControlNaturalVentilationH
#define VICUS_ZoneControlNaturalVentilationH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of all zone control natural ventilation.

*/

class ZoneControlNaturalVentilation : public AbstractDBElement {
public:

	///TODO Dirk->Andreas das sind hier die verschiedenen Zeitpläne im vector. Muss ich da eine Einheit definieren?
	/*! Schedule types for control. */
	enum ScheduleType{
		ST_TemperatureAirMax,		// Keyword: TemperatureAirMax					[C]		'Upper limit for room air temperature.'
		ST_TemperatureAirMin,		// Keyword: TemperatureAirMin					[C]		'Lower limit for room air temperature.'
		ST_TemperatureOutsideMax,	// Keyword: TemperatureOutsideMax				[C]		'Upper limit for outside air temperature.'
		ST_TemperatureOutsideMin,	// Keyword: TemperatureOutsideMin				[C]		'Lower limit for outside air temperature.'
		ST_TemperatureDifference,	// Keyword: TemperatureDifference				[K]		'Temperature difference limit (inside - outside).'
		ST_WindSpeedMax,			// Keyword: WindSpeedMax						[m/s]	'Limit for wind speed .'
		NUM_ST
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/// TODO Dirk->Andreas wie initialisiere ich alle im array mit einer invalid id vor?
	/// TODO Dirk, int var[xxx] geht so nicht!
	/*! Schedule ID. */
	unsigned int					m_scheduleId[NUM_ST];					// XML:E

	/// TODO Dirk Das ist nur vorläufig bis die Zeitpläne funktionieren.
	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_ST];							// XML:E

};

}
#endif // VICUS_ZoneControlNaturalVentilationH
