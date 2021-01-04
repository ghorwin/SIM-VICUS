#ifndef SVUndoAddSurfaceH
#define SVUndoAddSurfaceH

#include <VICUS_Surface.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Action for adding a new surface, either to a room, or as anonymous surface object. */
class SVUndoAddSurface : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddSurface)
public:
	/*! Constructor, allowing different ways for adding a surface:
		1. annonymous surface (actually just a polygon) without associated room or component.
		2. surface belonging to a room
		3. surface belonging to a room, and getting a component instance association.

		\param compInstance If not nullptr, the component instance is being added to the project. No ownership transfer!
	*/
	SVUndoAddSurface(const QString & label, const VICUS::Surface & addedSurface,
					 unsigned int parentNodeID = 0,
					 const VICUS::ComponentInstance * compInstance = nullptr);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added surface. */
	VICUS::Surface			m_addedSurface;
	/*! Parent room (if any) that this surface belongs to. */
	unsigned int			m_parentNodeID = 0;
	/*! Optionally added component instance. */
	VICUS::ComponentInstance	m_componentInstance;
};


#endif // SVUndoAddSurfaceH
