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

#ifndef SVUndoModifySubSurfaceGeometryH
#define SVUndoModifySubSurfaceGeometryH

#include <VICUS_Surface.h>

#include "SVUndoCommandBase.h"

/*! Action for modifying a surface. */
class SVUndoModifySubSurfaceGeometry : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySurfaceGeometry)
public:
	SVUndoModifySubSurfaceGeometry(const QString & label, const std::vector<VICUS::Surface> & surfaces,
									const std::vector<VICUS::SubSurfaceComponentInstance> * subSurfaceComponentInstances = nullptr);

	virtual void undo();
	virtual void redo();

private:

	/*! Object copies of surfaces with modified subsurfaces. */
	std::vector<VICUS::Surface>									m_surfaces;

	/*! True, if we also modify the sub-surface component instances (i.e. we add windows). */
	bool														m_modifySubSurfaceComponentInstances = false;
	/*! Copies of modified sub-surface component instances. */
	std::vector<VICUS::SubSurfaceComponentInstance>				m_subSurfaceComponentInstances;
};


#endif // SVUndoModifySurfaceGeometryH
