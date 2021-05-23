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

#ifndef Vic3DCameraH
#define Vic3DCameraH

#include "Vic3DTransform3D.h"

namespace Vic3D {

/*! A transformation class with additional functions related to perspective transformation (camera lens). */
class Camera : public Transform3D {
public:

	/*! Returns a forward vector (with respect to the camera's local coordinate/view system).
		The local coordinate system of the camera is perpendicular to the x-y-plane, looking into negative z direction.
		Hence, the local forward vector is 0,0,-1. The left-right is along y-axis, up, down along x-axis.
		Mind: this is the local view of the camera, not the world's coordinate view.
	*/
	QVector3D forward() const {
		const QVector3D LocalForward(0.0f, 0.0f, -1.0f);
		return m_rotation.rotatedVector(LocalForward);
	}

	/*! Returns vector pointing up (with respect to the camera's local coordinate/view system) */
	QVector3D up() const {
		const QVector3D LocalUp(0.0f, 1.0f, 0.0f);
		return m_rotation.rotatedVector(LocalUp);
	}

	/*! Returns vector pointing to the right (with respect to the camera's local coordinate/view system) */
	QVector3D right() const {
		const QVector3D LocalRight(1.0f, 0.0f, 0.0f);
		return m_rotation.rotatedVector(LocalRight);
	}

	/*! Transformation matrix to convert from world to view coordinates when left-multiplied with world coords.
		Mind: no scaling applied.
	*/
	const QMatrix4x4 & toMatrix() const {
		if (m_dirty) {
			m_dirty = false;
			m_world.setToIdentity();
			m_world.rotate(m_rotation.conjugated());
			m_world.translate(-m_translation);
		}
		return m_world;
	}
};

} // namespace Vic3D

#endif // Vic3DCameraH
