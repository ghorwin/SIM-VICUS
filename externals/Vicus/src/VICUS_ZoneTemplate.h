#ifndef VICUS_ZoneTemplateH
#define VICUS_ZoneTemplateH

#include <IBK_IntPara.h>

#include <QColor>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of a *single* ZoneTemplated quantity (basically a value over time data set).
 *  This ZoneTemplate does not have a unit.
*/
class ZoneTemplate : public AbstractDBElement {
public:

	/*! Types used to identify individual sub-templates. */
	enum SubTemplateType {
		ST_IntLoadPerson,			// Keyword: IntLoadPerson
		ST_IntLoadEquipment,		// Keyword: IntLoadEquipment
		ST_IntLoadLighting,			// Keyword: IntLoadLighting
		ST_IntLoadOther,			// Keyword: IntLoadOther
		ST_ControlThermostat,		// Keyword: ControlThermostat
		NUM_ST
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	ZoneTemplate();

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced ZoneTemplate is valid. */
	bool isValid() const;

	/*! Returns number of assigned sub-templates (needed by tree-model). */
	unsigned int subTemplateCount() const;

	/*! Returns the type of reference by index, counting only the used references, i.e. references not INVALID_ID.
		For example, if m_idIntLoadPerson == INVALID_ID and m_idIntLoadElectricEquipment has a valid ID, than
		usedReference(0) returns ST_IntLoadEquipment.
		\return Returns type of corresponding id reference or NUM_ST, if index is larger than the number of non-empty
				id references.
	*/
	SubTemplateType usedReference(unsigned int index) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of ZoneTemplate. */
	unsigned int					m_id						= INVALID_ID;		// XML:A:required

	/*! Display name of ZoneTemplate. */
	IBK::MultiLanguageString		m_displayName;									// XML:A

	/*! False color. */
	QColor							m_color;										// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;										// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;									// XML:E

	/*! Stores id references for all sub-templates. */
	IDType							m_idReferences[NUM_ST];							// XML:E

#if 0
	/*! Internal loads person model id. */
	unsigned int					m_idIntLoadPerson			= INVALID_ID;

	/*! Internal loads electric equipment model id. */
	unsigned int					m_idIntLoadElectricEquipment = INVALID_ID;

	/*! Internal loads electric lighting model id. */
	unsigned int					m_idIntLoadLighting			= INVALID_ID;

	/*! Internal loads other equipment model id. */
	unsigned int					m_idIntLoadOther			= INVALID_ID;

	/*! Control thermostat model id. */
	unsigned int					m_idControlThermostat		= INVALID_ID;

	/*! Control shading model id. */
	unsigned int					m_idControlShading			= INVALID_ID;

	/*! Natural ventilation model id. */
	unsigned int					m_idNaturalVentilation		= INVALID_ID;

	/*! Mechanical ventilation model id. */
	unsigned int					m_idMechanicalVentilation	= INVALID_ID;

	/*! Infiltration model id. */
	unsigned int					m_idInfiltration			= INVALID_ID;
#endif
};


} // namespace VICUS


#endif // VICUS_ZoneTemplateH
