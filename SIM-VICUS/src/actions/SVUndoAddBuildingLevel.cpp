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

#include "SVUndoAddBuildingLevel.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoAddBuildingLevel::SVUndoAddBuildingLevel(const QString & label, unsigned int buildingUUID, const VICUS::BuildingLevel & addedLevel, bool topologyOnly) :
	m_addedLevel(addedLevel),
	m_topologyOnly(topologyOnly),
	m_buildingUUID(buildingUUID)
{
	setText( label );
}


void SVUndoAddBuildingLevel::undo() {
	// lookup modified building
	const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(theProject().objectById(m_buildingUUID));
	Q_ASSERT(b != nullptr);

	// remove last building level
	Q_ASSERT(!b->m_buildingLevels.empty());

	const_cast<VICUS::Building *>(b)->m_buildingLevels.pop_back();
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddBuildingLevel::redo() {
	// lookup modified building
	const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(theProject().objectById(m_buildingUUID));
	Q_ASSERT(b != nullptr);

	// append building level
	const_cast<VICUS::Building *>(b)->m_buildingLevels.push_back(m_addedLevel);
	theProject().updatePointers();

	// tell project that the building geometry has changed
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

