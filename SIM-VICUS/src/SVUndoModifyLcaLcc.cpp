#include "SVUndoModifyLcaLcc.h"

#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoModifyLcaLcc::SVUndoModifyLcaLcc(const QString & label, const VICUS::LcaSettings &lca, const VICUS::LccSettings lcc):
	m_lca(lca),
	m_lcc(lcc)
{
	setText(label);
}

void SVUndoModifyLcaLcc::undo() {
	// exchange Project data
	std::swap( theProject().m_lcaSettings, m_lca);
	std::swap( theProject().m_lccSettings, m_lcc);

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified(SVProjectHandler::LcaLccModified);
}

void SVUndoModifyLcaLcc::redo() {
	undo();
}
