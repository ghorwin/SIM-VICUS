#include "SVUndoCopySurfaces.h"
#include "SVProjectHandler.h"


SVUndoCopySurfaces::SVUndoCopySurfaces(const QString &label, const std::vector<VICUS::Surface> &copiedSurfaces,
									   const std::set<unsigned int> & deselectedSurfaceUniqueIDs,
									   const std::vector<VICUS::ComponentInstance> & newCompInstances) :
	m_copiedSurfaces(copiedSurfaces),
	m_deselectedSurfaceUniqueIDs(deselectedSurfaceUniqueIDs),
	m_newComponentInstances(newCompInstances)
{
	setText( label );
}


void SVUndoCopySurfaces::undo() {
	// append copied surfaces
	for (const VICUS::Surface &s : m_copiedSurfaces ) {
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(s.m_parent);
		if (r == nullptr)
			theProject().m_plainGeometry.pop_back();
		else
			const_cast<VICUS::Room *>(r)->m_surfaces.pop_back();
	}

	// re-select surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces)
					if (m_deselectedSurfaceUniqueIDs.find(s.uniqueID()) != m_deselectedSurfaceUniqueIDs.end())
						s.m_selected = true;

	// remove appended component instances (if any)
	Q_ASSERT(theProject().m_componentInstances.size() >= m_newComponentInstances.size());
	theProject().m_componentInstances.resize(theProject().m_componentInstances.size() - m_newComponentInstances.size());

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoCopySurfaces::redo() {
	// remove appended surfaces
	for (const VICUS::Surface &s : m_copiedSurfaces ) {
		const VICUS::Room *r = dynamic_cast<const VICUS::Room *>(s.m_parent);
		if (r == nullptr)
			theProject().m_plainGeometry.push_back(s);
		else
			const_cast<VICUS::Room *>(r)->m_surfaces.push_back(s);
	}

	// deselect surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces)
					if (m_deselectedSurfaceUniqueIDs.find(s.uniqueID()) != m_deselectedSurfaceUniqueIDs.end())
						s.m_selected = false;


	// append component instances (if vector is empty, nothing happens here)
	theProject().m_componentInstances.insert(theProject().m_componentInstances.end(),
										 m_newComponentInstances.begin(), m_newComponentInstances.end());
	theProject().updatePointers();

	// tell project that the building geometry has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}
