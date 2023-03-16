/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
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
	if (m_glazingSystemId != other.m_glazingSystemId)		return true;
	if (m_frame != other.m_frame)							return true;
	if (m_divider != other.m_divider)						return true;

	return false; // not different
}


void EmbeddedObjectWindow::checkParameters(double grossArea,
										   const std::vector<Material> & materials,
										   const std::vector<WindowGlazingSystem> & glazingSystems,
										   const std::vector<ShadingControlModel> & controlModels)
{
	FUNCID(EmbeddedObjectWindow::checkParameters);

	// only check parameters if model is enabled
	if (!hasParameters())
		return;

	m_area = grossArea;
	try {
		m_frame.checkParameters(materials);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error in window frame parameters.", FUNC_ID);
	}
	try {
		m_divider.checkParameters(materials);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error in window dividers parameters.", FUNC_ID);
	}
	try {
		m_shading.checkParameters(controlModels);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error in window shading parameters.", FUNC_ID);
	}

	double frameDividerArea = 0;
	if (m_frame.m_materialId != INVALID_ID)
		frameDividerArea += m_frame.m_area.value;
	if (m_divider.m_materialId != INVALID_ID)
		frameDividerArea += m_divider.m_area.value;

	// check that sum doesn't exceed limit
	if (frameDividerArea >= grossArea) {
		throw IBK::Exception( IBK::FormatString("Cross section of frame and divider "
												"(= %1 m2) exceeds cross section %2 m2 of embedded object.")
							  .arg(frameDividerArea).arg(grossArea), FUNC_ID);
	}

	m_glasArea = grossArea - frameDividerArea;

	// now lookup glazing system
	std::vector<WindowGlazingSystem>::const_iterator it = std::find(glazingSystems.begin(), glazingSystems.end(), m_glazingSystemId);
	if (it == glazingSystems.end())
		throw IBK::Exception(IBK::FormatString("Glazing system with ID %1 not defined.").arg(m_glazingSystemId), FUNC_ID);
	// cache pointer
	m_glazingSystem = &(*it);


	// TODO: epsilon here
	// TODO: if epsilon > 0 check view factors here
}


} // namespace NANDRAD

