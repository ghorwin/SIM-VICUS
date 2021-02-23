#ifndef SVUndoAddSurfaceH
#define SVUndoCopySurfaceH

#include <VICUS_Surface.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Action for copying a surface, either to a room, or as anonymous surface object. */
class SVUndoCopySurfaces : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoCopySurfaces)
public:
	/*! Constructor, allowing different ways for copying a surface:
		1. annonymous surface (actually just a polygon) without associated room or component.
		2. surface belonging to a room
		3. surface belonging to a room, and getting a component instance association.

		\param compInstance If not nullptr, the component instance is being added to the project. No ownership transfer!
	*/
	SVUndoCopySurfaces(const QString & label, const std::vector<VICUS::Surface> & copiedSurfaces,
						unsigned int parentNodeID = 0,
						const std::vector<VICUS::ComponentInstance> * compInstances = nullptr);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for copied surface. */
	std::vector<VICUS::Surface>				m_copiedSurfaces;
	/*! Parent room (if any) that this surface belongs to. */
	unsigned int							m_parentNodeID = 0;
	/*! Optionally copied component instances. */
	std::vector<VICUS::ComponentInstance>	m_componentInstances;
};


#endif // SVUndoCopySurfaceH
