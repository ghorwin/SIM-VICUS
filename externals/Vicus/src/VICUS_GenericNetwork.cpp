#include "VICUS_GenericNetwork.h"

#include "VICUS_Project.h"
#include "VICUS_utilities.h"

#include <QFileInfo>


namespace VICUS {

bool GenericNetwork::isValid() const
{
	FUNCID(GenericNetwork::isValid);
	try {

		// check parameters for stand alone mode
		if(m_supplyType == ST_StandAlone) {
			m_para[P_MaximumMassFlux].checkedValue("MaximumMassFlux", "kg/s", "kg/s", 0.0, false,
												   std::numeric_limits<double>::max(), false,
												   "Maximum mass flux must be > 0");
			m_para[P_SupplyTemperature].checkedValue("SupplyTemperature", "C", "C", 0.0, false,
												   std::numeric_limits<double>::max(), false,
												   "Supply temperature must above water freezing point.");
			return true;
		}

		// check parameters for database mode

		if(m_supplyType == ST_DatabaseFMU) {
			if(m_supplyFMUId == VICUS::INVALID_ID) {
				throw IBK::Exception("Database FMU is not set!", FUNC_ID);
			}
		}

		// check parameters for user defined mode
		if(m_supplyType == ST_UserDefinedFMU) {
			if(m_supplyFMUPath.isEmpty() ) {
				throw IBK::Exception("Path to user defined FMU is not set!", FUNC_ID);
			}

			QFileInfo fileInfo(m_supplyFMUPath);
			if( !fileInfo.exists() || fileInfo.suffix() != "fmu") {
				throw IBK::Exception(QString("Invalid path %1 to user defined FMU!").arg(m_supplyFMUPath).toStdString(),
									 FUNC_ID);
			}
			return true;
		}
		return false;

	} catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return false;
	}
}


} // namespace VICUS
