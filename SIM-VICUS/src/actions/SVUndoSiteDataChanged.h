#ifndef SVUndoSiteDataChangedH
#define SVUndoSiteDataChangedH

#include <VICUS_Project.h>

#include "SVUndoCommandBase.h"

class SVUndoSiteDataChanged : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoSiteDataChanged)
public:
	SVUndoSiteDataChanged(const QString & label,
				   double gridWidth, double gridSpacing, double farDistance);

	virtual void undo();
	virtual void redo();

private:

	double 	m_gridWidth;
	double 	m_gridSpacing;
	double 	m_farDistance;

};

#endif // SVUndoSiteDataChangedH
