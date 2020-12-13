#ifndef SVUndoModifySurfaceGeometryH
#define SVUndoModifySurfaceGeometryH

#include <VICUS_Surface.h>

#include "SVUndoCommandBase.h"

/*! Action for adding a new surface, either to a room, or as anonymous surface object. */
class SVUndoModifySurfaceGeometry : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySurfaceGeometry)
public:
	SVUndoModifySurfaceGeometry(const QString & label, std::vector<VICUS::Surface*> surfaces);

	virtual void undo();
	virtual void redo();

private:

	/*! Vector with plain (dumb) geometry. */
	std::vector<VICUS::Surface>					m_surfaces;			// XML:E
};


#endif // SVUndoModifySurfaceGeometryH
