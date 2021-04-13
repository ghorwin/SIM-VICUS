#include "NANDRAD_WindowShading.h"

#include "NANDRAD_ShadingControlModel.h"

namespace NANDRAD {

bool WindowShading::operator!=(const WindowShading & other) const {
	if (m_controlModelID != other.m_controlModelID) return true;
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
			m_shadingFactor.checkAndInitialize("ShadingFactor", IBK::Unit("s"), IBK::Unit("---"), IBK::Unit("---"), 0, true, 1, true,
								"Shading factor be between 0 and 1.");
		break;

		case MT_Controlled :
			if (m_controlModelID == INVALID_ID)
				throw IBK::Exception("Shading model requires reference to shading control model (tag 'ControlModelID')", FUNC_ID);
			// check for existence of shading model parameter block
			std::vector<ShadingControlModel>::const_iterator sit =
					std::find(controlModels.begin(), controlModels.end(), m_controlModelID);

			if(sit == controlModels.end()) {
				throw IBK::Exception(IBK::FormatString("ShadingControlModel with id #%1 does not exist.")
									 .arg(m_controlModelID), FUNC_ID);
			}
		break;
	}
}


} // namespace NANDRAD

