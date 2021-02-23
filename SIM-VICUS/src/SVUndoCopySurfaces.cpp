#include "SVUndoCopySurfaces.h"


SVUndoCopySurfaces::SVUndoCopySurfaces(const QString &label, const std::vector<VICUS::Surface> &copiedSurfaces, unsigned int parentNodeID,
									   const std::vector<VICUS::ComponentInstance> * compInstances) :
	m_copiedSurfaces(copiedSurfaces),
	m_parentNodeID(parentNodeID)
{
	setText( label );
	if (!compInstances->empty())
		m_componentInstances = *compInstances;
}

void SVUndoCopySurfaces::undo()
{

}

void SVUndoCopySurfaces::redo()
{

}
