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

#include "SVUndoCopyBuildingGeometry.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoCopyBuildingGeometry::SVUndoCopyBuildingGeometry(const QString & label,
												   const std::vector<VICUS::Building> & modifiedBuilding,
												   const std::vector<VICUS::ComponentInstance> & modifiedComponentInstances,
												   const std::vector<VICUS::SubSurfaceComponentInstance> & modifiedSubSurfaceComponentInstances) :
	m_modifiedBuilding(modifiedBuilding),
	m_modifiedComponentInstances(modifiedComponentInstances),
	m_modifiedSubSurfaceComponentInstances(modifiedSubSurfaceComponentInstances)
{
	setText( label );
}

SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopySubSurfaces(const std::vector<const VICUS::SubSurface *> & selectedSubSurfaces, const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances);
}

SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopySurfaces(const std::vector<const VICUS::Surface *> & selectedSurfaces, const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances);
}

SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopyRooms(const std::vector<const VICUS::Room *> & selectedRooms, const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances);
}


SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopyBuildingLevels(const std::vector<const VICUS::BuildingLevel *> & selectedBuildingLevels,
																				  const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances);
	// get translation offset

	// we take all selected building levels and copy them as new children of their building
	std::vector<VICUS::Building>	newBuildings(project().m_buildings);
	// NOTE: copy keeps all IDs of original, but pointers in copy are invalidated!



	unsigned int newID = project().nextUnusedID();
	for (const VICUS::BuildingLevel * bl : selectedBuildingLevels) {
		// we copy *everything* in the entire building level
		VICUS::BuildingLevel newBl(*bl);
		// now modify *all* ID s
		newBl.m_id = newID;
		for (unsigned int i=0; i<newBl.m_rooms.size(); ++i) {
			VICUS::Room & r = newBl.m_rooms[i];
			r.m_id = ++newID;
			for (unsigned int j=0; j<r.m_surfaces.size(); ++i) {
				VICUS::Surface & s = r.m_surfaces[j];
				s.m_id = ++newID;
				for (unsigned int k=0; k<s.subSurfaces().size(); ++k) {
					VICUS::SubSurface & sub = const_cast<VICUS::SubSurface &>(s.subSurfaces()[j]);
					sub.m_id = ++newID;
				}
			}
		}
		// now insert into vector with building levels
		// lookup parent building id in unmodified, original data structure
		unsigned int parentBuildingID = bl->m_parent->m_id;
//		for (int i=0; i<newBuildings.)

//		newBuildings.m_buildingLevels.push_back(newBl);
	};
}

SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopyBuildings(const std::vector<const VICUS::Building *> & selectedBuildings, const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances);
}


void SVUndoCopyBuildingGeometry::undo() {

	theProject().m_buildings.swap(m_modifiedBuilding);
	theProject().m_componentInstances.swap(m_modifiedComponentInstances);
	theProject().m_subSurfaceComponentInstances.swap(m_modifiedSubSurfaceComponentInstances);

	// update pointers
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoCopyBuildingGeometry::redo() {
	undo();
}

