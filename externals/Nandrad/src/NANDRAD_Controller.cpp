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

#include "NANDRAD_Controller.h"

#include "NANDRAD_KeywordList.h"


namespace NANDRAD {

void Controller::checkParameters() {
	FUNCID(Controller::checkParameters);

	try {
		// decide which parameters are needed
		switch (m_modelType) {
			case MT_DigitalDirect:
			case MT_DigitalHysteresis:
				break;
			case MT_PController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;
			case MT_PIController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Ki].checkedValue("Ki", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;
			default: break;
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for controller %1.")
				 .arg(KeywordList::Keyword("Controller::Type", m_modelType)),
				 FUNC_ID);
	}
}

}
