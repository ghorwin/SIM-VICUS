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

#include "SVUndoModifyBuildingTopology.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoModifyBuildingTopology::SVUndoModifyBuildingTopology(const QString & label, const std::vector<VICUS::Building> & buildings,
														   const std::vector<VICUS::ComponentInstance> *surfaceComponentInstances) :
	m_buildings(buildings)
{
	setText( label );
	if (surfaceComponentInstances != nullptr) {
		m_modifySurfaceComponentInstances = true;
		m_surfaceComponentInstances = *surfaceComponentInstances;
	}
}


void SVUndoModifyBuildingTopology::undo() {
	// exchange building meta data
	std::swap( theProject().m_buildings, m_buildings);

	// also modified sub-surface components, if needed
	if (m_modifySurfaceComponentInstances) {
		m_surfaceComponentInstances.swap(theProject().m_componentInstances);
	}

	theProject().updatePointers();
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoModifyBuildingTopology::redo() {
	undo(); // same code as undo
}
