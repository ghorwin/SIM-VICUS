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

#include "SVUndoAddProject.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoAddProject::SVUndoAddProject(const QString & label, const VICUS::Project & p) {
	setText( label );

	// we create here a combined project that holds all data

	// start with our current project
	m_project = project();

	// now we copy all lists
	// TODO : ensure unique IDs.... this is tricky. So far, we assume that importer plugins use
	// a dedicated ID space, but this is not reliable

	// process all buildings, copy them and update the IDs if there is a duplicate

	m_project.m_buildings.insert(m_project.m_buildings.end(), p.m_buildings.begin(), p.m_buildings.end());
	m_project.m_componentInstances.insert(m_project.m_componentInstances.end(),
										  p.m_componentInstances.begin(), p.m_componentInstances.end() );
	m_project.m_subSurfaceComponentInstances.insert(m_project.m_subSurfaceComponentInstances.end(),
										  p.m_subSurfaceComponentInstances.begin(), p.m_subSurfaceComponentInstances.end() );
	m_project.m_plainGeometry.insert(m_project.m_plainGeometry.end(),
										  p.m_plainGeometry.begin(), p.m_plainGeometry.end() );
	m_project.m_geometricNetworks.insert(m_project.m_geometricNetworks.end(),
										  p.m_geometricNetworks.begin(), p.m_geometricNetworks.end() );
}


void SVUndoAddProject::undo() {
	// swap project
	std::swap(theProject(), m_project);
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::AllModified);
}


void SVUndoAddProject::redo() {
	undo();
}

