#ifndef VICUS_ZoneTemplateH
#define VICUS_ZoneTemplateH

#include <IBK_LinearSpline.h>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of a *single* ZoneTemplated quantity (basically a value over time data set).
 *  This ZoneTemplate does not have a unit.
*/
class ZoneTemplate : public AbstractDBElement {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced ZoneTemplate is valid. */
	bool isValid() const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of ZoneTemplate. */
	unsigned int					m_id						= INVALID_ID;		// XML:A:required

	/*! Display name of ZoneTemplate. */
	IBK::MultiLanguageString		m_displayName;									// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;										// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;									// XML:E

	/*! Internal loads person model id. */
	unsigned int					m_idIntLoadPerson			= INVALID_ID;		// XML:E

	/*! Internal loads electric equipment model id. */
	unsigned int					m_idIntLoadElectricEquipment = INVALID_ID;		// XML:E

	/*! Internal loads electric lighting model id. */
	unsigned int					m_idIntLoadLighting			= INVALID_ID;		// XML:E

	/*! Internal loads other equipment model id. */
	unsigned int					m_idIntLoadOther			= INVALID_ID;		// XML:E

	/*! Control thermostat model id. */
	unsigned int					m_idControlThermostat		= INVALID_ID;		// XML:E

	/*! Control shading model id. */
	unsigned int					m_idControlShading			= INVALID_ID;		// XML:E

	/*! Natural ventilation model id. */
	unsigned int					m_idNaturalVentilation		= INVALID_ID;		// XML:E

	/*! Mechanical ventilation model id. */
	unsigned int					m_idMechanicalVentilation	= INVALID_ID;		// XML:E

	/*! Infiltration model id. */
	unsigned int					m_idInfiltration			= INVALID_ID;		// XML:E

};

} // namespace VICUS


#endif // VICUS_ZoneTemplateH
