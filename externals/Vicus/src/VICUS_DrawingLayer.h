#ifndef VICUS_DrawingLayerH
#define VICUS_DrawingLayerH

#include "IBKMK_Vector2D.h"
#include "qcolor.h"

#include <VICUS_CodeGenMacros.h>
#include <VICUS_Object.h>

namespace VICUS {

/*! Layer struct with relevant attributes */
class DrawingLayer : public Object {
public:

	DrawingLayer() {}

	const char * typeinfo() const override {
		return "DrawingLayer";
	}

	// TODO Maik: dokustring
	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	//:inherited	unsigned int		m_id = INVALID_ID;			// XML:A:required
	//:inherited	QString				m_displayName;				// XML:A
	//:inherited	bool				m_visible = true;			// XML:A

	/*! Color of layer if defined */
	QColor			m_color = QColor();			// XML:A
	/*! Line weight of layer if defined */
	int				m_lineWeight;				// XML:A
	/*! ID of block. */
	unsigned int	m_idBlock = INVALID_ID;		// XML:A
};

}

#endif // VICUS_DrawingLayerH
