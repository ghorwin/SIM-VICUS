#include "NANDRAD_WindowShading.h"

namespace NANDRAD {

bool WindowShading::operator!=(const WindowShading & other) const {
	if (m_controlModelID != other.m_controlModelID) return true;
	if (m_modelType != other.m_modelType) return true;
	for (unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	return false;
}

void WindowShading::checkParameters() {
	FUNCID(WindowShading::checkParameters);

	if (m_modelType == NUM_MT)
		return;

	switch (m_modelType) {
		case NUM_MT :
			return; // only check if enabled

		case MT_Standard :
			if (m_controlModelID == INVALID_ID)
				throw IBK::Exception("Shading model requires reference to shading control model (tag 'ControlModelID')", FUNC_ID);
			m_para[P_ReductionFactor].checkedValue("ReductionFactor", "%", "%", 0, true, 1, true,
								"Reduction factor be between 0 and 1.");
		break;

		case MT_Precomputed :
			/// TODO
		break;

		case MT_Controlled :
			/// TODO
		break;
	}
}


} // namespace NANDRAD

