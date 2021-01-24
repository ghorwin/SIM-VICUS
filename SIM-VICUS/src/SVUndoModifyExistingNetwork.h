#ifndef SVUndoModifyExistingNetworkH
#define SVUndoModifyExistingNetworkH

#include <VICUS_Network.h>

#include "SVUndoCommandBase.h"

class SVUndoModifyExistingNetwork : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyExistingNetwork)
public:
	SVUndoModifyExistingNetwork(const QString & label, unsigned int networkIndex, const VICUS::Network & modNetwork);

	virtual void undo();
	virtual void redo();

private:

	/*! Index of modified network. */
	unsigned int m_networkIndex;
	/*! Cache for added network. */
	VICUS::Network	m_network;
};

#endif // SVUndoModifyExistingNetworkH
