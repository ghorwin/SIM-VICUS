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

	enum SurfaceType{
		SC_Wall,
		SC_Floor,
		SC_Roof,
		NUM_SC
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Update surface colors by surface type. */
	void updateColor(const SurfaceType& type);

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! The actual geometry. */
	PlaneGeometry						m_geometry;					// XML:E

	/*! Stores visibility information for this surface. */
	bool								m_visible = true;			// XML:A

	// *** Extend parameters for thermal surfaces ***
	/*! */
	unsigned int						m_componentId = INVALID_ID;	// XML:A

	// *** Runtime Variables ***

	/*! Color to be used when next updating the geometry.
		Color is set based on hightlighting/selection algorithm.

		Color is not a regular property of a surface, but rather of the associated parameter elements.
	*/
	QColor								m_color;

};

} // namespace VICUS


#endif // VICUS_SurfaceH
