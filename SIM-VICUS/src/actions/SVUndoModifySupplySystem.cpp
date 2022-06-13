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

#include "SVUndoModifySupplySystem.h"
#include "SVProjectHandler.h"

#include "SVSettings.h"

SVUndoModifySupplySystem::SVUndoModifySupplySystem(const QString &label, unsigned int supplyIndex, const VICUS::SupplySystem & supply) :
	m_SupplySystemIndex(supplyIndex),
	m_SupplySystem(supply)
{
	setText( label );
}


void SVUndoModifySupplySystem::undo() {
	IBK_ASSERT(m_SupplySystemIndex < project().m_embeddedDB.m_supplySystems.size());
	std::swap(theProject().m_embeddedDB.m_supplySystems[m_SupplySystemIndex], m_SupplySystem); // exchange supply in project with supply stored in this class

	// TODO: check signal
	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}


void SVUndoModifySupplySystem::redo() {
	undo(); // same as undo
}
