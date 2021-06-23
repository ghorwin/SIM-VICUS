#include "VICUS_NetworkController.h"

namespace VICUS {

}

VICUS::NetworkController::NetworkController()
{
}

bool VICUS::NetworkController::isValid(const Database<Schedule> &scheduleDB) const
{
	// call check function of NANDRAD::HydraulicNetworkControlElement
	try {
		// TODO Hauke:
		// we should know the zone ids here for complete check!
		checkParameters(nullptr);
	} catch (IBK::Exception) {
		return false;
	}

	// check if schedule exists
	if (m_modelType == MT_Scheduled){
		const Schedule * setPointSched = scheduleDB[m_scheduleId];
		if (setPointSched == nullptr)
			return false;
		if (!setPointSched->isValid())
			return false;
	}

	return true;
}


VICUS::AbstractDBElement::ComparisonResult VICUS::NetworkController::equal(const VICUS::AbstractDBElement *other) const
{
	const NetworkController * otherCtrl = dynamic_cast<const NetworkController*>(other);
	if (otherCtrl == nullptr)
		return Different;

	// check important parameters

	if (m_id != otherCtrl->m_id)
		return Different;

	if (m_modelType != otherCtrl->m_modelType
		|| m_controllerType != otherCtrl->m_controllerType
		|| m_controlledProperty != otherCtrl->m_controlledProperty
		|| m_scheduleId != otherCtrl->m_scheduleId
		|| m_maximumControllerResultValue > otherCtrl->m_maximumControllerResultValue
		|| m_maximumControllerResultValue < otherCtrl->m_maximumControllerResultValue)
		return Different;

	for(unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_P; ++i){
		if(m_para[i] != otherCtrl->m_para[i])
			return Different;
	}

	for(unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_ID; ++i){
		if(m_idReferences[i] != otherCtrl->m_idReferences[i])
			return Different;
	}


	//check meta data
	if (m_displayName != otherCtrl->m_displayName ||
		m_color != otherCtrl->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}
