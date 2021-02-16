#ifndef SVUndoModifySiteDataH
#define SVUndoModifySiteDataH

#include <VICUS_Project.h>

#include "SVUndoCommandBase.h"

class SVUndoModifySiteData : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySiteData)
public:
	SVUndoModifySiteData(const QString & label,
				   double gridWidth, double gridSpacing, double farDistance);

	virtual void undo();
	virtual void redo();

private:

	double 	m_gridWidth;
	double 	m_gridSpacing;
	double 	m_farDistance;

};

#endif // SVUndoModifySiteDataH
