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

#include "NANDRAD_ControlElement.h"

#include <algorithm>

#include "NANDRAD_Controller.h"


namespace NANDRAD {


void ControlElement::checkParameters(const std::vector<Controller> &controllers) {
	FUNCID("ControlElement::checkParameters");

	if (m_controlType != NUM_CT){
		auto it = std::find(controllers.begin(), controllers.end(), m_controllerId);
		if (it == controllers.end())
			throw IBK::Exception(IBK::FormatString("Controller with id #%1 does not exist.")
								 .arg(m_controllerId), FUNC_ID);

		// assign controller pointer
		m_controller = &(*it);

		switch (m_controlType) {
			case CT_ControlTemperatureDifference:
				m_setPoint.checkedValue("SetPoint", "K", "K", 0, false, std::numeric_limits<double>::max(), false, nullptr);
				break;
			case CT_ControlMassFlow:
				m_setPoint.checkedValue("SetPoint", "kg/s", "kg/s", 0, false, std::numeric_limits<double>::max(), false, nullptr);
				break;
			case NUM_CT:
				break;
		}

	}


	// TODO: other checks
}


} // namespace NANDRAD
