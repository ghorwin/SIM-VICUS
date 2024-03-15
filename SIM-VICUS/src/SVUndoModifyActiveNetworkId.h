#ifndef SVUNDOMODIFYACTIVENETWORKID_H
#define SVUNDOMODIFYACTIVENETWORKID_H

#include "SVUndoCommandBase.h"

#include "VICUS_Constants.h"

class SVUndoModifyActiveNetworkId: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyActiveNetworkId)
public:
	SVUndoModifyActiveNetworkId(unsigned int newId);

	virtual void undo();
	virtual void redo();

private:
	unsigned int m_activeNetworkId = VICUS::INVALID_ID;
};

#endif // SVUNDOMODIFYACTIVENETWORKID_H

