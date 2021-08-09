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

#include "NANDRAD_Zone.h"

#include "NANDRAD_KeywordList.h"


namespace NANDRAD {

void Zone::checkParameters()
{
	switch(m_type) {
		// enforce parameter 'Tempertaure' for Zone attribute 'Constant'
		// this parametzer may be overwritten by a schedule later
		case ZT_Constant: {
			if(!m_para[P_Temperature].name.empty()) {
				m_para[P_Temperature].checkedValue("Temperature", "K", "K", 0, false, std::numeric_limits<double>::max(), false,
												   "Parameter 'Temperature' must be > 0 K.");
			}
		} break;
		default: break;
	}
}

} // namespace NANDRAD
