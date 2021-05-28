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

#ifndef SVUndoDeleteBuildingLevelH
#define SVUndoDeleteBuildingLevelH

#include "SVUndoCommandBase.h"

#include <VICUS_BuildingLevel.h>

class SVUndoDeleteBuildingLevel: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteBuildingLevel)
public:
	SVUndoDeleteBuildingLevel(const QString & label, unsigned int buildingIndex, unsigned int buildingLevelIndex);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for deleted building level. */
	VICUS::BuildingLevel	m_deletedLevel;

	/*! Index of building in project's building vector to be modified. */
	unsigned int	m_buildingIndex;

	/*! Index of level to be removed. */
	unsigned int	m_levelIndex;
};

#endif // SVUndoDeleteBuildingLevelH
