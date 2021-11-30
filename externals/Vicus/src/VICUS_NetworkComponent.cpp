/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#include "VICUS_NetworkComponent.h"
#include "VICUS_KeywordList.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

namespace VICUS {


bool NetworkComponent::isValid(const Database<Schedule> &scheduleDB) const {

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType = nandradNetworkComponentModelType(m_modelType);

	// check standard parameter
	std::vector<unsigned int> paraVec = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	for (unsigned int i: paraVec){
		try {
			NANDRAD::HydraulicNetworkComponent::checkModelParameter(m_para[i], i);
		} catch (...) {
			return false;
		}
	}

	// check additional parameter
	std::vector<unsigned int> paraVecAdd = additionalRequiredParameter(m_modelType);
	for (unsigned int i: paraVecAdd){
		try {
			checkAdditionalParameter(m_para[i], i);
		} catch (...) {
			return false;
		}
	}

	// check integer parameter
	std::vector<unsigned int> paraVecInt = requiredIntParameter(m_modelType);
	for (unsigned int i: paraVecInt){
		try {
			checkIntParameter(m_intPara[i], i);
		} catch (...) {
			return false;
		}
	}

	// check if there is the correct number of schedules and given schedules really exist
	std::vector<std::string> reqSchedules = NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(nandradModelType);
	if (reqSchedules.size() != m_scheduleIds.size())
		return false;
	for (unsigned int id: m_scheduleIds){
		const Schedule *sched = scheduleDB[id];
		if (sched == nullptr)
			return false;
		// TODO: implement sched.isValid() ?
		// problem: does currently not work properly for annual schedules
	}

	// check if required schedules are given
	if (reqSchedules.size() != m_scheduleIds.size())
		return false;

	// pipe properties
	if (hasPipeProperties(m_modelType) && m_pipePropertiesId == INVALID_ID)
		return false;

	return true;
}


AbstractDBElement::ComparisonResult NetworkComponent::equal(const AbstractDBElement *other) const {

	const NetworkComponent * otherNetComp = dynamic_cast<const NetworkComponent*>(other);
	if (otherNetComp == nullptr)
		return Different;

	//check parameters
	for (unsigned int i=0; i<NetworkComponent::NUM_P; ++i){
		if (m_para[i] != otherNetComp->m_para[i])
			return Different;
	}
	for (unsigned int i=0; i<NetworkComponent::NUM_IP; ++i){
		if (m_intPara[i] != otherNetComp->m_intPara[i])
			return Different;
	}

	if (m_modelType != otherNetComp->m_modelType)
		return Different;

	// check data table
	if (m_polynomCoefficients != otherNetComp->m_polynomCoefficients)
		return Different;

	// check schedule ids
	if (m_scheduleIds.size() != otherNetComp->m_scheduleIds.size())
		return Different;
	for (unsigned int i=0; i<m_scheduleIds.size(); ++i){
		if (m_scheduleIds[i] != otherNetComp->m_scheduleIds[i])
			return Different;
	}

	//check meta data
	if (m_displayName != otherNetComp->m_displayName ||
			m_color != otherNetComp->m_color ||
			m_dataSource != otherNetComp->m_dataSource ||
			m_manufacturer != otherNetComp->m_manufacturer ||
			m_notes != otherNetComp->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


NANDRAD::HydraulicNetworkComponent::ModelType NetworkComponent::nandradNetworkComponentModelType(NetworkComponent::ModelType modelType) {
	// special case handling
	if (modelType == MT_HorizontalGroundHeatExchanger)
		return NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe;
	// default
	Q_ASSERT((unsigned int)modelType <= (unsigned int)NANDRAD::HydraulicNetworkComponent::NUM_MT);
	return NANDRAD::HydraulicNetworkComponent::ModelType(modelType);
}


void NetworkComponent::nandradNetworkComponentParameter(IBK::Parameter *para) const {
	for (unsigned int i=0; i<P_LengthOfGroundHeatExchangerPipes; ++i)
		para[i] = m_para[i];
}


std::vector<unsigned int> NetworkComponent::additionalRequiredParameter(const NetworkComponent::ModelType modelType) {
	// we use switch for maintanance reasons
	switch (modelType) {
		case MT_SimplePipe:
		case MT_DynamicPipe:
		case MT_ConstantPressurePump:
		case MT_ConstantMassFluxPump:
		case MT_ControlledPump:
		case MT_VariablePressurePump:
		case MT_HeatExchanger:
		case MT_HeatPumpIdealCarnotSourceSide:
		case MT_HeatPumpIdealCarnotSupplySide:
		case MT_HeatPumpRealSourceSide:
		case MT_ControlledValve:
		case MT_IdealHeaterCooler:
		case MT_ConstantPressureLossValve:
		case MT_PressureLossElement:
		case NUM_MT:
			break;
		case MT_HorizontalGroundHeatExchanger:
			return {P_LengthOfGroundHeatExchangerPipes};
	}
	return {};
}


std::vector<unsigned int> NetworkComponent::optionalParameter(const NetworkComponent::ModelType modelType) {
	// we use switch for maintanance reasons
	switch (modelType) {
		case MT_SimplePipe:
		case MT_DynamicPipe:
		case MT_ConstantMassFluxPump:
		case MT_ControlledPump:
		case MT_HeatExchanger:
		case MT_HeatPumpIdealCarnotSourceSide:
		case MT_HeatPumpIdealCarnotSupplySide:
		case MT_HeatPumpRealSourceSide:
		case MT_ControlledValve:
		case MT_IdealHeaterCooler:
		case MT_ConstantPressureLossValve:
		case MT_PressureLossElement:
		case MT_HorizontalGroundHeatExchanger:
		case NUM_MT:
			break;
		case MT_ConstantPressurePump:
		case MT_VariablePressurePump:
			return {P_MaximumPressureHead, P_PumpMaximumElectricalPower, P_FractionOfMotorInefficienciesToFluidStream};
	}
	return {};
}


std::vector<unsigned int> NetworkComponent::requiredIntParameter(const NetworkComponent::ModelType modelType) {
	// we use switch for maintanance reasons
	switch (modelType) {
		case MT_SimplePipe:
		case MT_DynamicPipe:
		case MT_ConstantPressurePump:
		case MT_ConstantMassFluxPump:
		case MT_ControlledPump:
		case MT_VariablePressurePump:
		case MT_HeatExchanger:
		case MT_HeatPumpIdealCarnotSourceSide:
		case MT_HeatPumpIdealCarnotSupplySide:
		case MT_HeatPumpRealSourceSide:
		case MT_ControlledValve:
		case MT_IdealHeaterCooler:
		case MT_ConstantPressureLossValve:
		case NUM_MT:
			break;
		case MT_HorizontalGroundHeatExchanger:
			return {IP_NumberParallelPipes};
		case MT_PressureLossElement:
			return {IP_NumberParallelElements};
	}
	return {};
}

void NetworkComponent::checkAdditionalParameter(const IBK::Parameter & para, const unsigned int numPara) {
	const char * enumName = "NetworkComponent::para_t";
	const char * name = KeywordList::Keyword(enumName, (int)numPara);
	const char * unit = KeywordList::Unit(enumName, (int)numPara);
	// we use switch for maintanance reasons
	switch ((para_t)numPara) {
		case P_HydraulicDiameter:
		case P_Volume:
		case P_MaximumHeatingPower:
		case P_MaximumPressureHead:
		case P_PumpMaximumElectricalPower:
		case P_PipeMaxDiscretizationWidth:
		case P_MassFlux:
		case P_PressureLoss:
		case P_PressureLossCoefficient:
		case P_CarnotEfficiency:
		case P_PumpEfficiency:
		case P_FractionOfMotorInefficienciesToFluidStream:
		case P_PressureHead:
		case P_PressureHeadReductionFactor:
		case P_DesignPressureHead:
		case P_DesignMassFlux:
		{
			Q_ASSERT(false); // it is not intended to check these parameters here
		} break;
		case P_LengthOfGroundHeatExchangerPipes:
			para.checkedValue(name, unit, unit, 0, false, std::numeric_limits<double>::max(), true, nullptr);
		break;
		case NUM_P: break;
	}
}


void NetworkComponent::checkIntParameter(const IBK::IntPara & para, const unsigned int numPara) {
	FUNCID(NetworkComponent::checkIntParameter);
	const char * enumName = "NetworkComponent::para_t";
	const char * name = KeywordList::Keyword(enumName, (int)numPara);
	switch (numPara) {
		case IP_NumberParallelPipes:
		case IP_NumberParallelElements:
		{
			if (para.value < 1)
				throw IBK::Exception(IBK::FormatString("% must be > 1").arg(name), FUNC_ID);
		} break;
		case NUM_IP: break;
	}

}


bool NetworkComponent::hasPipeProperties(const NetworkComponent::ModelType modelType)
{
	switch (modelType) {
		case MT_ConstantPressurePump:
		case MT_ConstantMassFluxPump:
		case MT_ControlledPump:
		case MT_VariablePressurePump:
		case MT_HeatExchanger:
		case MT_HeatPumpIdealCarnotSourceSide:
		case MT_HeatPumpIdealCarnotSupplySide:
		case MT_HeatPumpRealSourceSide:
		case MT_ControlledValve:
		case MT_IdealHeaterCooler:
		case MT_ConstantPressureLossValve:
		case MT_PressureLossElement:
		case NUM_MT:
			return false;
		case MT_SimplePipe:
		case MT_DynamicPipe:
		case MT_HorizontalGroundHeatExchanger:
			return true;
	}
	return false; // just for compiler
}


} // namespace VICUS
