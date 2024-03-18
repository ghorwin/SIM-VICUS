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

#include "SVUndoAddNetwork.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"

SVUndoAddNetwork::SVUndoAddNetwork(const QString & label, const VICUS::Network & addedNetwork, bool networkGeometryModified, bool modifyFarDist) :
	m_addedNetwork(addedNetwork),
	m_networkGeometryModified(networkGeometryModified),
	m_modifyGridDist(modifyFarDist)
{
	setText( label );
	m_gridWidth = 1.2 * std::max(addedNetwork.m_extends.width(), addedNetwork.m_extends.height());
	if (m_gridWidth > 9999)
		m_gridSpacing = 1000;
	else if (m_gridWidth > 999)
		m_gridSpacing = 100;
	else
		m_gridSpacing = 10;
	m_farDistance = std::max(1000.0, 2*m_gridWidth);
}


void SVUndoAddNetwork::undo() {

	// remove last network
	Q_ASSERT(!theProject().m_geometricNetworks.empty());

	theProject().m_geometricNetworks.pop_back();
	theProject().updatePointers();
	if (!theProject().m_geometricNetworks.empty()) {
		const SVDatabase & db = SVSettings::instance().m_db;
		theProject().m_geometricNetworks.back().updateVisualizationRadius(db.m_pipes);
	}
	theProject().m_activeNetworkId = m_previouslyActiveNetworkId;

	if (m_modifyGridDist) {
		std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);
		if (theProject().m_viewSettings.m_gridPlanes.size() > 0) {
			std::swap(theProject().m_viewSettings.m_gridPlanes[0].m_width, m_gridWidth);
			std::swap(theProject().m_viewSettings.m_gridPlanes[0].m_spacing, m_gridSpacing);
		}
		SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
	}

	// tell project that the network has changed
	if (m_networkGeometryModified)
		SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::NetworkDataChanged);

}


void SVUndoAddNetwork::redo() {
	// append network

	theProject().m_geometricNetworks.push_back(m_addedNetwork);
	theProject().updatePointers();
	// set the added network as active
	m_previouslyActiveNetworkId = theProject().m_activeNetworkId;
	theProject().m_activeNetworkId = theProject().m_geometricNetworks.back().m_id;

	if (m_modifyGridDist) {
		std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);
		if (theProject().m_viewSettings.m_gridPlanes.size() > 0) {
			std::swap(theProject().m_viewSettings.m_gridPlanes[0].m_width, m_gridWidth);
			std::swap(theProject().m_viewSettings.m_gridPlanes[0].m_spacing, m_gridSpacing);
		}
		SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
	}

	// tell project that the network has changed
	if (m_networkGeometryModified)
		SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::NetworkDataChanged);

}

