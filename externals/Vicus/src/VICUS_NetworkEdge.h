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

#ifndef VICUS_NetworkEdgeH
#define VICUS_NetworkEdgeH

#include "VICUS_NetworkNode.h"
#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Object.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_NetworkComponent.h"


namespace VICUS {

/*! A network edge defines all parameters needed to generate pipe hydraulic network elements.
	The pipe-specific hydraulic parameters are stored in the pipe database (NetworkPipe).
	The type of pipe model is defined in m_modelType and the kind of heat exchange to be considered
	is defined in m_heatExchangeType.
	Parameters for generation of the hydraulic element are stored in m_para. Also, the double-parameters needed
	for the heat exchange models are stored in m_para.

	NOTE: NetworkEdge also uses the unique m_id to identify object and respective navigation tree items and scene
		  objects. However, the ID is nowhere else used (referenced) in the data model and thus not persistently
		  saved in the project file. It is generated on first use within VICUS::Project::updatePointers().

	TODO Hauke, Dokumentation der Memberfunktionen!
*/
class NetworkEdge : public Object {
public:
	/*! Type-info string. */
	const char * typeinfo() const override { return "NetworkEdge"; }

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	NetworkEdge() = default;

	NetworkEdge(const unsigned id, const unsigned nodeId1, const unsigned nodeId2, const bool supply, const double &length, const unsigned pipeId):
		m_supply(supply),
		m_idPipe(pipeId),
		m_idNode1(nodeId1),
		m_idNode2(nodeId2),
		m_length(length)
	{
		m_id = id;
	}

	void collectConnectedNodes(std::set<const NetworkNode*> & connectedNodes,
								std::set<const NetworkEdge*> & connectedEdge) const;

	void setInletOutletNode(std::set<const NetworkNode*> & visitedNodes, std::vector<const NetworkEdge*> & orderedEdges);

	// TODO Hauke, Vergleichsoperatoren sollten immer das ganze Objekt vergleichen - eher dann eine andere Funktion verwenden
	//             mit eigenem Namen
	bool operator==(NetworkEdge &e2) const{
		return (m_idNode1 == e2.m_idNode1) && (m_idNode2 == e2.m_idNode2);
	}

	bool operator==(const NetworkEdge &e2) const{
		return (m_idNode1 == e2.m_idNode1) && (m_idNode2 == e2.m_idNode2);
	}

	/*! returns opposite node of the given one */
	NetworkNode * neighbourNode(const NetworkNode *node) const;

	unsigned neighbourNode(unsigned nodeId) const;

	double length() const {	return m_length; }

	void setLengthFromCoordinates();

	unsigned int nodeId1() const { return m_idNode1; }

	unsigned int nodeId2() const {return m_idNode2; }

	// swaps current nodeId1 with given node id, sets pointer and calculates the new length of this edge
	void changeNode1(NetworkNode *node);

	// swaps current nodeId1 with given node id, sets pointer and calculates the new length of this edge
	void changeNode2(NetworkNode *node);


	// *** PUBLIC MEMBER VARIABLES ***

	/*! If true, nodes of type Building can connect to this edge.
		This is used for the automatic algorithm that connects buildings with the network */
	bool												m_supply;						// XML:A

	/*! ID of pipe in database */
	IDType												m_idPipe = INVALID_ID;			// XML:A

	//:inherited	QString								m_displayName;					// XML:A

	/*! Whether the node is visible or not. */
	//:inherited	bool								m_visible = true;				// XML:A

	/*! Defines the heat exchange properties for this edge (ambient temperature, heat flux etc.) */
	NANDRAD::HydraulicNetworkHeatExchange				m_heatExchange;					// XML:E


	// *** RUNTIME VARIABLES ***

	/*! The radius [m] used for the visualization of this edge in the 3D scene
		Updated whenever the scale factor Network::m_scaleEdges changes, or the pipe ID.
	*/
	double												m_visualizationRadius;
	/*! Color to be used for displaying (visible) nodes. */
	mutable QColor										m_color;

	/*! Sum of maximum heating demand of all connected buildings [W], will be determined in sizePipeDimensions() */
	double												m_nominalHeatingDemand = 0;

	/*! Mass flow [kg/s] at nominal temperature difference, will be determined in sizePipeDimensions() */
	double												m_nominalMassFlow = 0;

	/*! Describes the fluid temperature change along this pipe with nominal mass flow, when there is a temperature difference of 1 K
		between fluid and sourrunding soil. This information can be used to create according soil models.
		Is dimensionless, but for interpretation, unit [K/K] can be used
	*/
	double												m_tempChangeIndicator = -1;
	double												m_cumulativeTempChangeIndicator = -1;

	NetworkNode											* m_node1 = nullptr;
	NetworkNode											* m_node2 = nullptr;

	unsigned int										m_idNodeInlet = INVALID_ID;
	unsigned int										m_idNodeOutlet = INVALID_ID;

	unsigned int										m_idNandradSupplyPipe = INVALID_ID;
	unsigned int										m_idNandradReturnPipe = INVALID_ID;

	unsigned int										m_idSoil = INVALID_ID;


private:

	// *** PRIVATE MEMBER VARIABLES ***

	unsigned int										m_idNode1 = 0;					// XML:A:required
	unsigned int										m_idNode2 = 0;					// XML:A:required

	/*! Effective length [m], might be different than geometric length between nodes. */
	double												m_length = 0;					// XML:E
};

} // namespace VICUS

#endif // VICUS_NetworkEdgeH
