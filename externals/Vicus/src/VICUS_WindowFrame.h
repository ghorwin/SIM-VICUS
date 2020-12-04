#ifndef VICUS_WindowFrameH
#define VICUS_WindowFrameH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_MaterialLayer.h"

#include <QString>
#include <QColor>

namespace VICUS {

/*! Window frame is a child of Window, but not a stand-alone
	database element.
*/
class WindowFrame {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of frame. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! Display name of frame. */
	QString							m_displayName;			// XML:A

	/*! Notes. */
	QString							m_notes;				// XML:E

	/*! Data source. */
	QString							m_dataSource;			// XML:E

	/*! Material layer for frame material with thickness. */
	MaterialLayer					m_materialLayer;		// XML:E

	/// TODO : Dirk, add geometry to describe frame

	/*! Frame area calculation.
		true: frame area is calculated by a percentage of the hole window
		false: in this case the circumference is used to calculate the frame area
	*/
	bool							m_isPercentageCalcMethode = true;	// XML:E


};

} // namespace VICUS


#endif // VICUS_WindowFrameH
