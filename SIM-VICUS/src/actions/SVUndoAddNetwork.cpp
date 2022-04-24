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

SVUndoAddNetwork::SVUndoAddNetwork(	const QString & label,
								const VICUS::Network & addedNetwork) :
	m_addedNetwork(addedNetwork)
{
	setText( label );
	double gridWidth = std::max(addedNetwork.m_extends.width(), addedNetwork.m_extends.height());
	m_farDistance = 2*gridWidth;
}


void SVUndoAddNetwork::undo() {

	// remove last network
	Q_ASSERT(!theProject().m_geometricNetworks.empty());

	theProject().m_geometricNetworks.pop_back();
	if (theProject().m_geometricNetworks.empty())
		return;
	theProject().m_geometricNetworks.back().updateNodeEdgeConnectionPointers(); // ensure pointers are correctly set
	const SVDatabase & db = SVSettings::instance().m_db;
	theProject().m_geometricNetworks.back().updateVisualizationRadius(db.m_pipes);
	theProject().updatePointers();

	std::swap(theProject().m_viewSettings.m_gridPlanes, m_gridPlanes);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}


void SVUndoAddNetwork::redo() {
	// append network

	theProject().m_geometricNetworks.push_back(m_addedNetwork);
	theProject().updatePointers();
	std::swap(theProject().m_viewSettings.m_gridPlanes, m_gridPlanes);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}

