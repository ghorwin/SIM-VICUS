#include "SVUndoCopySurfaces.h"


SVUndoCopySurfaces::SVUndoCopySurfaces(const QString &label, const std::vector<VICUS::Surface> &copiedSurfaces, unsigned int parentNodeID,
									   const std::vector<VICUS::ComponentInstance> * compInstances) :
	m_copiedSurfaces(copiedSurfaces),
	m_parentNodeID(parentNodeID)
{
	setText( label );
//	if (!compInstances->empty())
//		m_componentInstances = *compInstances;
}

void SVUndoCopySurfaces::undo() {
	// append copied surfaces
	for (const VICUS::Surface &s : m_copiedSurfaces ) {
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(s.m_parent);
		// lookup modified building level
		Q_ASSERT(r != nullptr);

		// remove last building level
		Q_ASSERT(!r->m_surfaces.empty());
		const_cast<VICUS::Room *>(r)->m_surfaces.pop_back();
	}

	// remove appended component instances (if any)
//	Q_ASSERT(theProject().m_componentInstances.size() >= m_componentInstances.size());
//	theProject().m_componentInstances.resize(theProject().m_componentInstances.size() - m_componentInstances.size());

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}

void SVUndoCopySurfaces::redo() {
	// remove appended surfaces
	for (const VICUS::Surface &s : m_copiedSurfaces ) {
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(s.m_parent);
		// lookup modified building level
		Q_ASSERT(r != nullptr);

		// remove last building level
		Q_ASSERT(!r->m_surfaces.empty());
		const_cast<VICUS::Room *>(r)->m_surfaces.push_back(s);
	}

	// append component instances (if vector is empty, nothing happens here)
//	theProject().m_componentInstances.insert(theProject().m_componentInstances.end(), m_componentInstances.begin(), m_componentInstances.end());
	theProject().updatePointers();

	// tell project that the building geometry has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}
