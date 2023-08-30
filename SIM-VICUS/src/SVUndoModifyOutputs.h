#ifndef SVUNDOMODIFYOUTPUTSH
#define SVUNDOMODIFYOUTPUTSH

#include "SVUndoCommandBase.h"

#include <VICUS_Outputs.h>

class SVUndoModifyOutputs : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySolverParams)
public:
	SVUndoModifyOutputs(const QString & label, const VICUS::Outputs &outputs);

	virtual void undo();
	virtual void redo();

private:
	VICUS::Outputs			m_outputs;

};


#endif // SVUNDOMODIFYOUTPUTSH
