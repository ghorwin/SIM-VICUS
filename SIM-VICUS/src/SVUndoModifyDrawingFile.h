#ifndef SVUNDOMODIFYDRAWINGFILEH
#define SVUNDOMODIFYDRAWINGFILEH


#include "SVUndoCommandBase.h"

#include <IBK_Path.h>


class SVUndoModifyDrawingFile : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyDrawingFile)
public:
	SVUndoModifyDrawingFile(const QString & label, const IBK::Path &drawingFile);

	virtual void undo();
	virtual void redo();

private:
	IBK::Path					m_drawingFile;

};


#endif // SVUNDOMODIFYDRAWINGFILEH
