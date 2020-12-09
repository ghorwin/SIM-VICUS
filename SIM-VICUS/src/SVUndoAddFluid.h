#ifndef SVUNDOADDFLUID_H
#define SVUNDOADDFLUID_H

#include "SVUndoCommandBase.h"


class SVUndoAddFluid: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddFluid)
public:
	SVUndoAddFluid(const QString & label, const VICUS::NetworkFluid & fluid);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added network. */
	VICUS::NetworkFluid	m_fluid;

};

#endif // SVUNDOADDFLUID_H
