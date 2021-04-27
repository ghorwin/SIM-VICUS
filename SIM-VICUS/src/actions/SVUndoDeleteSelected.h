#ifndef SVUndoDeleteSelectedH
#define SVUndoDeleteSelectedH

#include <VICUS_Project.h>

#include "SVUndoCommandBase.h"

/*! An undo action for deleting selected geometric shapes (things that are shown in the scene). */
class SVUndoDeleteSelected : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteSelected)
public:
	SVUndoDeleteSelected(const QString & label, const std::vector<unsigned int> & objectIDsToBeRemoved);

	virtual void undo();
	virtual void redo();

private:

	/*! Stores the unique IDs of all selected items. */
	const std::set<unsigned int>						m_selectedIDs;

	/*! Stores vector of component instances. */
	std::vector<VICUS::ComponentInstance>				m_compInstances;
	/*! Stores vector of buildings. */
	std::vector<VICUS::Building>						m_buildings;
	/*! Stores vector of anonymous surfaces. */
	std::vector<VICUS::Surface>							m_plainGeometry;

	// TODO Hauke : add network nodes/edges here
};


#endif // SVUndoDeleteSelectedH
