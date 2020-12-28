#include "SVUndoDeleteSelected.h"
#include "SVProjectHandler.h"

SVUndoDeleteSelected::SVUndoDeleteSelected(const QString & label, const std::vector<unsigned int> & selectedIDs) :
	m_selectedIDs(selectedIDs)
{
	setText( label );
}

class hasUniqueID {
public:
	hasUniqueID(unsigned int uID) : m_uID(uID) {}

	bool operator()(const VICUS::Surface & s) {
		return s.uniqueID() == m_uID;
	}

	unsigned int m_uID;
};


void SVUndoDeleteSelected::undo() {


	// rebuild pointer hierarchy
	theProject().updatePointers();
	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::GeometryChanged);
}


void SVUndoDeleteSelected::redo() {

	VICUS::Project & prj = theProject();

	m_roomGeometry.clear();
	m_plainGeometry.clear();

	// now process all IDs
	for (unsigned int uID : m_selectedIDs) {
		// process all building surfaces
		bool found = false;
		for (std::vector<VICUS::Building>::iterator buildingIt = prj.m_buildings.begin();
			 !found && buildingIt != prj.m_buildings.end(); ++buildingIt)
		{
			for (std::vector<VICUS::BuildingLevel>::iterator levelIt = buildingIt->m_buildingLevels.begin();
				 !found && levelIt != buildingIt->m_buildingLevels.end(); ++levelIt)
			{
				for (std::vector<VICUS::Room>::iterator roomIt = levelIt->m_rooms.begin();
					 !found && roomIt != levelIt->m_rooms.end(); ++roomIt)
				{
					std::vector<VICUS::Surface>::iterator it = std::find_if(
								roomIt->m_surfaces.begin(), roomIt->m_surfaces.end(), hasUniqueID(uID));
					if (it != roomIt->m_surfaces.end()) {
						// copy surface over to data store
						m_roomGeometry.push_back(RemovedSurfaceInfo(roomIt->uniqueID(), *it));
						// now remove the surface
						roomIt->m_surfaces.erase(it);
						found = true;
					}
				}
			}
		}
		if (found)
			continue; // next ID

		// also search through naive geometry
		found = false;
		std::vector<VICUS::Surface>::iterator it = std::find_if(
					prj.m_plainGeometry.begin(), prj.m_plainGeometry.end(), hasUniqueID(uID));
		if (it != prj.m_plainGeometry.end()) {
			// copy surface over to data store
			m_plainGeometry.push_back(*it);
			// now remove the surface
			prj.m_plainGeometry.erase(it);
			found = true;
		}

		// TODO : search through nodes/edges
	}

	// rebuild pointer hierarchy
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::GeometryChanged);
}

