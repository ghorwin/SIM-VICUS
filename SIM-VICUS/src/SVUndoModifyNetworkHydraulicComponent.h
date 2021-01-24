#ifndef SVUndoModifyNetworkHydraulicComponentH
#define SVUndoModifyNetworkHydraulicComponentH

#include "SVUndoCommandBase.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

/*! An undo action for modifying an existing network component definition. */
class SVUndoModifyNetworkHydraulicComponent: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyNetworkHydraulicComponent)
public:
	/*! Constructor, both networkID and componentIndex must be valid.
		Note: component is identified via index, since comp may contain a modified id.
	*/
	SVUndoModifyNetworkHydraulicComponent(const QString & label,
										  const unsigned int networkId,
										  const unsigned int componentIndex,
										  const NANDRAD::HydraulicNetworkComponent &comp);

	virtual void undo();
	virtual void redo();

private:
	/*! Component cache. */
	NANDRAD::HydraulicNetworkComponent m_component;
	/*! ID of network containing the component definition. */
	unsigned int m_networkId;
	/*! Index of modified component within m_hydraulicComponents vector. */
	unsigned int m_componentIndex;
};

#endif // SVUndoModifyNetworkHydraulicComponentH
