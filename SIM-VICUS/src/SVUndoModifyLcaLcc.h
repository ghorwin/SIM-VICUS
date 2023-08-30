#ifndef SVUNDOMODIFYLCALCC_H
#define SVUNDOMODIFYLCALCC_H

#include "SVUndoCommandBase.h"

#include <VICUS_LcaSettings.h>
#include <VICUS_LccSettings.h>

/*! Modification of component instances (associations between surfaces and components). */
class SVUndoModifyLcaLcc : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyComponentInstances)
public:
	SVUndoModifyLcaLcc(const QString & label, const VICUS::LcaSettings &lca, const VICUS::LccSettings lcc);

	virtual void undo();
	virtual void redo();

private:

	VICUS::LcaSettings		m_lca;

	VICUS::LccSettings		m_lcc;
};


#endif // SVUNDOMODIFYLCALCC_H
