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
	/*! Type-info string. */
	const char * typeinfo() const override { return "Surface"; }

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Sets default surface color based on inclination of associated plane geometry. */
	void initializeColorBasedOnInclination();

	void updateParents();

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Gives read-access to the surface's main polygon. */
	const IBKMK::Polygon3D &			polygon3D() const {	return m_geometry.polygon3D(); }

	/*! Sets the polygon. */
	void setPolygon3D(const IBKMK::Polygon3D & polygon3D);

	const std::vector<SubSurface> &		subSurfaces() const { return m_subSurfaces; }
	const std::vector<Surface> &		childSurfaces() const { return m_childSurfaces; }

	void setChildAndSubSurfaces(const std::vector<SubSurface>  & subSurfaces,
								const std::vector<Surface>     &childSurfaces);

	/*! Gives read-access to the surface's geometry. */
	const PlaneGeometry & geometry() const { return m_geometry; }

	/*! Flips the normal vector of polygon.
		This also swaps local X and localY axes, so the x and y coordinates of our sub-surface
		polygons are swapped as well.
	*/
	void flip();

	void changeOrigin(unsigned int idx);

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int		m_id = INVALID_ID;			// XML:A:required
	//:inherited	QString				m_displayName;				// XML:A
	//:inherited	bool				m_visible = true;			// XML:A

	/*! The color to be used when rendering the surface in regular mode (not in false-color mode).
		Important also for daylight calculation.
	*/
	QColor								m_displayColor;				// XML:A

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
	/*! Subsurfaces of the surface. */
	std::vector<SubSurface>				m_subSurfaces;				// XML:E

	/*! ChildSurfaces of the surface. */
	std::vector<Surface>				m_childSurfaces;			// XML:E

	// *** RUNTIME VARIABLES ***

	/*! The actual geometry. This object manages the triangulation of the surface's polygon and
		its embedded subsurfaces.
	*/
	PlaneGeometry						m_geometry;

};

} // namespace VICUS


#endif // VICUS_SurfaceH
