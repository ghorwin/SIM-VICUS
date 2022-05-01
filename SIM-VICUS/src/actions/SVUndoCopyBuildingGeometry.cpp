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
#include "SVUndoTreeNodeState.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoCopyBuildingGeometry::SVUndoCopyBuildingGeometry(const QString & label,
													   const std::vector<VICUS::Building> & modifiedBuilding,
													   const std::vector<VICUS::ComponentInstance> & modifiedComponentInstances,
													   const std::vector<VICUS::SubSurfaceComponentInstance> & modifiedSubSurfaceComponentInstances,
													   const std::vector<unsigned int> & deselectedNodeIDs) :
	m_modifiedBuilding(modifiedBuilding),
	m_modifiedComponentInstances(modifiedComponentInstances),
	m_modifiedSubSurfaceComponentInstances(modifiedSubSurfaceComponentInstances),
	m_deselectedNodeIDs(deselectedNodeIDs)
{
	setText( label );
}

SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopySubSurfaces(const std::vector<const VICUS::SubSurface *> & selectedSubSurfaces,
																				   const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances, std::vector<unsigned int>());
}

SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopySurfaces(const std::vector<const VICUS::Surface *> & selectedSurfaces, const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances, std::vector<unsigned int>());
}

SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopyRooms(const std::vector<const VICUS::Room *> & selectedRooms, const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances, std::vector<unsigned int>());
}


SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopyBuildingLevels(const std::vector<const VICUS::BuildingLevel *> & selectedBuildingLevels,
																				  const IBKMK::Vector3D & translation)
{
	std::vector<VICUS::Building>					newBuildings(project().m_buildings);
	std::vector<VICUS::ComponentInstance>			newCI(project().m_componentInstances);
	std::vector<VICUS::SubSurfaceComponentInstance> newSubCI(project().m_subSurfaceComponentInstances);
	std::vector<unsigned int>						deselectedObjectIDs;
	// NOTE: copy keeps all IDs of original, but pointers in copy are invalidated!

	// we take all selected building levels and copy them as new children of their building
	unsigned int newID = project().nextUnusedID();
	for (const VICUS::BuildingLevel * bl : selectedBuildingLevels) {
		// we copy *everything* in the entire building level
		VICUS::BuildingLevel newBl(*bl);
		// adjust elevation
		newBl.m_elevation += translation.m_z;
		// now modify *all* ID s

		newBl.m_id = newID;
		for (unsigned int i=0; i<newBl.m_rooms.size(); ++i) {
			VICUS::Room & r = newBl.m_rooms[i];
			r.m_id = ++newID;
			for (unsigned int j=0; j<r.m_surfaces.size(); ++j) {
				VICUS::Surface & s = r.m_surfaces[j];
				s.m_id = ++newID;
				// apply transformation
				IBKMK::Polygon3D poly = s.polygon3D();
				poly.translate(translation);
				s.setPolygon3D(poly);
				// lookup potentially existing, single-sided surface component instance and duplicate it
				for (const VICUS::ComponentInstance & CI : project().m_componentInstances) {
					if ( (CI.m_idSideASurface == s.m_id && CI.m_idSideBSurface == VICUS::INVALID_ID) ||
						 (CI.m_idSideBSurface == s.m_id && CI.m_idSideASurface == VICUS::INVALID_ID))
					{
						newCI.push_back(CI);
						newCI.back().m_id = ++newID;
						break;
					}
				}

				// copy sub-surfaces
				for (unsigned int k=0; k<s.subSurfaces().size(); ++k) {
					VICUS::SubSurface & sub = const_cast<VICUS::SubSurface &>(s.subSurfaces()[k]);
					sub.m_id = ++newID;
					// NOTE: we only duplicate component instances (and subsurface component instances)

					// lookup potentially existing, single-sided subsurface component instance and duplicate it as well
					for (const VICUS::SubSurfaceComponentInstance & subCI : project().m_subSurfaceComponentInstances) {
						if ( (subCI.m_idSideASurface == sub.m_id && subCI.m_idSideBSurface == VICUS::INVALID_ID) ||
							 (subCI.m_idSideBSurface == sub.m_id && subCI.m_idSideASurface == VICUS::INVALID_ID))
						{
							newSubCI.push_back(subCI);
							newSubCI.back().m_id = ++newID;
							break;
						}
					}
				}
			}
		}
		// lookup parent building id in unmodified, original data structure
		unsigned int parentBuildingID = bl->m_parent->m_id;
		// now insert into vector with building levels
		unsigned int i=0;
		for (; i<newBuildings.size(); ++i)
			if (newBuildings[i].m_id == parentBuildingID) {
				newBuildings[i].m_buildingLevels.push_back(newBl);
				break;
			}
		Q_ASSERT(i != newBuildings.size()); // we must have found a valid parent building level, otherwise data model is corrupt

		// finally de-select everything in the original building level
		bool found = false;
		for (VICUS::Building & modB : newBuildings) {
			for (VICUS::BuildingLevel & modBl : modB.m_buildingLevels) {
				if (modBl.m_id == bl->m_id) {
					found = true;
					modBl.deselectWithChildren();
					std::set<unsigned int> ids;
					// collect IDs of deselected objects
					modBl.collectChildIDs(ids);
					deselectedObjectIDs = std::vector<unsigned int>(ids.begin(), ids.end());
					break;
				}
			}
			if (found)
				break;
		}
	}
	// new create the undo-action
	return new SVUndoCopyBuildingGeometry(tr("Copied entire building levels"), newBuildings, newCI, newSubCI, deselectedObjectIDs);
}


SVUndoCopyBuildingGeometry * SVUndoCopyBuildingGeometry::createUndoCopyBuildings(const std::vector<const VICUS::Building *> & selectedBuildings, const IBKMK::Vector3D & translation)
{
	return new SVUndoCopyBuildingGeometry(QString(), project().m_buildings, project().m_componentInstances, project().m_subSurfaceComponentInstances, std::vector<unsigned int>());
}


void SVUndoCopyBuildingGeometry::undo() {

	theProject().m_buildings.swap(m_modifiedBuilding);
	theProject().m_componentInstances.swap(m_modifiedComponentInstances);
	theProject().m_subSurfaceComponentInstances.swap(m_modifiedSubSurfaceComponentInstances);

	// update pointers
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	SVUndoTreeNodeState::ModifiedNodes modInfo;
	modInfo.m_changedStateType = SVUndoTreeNodeState::SelectedState;
	modInfo.m_nodeIDs = m_deselectedNodeIDs;
	SVProjectHandler::instance().setModified( SVProjectHandler::NodeStateModified, &modInfo);
}


void SVUndoCopyBuildingGeometry::redo() {
	undo();
}

