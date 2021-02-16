#ifndef SVUndoDeleteNetworkH
#define SVUndoDeleteNetworkH

#include "SVUndoCommandBase.h"

#include <VICUS_Network.h>

class SVUndoDeleteNetwork: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteNetwork)
public:
	SVUndoDeleteNetwork(const QString & label, unsigned int networkIndex);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for deleted network. */
	VICUS::Network	m_deletedNetwork;

	/*! Index of network in project's network vector to be removed. */
	unsigned int	m_networkIndex;
};

#endif // SVUndoDeleteNetworkH
