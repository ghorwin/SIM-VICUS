#ifndef VICUS_WindowDividerH
#define VICUS_WindowDividerH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_MaterialLayer.h"

#include <QString>
#include <QColor>

namespace VICUS {

/*! Window divider is a child of Window, but not a stand-alone
	database element.
*/
class WindowDivider {
public:

	/*! Basic parameters. */
	enum para_t {
		P_Area,						// Keyword: Area					[m2]	'Area of the divider.'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of divider. */
	unsigned int					m_id = INVALID_ID;			// XML:A:required

	/*! Display name of divider. */
	QString							m_displayName;				// XML:A

	/*! Notes. */
	QString							m_notes;					// XML:E

	/*! Data source. */
	QString							m_dataSource;				// XML:E

	/*! Material id of divider. */
	unsigned int					m_idMaterial = INVALID_ID;	// XML:A:required

	/// TODO : Dirk, add geometry to describe divider

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];				// XML:E


};

} // namespace VICUS


#endif // VICUS_WindowDividerH
