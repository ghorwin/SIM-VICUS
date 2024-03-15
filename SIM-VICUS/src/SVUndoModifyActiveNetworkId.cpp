#include "SVUndoModifyActiveNetworkId.h"

#include "SVProjectHandler.h"

#include <VICUS_Project.h>


SVUndoModifyActiveNetworkId::SVUndoModifyActiveNetworkId(unsigned int newId):
	m_activeNetworkId(newId)
{

}

void SVUndoModifyActiveNetworkId::undo() {
	redo();
}

void SVUndoModifyActiveNetworkId::redo() {
	std::swap(theProject().m_activeNetworkId, m_activeNetworkId);
	// tell project that the network has changed
	SVProjectHandler::instance().setModified(SVProjectHandler::NetworkGeometryChanged);
}
