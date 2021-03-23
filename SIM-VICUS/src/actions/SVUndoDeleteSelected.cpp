#include "SVUndoDeleteSelected.h"
#include "SVProjectHandler.h"

#include "SVViewStateHandler.h"

SVUndoDeleteSelected::SVUndoDeleteSelected(const QString & label, const std::vector<unsigned int> & selectedIDs,
										   const std::vector<VICUS::ComponentInstance> & componentIDs) :
	m_selectedIDs(selectedIDs),
	m_compInstances(componentIDs)
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

	// we now insert the cached data back into the data model
	// Note: we need to do this backwards, in order to keep the insert positions!
	for (std::vector<RemovedSurfaceInfo>::const_reverse_iterator rit = m_roomGeometry.rbegin();
		 rit != m_roomGeometry.rend(); ++rit)
	{
		const RemovedSurfaceInfo & rsi = *rit;
		// find the room with given ID
		const VICUS::Object * o = theProject().objectById(rsi.m_parentRoomId);
		Q_ASSERT(o != nullptr);
		const VICUS::Room * r = dynamic_cast<const VICUS::Room*>(o);
		Q_ASSERT(r != nullptr);
		VICUS::Room * rptr = const_cast<VICUS::Room*>(r);
		rptr->m_surfaces.insert(rptr->m_surfaces.begin()+rsi.m_insertIdx, rsi.m_surface);
	}

	// we now insert the cached data back into the data model
	// Note: we need to do this backwards, in order to keep the insert positions!
	for (std::vector<RemovedSurfaceInfo>::const_reverse_iterator rit = m_plainGeometry.rbegin();
		 rit != m_plainGeometry.rend(); ++rit)
	{
		const RemovedSurfaceInfo & rsi = *rit;
		theProject().m_plainGeometry.insert(theProject().m_plainGeometry.begin()+rsi.m_insertIdx, rsi.m_surface);
	}

	theProject().m_componentInstances.swap(m_compInstances);

	// rebuild pointer hierarchy
	theProject().updatePointers();
	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
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
						unsigned int idx = std::distance(roomIt->m_surfaces.begin(), it);
						m_roomGeometry.push_back(RemovedSurfaceInfo(roomIt->uniqueID(), idx, *it));
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
			unsigned int idx = std::distance(prj.m_plainGeometry.begin(), it);
			m_plainGeometry.push_back(RemovedSurfaceInfo(0, idx, *it));
			// now remove the surface
			prj.m_plainGeometry.erase(it);
			found = true;
		}

		// TODO : search through nodes/edges
	}

	theProject().m_componentInstances.swap(m_compInstances);

	// rebuild pointer hierarchy
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

