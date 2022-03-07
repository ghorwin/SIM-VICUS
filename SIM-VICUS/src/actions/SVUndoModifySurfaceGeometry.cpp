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

#include "SVUndoModifySurfaceGeometry.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

#include "SVViewStateHandler.h"
#include "Vic3DWireFrameObject.h"

SVUndoModifySurfaceGeometry::SVUndoModifySurfaceGeometry(const QString & label,
														 const std::vector<VICUS::Surface> & surfaces,
														 const std::vector<VICUS::SubSurfaceComponentInstance> * subSurfaceComponentInstances)
	: m_surfaces(surfaces)
{
	setText( label );
	if (subSurfaceComponentInstances != nullptr) {
		m_modifySubSurfaceComponentInstances = true;
		m_subSurfaceComponentInstances = *subSurfaceComponentInstances;
	}
}


void SVUndoModifySurfaceGeometry::undo() {

	std::vector<const VICUS::Surface*> surfacesProject;

	// since selection change is also an undo property, we can rely on having the same selection
	// here as when the surface properties were modified
	theProject().selectedSurfaces(surfacesProject, VICUS::Project::SG_All);

	for (const VICUS::Surface *sOld : surfacesProject ) {
		for ( VICUS::Surface &sNew : m_surfaces ) {
			if ( sOld->m_id == sNew.m_id ) {
				// we swap the surface's polygon and the subsurfaces polygons
				VICUS::Surface * oldS = const_cast<VICUS::Surface *>(sOld);

				IBKMK::Polygon3D oldPolygon = oldS->polygon3D();
				oldS->setPolygon3D(VICUS::Polygon3D(sNew.polygon3D()));
				sNew.setPolygon3D(VICUS::Polygon3D(oldPolygon));

				std::vector<VICUS::SubSurface> oldSubSurfaces = oldS->subSurfaces();
				oldS->setSubSurfaces(sNew.subSurfaces());
				sNew.setSubSurfaces(oldSubSurfaces);

				// Note: triangulation is only updated once requested
				break;
			}
		}
	}

	// also modified sub-surface components, if needed
	if (m_modifySubSurfaceComponentInstances) {
		m_subSurfaceComponentInstances.swap(theProject().m_subSurfaceComponentInstances);
	}

	theProject().updatePointers();

	// tell project that geometry has changed
	// NOTE: this may be slow for larger geometries...
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged );
}


void SVUndoModifySurfaceGeometry::redo() {
	undo();
}

