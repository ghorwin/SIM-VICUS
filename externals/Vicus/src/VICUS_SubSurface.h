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

#ifndef VICUS_SubSurfaceH
#define VICUS_SubSurfaceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Polygon2D.h"
#include "VICUS_Object.h"

#include "NANDRAD_IDVectorMap.h"

#include <IBK_LinearSpline.h>
#include <IBK_point.h>

#include <QString>
#include <QColor>

namespace VICUS {

class SubSurfaceComponentInstance;

/*! Represents a SubSurface and its associated properties. */
class SubSurface : public Object {
public:
	/*! Type-info string. */
	const char * typeinfo() const override { return "SubSurface"; }

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Sets color based on sub-surface type. */
	void updateColor();

	/*! Offset from first point of the parent surface to the first point this sub surface. */
	const IBK::point2D<double> & offset() const { return m_polygon2D.vertexes()[0]; }

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int		m_id = INVALID_ID;			// XML:A:required
	//:inherited	QString				m_displayName;				// XML:A
	//:inherited	bool				m_visible = true;			// XML:A

	/*! The actual geometry. */
	Polygon2D							m_polygon2D;				// XML:E


	// *** RUNTIME VARIABLES ***

	/*! Color to be used when next updating the geometry.
		Color is set based on hightlighting/selection algorithm.

		Color is not a regular property of a SubSurface, but rather of the associated parameter elements.
	*/
	mutable QColor						m_color; // Note: mutable so that it can be modified on const project

	/*! Runtime-only pointer to the associated subsurface component instance (or nullptr, if SubSurface
		is not yet connected to any component. This would be considered an incomplete
		data model).
		The pointer is updated in VICUS::Project::updatePointers().
	*/
	SubSurfaceComponentInstance			*m_subSurfaceComponentInstance = nullptr;

	/*! Map that stores the id of a (sub)surface and the viewFactor onto that (subSurface) */
	NANDRAD::IDVectorMap<double>		m_viewFactors;				// XML:E
};

} // namespace VICUS


#endif // VICUS_SubSurfaceH
