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

#include "SVUndoCopyBuildingLevels.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoCopyBuildingLevels::SVUndoCopyBuildingLevels(const QString & label,
												   const std::vector<VICUS::Building> & modifiedBuilding,
												   const std::vector<VICUS::ComponentInstance> & modifiedComponentInstances,
												   const std::vector<VICUS::SubSurfaceComponentInstance> & modifiedSubSurfaceComponentInstances) :
	m_modifiedBuilding(modifiedBuilding),
	m_modifiedComponentInstances(modifiedComponentInstances),
	m_modifiedSubSurfaceComponentInstances(modifiedSubSurfaceComponentInstances)
{
	setText( label );
}


SVUndoCopyBuildingLevels * SVUndoCopyBuildingLevels::createUndoCopyBuildingLevels(const std::vector<const VICUS::BuildingLevel *> & selectedBuildingLevels,
																				  const IBKMK::Vector3D & translation)
{

}


void SVUndoCopyBuildingLevels::undo() {

	theProject().m_buildings.swap(m_modifiedBuilding);
	theProject().m_componentInstances.swap(m_modifiedComponentInstances);
	theProject().m_subSurfaceComponentInstances.swap(m_modifiedSubSurfaceComponentInstances);

	// update pointers
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoCopyBuildingLevels::redo() {
	undo();
}

