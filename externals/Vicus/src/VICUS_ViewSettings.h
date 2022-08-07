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

#ifndef VICUS_ViewSettingsH
#define VICUS_ViewSettingsH

#include <IBK_Flag.h>

#include <QColor>
#include <QQuaternion>

#include <IBKMK_Vector3D.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_RotationMatrix.h"
#include "VICUS_GridPlane.h"

namespace VICUS {

/*! Stores general settings about user interface appearance. */
class ViewSettings {
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! C'tor, initializes defaults. */
	ViewSettings() :
		m_cameraTranslation(0, -100, 50),
		m_cameraRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f,0.f, 0.f), 60))
	{
		m_gridPlanes.push_back( VICUS::GridPlane(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,0,1),
												 IBKMK::Vector3D(1,0,0), QColor("white"), 500, 10 ) );
#if 0
		m_gridPlanes.push_back( VICUS::GridPlane(IBKMK::Vector3D(0,0,-2), IBKMK::Vector3D(0,0,1),
												 IBKMK::Vector3D(1,0,0), QColor("#3030a0"), 100, 10 ) );
		m_gridPlanes.push_back( VICUS::GridPlane(IBKMK::Vector3D(0,0,6), IBKMK::Vector3D(0,0.2,0.8).normalized(),
												 IBKMK::Vector3D(1,0,0), QColor("#e09040"), 50, 10 ) );
#endif
	}

	VICUS_READWRITE_IFNOTEMPTY(ViewSettings)
	VICUS_COMP(ViewSettings)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Camera position. */
	IBKMK::Vector3D						m_cameraTranslation;			// XML:E

	/*! Camera rotation. */
	RotationMatrix						m_cameraRotation;				// XML:E

	/*! Visibility of scene in [m]. */
	double								m_farDistance = 10000;			// XML:E

	/*! Vector with grid planes.
		This vector always has at least one main grid.
	*/
	std::vector<VICUS::GridPlane>		m_gridPlanes;					// XML:E
};


inline bool ViewSettings::operator!=(const ViewSettings & other) const {
	if (m_cameraTranslation != other.m_cameraTranslation) return true;
	if (m_cameraRotation != other.m_cameraRotation) return true;
	if (m_farDistance != other.m_farDistance) return true;
	if (m_gridPlanes != other.m_gridPlanes) return true;
	return false;
}

} // namespace VICUS


#endif // VICUS_ViewSettingsH
