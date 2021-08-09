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
#include <IBK_messages.h>

namespace NANDRAD {

void Zone::checkParameters() const {
	FUNCID(Zone::checkParameters);

	switch (m_type) {
		// require parameter 'Temperature' for Zone attribute 'Constant'
		// this parameter may be overwritten by a schedule later
		case ZT_Constant : {
			m_para[P_Temperature].checkedValue("Temperature", "K", "K", 173.15, false, std::numeric_limits<double>::max(), false,
											   "Parameter 'Temperature' must be > -100 C.");
		} break;

		case ZT_Active : {
			m_para[P_Volume].checkedValue( "Volume", "m3", "m3", 0, false, std::numeric_limits<double>::max(), false,
										   "Volume must be > 0 m3!");

			m_para[P_Area].checkedValue("Area", "m2", "m2", 0, true, std::numeric_limits<double>::max(), true,
										"Zone area must be >= 0 W/m2!");

			// warn if temperature parameter is given in active zone
			if (!m_para[P_Temperature].name.empty())
				IBK::IBK_Message("Temperature parameter in active zone ignored. Using global default initial temperature "
								 "from simulation parameters.", IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);

			// optional parameters
			if (!m_para[P_HeatCapacity].name.empty())
				m_para[P_HeatCapacity].checkedValue( "HeatCapacity", "J/K", "J/K", 0, true, std::numeric_limits<double>::max(), false,
											   "Heat capacity must be >= 0 J/K!");

		} break;

		case ZT_Ground : {
			// TODO : check parameters
		} break;

		case NUM_ZT : ; // just to make compiler happy
	}

	// TODO : check view factors
}

} // namespace NANDRAD
