#ifndef SVUNDOMODIFYEXISTINGNETWORK_H
#define SVUNDOMODIFYEXISTINGNETWORK_H

#include <VICUS_Network.h>

#include "SVUndoCommandBase.h"

class SVUndoModifyExistingNetwork : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddToExistingNetwork)
public:
	SVUndoModifyExistingNetwork(const QString & label, const VICUS::Network & modNetwork);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added network. */
	VICUS::Network	m_oldNetwork;
	VICUS::Network	m_newNetwork;

	double			m_gridWidth;
	double			m_gridSpacing;
	double			m_farDistance;
};

#endif // SVUNDOMODIFYEXISTINGNETWORK_H
