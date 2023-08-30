/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef Vic3DPickObjectH
#define Vic3DPickObjectH

#include <IBKMK_Vector3D.h>
#include <IBK_bitfield.h>

#include <QPoint>

namespace Vic3D {

/*! An object to hold information on what to pick and also store the result.
	Basically, whenever you ask the scene to pick a point (after a mouse click, or if the mouse
	cursor has moved), configure such a pick object and call Scene::pick(obj).

	The pick object will now process all objects in the scene and for all activated for picking
	perform a collision test with the line-of-sight. All objects/planes that are hit by the
	line-of-sight are stored in the pick result candidate vector m_candidates.

	Afterwards you can access the data. If you are only interested in the nearest point, you
	can use the convenience function nearestHit() to retrieve the hit closest to the near plane.

*/
struct PickObject {

	/*! Identifies the type of snap point found. */
	enum ResultType {
		/*! An object was hit.
			The unique ID of the object is stored in m_uniqueObjectID.
		*/
		RT_Object,
		/*! The global XY plane or any other artifical plane.
			The index of the plane in the list of planes is stored in m_uniqueObjectID.
		*/
		RT_GridPlane,

		/*! Center of the local coordinate system when in "translate" mode */
		RT_CoordinateSystemCenter,
		/*! One of the marker objects at the end of the axis in "rotation" and "scale" modes have been picked,
			m_uniqueObjectID in pick object identifies axis: 0 = x, 1 = y, 2 = z. */
		RT_AxisEndMarker,
		/*! X-Y-Plane. */
		RT_xyPlane
	};

	/*! Stores information about a particular snap point candidate. */
	struct PickResult {
		bool operator<(const PickResult & other) const {
			return m_depth < other.m_depth;
		}

		ResultType		m_resultType;
		/*! Distance from observer/near plane point. */
		double			m_depth;

		/*! Stores the unique ID of the clicked-on object for m_snapPointType == SP_Object.
			For m_snapPointType == RT_GridPlane, this holds the index of the plane in the Vic3DScene::m_gridPlanes vector.
		*/
		unsigned int	m_objectID = 0;

		/*! Special handling for drawing objects, since it is not a vicus object
			and it origin has to be stored separately. This is needed to perform
			correct picking of drawing objects. */
		unsigned int	m_drawingID = 0;

		/*! Stores the index of a hole/window or any other embedded object. */
		int				m_holeIdx = -1;

		/*! Coordinates of the picked point. */
		IBKMK::Vector3D	m_pickPoint;
	};


	PickObject(const QPoint & localMousePos) :
		m_localMousePos(localMousePos)
	{
	}

	/*! Set to true, if picking was already performed. */
	bool					m_pickPerformed = false;

	/*! The local mouse position. */
	QPoint					m_localMousePos;

	/*! Here we store all possible pick candidates.
		Added will be only those, whose depths is between 0 and 1 (line-of-sight factor).
	*/
	std::vector<PickResult>	m_candidates;

	IBKMK::Vector3D			m_lineOfSightOffset;
	IBKMK::Vector3D			m_lineOfSightDirection;
};


} // namespace Vic3D

#endif // Vic3DPickObjectH
