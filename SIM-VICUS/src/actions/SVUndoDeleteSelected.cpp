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

#include "SVUndoDeleteSelected.h"
#include "SVProjectHandler.h"

#include "SVViewStateHandler.h"

// Utility comparison operator to search for unique ID
class hasUniqueID {
public:
	hasUniqueID(unsigned int uID) : m_uID(uID) {}

	bool operator()(const VICUS::Surface & s) {
		return s.uniqueID() == m_uID;
	}

	unsigned int m_uID;
};


SVUndoDeleteSelected::SVUndoDeleteSelected(const QString & label,
										   const std::set<const VICUS::Object *> & objectsToBeRemoved)
{
	setText( label );

	std::set<unsigned int>						selectedUniqueIDs;
	for (const VICUS::Object * p : objectsToBeRemoved)
		selectedUniqueIDs.insert(p->uniqueID());

	// create a copy of all data structures that need something removed from
	m_buildings = project().m_buildings;

	// we now erase objects throughout the hierarchy
	// surface -> (empty) rooms -> (empty) building levels -> (empty) buildings
	for (unsigned int bIdx = 0; bIdx < m_buildings.size(); /* no counter increase here */) {
		VICUS::Building & b = m_buildings[bIdx];
		for (unsigned int blIdx = 0; blIdx < b.m_buildingLevels.size(); /* no counter increase here */) {
			VICUS::BuildingLevel & bl = b.m_buildingLevels[blIdx];
			for (unsigned int rIdx = 0; rIdx < bl.m_rooms.size(); /* no counter increase here */) {
				VICUS::Room & r = bl.m_rooms[rIdx];
				for (unsigned int sIdx = 0; sIdx < r.m_surfaces.size(); /* no counter increase here */) {
					// is surface selected for deletion?
					if (selectedUniqueIDs.find(r.m_surfaces[sIdx].uniqueID()) != selectedUniqueIDs.end())
						r.m_surfaces.erase(r.m_surfaces.begin() + sIdx); // remove surface, keep counter
					else {
						// search through all subsurfaces of current surface and remove those
						std::vector<VICUS::SubSurface> remainingSubSurfaces;
						for (const VICUS::SubSurface & sub : r.m_surfaces[sIdx].subSurfaces())
							if (selectedUniqueIDs.find(sub.uniqueID()) == selectedUniqueIDs.end() )
								remainingSubSurfaces.push_back(sub); // keep only those that are not selected
						r.m_surfaces[sIdx].setSubSurfaces(remainingSubSurfaces); // keep remaining subs
						++sIdx; // go to next surface
					}
				}
				// selected for deletion and empty?
				if (selectedUniqueIDs.find(r.uniqueID()) != selectedUniqueIDs.end() && r.m_surfaces.empty())
					bl.m_rooms.erase(bl.m_rooms.begin() + rIdx); // remove, keep counter
				else
					++rIdx; // go to next room
			}
			// selected for deletion and empty?
			if (selectedUniqueIDs.find(bl.uniqueID()) != selectedUniqueIDs.end() && bl.m_rooms.empty())
				b.m_buildingLevels.erase(b.m_buildingLevels.begin() + blIdx); // remove, keep counter
			else
				++blIdx; // go to next room
		}
		// selected for deletion and empty?
		if (selectedUniqueIDs.find(b.uniqueID()) != selectedUniqueIDs.end() && b.m_buildingLevels.empty())
			m_buildings.erase(m_buildings.begin() + bIdx); // remove, keep counter
		else
			++bIdx; // go to next room
	}

	// reserve memory for plain geometry and then copy all but selected surfaces
	m_plainGeometry.reserve(project().m_plainGeometry.size());
	for (const VICUS::Surface & s : project().m_plainGeometry) {
		// not found? -> not marked for deletion!
		if (selectedUniqueIDs.find(s.uniqueID()) == selectedUniqueIDs.end())
			m_plainGeometry.push_back(s); // copy surface over to data store
	}


	// also collect component instances that can be safely deleted, i.e. keep only those that reference at least one surface
	// that is not selected
	m_compInstances.reserve(project().m_componentInstances.size());
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {

		VICUS::ComponentInstance modCi(ci);
		const VICUS::Surface * sideASurf = ci.m_sideASurface;
		const VICUS::Surface * sideBSurf = ci.m_sideBSurface;

		// if side A references a surface marked for deletion, clear the ID
		if (ci.m_sideASurface != nullptr &&
				objectsToBeRemoved.find(ci.m_sideASurface) != objectsToBeRemoved.end())
		{
			sideASurf = nullptr;
			modCi.m_idSideASurface = VICUS::INVALID_ID;
		}
		// same for side B
		if (ci.m_sideBSurface != nullptr &&
				objectsToBeRemoved.find(ci.m_sideBSurface) != objectsToBeRemoved.end())
		{
			sideBSurf = nullptr;
			modCi.m_idSideBSurface = VICUS::INVALID_ID;
		}

		// only keep component instance around if at least one side-ID is valid
		if (sideASurf != nullptr || sideBSurf != nullptr)
			m_compInstances.push_back(modCi);
	}

	// sub-surface component instances
	m_subCompInstances.reserve(project().m_subSurfaceComponentInstances.size());
	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
		const VICUS::SubSurface * sideASurf = ci.m_sideASubSurface;
		const VICUS::SubSurface * sideBSurf = ci.m_sideBSubSurface;

		VICUS::SubSurfaceComponentInstance modCi(ci);
		// if side A references a surface marked for deletion, clear the ID
		if (ci.m_sideASubSurface != nullptr &&
				objectsToBeRemoved.find(ci.m_sideASubSurface) != objectsToBeRemoved.end())
		{
			sideASurf = nullptr;
			modCi.m_idSideASurface = VICUS::INVALID_ID;
		}
		// same for side B
		if (ci.m_sideBSubSurface != nullptr &&
				objectsToBeRemoved.find(ci.m_sideBSubSurface) != objectsToBeRemoved.end())
		{
			sideBSurf = nullptr;
			modCi.m_idSideBSurface = VICUS::INVALID_ID;
		}

		// only keep component instance around if at least one side-ID is valid
		if (sideASurf != nullptr || sideBSurf != nullptr)
			m_subCompInstances.push_back(modCi);
	}
}


void SVUndoDeleteSelected::undo() {
	redo();  // same as redo, just swap memory
}


void SVUndoDeleteSelected::redo() {

	VICUS::Project & prj = theProject();
	prj.m_buildings.swap(m_buildings);
	prj.m_plainGeometry.swap(m_plainGeometry);
	prj.m_componentInstances.swap(m_compInstances);
	prj.m_subSurfaceComponentInstances.swap(m_subCompInstances);

	// rebuild pointer hierarchy
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

