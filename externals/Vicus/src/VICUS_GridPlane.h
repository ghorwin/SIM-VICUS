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

#ifndef VICUS_GridPlaneH
#define VICUS_GridPlaneH

#include <QColor>
#include <IBKMK_Vector3D.h>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

/*! Contains definition of a single grid plane and related functionality.

	A grid plane is defined by the offset (always in the middle), its normal vector and
	local X vector, that defines the orientation.

	The dimension of the grid plane is defined by width (total width) and the major grid spacing.
	The width is automatically adjusted to hold an even number number of grid spacings.
	For example, if the grid is defined with a width of 125 m, and grid spacing is 10 m, than
	the final grid will extend from -70..70 covering a total of 140 m.

	The minor grid spacing is computed automatically to be 1/10 of the major grid.
*/
class GridPlane {
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	GridPlane() = default;

	/*! Initializing constructor, generated grid will be visible and active by default. */
	GridPlane(const IBKMK::Vector3D & offset, const IBKMK::Vector3D & normal, const IBKMK::Vector3D & localX,
			  const QColor majorGridColor, unsigned int width, unsigned int majorGridSpacing) :
		m_offset(offset),
		m_normal(normal),
		m_localX(localX),
		m_color(majorGridColor),
		m_width(width),
		m_spacing(majorGridSpacing)
	{
	}

	VICUS_READWRITE
	VICUS_COMP(GridPlane)

	/*! Computes intersection point of a line defined by point p and direction with the plane,
		and also returns the nearest grid snap point.
		Throws an exception if plane is invalid (null vector).
		\param p Start point of the line
		\param direction Direction vector of the line
		\param t Here the scale factor for the line to hit the plane is stored (0...1)
		\param intersectionPoint Will hold the point where line hits the surface
		\return Returns true, if an intersection was found, otherwise false. If plane is inactive, function
			always returns false;
	*/
	bool intersectsLine(const IBKMK::Vector3D & p, const IBKMK::Vector3D & direction,
						double & t, IBKMK::Vector3D & intersectionPoint) const;

	/*! Determines the snap point nearest to the intersection point (which must lie on the plane).
		\param intersectionPoint Point on the plane
		\param snapPoint Will hold the snap point on grid nearest to the intersection point
	*/
	void closestSnapPoint(const IBKMK::Vector3D & intersectionPoint, IBKMK::Vector3D & snapPoint) const;

	/*! Some descriptive name of the grid. */
	QString			m_name;				// XML:A
	/*! True, if grid should be shown. */
	bool			m_isVisible = true;	// XML:A
	/*! True, if grid should be clickable. */
	bool			m_isActive = true;	// XML:A
	/*! Offset vector of grid plane */
	IBKMK::Vector3D m_offset;			// XML:E
	/*! Normal vector of grid plane, must have magnitude 1 for intersectsLine() function to work. */
	IBKMK::Vector3D m_normal;			// XML:E
	/*! Local X vector of grid plane, must be perpendicular to m_normal; indicates the orientation of the grid. */
	IBKMK::Vector3D m_localX;			// XML:E
	/*! Major grid color (minor grid color is automatically adjusted, either brighter or dimmer depending on current theme) */
	QColor			m_color;			// XML:E
	/*! Grid width in [m]. */
	unsigned int	m_width = 10;		// XML:E
	/*! Major grid spacing in [m]. */
	unsigned int	m_spacing = 10;		// XML:E

private:

	// *** RUNTIME VARIABLES ***

	// The variables below are used in the snapping algorithm and are cached to avoid calculation
	// They are recomputed whenever the grid is prepared for rendering (see Vic3D::GridObject).

	/*! Local Y-axis. */
	IBKMK::Vector3D m_localY;
	/*! Minimum/Maximum local x/y coordinate of grid. */
	double			m_gridExtends;
	/*! Number of major grid lines (always an odd number). */
	unsigned int	m_nGridLines = 0;
};

} // namespace VICUS


#endif // VICUS_GridPlaneH
