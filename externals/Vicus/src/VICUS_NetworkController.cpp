#include "VICUS_NetworkController.h"

#include "VICUS_KeywordList.h"

namespace VICUS {


bool NetworkController::isValid(const Database<Schedule> &scheduleDB) const {
	// call check function of NANDRAD::HydraulicNetworkControlElement
	try {
		// TODO Hauke:
		// we should know the zone ids here for complete check!
		checkParameters();
	} catch (...) {
		return false;
	}

	// check if schedule exists
	if (m_modelType == MT_Scheduled){
		const Schedule * setPointSched = scheduleDB[m_idReferences[VICUS::NetworkController::ID_Schedule]];
		if (setPointSched == nullptr)
			return false;
		if (!setPointSched->isValid())
			return false;
	}

	return true;
}


AbstractDBElement::ComparisonResult NetworkController::equal(const VICUS::AbstractDBElement *other) const {
	const NetworkController * otherCtrl = dynamic_cast<const NetworkController*>(other);
	if (otherCtrl == nullptr)
		return Different;

	// check important parameters

	if (m_modelType != otherCtrl->m_modelType
		|| m_controllerType != otherCtrl->m_controllerType
		|| m_controlledProperty != otherCtrl->m_controlledProperty
		|| m_maximumControllerResultValue > otherCtrl->m_maximumControllerResultValue
		|| m_maximumControllerResultValue < otherCtrl->m_maximumControllerResultValue)
		return Different;

	for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_P; ++i) {
		if (m_para[i] != otherCtrl->m_para[i])
			return Different;
	}

	for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_ID; ++i) {
		if (m_idReferences[i] != otherCtrl->m_idReferences[i])
			return Different;
	}


	//check meta data
	if (m_displayName != otherCtrl->m_displayName ||
		m_color != otherCtrl->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}



void NetworkController::checkParameters() const {
	FUNCID(HydraulicNetworkControlElement::checkParameters);

	// NOTE: the check below is unecessary - should be ensured already through the "xml:required" specification!

	if (m_controlledProperty == NUM_CP)
		throw IBK::Exception("Missing attribute 'controlledProperty'.", FUNC_ID);

	if (m_modelType == NUM_MT)
		throw IBK::Exception("Missing attribute 'modelType'.", FUNC_ID);

	if (m_controlledProperty == CP_PumpOperation){
		if (m_controllerType != CT_OnOffController)
			throw IBK::Exception("Controlled property 'PumpOperation' can only be used with 'OnOffController'.", FUNC_ID);
	}
	else {
		if (!(m_controllerType == CT_PController || m_controllerType == CT_PIController || m_controllerType == CT_PIDController))
			throw IBK::Exception(IBK::FormatString("Controlled property '%1' can only be used with 'PController', 'PIController' or 'PIDController'.")
								 .arg(KeywordList::Keyword("NetworkController::ControlledProperty", m_controlledProperty)),
								 FUNC_ID);
	}

	try {
		// check individual configuations for different controller properties
		switch (m_controlledProperty) {
			case CP_TemperatureDifference:
			case CP_TemperatureDifferenceOfFollowingElement: {
				if (m_controllerType == NUM_CT)
					throw IBK::Exception("Missing attribute 'controllerType'.", FUNC_ID);

				if (m_modelType == MT_Constant)
					m_para[P_TemperatureDifferenceSetpoint].checkedValue("TemperatureDifferenceSetpoint", "K", "K",
						 0, false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case CP_ThermostatValue: {
				if (m_idReferences[ID_ThermostatZone] == NANDRAD::INVALID_ID)
					throw IBK::Exception("Missing 'ThermostatZoneId' for controlled property 'ThermostatValue'!", FUNC_ID);

//				// check validity of thermostat zone
//				std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(), zones.end(), m_idReferences[ID_ThermostatZone]);

//				if (zone_it == zones.end())
//					throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermostatZoneId.")
//										 .arg(m_idReferences[ID_ThermostatZone]), FUNC_ID);
			} break;

			case CP_MassFlux : {
				// we need mass flux, but > 0 (cannot set mass flux to zero)
				if (m_modelType == MT_Constant)
					m_para[P_MassFluxSetpoint].checkedValue("MassFluxSetpoint", "kg/s", "kg/s",
						 0, false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case CP_PumpOperation : {
					m_para[P_HeatLossOfFollowingElementThreshold].checkedValue("HeatLossOfFollowingElementThreshold", "W", "W",
						 0, false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case NUM_CP:
				throw IBK::Exception("Missing or invalid attribute 'controlledProperty'.", FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Missing/invalid parameters.", FUNC_ID);
	}

	try {

		// decide which parameters are needed
		switch (m_controllerType) {

			case CT_PController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;

			case CT_PIController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Ki].checkedValue("Ki", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_RelControllerErrorForIntegratorReset].checkedValue("RelControllerErrorForIntegratorReset", "---", "---", 0, true, 1, true, nullptr);
			} break;

			case CT_PIDController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Ki].checkedValue("Ki", "---", "---", 0, true, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Kd].checkedValue("Kd", "---", "---", 0, true, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_RelControllerErrorForIntegratorReset].checkedValue("RelControllerErrorForIntegratorReset", "---", "---", 0, true, 1, true, nullptr);
			} break;

			case CT_OnOffController: {
				m_para[P_HeatLossOfFollowingElementThreshold].checkedValue("HeatLossOfFollowingElementThreshold",
																"W", "W", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;

			case NUM_CT:
				throw IBK::Exception("Missing or invalid attribute 'controllerType'.", FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Missing/invalid parameters.", FUNC_ID);
	}
}


} // namespace VICUS
