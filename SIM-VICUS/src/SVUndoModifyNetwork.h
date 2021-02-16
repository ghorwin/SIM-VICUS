#ifndef SVUndoModifyNetworkH
#define SVUndoModifyNetworkH

#include <VICUS_Network.h>

#include "SVUndoCommandBase.h"

class SVUndoModifyNetwork : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyNetwork)
public:
	SVUndoModifyNetwork(const QString & label, unsigned int networkIndex, const VICUS::Network & modNetwork);

	virtual void undo();
	virtual void redo();

private:

	/*! Index of modified network. */
	unsigned int m_networkIndex;
	/*! Cache for added network. */
	VICUS::Network	m_network;
};

#endif // SVUndoModifyNetworkH
