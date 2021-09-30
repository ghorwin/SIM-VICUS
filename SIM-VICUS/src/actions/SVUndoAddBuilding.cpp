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

#include "SVUndoAddBuilding.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoAddBuilding::SVUndoAddBuilding(const QString & label, const VICUS::Building & addedBuilding, bool topologyOnly) :
	m_addedBuilding(addedBuilding),
	m_topologyOnly(topologyOnly)
{
	setText( label );
}


void SVUndoAddBuilding::undo() {

	// remove last building
	Q_ASSERT(!theProject().m_buildings.empty());

	theProject().m_buildings.pop_back();
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddBuilding::redo() {
	// append building
	theProject().m_buildings.push_back(m_addedBuilding);
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

