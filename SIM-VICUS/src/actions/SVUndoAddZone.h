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

#ifndef SVUndoAddZoneH
#define SVUndoAddZoneH

#include <VICUS_BuildingLevel.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Undo action for adding a new or copied zone/room to an existing building level. */
class SVUndoAddZone : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoAddZone)
public:
	SVUndoAddZone(const QString & label, unsigned int buildingLevelUUID,
				  const VICUS::Room & addedRoom, bool topologyOnly,
				  const std::vector<VICUS::ComponentInstance> * componentInstances = nullptr);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added item. */
	VICUS::Room				m_addedRoom;

	/*! If true, the change event sent is BuildingTopologyChanged, otherwise BuildingGeometryChanged. */
	bool					m_topologyOnly;

	unsigned int			m_buildingLevelID;

	/*! If not empty, this vector contains component instances that are created alongside the room's surfaces. */
	std::vector<VICUS::ComponentInstance>	m_componentInstances;
};


#endif // SVUndoAddZoneH
