#include "SVUndoCommandBase.h"

#include "SVMainWindow.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"

void SVUndoCommandBase::push() {
	SVMainWindow::addUndoCommand(this);
}

VICUS::Project & SVUndoCommandBase::theProject() const {
	return const_cast<VICUS::Project &>( project() );
}
