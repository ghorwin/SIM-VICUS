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

#ifndef SVUndoCopyBuildingGeometryH
#define SVUndoCopyBuildingGeometryH

#include <VICUS_Building.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Undo action for copying buildings, building levels, rooms, surfaces and or subsurfaces.
	Different geometrical objects may be copied together.
	The undo-action is actually very generic - it modifies the entire vector of buildings and
	all compoment and subsurface component instances.

	To simplify generation of the undo-action data, there are several static convience functions that assist in
	creating the undo-action data.
*/
class SVUndoCopyBuildingGeometry : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoCopyZone)
public:
	/*! Constructor, takes modified data to switch with project. */
	SVUndoCopyBuildingGeometry(const QString & label,
							 const std::vector<VICUS::Building> & modifiedBuilding,
							 const std::vector<VICUS::ComponentInstance> & modifiedComponentInstances,
							 const std::vector<VICUS::SubSurfaceComponentInstance> & modifiedSubSurfaceComponentInstances);

	/*! Convenience function to generate the undo-action for copying of subsurfaces.
		Function takes list of selected objects and a translation vector and generates the respective
		undo action by pulling all data from project().
	*/
	static SVUndoCopyBuildingGeometry * createUndoCopySubSurfaces(
			const std::vector<const VICUS::SubSurface *> & selectedSubSurfaces,
			const IBKMK::Vector3D & translation);

	/*! Convenience function to generate the undo-action for copying of surfaces.
		Function takes list of selected objects and a translation vector and generates the respective
		undo action by pulling all data from project().
	*/
	static SVUndoCopyBuildingGeometry * createUndoCopySurfaces(
			const std::vector<const VICUS::Surface *> & selectedSurfaces,
			const IBKMK::Vector3D & translation);

	/*! Convenience function to generate the undo-action for copying of rooms.
		Function takes list of selected objects and a translation vector and generates the respective
		undo action by pulling all data from project().
	*/
	static SVUndoCopyBuildingGeometry * createUndoCopyRooms(
			const std::vector<const VICUS::Room *> & selectedRooms,
			const IBKMK::Vector3D & translation);

	/*! Convenience function to generate the undo-action for copying of building levels.
		Function takes list of selected objects and a translation vector and generates the respective
		undo action by pulling all data from project().
	*/
	static SVUndoCopyBuildingGeometry * createUndoCopyBuildingLevels(
			const std::vector<const VICUS::BuildingLevel *> & selectedBuildingLevels,
			const IBKMK::Vector3D & translation);

	/*! Convenience function to generate the undo-action for copying of buildings.
		Function takes list of selected objects and a translation vector and generates the respective
		undo action by pulling all data from project().
	*/
	static SVUndoCopyBuildingGeometry * createUndoCopyBuildings(
			const std::vector<const VICUS::Building *> & selectedBuildings,
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


#endif // SVUndoCopyBuildingGeometryH
