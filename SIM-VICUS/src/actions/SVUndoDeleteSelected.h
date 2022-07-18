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

#ifndef SVUndoDeleteSelectedH
#define SVUndoDeleteSelectedH

#include <VICUS_Project.h>

#include "SVUndoCommandBase.h"

/*! An undo action for deleting selected geometric shapes (things that are shown in the scene). */
class SVUndoDeleteSelected : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteSelected)
public:
	SVUndoDeleteSelected(const QString & label, const std::set<const VICUS::Object*> & objectIDsToBeRemoved);

	virtual void undo();
	virtual void redo();

private:

	/*! Stores vector of component instances. */
	std::vector<VICUS::ComponentInstance>				m_compInstances;
	/*! Stores vector of sub-surface component instances. */
	std::vector<VICUS::SubSurfaceComponentInstance>		m_subCompInstances;
	/*! Stores vector of buildings. */
	std::vector<VICUS::Building>						m_buildings;
	/*! Stores vector of anonymous surfaces. */
	std::vector<VICUS::Surface>							m_plainGeometry;
	/*! Stores vector of networks. */
	std::vector<VICUS::Network>							m_networks;

};


#endif // SVUndoDeleteSelectedH
