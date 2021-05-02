#ifndef VICUS_SurfaceH
#define VICUS_SurfaceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_PlaneGeometry.h"
#include "VICUS_Object.h"
#include "VICUS_SubSurface.h"
#include "VICUS_Polygon3D.h"

#include <IBK_LinearSpline.h>

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

	/*! Gives read-access to the surface's geometry. */
	const PlaneGeometry					geometry() const { return m_geometry; }

	/*! This function updates the triangulation of the surface and its subsurfaces.
		Call this function whenever you change m_polygon3D and/or m_subsurfaces.
	*/
	void computeGeometry();

	/*! Flip the geometry of the polygon and recompute local coordinates of all embedded (and also flipped) subsurfaces */
	void flip();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of surface. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! The polygon describing this surface. */
	Polygon3D							m_polygon3D;				// XML:E

	/*! Linear Spline that holds annual shading factors, only for visualization
		TODO : move to different places or use for visualization
	*/
	IBK::LinearSpline					m_shadingFactor;

	/*! Subsurfaces of the surface. */
	std::vector<SubSurface>				m_subSurfaces;				// XML:E

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

private:

	/*! The actual geometry. This object manages the triangulation of the surface's polygon and
		its embedded subsurfaces.
	*/
	PlaneGeometry						m_geometry;

};

} // namespace VICUS


#endif // VICUS_SurfaceH
