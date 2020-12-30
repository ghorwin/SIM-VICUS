#ifndef SVUNDODELETENETWORKHYDRAULICCOMPONENT_H
#define SVUNDODELETENETWORKHYDRAULICCOMPONENT_H

#include "SVUndoCommandBase.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

class SVUndoDeleteNetworkHydraulicComponent: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteNetworkHydraulicComponent)

public:
	SVUndoDeleteNetworkHydraulicComponent(const QString & label, const unsigned int networkId,
										const NANDRAD::HydraulicNetworkComponent & comp);

	virtual void undo();
	virtual void redo();

private:
	NANDRAD::HydraulicNetworkComponent m_component;
	unsigned int m_networkId;
};

#endif // SVUNDODELETENETWORKHYDRAULICCOMPONENT_H
