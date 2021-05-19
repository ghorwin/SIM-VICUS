#ifndef SVUndoModifySubSurfaceComponentInstancesH
#define SVUndoModifySubSurfaceComponentInstancesH

#include <VICUS_SubSurfaceComponentInstance.h>
#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of component instances (associations between surfaces and components). */
class SVUndoModifySubSurfaceComponentInstances : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySubSurfaceComponentInstances)
public:
	SVUndoModifySubSurfaceComponentInstances(const QString & label, const std::vector<VICUS::SubSurfaceComponentInstance> & ci);

	virtual void undo();
	virtual void redo();

private:

	/*! Stored vector of new component instances. */
	std::vector<VICUS::SubSurfaceComponentInstance> m_componentInstances;
};

#endif // SVUndoModifySubSurfaceComponentInstancesH
