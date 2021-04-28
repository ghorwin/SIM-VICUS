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


SVUndoDeleteSelected::SVUndoDeleteSelected(const QString & label, const std::vector<unsigned int> & objectIDsToBeRemoved) :
	m_selectedIDs(objectIDsToBeRemoved.begin(), objectIDsToBeRemoved.end())
{
	setText( label );

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
					if (m_selectedIDs.find(r.m_surfaces[sIdx].uniqueID()) != m_selectedIDs.end())
						r.m_surfaces.erase(r.m_surfaces.begin() + sIdx); // remove surface, keep counter
					else
						++sIdx; // go to next surface
				}
				// selected for deletion and empty?
				if (m_selectedIDs.find(r.uniqueID()) != m_selectedIDs.end() && r.m_surfaces.empty())
					bl.m_rooms.erase(bl.m_rooms.begin() + rIdx); // remove, keep counter
				else
					++rIdx; // go to next room
			}
			// selected for deletion and empty?
			if (m_selectedIDs.find(bl.uniqueID()) != m_selectedIDs.end() && bl.m_rooms.empty())
				b.m_buildingLevels.erase(b.m_buildingLevels.begin() + blIdx); // remove, keep counter
			else
				++blIdx; // go to next room
		}
		// selected for deletion and empty?
		if (m_selectedIDs.find(b.uniqueID()) != m_selectedIDs.end() && b.m_buildingLevels.empty())
			m_buildings.erase(m_buildings.begin() + bIdx); // remove, keep counter
		else
			++bIdx; // go to next room
	}

	// reserve memory for plain geometry and then copy all but selected surfaces
	m_plainGeometry.reserve(project().m_plainGeometry.size());
	for (const VICUS::Surface & s : project().m_plainGeometry) {
		// not found? -> not marked for deletion!
		if (m_selectedIDs.find(s.uniqueID()) == m_selectedIDs.end())
			m_plainGeometry.push_back(s); // copy surface over to data store
	}


	// also collect component instances that can be safely deleted, i.e. keep only those that reference at least one surface
	// that is not selected
	m_compInstances.reserve(project().m_componentInstances.size());
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		unsigned int sideASurfID = ci.m_sideASurfaceID;
		unsigned int sideBSurfID = ci.m_sideBSurfaceID;

		// if side A references a surface marked for deletion, clear the ID
		if (ci.m_sideASurfaceID != VICUS::INVALID_ID && m_selectedIDs.find(ci.m_sideASurfaceID) != m_selectedIDs.end())
			sideASurfID = VICUS::INVALID_ID;
		// same for side B
		if (ci.m_sideBSurfaceID != VICUS::INVALID_ID && m_selectedIDs.find(ci.m_sideBSurfaceID) != m_selectedIDs.end())
			sideBSurfID = VICUS::INVALID_ID;

		// only keep component instance around if at least one side-ID is valid
		if (sideASurfID != VICUS::INVALID_ID || sideBSurfID != VICUS::INVALID_ID)
			m_compInstances.push_back(ci);
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

	// rebuild pointer hierarchy
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

