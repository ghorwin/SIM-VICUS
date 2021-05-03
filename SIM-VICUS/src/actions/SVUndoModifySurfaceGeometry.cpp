#include "SVUndoModifySurfaceGeometry.h"
#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"

#include "Vic3DWireFrameObject.h"

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
			if ( sOld->uniqueID() == sNew.uniqueID() ) {
				// we swap the surface's polygon and the subsurfaces polygons
				// and then update the geometry object
				VICUS::Surface * oldS = const_cast<VICUS::Surface *>(sOld);

				// TODO : This is slow! Not only polygon memory is copied around, but also the triangulation
				//        is done with every setXXX call, so 4 times in total. While this code is safe and ensures
				//        consistency, it might scale badly for large geometries.
				VICUS::Polygon3D oldPolygon = oldS->polygon3D();
				oldS->setPolygon3D(sNew.polygon3D());
				sNew.setPolygon3D(oldPolygon);
				std::vector<VICUS::SubSurface> oldSubSurfaces = oldS->subSurfaces();
				oldS->setSubSurfaces(sNew.subSurfaces());
				sNew.setSubSurfaces(oldSubSurfaces);
				break;
			}
		}
	}

	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();

	// tell project that geometry has changed
	// NOTE: this may be slow for larger geometries...
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged );
}


void SVUndoModifySurfaceGeometry::redo() {
	undo();
}

