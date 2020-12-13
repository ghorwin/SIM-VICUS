#include "SVUndoModifySurfaceGeometry.h"
#include "SVProjectHandler.h"

SVUndoModifySurfaceGeometry::SVUndoModifySurfaceGeometry(const QString & label, /*const std::vector<VICUS::Building> & b,
														 const std::vector<VICUS::Surface> & s*/ std::vector<VICUS::Surface*> surfaces) /*:*/
//	m_buildings(b),
//	m_surfaces(surfaces)
{
	for (VICUS::Surface *s : surfaces) {
		m_surfaces.push_back(*s);
	}

	setText( label );

}


void SVUndoModifySurfaceGeometry::undo() {

	std::vector<VICUS::Surface*> surfacesProject;

	theProject().selectedSurfaces(surfacesProject);

	for ( VICUS::Surface *sOld : surfacesProject ) {
		for ( VICUS::Surface &sNew : m_surfaces ) {
			if ( sOld->m_id == sNew.m_id )
				std::swap(sOld->m_geometry, sNew.m_geometry);
		}
	}

	// remove last network
//	for (std::map< VICUS::Surface*, std::vector<IBKMK::Vector3D> >::const_iterator itSurfPointerToVertexes = m_surfaceMap.begin();
//		 itSurfPointerToVertexes != m_surfaceMap.end(); ++itSurfPointerToVertexes ) {
//		std::swap( itSurfPointerToVertexes->first->m_geometry.vertexes(), itSurfPointerToVertexes->second )

//	}

//	std::swap( theProject().m_buildings, m_buildings );
//	std::swap( theProject().m_plainGeometry, m_surfaces );


	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::GeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::SelectionModified);
}


void SVUndoModifySurfaceGeometry::redo() {

	undo();
}

