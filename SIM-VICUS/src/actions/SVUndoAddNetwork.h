#ifndef SVUndoAddNetworkH
#define SVUndoAddNetworkH

#include <VICUS_Network.h>

#include "SVUndoCommandBase.h"

class SVUndoAddNetwork : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddNetwork)
public:
	SVUndoAddNetwork(const QString & label, const VICUS::Network & addedNetwork);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added network. */
	VICUS::Network	m_addedNetwork;

	double			m_gridWidth;
	double			m_gridSpacing;
	double			m_farDistance;
};


class SVUndoAddToExistingNetwork : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddToExistingNetwork)
public:
	SVUndoAddToExistingNetwork(const QString & label, const VICUS::Network & modNetwork);

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

#endif // SVUndoAddNetworkH
