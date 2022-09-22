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
	FUNCID(NetworkComponent::isValid);

	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType = nandradNetworkComponentModelType(m_modelType);

	// check standard parameter
	std::vector<unsigned int> paraVec = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, 1);
	for (unsigned int i: paraVec){
		try {
			NANDRAD::HydraulicNetworkComponent::checkModelParameter(m_para[i], i);
		} catch (...) {
			m_errorMsg = "Standard parameters of component are invalid.";
			return false;
		}
	}

	// check additional parameter
	std::vector<unsigned int> paraVecAdd = additionalRequiredParameter(m_modelType);
	for (unsigned int i: paraVecAdd){
		try {
			checkAdditionalParameter(m_para[i], i);
		} catch (...) {
			m_errorMsg = "Aditional parameters of component are invalid.";
			return false;
		}
	}

	// check integer parameter
	std::vector<unsigned int> paraVecInt = requiredIntParameter(m_modelType);
	for (unsigned int i: paraVecInt){
		try {
			checkIntParameter(m_intPara[i], i);
		} catch (...) {
			m_errorMsg = "Integer parameters of component are invalid.";
			return false;
		}
	}

	// check if there is the correct number of schedules and given schedules really exist
	std::vector<std::string> reqSchedules = NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(nandradModelType);
	if (reqSchedules.size() != m_scheduleIds.size())
		return false;
	for (unsigned int id: m_scheduleIds){
		const Schedule *sched = scheduleDB[id];
		if (sched == nullptr) {
			m_errorMsg = "Schedule is not set properly.";
			return false;
		}
		// TODO: implement sched.isValid() ?
		// problem: does currently not work properly for annual schedules
	}

	// check if required schedules are given
	if (reqSchedules.size() != m_scheduleIds.size()) {
		m_errorMsg = "Required schedules are not properly set.";
		return false;
	}

	// pipe properties
	if (hasPipeProperties(m_modelType) && m_pipePropertiesId == INVALID_ID) {
		m_errorMsg = "Pipe properties are not set.";
		return false;
	}


	try {
		// check data table heat pumps
		if (m_modelType == MT_HeatPumpOnOffSourceSide ) { //|| m_modelType == MT_HeatPumpOnOffSourceSideWithBuffer) {
			if (m_polynomCoefficients.m_values.at("QdotCondensator").size() != 6)
				throw IBK::Exception(IBK::FormatString("'%1' requires polynom coefficient parameter 'QdotCondensator' with exactly 6 values.")
									 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
			if (m_polynomCoefficients.m_values.at("ElectricalPower").size() != 6)
				throw IBK::Exception(IBK::FormatString("'%1' requires polynom coefficient parameter 'ElectricalPower' with exactly 6 values.")
									 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
		}
		if (m_modelType == MT_HeatPumpVariableSourceSide) {
			if (m_polynomCoefficients.m_values.at("COP").size() != 6)
				throw IBK::Exception(IBK::FormatString("'%1' requires polynom coefficient parameter 'COP' with exactly 6 values.")
									 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
		}

		// check data table pumps:
		// if there is a data table, it should have correct entries
		// if correct entries exist, they should have correct number of parameters
		if (m_modelType == MT_ConstantPressurePump || m_modelType == MT_ControlledPump || m_modelType == MT_VariablePressurePump) {
			if (!m_polynomCoefficients.m_values.empty()) {
				if (m_polynomCoefficients.m_values.find("MaximumElectricalPower") == m_polynomCoefficients.m_values.end() ||
					m_polynomCoefficients.m_values.find("MaximumPressureHead") == m_polynomCoefficients.m_values.end() )
					throw IBK::Exception(IBK::FormatString("'%1' data table should contain entries 'MaximumElectricalPower' and 'MaximumPressureHead'.")
										 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
				// data table should have correct number of parameters
				if (m_polynomCoefficients.m_values.at("MaximumElectricalPower").empty() && m_polynomCoefficients.m_values.at("MaximumElectricalPower").size() != 3 )
					throw IBK::Exception(IBK::FormatString("'%1' polynom coefficient parameter 'MaximumElectricalPower' should have exactly 3 values (quadratic polynom).")
										 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
				if (m_polynomCoefficients.m_values.at("MaximumPressureHead").empty() && m_polynomCoefficients.m_values.at("MaximumPressureHead").size() != 3 )
					throw IBK::Exception(IBK::FormatString("'%1' polynom coefficient parameter 'MaximumPressureHead' should have exactly 3 values (quadratic polynom).")
										 .arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
			}
		}

		// for MT_ConstantPressurePump and MT_VariablePressurePump, we enforce existance of complete parameter set (pump efficiency,
		// maximum pressure head and maximum electric power) once one of the parameters or maximum pressure head and maximum electric power
		// is given
		if (m_modelType == MT_ConstantPressurePump || m_modelType == MT_VariablePressurePump) {
			// general case
			if(!m_para[NANDRAD::HydraulicNetworkComponent::P_PumpMaximumElectricalPower].name.empty() &&
					m_para[NANDRAD::HydraulicNetworkComponent::P_MaximumPressureHead].empty())
				throw IBK::Exception("Missing paramneter 'MaximumPressureHead'!", FUNC_ID);
			if(!m_para[NANDRAD::HydraulicNetworkComponent::P_MaximumPressureHead].name.empty() &&
					m_para[NANDRAD::HydraulicNetworkComponent::P_PumpMaximumElectricalPower].empty())
				throw IBK::Exception("Missing paramneter 'PumpMaximumElectricalPower'!", FUNC_ID);
		}

	}  catch (IBK::Exception &ex) {
		m_errorMsg = ex.what();
		return false;
	} catch (std::exception &ex) {
		m_errorMsg = ex.what();
		return false;
	}

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
	// default
	Q_ASSERT((unsigned int)modelType <= (unsigned int)NANDRAD::HydraulicNetworkComponent::NUM_MT);
	return NANDRAD::HydraulicNetworkComponent::ModelType(modelType);
}


void NetworkComponent::nandradNetworkComponentParameter(IBK::Parameter *para) const {
	for (unsigned int i=0; i<P_PipeLength; ++i)
		para[i] = m_para[i];
}


std::vector<unsigned int> NetworkComponent::additionalRequiredParameter(const NetworkComponent::ModelType modelType) {
	switch (modelType) {
		case MT_SimplePipe:
		case MT_DynamicPipe:
			return {P_PipeLength};
		default: break;
	}
	return {};
}


std::vector<unsigned int> NetworkComponent::optionalParameter(const NetworkComponent::ModelType modelType) {
	// we use switch for maintanance reasons
	switch (modelType) {
		case MT_ConstantPressurePump:
		case MT_VariablePressurePump:
			return {P_MaximumPressureHead, P_PumpMaximumElectricalPower, P_FractionOfMotorInefficienciesToFluidStream};
		case MT_HeatExchanger:
			return {P_MinimumOutletTemperature};
		default:;
	}
	return {};
}


std::vector<unsigned int> NetworkComponent::requiredIntParameter(const NetworkComponent::ModelType modelType) {
	switch (modelType) {
		case MT_SimplePipe:
		case MT_DynamicPipe:
			return {IP_NumberParallelPipes};
		case MT_PressureLossElement:
			return {IP_NumberParallelElements};
		default: break;
	}
	return {};
}

void NetworkComponent::checkAdditionalParameter(const IBK::Parameter & para, const unsigned int numPara) {
	const char * enumName = "NetworkComponent::para_t";
	const char * name = KeywordList::Keyword(enumName, (int)numPara);
	const char * unit = KeywordList::Unit(enumName, (int)numPara);
	switch ((para_t)numPara) {
		case P_PipeLength:
			para.checkedValue(name, unit, unit, 0, false, std::numeric_limits<double>::max(), true, nullptr);
		break;
		default: break;
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


bool NetworkComponent::hasPipeProperties(const NetworkComponent::ModelType modelType) {
	switch (modelType) {
		case MT_SimplePipe:
		case MT_DynamicPipe:
			return true;
		default: break;
	}
	return false; // just for compiler
}


} // namespace VICUS
