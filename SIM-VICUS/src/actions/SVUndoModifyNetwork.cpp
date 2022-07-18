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

#include "SVUndoModifyNetwork.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"

#include <VICUS_utilities.h>

SVUndoModifyNetwork::SVUndoModifyNetwork(const QString &label, const VICUS::Network & modNetwork) :
	m_network(modNetwork)
{
	setText( label );
	m_networkIndex = VICUS::elementIndex(theProject().m_geometricNetworks, m_network.m_id);
}


void SVUndoModifyNetwork::undo() {
	IBK_ASSERT(m_networkIndex < project().m_geometricNetworks.size());
	std::swap(theProject().m_geometricNetworks[m_networkIndex], m_network); // exchange network in project with network stored in this class
	theProject().m_geometricNetworks[m_networkIndex].updateNodeEdgeConnectionPointers();
	const SVDatabase & db = SVSettings::instance().m_db;
	theProject().m_geometricNetworks[m_networkIndex].updateVisualizationRadius(db.m_pipes);
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}


void SVUndoModifyNetwork::redo() {
	undo(); // same as undo
}
