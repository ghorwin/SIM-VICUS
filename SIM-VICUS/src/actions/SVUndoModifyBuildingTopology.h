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

#ifndef SVUndoModifyBuildingTopologyH
#define SVUndoModifyBuildingTopologyH

#include <VICUS_Building.h>
#include <vector>

#include "SVUndoCommandBase.h"

/*! Modification of the building topology, i.e. building levels or rooms are moved around (but not deleted/added).
	Notification type BuildingTopologyChanged is used.
*/
class SVUndoModifyBuildingTopology : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifyBuildingTopology)
public:
	/*! Replaces building entity at given index in buildings vector. */
	SVUndoModifyBuildingTopology(const QString & label, const std::vector<VICUS::Building> & buildings,
								 const std::vector<VICUS::ComponentInstance> *surfaceComponentInstances = nullptr,
								 const std::vector<VICUS::SubSurfaceComponentInstance> *subSurfaceComponentInstances = nullptr);

	virtual void undo();
	virtual void redo();

private:
	/*! Data member to hold modified buildings vector. */
	std::vector<VICUS::Building> m_buildings;

	/*! Copies of modified surface component instances. */
	std::vector<VICUS::ComponentInstance>			m_surfaceComponentInstances;

	/*! Copies of modified sub-surface component instances. */
	std::vector<VICUS::SubSurfaceComponentInstance> m_subSurfaceComponentInstances;

	bool                                            m_modifySurfaceComponentInstances = false;

	bool                                            m_modifySubSurfaceComponentInstances = false;
};

#endif // SVUndoModifyBuildingTopologyH
