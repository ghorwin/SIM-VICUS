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

class GridPlane {
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	GridPlane() = default;
	GridPlane(const IBKMK::Vector3D & offset, const IBKMK::Vector3D & normal, const QColor gridColor,
		  unsigned int gridSpacing) :
		m_offset(offset),
		m_normal(normal),
		m_gridColor(gridColor),
		m_gridSpacing(gridSpacing)
	{
	}

	VICUS_READWRITE
	VICUS_COMP(GridPlane)

	/*! Computes intersection point of a line defined by point p and direction with the plane,
		and also returns the nearest grid snap point.
		Throws an exception if plane is invalid (null vector).
	*/
	bool intersectsLine(const IBKMK::Vector3D & p, const IBKMK::Vector3D & direction,
						unsigned int & t, IBKMK::Vector3D & intersectionPoint, IBKMK::Vector3D & snapPoint) const;

	IBKMK::Vector3D m_offset;			// XML:E
	IBKMK::Vector3D m_normal;			// XML:E
	QColor			m_gridColor;		// XML:E
	unsigned int	m_gridSpacing = 5;	// XML:E
};

} // namespace VICUS


#endif // VICUS_GridPlaneH
