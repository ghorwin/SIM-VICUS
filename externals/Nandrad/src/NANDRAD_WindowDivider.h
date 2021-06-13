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

#ifndef NANDRAD_WindowDividerH
#define NANDRAD_WindowDividerH

#include <algorithm>
#include <numeric>

#include <IBK_Parameter.h>
#include <IBK_Exception.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_Material.h"

namespace NANDRAD {

/*!	WindowDivider defines the divider of a window. */
class WindowDivider  {
	NANDRAD_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(WindowDivider)
	NANDRAD_COMP(WindowDivider)

	void checkParameters(const std::vector<Material> & materials);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Material used for divider (INVALID_ID for no divider). */
	unsigned int	m_materialId = INVALID_ID;						// XML:A:required
	/*! Cross section occupied by all divider elements in this window (required when m_materialID is given). */
	IBK::Parameter	m_area;											// XML:E:required
	/*! Thickness of frame */
	IBK::Parameter	m_thickness;									// XML:E:required

	// *** Variables used only during simulation ***

	/*! Cached thermal conductivity in [W/mK], retrieved from referenced material in checkParameters(). */
	double			m_lambda = 999;

}; // WindowDivider


inline bool WindowDivider::operator!=(const WindowDivider & other) const {
	if (m_materialId != other.m_materialId) return true;
	if (m_area != other.m_area) return true;
	return false;
}


inline void WindowDivider::checkParameters(const std::vector<Material> & materials) {
	FUNCID(WindowDivider::checkParameters);

	if (m_materialId == INVALID_ID)
		return;
	// search material list for required material
	std::vector<Material>::const_iterator it = std::find(materials.begin(), materials.end(), m_materialId);
	if (it == materials.end())
		throw IBK::Exception(IBK::FormatString("Material with ID %1 not defined.").arg(m_materialId), FUNC_ID);
	m_lambda = it->m_para[Material::P_Conductivity].value;

	m_area.checkedValue("Area", "m2", "m2", 0, true, (std::numeric_limits<double>::max)(), true,
						"Cross section area of dividers must be >= 0 m2.");
	m_thickness.checkedValue("Thickness", "m", "m", 0, false, (std::numeric_limits<double>::max)(), true,
						"Thickness of dividers must be > 0 m.");
}

} // namespace NANDRAD

#endif // NANDRAD_WindowDividerH
