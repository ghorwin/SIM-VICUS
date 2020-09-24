#ifndef SVUndoProjectH
#define SVUndoProjectH

#include <VICUS_Project.h>

#include "SVUndoCommandBase.h"

class SVUndoProject : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoProject)
public:
	SVUndoProject(const QString & label,
				   const VICUS::Project & newProject
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for entire project data (this might be large!). */
	VICUS::Project	m_project;

};

#endif // SVUndoProjectH
