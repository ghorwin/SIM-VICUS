#include "SVUndoModifySurfaceGeometry.h"
#include "SVProjectHandler.h"

SVUndoModifySurfaceGeometry::SVUndoModifySurfaceGeometry(const QString & label,
														 const std::vector<VICUS::Surface> & surfaces)
	: m_surfaces(surfaces)
{
	setText( label );
}


void SVUndoModifySurfaceGeometry::undo() {

	std::vector<const VICUS::Surface*> surfacesProject;

	// since selection change is also an undo property, we can rely on having the same selection
	// here as when the surface properties were modified
	theProject().selectedSurfaces(surfacesProject, VICUS::Project::SG_All);

	for (const VICUS::Surface *sOld : surfacesProject ) {
		for ( VICUS::Surface &sNew : m_surfaces ) {
			if ( sOld->m_id == sNew.m_id )
				std::swap(const_cast<VICUS::Surface *>(sOld)->m_geometry, sNew.m_geometry);
		}
	}


	// tell project that geometry has changed
	// NOTE: this may be slow for larger geometries...
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged );
}


void SVUndoModifySurfaceGeometry::redo() {
	undo();
}

