#ifndef SVUNDOMODIFYNETWORKHYDRAULICCOMPONENT_H
#define SVUNDOMODIFYNETWORKHYDRAULICCOMPONENT_H

#include "SVUndoCommandBase.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

class SVUndoModifyNetworkHydraulicComponent: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyNetworkHydraulicComponent)

public:
	SVUndoModifyNetworkHydraulicComponent(const QString & label, const unsigned int networkId,
										const NANDRAD::HydraulicNetworkComponent &comp);

	virtual void undo();
	virtual void redo();

private:
	NANDRAD::HydraulicNetworkComponent m_newComponent;
	NANDRAD::HydraulicNetworkComponent m_oldComponent;
	unsigned int m_networkId;
};

#endif // SVUNDOMODIFYNETWORKHYDRAULICCOMPONENT_H
