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

#ifndef SVUndoCopyBuildingLevelsH
#define SVUndoCopyBuildingLevelsH

#include <VICUS_Building.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Undo action for copying building levels entirely.
	Different building levels of different buildings may be copied together.
*/
class SVUndoCopyBuildingLevels : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoCopyZone)
public:
	/*! Constructor, takes modified data to switch with project. */
	SVUndoCopyBuildingLevels(const QString & label,
							 const std::vector<VICUS::Building> & modifiedBuilding,
							 const std::vector<VICUS::ComponentInstance> & modifiedComponentInstances,
							 const std::vector<VICUS::SubSurfaceComponentInstance> & modifiedSubSurfaceComponentInstances);

	/*! Convenience function to generate the undo-action.
		Function takes list of building levels and a translation vector and generates the respective
		undo action by pulling all data from project().
	*/
	static SVUndoCopyBuildingLevels * createUndoCopyBuildingLevels(
			const std::vector<const VICUS::BuildingLevel *> & selectedBuildingLevels,
			const IBKMK::Vector3D & translation);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for added item. */
	std::vector<VICUS::Building>					m_modifiedBuilding;

	/*! If not empty, this vector contains component instances that are created alongside the room's surfaces. */
	std::vector<VICUS::ComponentInstance>			m_modifiedComponentInstances;

	/*! If not empty, this vector contains sub surface component instances that are created alongside the room's surfaces. */
	std::vector<VICUS::SubSurfaceComponentInstance>	m_modifiedSubSurfaceComponentInstances;
};


#endif // SVUndoCopyBuildingLevelsH
