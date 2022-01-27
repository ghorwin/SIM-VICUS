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

#ifndef SVUndoModifyObjectNameH
#define SVUndoModifyObjectNameH

#include <vector>

#include "SVUndoCommandBase.h"

namespace VICUS {
	class Object;
}

/*! Modification of the display name of an object. */
class SVUndoModifyObjectName : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyObjectName)
public:

	/*! The modification data object passed along with the undo action. */
	class Data : public ModificationInfo {
	public:
		const VICUS::Object * m_object = nullptr;
	};

	/*! Replaces building entity at given index in buildings vector. */
	SVUndoModifyObjectName(const QString & label, const VICUS::Object * o, const QString & newName);

	virtual void undo();
	virtual void redo();

private:
	unsigned int	m_objectID;
	QString			m_displayName;
};

#endif // SVUndoModifyObjectNameH
