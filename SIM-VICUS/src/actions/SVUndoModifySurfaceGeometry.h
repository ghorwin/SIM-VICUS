#ifndef SVUndoModifySurfaceGeometryH
#define SVUndoModifySurfaceGeometryH

#include <VICUS_Surface.h>

#include "SVUndoCommandBase.h"

/*! Action for modifying a surface. */
class SVUndoModifySurfaceGeometry : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySurfaceGeometry)
public:
	SVUndoModifySurfaceGeometry(const QString & label, const std::vector<VICUS::Surface> & surfaces);

	virtual void undo();
	virtual void redo();

private:

	/*! Object copies of modified surfaces. */
	std::vector<VICUS::Surface>					m_surfaces;
};


#endif // SVUndoModifySurfaceGeometryH
