/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef VICUS_NetworkNodeH
#define VICUS_NetworkNodeH

#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Object.h"

#include <NANDRAD_HydraulicNetworkHeatExchange.h>

#include <vector>
#include <set>
#include <limits>
#include <IBKMK_Vector3D.h>

#include <QColor>

namespace VICUS {

class NetworkEdge;

class NetworkNode : public Object {
public:
	/*! Type-info string. */
	const char * typeinfo() const override { return "NetworkNode"; }

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	enum NodeType {
		NT_Building,		// Keyword: Building
		NT_Mixer,			// Keyword: Mixer
		NT_Source,			// Keyword: Source
		NUM_NT
	};

	NetworkNode() = default;

	NetworkNode(const unsigned id, const NodeType type, const IBKMK::Vector3D &v, const double heatDemand=0):
		m_position(v),
		m_type(type),
		m_maxHeatingDemand("MaxHeatingDemand", heatDemand, "W")
	{
		m_id = id;
	}

	/*! check connectivity of graph trhough recursive search */
	void collectConnectedEdges(std::set<const NetworkNode*> & connectedNodes, std::set<const NetworkEdge*> & connectedEdge) const;

	void setInletOutletNode(std::set< const NetworkNode*> & visitedNodes, std::vector<const NetworkEdge*> & orderedEdges) const;

	/*! updates m_isDeadEnd. If node has less than two neighbours which are not a deadEnd and node is not a building
	 * nor a source: m_isDeadEdnd = true */
	void updateIsDeadEnd();

	/*! Caution: for some applications this definition may needs to be more precise
	 * e.g. compare types of connected edges */
	bool isRedundant() const{
		return (m_edges.size()==2);
	}

	/*! Only callable if node has exactly two edges: return the edge which is not the given edge */
	NetworkEdge * neighborEdge(const NetworkEdge * e) const;

	/*! get a set of all redundant nodes in the graph */
	void findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const NetworkEdge*> & visitedEdges) const;

	/*! looking from this node in the direction of the given edgeToVisit: return the next node that is not redundant
	 * and the distance to this node */
	const NetworkNode * findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & distance, const NetworkEdge* edgeToVisit) const;

	/*! simple algorithm to find the path from this node to the node of type NT_SOURCE.
	 * The path is stored as a set of edges */
	bool findPathToSource(std::set<NetworkEdge*> &path, std::set<NetworkEdge*> &visitedEdges, std::set<unsigned> &visitedNodes);

	/*! used for dijkstra algorithm. Look at all neighbour nodes: if the m_distanceToStart of this node + the distance to the neighbour
	 * is shorter than the current m_distanceToStart of the neighbour, update it. This makes sure the neighbour nodes have assigned
	 * the currently smallest distance from start */
	void updateNeighbourDistances() const;

	/*! used for dijkstra algorithm. appends the edge which leads to the predecessor node to path and calls itself for the predecessor node
	 * until a node without predecessor is reached. this way the path from a building to the source can be created, if the predecessors have been set */
	void pathToNull(std::vector<NetworkEdge * > & path) const;

	/*! looks at all adjacent nodes to find a node which has a heating demand >0 and returns it. */
	double adjacentHeatingDemand(std::set<NetworkEdge*> visitedEdges);


	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int				m_id = INVALID_ID;										// XML:A:required
	//:inherited	QString						m_displayName;											// XML:A
	//:inherited	bool						m_visible = true;										// XML:A

	IBKMK::Vector3D								m_position = IBKMK::Vector3D(-9.99,-9.99,-9.99);		// XML:E:required

	NodeType									m_type = NUM_NT;										// XML:A:required

	/*! Heating demand.	*/
	IBK::Parameter								m_maxHeatingDemand;										// XML:E

	/*! Reference id to a VICUS::SubNetwork */
	unsigned int								m_idSubNetwork = INVALID_ID;							// XML:A

	NANDRAD::HydraulicNetworkHeatExchange		m_heatExchange;											// XML:E


	// *** RUNTIME VARIABLES ***

	/*! Pointers to adjacent edges */
	std::vector<NetworkEdge*>			m_edges;

	/*! The radius used for the visualization of this node in the 3D scene [m].
		Updated whenever the scale factor Network::m_scaleNodes changes.
	*/
	mutable double								m_visualizationRadius;

	/*! Color to be used for displaying (visible) nodes. */
	mutable QColor								m_color;

	/*! Used in dijkstra algorithm. */
	mutable double								m_distanceToStart = (std::numeric_limits<double>::max)();
	mutable const NetworkNode *					m_predecessor = nullptr;

	/*! Defines wether this node is a dead end. */
	mutable bool								m_isDeadEnd = false;


};

} // namespace VICUS

#endif // VICUS_NetworkNodeH
