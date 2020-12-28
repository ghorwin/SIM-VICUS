#ifndef SVUndoDeleteSelectedH
#define SVUndoDeleteSelectedH

#include <VICUS_Project.h>

#include "SVUndoCommandBase.h"

/*! An undo action for deleting selected geometric shapes (things that are shown in the scene).
	Currently, these are surfaces, anonymouse planes, edges, network nodes.

	This UNDO action is a little tricky, since we identify removed surfaces by their unique ID.
	Hence, when we call undo(), we must ensure that the re-added objects will get the same IDs back.
	This is done by creating copies of the objects to be removed and storing them in lists.
	Then, during undo we re-add those objects, hereby keeping their unique IDs.
*/
class SVUndoDeleteSelected : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteSelected)
public:
	SVUndoDeleteSelected(const QString & label, const std::vector<unsigned int> & selectedIDs);

	virtual void undo();
	virtual void redo();

private:
	struct RemovedSurfaceInfo {
		RemovedSurfaceInfo() {}
		RemovedSurfaceInfo(unsigned int	parentRoomId, unsigned int insertIdx, const VICUS::Surface & surface) :
			m_parentRoomId(parentRoomId), m_insertIdx(insertIdx), m_surface(surface)
		{}
		/*! The unique ID of the containing room object */
		unsigned int	m_parentRoomId;
		/*! The index of the object when inserted in the original vector. */
		unsigned int	m_insertIdx;
		/*! The cached object. */
		VICUS::Surface	m_surface;
	};

	/*! Stores the unique IDs of all selected items. */
	const std::vector<unsigned int> m_selectedIDs;

	/*! Vector with planar geometry. */
	std::vector<RemovedSurfaceInfo>						m_roomGeometry;
	/*! Vector with planar anonymous geometry. */
	std::vector<RemovedSurfaceInfo>						m_plainGeometry;
};


#endif // SVUndoDeleteSelectedH
