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

#include "SVUndoDeleteBuildingLevel.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoDeleteBuildingLevel::SVUndoDeleteBuildingLevel(const QString & label, unsigned int buildingIndex, unsigned int buildingLevelIndex)
	: m_buildingIndex(buildingIndex), m_levelIndex(buildingLevelIndex)
{
	setText( label );

	Q_ASSERT(project().m_buildings.size() > buildingIndex);

	const VICUS::Building & b = project().m_buildings[buildingIndex];
	Q_ASSERT(b.m_buildingLevels.size() > buildingLevelIndex);

	m_deletedLevel = b.m_buildingLevels[buildingLevelIndex];
}


void SVUndoDeleteBuildingLevel::undo() {

	// re-insert level
	theProject().m_buildings[m_buildingIndex].m_buildingLevels.insert(
				theProject().m_buildings[m_buildingIndex].m_buildingLevels.begin() + m_levelIndex, m_deletedLevel);
	theProject().updatePointers();

	// tell project that the building has changed
	// if level object is empty, we only have a topolopy change, otherwise a full geometry update
	if (m_deletedLevel.m_rooms.empty())
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoDeleteBuildingLevel::redo() {
	theProject().m_buildings[m_buildingIndex].m_buildingLevels.erase(
				theProject().m_buildings[m_buildingIndex].m_buildingLevels.begin() + m_levelIndex);
	theProject().updatePointers();

	// tell project that the building has changed
	// if level object is empty, we only have a topolopy change, otherwise a full geometry update
	if (m_deletedLevel.m_rooms.empty())
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

