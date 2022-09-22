#include "VICUS_SupplySystem.h"

#include "VICUS_Project.h"
#include "VICUS_utilities.h"

#include <QFileInfo>


namespace VICUS {

AbstractDBElement::ComparisonResult SupplySystem::equal(const AbstractDBElement * other) const
{
	const SupplySystem * otherSupSys = dynamic_cast<const SupplySystem*>(other);
	if (otherSupSys == nullptr)
		return Different;

	// check parameters

	if (m_supplyType != otherSupSys->m_supplyType ||
		m_supplyFMUPath != otherSupSys->m_supplyFMUPath ||
		m_idSupplyFMU != otherSupSys->m_idSupplyFMU ||
		m_idSubNetwork != otherSupSys->m_idSubNetwork)
		return Different;

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (m_para[i] != otherSupSys->m_para[i])
			return Different;
	}

	// check meta data

	if (m_displayName != otherSupSys->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}


bool SupplySystem::isValid(const Database<SubNetwork> &subNetworkDB,
						   const Database<NetworkComponent> &compDB,
						   const Database<NetworkController> &ctrlDB,
						   const Database<Schedule> &scheduleDB) const
{
	FUNCID(SupplySystem::isValid);
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
			if(m_idSupplyFMU == VICUS::INVALID_ID) {
				throw IBK::Exception("Database FMU is not set!", FUNC_ID);
			}

			m_para[P_MaximumMassFluxFMU].checkedValue("MaximumMassFluxFMU", "kg/s", "kg/s", 0.0, false,
												   std::numeric_limits<double>::max(), false,
												   "Maximum mass flux must be > 0");
			m_para[P_HeatingPowerFMU].checkedValue("HeatingPowerFMU", "kW", "kW", 0.0, false,
												   std::numeric_limits<double>::max(), false,
												   "Maximum heating power flux must be > 0");
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

			m_para[P_MaximumMassFluxFMU].checkedValue("MaximumMassFluxFMU", "kg/s", "kg/s", 0.0, false,
												   std::numeric_limits<double>::max(), false,
												   "Maximum mass flux must be > 0");
			m_para[P_HeatingPowerFMU].checkedValue("HeatingPowerFMU", "kW", "kW", 0.0, false,
												   std::numeric_limits<double>::max(), false,
												   "Maximum heating power flux must be > 0");
			return true;
		}

		// check sub network
		if (m_supplyType == ST_SubNetwork) {
			m_subNetwork = subNetworkDB[m_idSubNetwork];
			if (m_subNetwork != nullptr) {
				if (m_subNetwork->isValid(compDB, ctrlDB, scheduleDB))
					return true;
			}
		}

		return false;

	} catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		m_errorMsg = ex.what();
		return false;
	}
}



} // namespace VICUS
