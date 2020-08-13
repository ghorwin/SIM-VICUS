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

#include "NANDRAD_LinearSplineParameter.h"

namespace NANDRAD {

bool LinearSplineParameter::operator!=(const LinearSplineParameter & other) const {
	if (m_name != other.m_name) return true;
	if (m_interpolationMethod != other.m_interpolationMethod) return true;
	if (m_values != other.m_values) return true;
	if (m_xUnit != other.m_xUnit) return true;
	if (m_yUnit != other.m_yUnit) return true;
	return false;
}

} // namespace NANDRAD

