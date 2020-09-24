#ifndef SVUndoCommandBaseH
#define SVUndoCommandBaseH

#include <QUndoCommand>
#include <QCoreApplication>

#include <VICUS_Project.h>
#include "SVProjectHandler.h"

/*! Abstract base class for all undo commands.
	It provides the member function push() which puts the command to the global stack.
	Also, via theProject() read/write access to the project data is granted.
*/
class SVUndoCommandBase : public QUndoCommand {
	Q_DECLARE_TR_FUNCTIONS(SVUndoCommands)
public:
	/*! Pushes the command to the global undo-stack (in the main window). */
	void push();

protected:
	/*! Returns a read/write reference to the project data. */
	VICUS::Project &	theProject() const;

};

#endif // SVUndoCommandBaseH
