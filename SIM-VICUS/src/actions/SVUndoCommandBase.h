/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVUndoCommandBaseH
#define SVUndoCommandBaseH

#include <QUndoCommand>
#include <QCoreApplication>

namespace VICUS {
	class Project;
}

/*! Abstract base class for all modification data containers. */
class ModificationInfo {
public:
	virtual ~ModificationInfo();
};

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
