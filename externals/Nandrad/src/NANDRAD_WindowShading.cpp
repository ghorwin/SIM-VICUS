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

#include "NANDRAD_WindowShading.h"

#include "NANDRAD_ShadingControlModel.h"

namespace NANDRAD {

bool WindowShading::operator!=(const WindowShading & other) const {
	if (m_controlModelId != other.m_controlModelId) return true;
	if (m_modelType != other.m_modelType) return true;
	for (unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	return false;
}


void WindowShading::checkParameters(const std::vector<ShadingControlModel> &controlModels) {
	FUNCID(WindowShading::checkParameters);

	if (m_modelType == NUM_MT)
		return;

	switch (m_modelType) {
		case NUM_MT :
			return; // no check necessary

		case MT_Constant :
			m_para[P_ReductionFactor].checkedValue("ReductionFactor", "---", "---", 0, true, 1, true,
								"Reduction factor must be between 0 and 1.");
		break;

		case MT_Precomputed :
			m_precomputedReductionFactor.checkAndInitialize("PrecomputedReductionFactor", IBK::Unit("s"), IBK::Unit("---"), IBK::Unit("---"), 0, true, 1, true,
								"Reduction factor must be between 0 and 1.");
		break;

		case MT_Controlled :
			if (m_controlModelId == INVALID_ID)
				throw IBK::Exception("Shading model requires reference to shading control model (tag 'ControlModelID')", FUNC_ID);
			// check for existence of shading model parameter block
			std::vector<ShadingControlModel>::const_iterator sit =
					std::find(controlModels.begin(), controlModels.end(), m_controlModelId);

			if(sit == controlModels.end()) {
				throw IBK::Exception(IBK::FormatString("ShadingControlModel with id #%1 does not exist.")
									 .arg(m_controlModelId), FUNC_ID);
			}
		break;
	}
}


} // namespace NANDRAD

