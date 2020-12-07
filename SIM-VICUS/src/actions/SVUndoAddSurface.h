#ifndef SVUndoAddSurfaceH
#define SVUndoAddSurfaceH

#include <VICUS_Surface.h>

#include "SVUndoCommandBase.h"

/*! Action for adding a new surface, either to a room, or as anonymous surface object. */
class SVUndoAddSurface : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddSurface)
public:
	SVUndoAddSurface(const QString & label, const VICUS::Surface & addedSurface, unsigned int parentNodeID = 0);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added surface. */
	VICUS::Surface			m_addedSurface;
	/*! Parent room (if any) that this surface belongs to. */
	unsigned int			m_parentNodeID = 0;
};


#endif // SVUndoAddSurfaceH
