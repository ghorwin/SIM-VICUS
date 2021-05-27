/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Update surface colors based on orientation of associated plane geometry.
		Color is only updated if current color is QColor::Invalid.
	*/
	void updateColor();

	/*! Creates a copy of the surface object but with a new unique ID. */
	Surface clone() const;

	void updateParents();

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Gives read-access to the surface's main polygon. */
	const Polygon3D &					polygon3D() const {	return m_polygon3D; }

	/*! Sets the polygon. */
	void setPolygon3D(const Polygon3D & polygon3D);

	const std::vector<SubSurface> &		subSurfaces() const { return m_subSurfaces; }
	void setSubSurfaces(const std::vector<SubSurface> & subSurfaces);

	/*! Gives read-access to the surface's geometry. */
	const PlaneGeometry &				geometry() const { return m_geometry; }

	/*! Flip the geometry of the polygon and recompute local coordinates of all embedded (and also flipped) subsurfaces */
	void flip();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of surface. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A


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

private:
	/*! The polygon describing this surface. */
	Polygon3D							m_polygon3D;				// XML:E

	/*! Subsurfaces of the surface. */
	std::vector<SubSurface>				m_subSurfaces;				// XML:E

	// *** Runtime Variables ***

	/*! This variable is set whenever the polygon or the subsurfaces have been modified.
		When the geometry is being accessed, and m_dirty is true, we perform the triangulation.
	*/
//	bool								m_dirty = false;

	/*! The actual geometry. This object manages the triangulation of the surface's polygon and
		its embedded subsurfaces.
	*/
	PlaneGeometry						m_geometry;

};

} // namespace VICUS


#endif // VICUS_SurfaceH
