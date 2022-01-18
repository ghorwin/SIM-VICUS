/* Common implementation of the FMI Interface Data Structure used by IBK solvers.
	Copyright 2016 Andreas Nicolai, LGPL
*/


#include "fmi2Functions.h"
#include "fmi2FunctionTypes.h"
#include "InstanceDataCommon.h"

#include <IBK_assert.h>
#include <IBK_messages.h>


InstanceDataCommon::InstanceDataCommon(SOLFRA::FMUModelInterface *model) :
	m_callbackFunctions(0),
	m_initializationMode(false),
	m_modelExchange(true),
	m_tInput(0),
	m_externalInputVarsModified(false),
	m_messageHandlerPtr(NULL),
	m_fmuStateSize(0),
	m_outputBehavior(OB_Disabled),
	m_model(model)
{
}


InstanceDataCommon::~InstanceDataCommon() {
	delete m_messageHandlerPtr; // MessageHandler automatically resets DefaultMessageHandler upon deletion
	for (std::set<void*>::iterator it = m_fmuStates.begin(); it != m_fmuStates.end(); ++it) {
		free(*it);
	}
}


// only for ModelExchange
void InstanceDataCommon::updateIfModified() {
	// Implement in derived class. We could have made this function pure/abstract, but we don't want to enforce
	// co-simulation-only models to implement a dummy updateIfModified() function and vice versa.
}


// only for Co-simulation
void InstanceDataCommon::integrateTo(double /*tCommunicationIntervalEnd*/) {
	// Implement in derived class. We could have made this function pure/abstract, but we don't want to enforce
	// co-simulation-only models to implement a dummy updateIfModified() function and vice versa.
}


void InstanceDataCommon::logger(fmi2Status state, fmi2String category, const IBK::FormatString & message) {
	if (m_loggingOn) {
		m_callbackFunctions->logger(m_callbackFunctions->componentEnvironment,
									m_instanceName.c_str(), state, category,
									message.str().c_str());
	}
}


void InstanceDataCommon::setupMessageHandler(const IBK::Path & logfile) {
	const char * const FUNC_ID = "[InstanceDataCommon::setupMessageHandler]";

	if (m_messageHandlerPtr != nullptr)
		throw IBK::Exception(IBK::FormatString("Message handler must not be created/initialized twice."), FUNC_ID);

	unsigned int verbosityLevel = IBK::VL_STANDARD;
	m_messageHandlerPtr = new IBK::MessageHandler;
	m_messageHandlerPtr->setConsoleVerbosityLevel(0); // disable console output
	m_messageHandlerPtr->setLogfileVerbosityLevel((int)verbosityLevel);
	m_messageHandlerPtr->m_contextIndentation = 48;
	std::string errmsg;
	if (!IBK::Path::makePath(logfile.parentPath()))
		throw IBK::Exception(IBK::FormatString("Error creating log directory '%1'.").arg(logfile.parentPath().absolutePath()), FUNC_ID);
	bool success = m_messageHandlerPtr->openLogFile(logfile.str(), false, errmsg);
	if (!success)
		throw IBK::Exception(IBK::FormatString("Error opening logfile '%1': %2").arg(logfile).arg(errmsg), FUNC_ID);
	IBK::MessageHandlerRegistry::instance().setMessageHandler(m_messageHandlerPtr);
}


void InstanceDataCommon::setReal(int varID, double value) {
	m_model->setReal(varID,value);
	m_externalInputVarsModified = true;
}


void InstanceDataCommon::setInteger(int varID, int value) {
	m_model->setInteger(varID,value);
	m_externalInputVarsModified = true;
}


void InstanceDataCommon::setString(int varID, fmi2String value) {
	// special handling for ResultsRootDir parameter
	if (varID == 42)
		m_resultsRootDir = value;
	else
		m_model->setString(varID,value);
	m_externalInputVarsModified = true;
}


void InstanceDataCommon::setBoolean(int varID, bool value) {
	m_model->setBoolean(varID,value);
	m_externalInputVarsModified = true;
}


void InstanceDataCommon::getReal(int varID, double & value) {
	// update procedure for model exchnge
	if(m_modelExchange)
		updateIfModified();
	m_model->getReal(varID,value);
}


void InstanceDataCommon::getInteger(int varID, int & value) {
	// update procedure for model exchnge
	if(m_modelExchange)
		updateIfModified();
	m_model->getInteger(varID,value);
}


void InstanceDataCommon::getString(int varID, fmi2String & value) {
	// special handling for results root dir
	if (varID == 42)
		value = &m_resultsRootDir[0];
	else {
		// update procedure for model exchnge
		if(m_modelExchange)
			updateIfModified();
		m_model->getString(varID,value);
	}
}


void InstanceDataCommon::getBoolean(int varID, bool & value) {
	// update procedure for model exchnge
	if(m_modelExchange)
		updateIfModified();
	m_model->getBoolean(varID,value);
}


void InstanceDataCommon::completedIntegratorStep() {
	// this function must only be called in ModelExchange mode!!!
	IBK_ASSERT(m_modelExchange);
	updateIfModified();
	// signal model that a step was completed in ModelExchange model
	m_model->completedIntegratorStep(m_tInput, &m_yInput[0]);
}

