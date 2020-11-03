/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NANDRAD_EmbeddedObjectWindow.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>


#include "NANDRAD_Constants.h"

namespace NANDRAD {

bool EmbeddedObjectWindow::operator!=(const EmbeddedObjectWindow & other) const {
	// model comparison
	if (m_glazingSystemID != other.m_glazingSystemID)		return true;
	if (m_frame != other.m_frame)							return true;
	if (m_divider != other.m_divider)						return true;

	return false; // not different
}


void EmbeddedObjectWindow::checkParameters(double maxArea,
										   const std::vector<Material> & materials,
										   const std::vector<WindowGlazingSystem> & glazingSystems) const
{
	FUNCID(EmbeddedObjectWindow::checkParameters);

	// only check parameters if model is enabled
	if (!hasParameters())
		return;

	double frameDividerArea = 0;
	if (m_frame.m_materialID != INVALID_ID) {
		frameDividerArea +=
			m_frame.m_area.checkedValue("m2", "m2", 0, true, std::numeric_limits<double>::max(), true,
									   "Cross section area of frame must be >= 0 m2.");
	}
	if (m_divider.m_materialID != INVALID_ID) {
		frameDividerArea +=
			m_divider.m_area.checkedValue("m2", "m2", 0, true, std::numeric_limits<double>::max(), true,
									   "Cross section area of dividers must be >= 0 m2.");
	}

	// check that sum doesn't exceed limit
	if (frameDividerArea >= maxArea) {
		throw IBK::Exception( IBK::FormatString("Cross section of frame and divider "
												"(= %1 m2) exceeds cross section %2 m2 of embedded object.")
							  .arg(frameDividerArea).arg(maxArea), FUNC_ID);
	}
}


} // namespace NANDRAD

