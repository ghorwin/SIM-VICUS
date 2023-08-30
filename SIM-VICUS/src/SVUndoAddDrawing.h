#ifndef SVUNDOADDDRAWINGH
#define SVUNDOADDDRAWINGH

#include "SVUndoCommandBase.h"

#include <VICUS_Drawing.h>


class SVUndoAddDrawing: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddNetwork)

public:
	SVUndoAddDrawing(const QString & label, const VICUS::Drawing & addedDrawing);

	virtual void undo();
	virtual void redo();

private:

	VICUS::Drawing		m_addedDrawing;

	double				m_farDistance;
	double				m_gridWidth;
	double				m_gridSpacing;
};



#endif // SVUNDOADDDRAWINGH
