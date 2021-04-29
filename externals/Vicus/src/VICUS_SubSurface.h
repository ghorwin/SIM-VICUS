#ifndef VICUS_SubSurfaceH
#define VICUS_SubSurfaceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Polygon2D.h"
#include "VICUS_Object.h"

#include <IBK_LinearSpline.h>
#include <IBK_point.h>

#include <QString>
#include <QColor>

namespace VICUS {

class ComponentInstance;

/*! Represents a SubSurface and its associated properties. */
class SubSurface : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Update SubSurface colors based on orientation of associated plane geometry.
		Color is only updated if current color is QColor::Invalid.
	*/
	void updateColor();

	/*! Creates a copy of the SubSurface object but with a new unique ID. */
	SubSurface clone() const{
		SubSurface r(*this); // create new SubSurface with same unique ID
		Object & o = r;
		(Object&)r = o.clone(); // assign new ID only
		return r;
	}

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of sub surface. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! The actual geometry. */
	Polygon2D							m_geometry;					// XML:E

	/*! Offset from first point of the parent surface to the first point this sub surface. */
	IBK::point2D<double>				m_offset;					// XML:E

	/*! Stores visibility information for this SubSurface.
		Note: keep the next line - this will cause the code generator to create serialization code
			  for the inherited m_visible variable.
	*/
	//:inherited	bool								m_visible = true;			// XML:A

	// *** Runtime Variables ***

	/*! Color to be used when next updating the geometry.
		Color is set based on hightlighting/selection algorithm.

		Color is not a regular property of a SubSurface, but rather of the associated parameter elements.
	*/
	mutable QColor						m_color; // Note: mutable so that it can be modified on const project

	/*! Runtime-only pointer to the associated component instance (or nullptr, if SubSurface
		is not yet connected to any component. This would be considered an incomplete
		data model.
		The pointer is updated in VICUS::Project::updatePointers().
	*/
	ComponentInstance					*m_componentInstance = nullptr;

};

} // namespace VICUS


#endif // VICUS_SubSurfaceH
