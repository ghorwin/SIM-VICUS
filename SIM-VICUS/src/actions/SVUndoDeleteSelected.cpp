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
		return s.m_id == m_uID;
	}

	unsigned int m_uID;
};

void selectChildSurfaces(const VICUS::Surface &surf, std::set<unsigned int> &selectedUniqueIDs) {
	for (const VICUS::Surface &cs : surf.childSurfaces()) {
		selectedUniqueIDs.insert(cs.m_id);
		selectChildSurfaces(cs, selectedUniqueIDs);
	}
}


SVUndoDeleteSelected::SVUndoDeleteSelected(const QString & label,
										   const std::set<const VICUS::Object *> & objectsToBeRemoved)
{
	setText( label );

	std::set<unsigned int>	selectedUniqueIDs;
	for (const VICUS::Object * p : objectsToBeRemoved)
		selectedUniqueIDs.insert(p->m_id);

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
					if (selectedUniqueIDs.find(r.m_surfaces[sIdx].m_id) != selectedUniqueIDs.end()) {
						for(const VICUS::SubSurface &sub : r.m_surfaces[sIdx].subSurfaces())
							selectedUniqueIDs.insert(sub.m_id);
						selectChildSurfaces(r.m_surfaces[sIdx], selectedUniqueIDs);
						r.m_surfaces.erase(r.m_surfaces.begin() + sIdx); // remove surface, keep counter
					}
					else {
						// search through all subsurfaces of current surface and remove those
						std::vector<VICUS::SubSurface> remainingSubSurfaces;
						for (const VICUS::SubSurface & sub : r.m_surfaces[sIdx].subSurfaces())
							if (selectedUniqueIDs.find(sub.m_id) == selectedUniqueIDs.end() )
								remainingSubSurfaces.push_back(sub); // keep only those that are not selected
						std::vector<VICUS::Surface> remainingChildSurfaces;
						for (const VICUS::Surface & child : r.m_surfaces[sIdx].childSurfaces())
							if (selectedUniqueIDs.find(child.m_id) == selectedUniqueIDs.end() )
								remainingChildSurfaces.push_back(child); // keep only those that are not selected
						r.m_surfaces[sIdx].setChildAndSubSurfaces(remainingSubSurfaces, remainingChildSurfaces); // keep remaining subs
						++sIdx; // go to next surface
					}
				}
				// selected for deletion and empty?
				if (selectedUniqueIDs.find(r.m_id) != selectedUniqueIDs.end() && r.m_surfaces.empty())
					bl.m_rooms.erase(bl.m_rooms.begin() + rIdx); // remove, keep counter
				else
					++rIdx; // go to next room
			}
			// selected for deletion and empty?
			if (selectedUniqueIDs.find(bl.m_id) != selectedUniqueIDs.end() && bl.m_rooms.empty())
				b.m_buildingLevels.erase(b.m_buildingLevels.begin() + blIdx); // remove, keep counter
			else
				++blIdx; // go to next room
		}
		// selected for deletion and empty?
		if (selectedUniqueIDs.find(b.m_id) != selectedUniqueIDs.end() && b.m_buildingLevels.empty())
			m_buildings.erase(m_buildings.begin() + bIdx); // remove, keep counter
		else
			++bIdx; // go to next room
	}

	// reserve memory for plain geometry and then copy all but selected surfaces
	m_plainGeometry.reserve(project().m_plainGeometry.m_surfaces.size());
	for (const VICUS::Surface & s : project().m_plainGeometry.m_surfaces) {
		// not found? -> not marked for deletion!
		if (selectedUniqueIDs.find(s.m_id) == selectedUniqueIDs.end())
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
		if (ci.m_idSideASurface != VICUS::INVALID_ID &&
				selectedUniqueIDs.find(ci.m_idSideASurface) != selectedUniqueIDs.end())
		{
			sideASurf = nullptr;
			modCi.m_idSideASurface = VICUS::INVALID_ID;
		}
		// same for side B
		if (ci.m_idSideBSurface != VICUS::INVALID_ID &&
				selectedUniqueIDs.find(ci.m_idSideBSurface) != selectedUniqueIDs.end())
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
		if (ci.m_idSideASurface != VICUS::INVALID_ID &&
				selectedUniqueIDs.find(ci.m_idSideASurface) != selectedUniqueIDs.end())
		{
			sideASurf = nullptr;
			modCi.m_idSideASurface = VICUS::INVALID_ID;
		}
		// same for side B
		if (ci.m_idSideBSurface != VICUS::INVALID_ID &&
				selectedUniqueIDs.find(ci.m_idSideBSurface) != selectedUniqueIDs.end())
		{
			sideBSurf = nullptr;
			modCi.m_idSideBSurface = VICUS::INVALID_ID;
		}

		// only keep component instance around if at least one side-ID is valid
		if (sideASurf != nullptr || sideBSurf != nullptr)
			m_subCompInstances.push_back(modCi);
	}


	// *** Networks

	// create a copy of all data structures that need something removed from
	m_networks = project().m_geometricNetworks;
	for (unsigned int idxNet=0; idxNet<m_networks.size(); /* no counter increase here */) {

		VICUS::Network & net = m_networks[idxNet];

		for (unsigned int idxNo=0; idxNo<net.m_nodes.size(); /* no counter increase here */) {
			VICUS::NetworkNode &no = net.m_nodes[idxNo];
			if (selectedUniqueIDs.find(no.m_id) != selectedUniqueIDs.end()) {
				// when deleting a node, the connected edges also need to be deleted
				for (const VICUS::NetworkEdge *e: no.m_edges) {
					if (e != nullptr)
						selectedUniqueIDs.insert(e->m_id);
				}
				net.m_nodes.erase(net.m_nodes.begin() + idxNo);
			}
			else
				++idxNo;
		}

		for (unsigned int idxE=0; idxE<net.m_edges.size(); /* no counter increase here */) {
			VICUS::NetworkEdge &e = net.m_edges[idxE];
			if (selectedUniqueIDs.find(e.m_id) != selectedUniqueIDs.end()){
				// if there is a mixer node connected, which is only connected to the current edge, we also delete that
				if (e.m_node1->m_type == VICUS::NetworkNode::NT_Mixer && e.m_node1->m_edges.size()==1)
					selectedUniqueIDs.insert(e.nodeId1());
				if (e.m_node2->m_type == VICUS::NetworkNode::NT_Mixer && e.m_node2->m_edges.size()==1)
					selectedUniqueIDs.insert(e.nodeId2());
				net.m_edges.erase(net.m_edges.begin() + idxE);
				}
			else
				++idxE;
		}

		// delete the additionally collected nodes
		for (unsigned int idxNo=0; idxNo<net.m_nodes.size(); /* no counter increase here */) {
			VICUS::NetworkNode &no = net.m_nodes[idxNo];
			if (selectedUniqueIDs.find(no.m_id) != selectedUniqueIDs.end())
				net.m_nodes.erase(net.m_nodes.begin() + idxNo);
			else
				++idxNo;
		}

		if (selectedUniqueIDs.find(net.m_id) != selectedUniqueIDs.end() && net.m_edges.empty() && net.m_nodes.empty())
			m_networks.erase(m_networks.begin() + idxNet);
		else
			++idxNet;
	}


	// *** Drawings

	m_drawings = project().m_drawings;
	for (unsigned int idxDraw=0; idxDraw<m_drawings.size();  ) {

		VICUS::Drawing &draw = m_drawings[idxDraw];

		for (unsigned int idxLayer=0; idxLayer<draw.m_drawingLayers.size();  ) {
			VICUS::DrawingLayer drawLayer = draw.m_drawingLayers[idxLayer];
			if (selectedUniqueIDs.find(drawLayer.m_id) != selectedUniqueIDs.end())
				draw.m_drawingLayers.erase( draw.m_drawingLayers.begin() + idxLayer );
			else
				++idxLayer;
		}

		if (selectedUniqueIDs.find(draw.m_id) != selectedUniqueIDs.end())
			m_drawings.erase( m_drawings.begin() + idxDraw );
		else
			++idxDraw;
	}

}


void SVUndoDeleteSelected::undo() {
	redo();  // same as redo, just swap memory
}


void SVUndoDeleteSelected::redo() {

	VICUS::Project & prj = theProject();
	prj.m_buildings.swap(m_buildings);
	prj.m_plainGeometry.m_surfaces.swap(m_plainGeometry);
	prj.m_componentInstances.swap(m_compInstances);
	prj.m_subSurfaceComponentInstances.swap(m_subCompInstances);
	prj.m_geometricNetworks.swap(m_networks);
	prj.m_drawings.swap(m_drawings);

	// rebuild pointer hierarchy
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::DrawingModified);
}

