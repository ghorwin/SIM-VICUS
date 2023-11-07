#include "VICUS_AcousticSoundAbsorptionPartition.h"

namespace VICUS {

bool AcousticSoundAbsorptionPartition::isValid(const VICUS::Database<AcousticSoundAbsorption> &soundAbsorption) const
{

		const AcousticSoundAbsorption * soundAbs = soundAbsorption[m_idSoundAbsorption];
		if (soundAbs == nullptr) {
			m_errorMsg = "Sound absorption with id '" + std::to_string(m_idSoundAbsorption) +"' does not exist.";
			return false; // error, sound absorption with this ID is not found
		}

		if (!soundAbs->isValid()) {
			m_errorMsg = "Sound absorption '" + soundAbs->m_displayName.string("de", true) + "' is not valid.";
			return false;
		}

		try {
			m_para[P_AreaFraction].checkedValue("AreaFraction", "---", "---", 0, true, std::numeric_limits<double>::max(), false,
									 "Area fraction >= 0 is required.");
		} catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			m_errorMsg = "Sound absorption '" + soundAbs->m_displayName.string("de", true) + "' has following error: " + ex.what();
			return false;
		}
		return true;
}


} // namespace VICUS
