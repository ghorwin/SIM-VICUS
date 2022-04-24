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

#ifndef SVUndoModifySiteDataH
#define SVUndoModifySiteDataH

#include <VICUS_Project.h>
#include <VICUS_GridPlane.h>

#include "SVUndoCommandBase.h"

class SVUndoModifySiteData : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySiteData)
public:
	SVUndoModifySiteData(const QString & label,
				   const std::vector<VICUS::GridPlane> & gridPlanes, double farDistance);

	virtual void undo();
	virtual void redo();

private:
	std::vector<VICUS::GridPlane>		m_gridPlanes;
	double								m_farDistance;

};

#endif // SVUndoModifySiteDataH
