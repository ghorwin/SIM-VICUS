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

#include "SVViewState.h"


bool SVViewState::operator!=(const SVViewState & other) const {

	if (m_sceneOperationMode != other.m_sceneOperationMode)
		return true;
	if (m_propertyWidgetMode != other.m_propertyWidgetMode)
		return true;
	if (m_objectColorMode != other.m_objectColorMode)
		return true;
	if (m_colorModePropertyID != other.m_colorModePropertyID)
		return true;
	if (m_snapOptionMask != other.m_snapOptionMask)
		return true;
	if (m_snapEnabled != other.m_snapEnabled)
		return true;
	if (m_locks != other.m_locks)
		return true;

	return false;
}
