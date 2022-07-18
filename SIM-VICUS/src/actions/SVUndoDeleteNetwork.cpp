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

#include "SVUndoDeleteNetwork.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoDeleteNetwork::SVUndoDeleteNetwork(const QString & label, unsigned int networkIndex)
	: m_networkIndex(networkIndex)
{
	setText( label );

	Q_ASSERT(project().m_geometricNetworks.size() > networkIndex);

	m_deletedNetwork = project().m_geometricNetworks[networkIndex];
}


void SVUndoDeleteNetwork::undo() {

	theProject().m_geometricNetworks.insert(theProject().m_geometricNetworks.begin() + m_networkIndex, m_deletedNetwork);
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}


void SVUndoDeleteNetwork::redo() {
	Q_ASSERT(!theProject().m_geometricNetworks.empty());

	theProject().m_geometricNetworks.erase(theProject().m_geometricNetworks.begin() + m_networkIndex);
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}

