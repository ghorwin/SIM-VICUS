#ifndef SVUndoModifyComponentInstancesH
#define SVUndoModifyComponentInstancesH

#include <VICUS_ComponentInstance.h>
#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of component instances (associations between surfaces and components). */
class SVUndoModifyComponentInstances : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyComponentInstances)
public:
	SVUndoModifyComponentInstances(const QString & label, const std::vector<VICUS::ComponentInstance> & ci);

	virtual void undo();
	virtual void redo();

private:

	/*! Stored vector of new component instances. */
	std::vector<VICUS::ComponentInstance> m_componentInstances;
};

#endif // SVUndoModifyComponentInstancesH
