#ifndef VICUS_SurfaceH
#define VICUS_SurfaceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_PlaneGeometry.h"
#include "VICUS_Object.h"

#include "IBK_LinearSpline.h"

#include <QString>
#include <QColor>

namespace VICUS {

class ComponentInstance;

/*! Represents a surface and its associated properties. */
class Surface : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Update surface colors based on orientation of associated plane geometry.
		Color is only updated if current color is QColor::Invalid.
	*/
	void updateColor();

	/*! Creates a copy of the surface object but with a new unique ID. */
	Surface clone() const{
		Surface r(*this); // create new surface with same unique ID
		Object & o = r;
		(Object&)r = o.clone(); // assign new ID only
		return r;
	}

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! The actual geometry. */
	PlaneGeometry						m_geometry;					// XML:E

	/*! Linear Spline that holds annual shading factors, only for visualization
		TODO : move to different places or use for visualization
	*/
	IBK::LinearSpline					m_shadingFactor;

	/*! Stores visibility information for this surface.
		Note: keep the next line - this will cause the code generator to create serialization code
			  for the inherited m_visible variable.
	*/
	//:inherited	bool								m_visible = true;			// XML:A

	// *** Runtime Variables ***

	/*! Color to be used when next updating the geometry.
		Color is set based on hightlighting/selection algorithm.

		Color is not a regular property of a surface, but rather of the associated parameter elements.
	*/
	mutable QColor						m_color; // Note: mutable so that it can be modified on const project

	/*! Runtime-only pointer to the associated component instance (or nullptr, if surface
		is not yet connected to any component. This would be considered an incomplete
		data model.
		The pointer is updated in VICUS::Project::updatePointers().
	*/
	ComponentInstance					*m_componentInstance = nullptr;

};

} // namespace VICUS


#endif // VICUS_SurfaceH
