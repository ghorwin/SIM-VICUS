#ifndef VICUS_WindowH
#define VICUS_WindowH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_WindowFrame.h"
#include "VICUS_WindowDivider.h"

#include <QString>
#include <QColor>

namespace VICUS {

/*! Data for a window (embedded object), defined through glazing system, frame a dividers. */
class Window : public AbstractDBElement {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	/*! Checks if references glazing system exist and if all parameters are valid. */
	bool isValid() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of frame. */
	unsigned int					m_id = INVALID_ID;					// XML:A:required

	/*! Display name of frame. */
	IBK::MultiLanguageString		m_displayName;						// XML:A

	/*! ID of glazing system referenced from this window definition. */
	unsigned int					m_glazingSystemID = INVALID_ID;		// XML:A

	/*! Notes. */
	QString							m_notes;							// XML:E

	/*! Data source. */
	QString							m_dataSource;						// XML:E

	/*! Frame parameters (optional). */
	WindowFrame						m_frame;							// XML:E
	/*! Divider parameters (optional). */
	WindowDivider					m_divider;							// XML:E

};

} // namespace VICUS


#endif // VICUS_WindowH
