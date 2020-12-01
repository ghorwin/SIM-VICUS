#ifndef VICUS_SurfaceH
#define VICUS_SurfaceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_PlaneGeometry.h"
#include "VICUS_Object.h"

#include <QString>
#include <QColor>

namespace VICUS {

/*! Represents a surface and its associated properties. */
class Surface : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! The actual geometry. */
	PlaneGeometry						m_geometry;					// XML:E

	/*! Stores visibility information for this surface. */
	bool								m_visible = true;			// XML:A
	/*! Stores selected information for this surface (not serialized, for now). */
	bool								m_selected = false;


	// *** Runtime Variables ***

	/*! Color to be used when next updating the geometry.
		Color is set based on hightlighting/selection algorithm.

		Color is not a regular property of a surface, but rather of the associated parameter elements.
	*/
	QColor								m_color;

};

} // namespace VICUS


#endif // VICUS_SurfaceH
