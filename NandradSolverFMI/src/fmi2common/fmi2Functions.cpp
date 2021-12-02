/*	FMI Interface for Model Exchange and CoSimulation Version 2

	The details of the actual model in question are encapsulated in InstanceData.
*/

#include <memory>
#include <iostream>

#include <IBK_assert.h>
#include <IBK_FormatString.h>
#include <IBK_MessageHandlerRegistry.h>

#ifdef IBK_DEBUG


#define FMI_ASSERT(p)	if (!(p)) \
	{ std::cerr << "Assertion failure\nCHECK: " << #p << "\nFILE:  " << myFilename(__FILE__) << "\nLINE:  " << __LINE__ << '\n'; \
	  return fmi2Error; }

#else

#define FMI_ASSERT(p) (void)0;

#endif //  IBK_DEBUG

#ifdef _WIN32

#if _WIN32_WINNT < 0x0501
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>

#endif // _WIN32

#include "fmi2Functions.h"

// This file is the model-specific implementation and must be in the INCLUDEPATH
#include <InstanceData.h>

/*! Utility class that handles setting of our own message handler and resetting the original upon destruction of the
	object.
*/
class MessageHandlerSwapper {
public:
	/*! Constructor, takes a pointer to our own message handler (does not take ownership!). */
	MessageHandlerSwapper(IBK::MessageHandler * msgHandler) {
		// cache currently set message handler
		m_masterMsgHandler = IBK::MessageHandlerRegistry::instance().messageHandler();
		// set the given message handler
		if (msgHandler != NULL)
			IBK::MessageHandlerRegistry::instance().setMessageHandler(msgHandler);
	}

	~MessageHandlerSwapper() {
		// reset the original message handler
		IBK::MessageHandlerRegistry::instance().setMessageHandler(m_masterMsgHandler);
	}

	IBK::MessageHandler * m_masterMsgHandler;
};

// *** FMI Interface Functions ***


/* Inquire version numbers of header files */


const char* fmi2GetTypesPlatform() {
	// returns platform type, currently "default"
	return fmi2TypesPlatform;
}


const char* fmi2GetVersion() {
	// returns fmi version, currently "2.0"
	return "2.0";
}


// Enables/disables debug logging
fmi2Status fmi2SetDebugLogging(void* c, fmi2Boolean loggingOn, size_t nCategories, const char* const categories[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2SetDebugLogging: logging switched %1.")
						  .arg(loggingOn ? "true" : "false"));
	modelInstance->m_loggingOn = (loggingOn == fmi2True);
	if (modelInstance->m_loggingOn) {
		modelInstance->m_loggingCategories.clear();
		for (size_t i=0; i<nCategories; ++i)
			modelInstance->m_loggingCategories.push_back(std::string(categories[i]));
	}
	return fmi2OK;
}



/* Creation and destruction of FMU instances */


void* fmi2Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String guid,
					  fmi2String fmuResourceLocation,
					  const fmi2CallbackFunctions* functions,
					  fmi2Boolean, fmi2Boolean loggingOn)
{
	// initial checks
	if (functions == NULL)
		return NULL;

	if (functions->logger == NULL)
		return NULL;

	std::string instanceNameString = instanceName;
	if (instanceNameString.empty()) {
		if (loggingOn)
			functions->logger(functions->componentEnvironment, instanceName, fmi2Error, "error", "fmi2Instantiate: Missing instance name.");
		return NULL;
	}

	// check for correct model
	if (std::string(InstanceData::GUID) != guid) {
		functions->logger(functions->componentEnvironment, instanceName, fmi2Error, "error", "fmi2Instantiate: Invalid/mismatching guid.");
		return NULL;
	}

	// instantiate data structure for instance-specific data
	InstanceData * data = new InstanceData;
	// transfer function arguments
	data->m_callbackFunctions = functions;
	data->m_instanceName = instanceName;
	data->m_modelExchange = (fmuType == fmi2ModelExchange);
	data->m_resourceLocation = fmuResourceLocation;
	data->m_loggingOn = loggingOn==1;

	// return data pointer
	return data;
}


// Free allocated instance data structure
void fmi2FreeInstance(void* c) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	modelInstance->finish();
	modelInstance->logger(fmi2OK, "progress", "fmi2FreeInstance: Model instance deleted.");
	delete modelInstance;
}


/* Enter and exit initialization mode, terminate and reset */


// Overrides project settings?
fmi2Status fmi2SetupExperiment(void* c, int, double, double startTime,
							   int, double)
{
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	modelInstance->logger(fmi2OK, "progress", "fmi2SetupExperiment: Call of setup experiment.");
	// store experiment specs
	modelInstance->m_tStart = startTime;
	return fmi2OK;
}


// All scalar variables with initial="exact" or "approx" can be set before
// fmi2SetupExperiment has to be called at least once before
fmi2Status fmi2EnterInitializationMode(void* c) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	modelInstance->logger(fmi2OK, "progress", "fmi2EnterInitializationMode: Go into initialization mode.");
	modelInstance->m_initializationMode = true;
	// store currently set IBK message handler
	IBK::MessageHandler * currentMsgHandler = IBK::MessageHandlerRegistry::instance().messageHandler();
	// let instance data initialize everything that's needed
	// now the output directory parameter should be set
	try {
		modelInstance->init(); // here, most IBK-based models will create and register their own message handler
		// compute and cache serialization size, might be zero if serialization is not supported
		if (!modelInstance->m_modelExchange)
			modelInstance->computeFMUStateSize();
		else
			// in ModelExchange mode we need to write outputs after each step
			/// \todo Revise output behavior.
			modelInstance->m_outputBehavior = InstanceDataCommon::OB_EveryCompletedStep;


		// reset original message handler
		IBK::MessageHandlerRegistry::instance().setMessageHandler(currentMsgHandler);

		// init successful
		return fmi2OK;
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		// reset original message handler
		IBK::MessageHandlerRegistry::instance().setMessageHandler(currentMsgHandler);
		modelInstance->logger(fmi2Error, "error", "Model initialization failed.");
		return fmi2Error;
	}
}


// Switch off all initialization equations
fmi2Status fmi2ExitInitializationMode(void* c) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	modelInstance->logger(fmi2OK, "progress", "fmi2ExitInitializationMode: Go out from initialization mode.");
	modelInstance->m_initializationMode = false;
	return fmi2OK;
}


fmi2Status fmi2Terminate(void* c) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	modelInstance->finish();
	modelInstance->logger(fmi2OK, "progress", "fmi2Terminate: Terminate model.");
	return fmi2OK;
}


fmi2Status fmi2Reset(void* c) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	modelInstance->logger(fmi2Warning, "warning", "fmi2Reset: Reset the whole model to default. Not implemented yet.");
	return fmi2OK;
}



/* Getting and setting variables values */

fmi2Status fmi2GetReal(void* c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			modelInstance->getReal(vr[i], value[i]);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2GetReal() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


fmi2Status fmi2GetInteger(void* c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			modelInstance->getInteger(vr[i], value[i]);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2GetInteger() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


fmi2Status fmi2GetBoolean(void* c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			bool val;
			modelInstance->getBoolean(vr[i], val);
			value[i] = val;
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2GetBoolean() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


fmi2Status fmi2GetString(void* c, const fmi2ValueReference vr[], size_t nvr, fmi2String value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			modelInstance->getString(vr[i], value[i]);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2GetString() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


fmi2Status fmi2SetReal (void* c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			modelInstance->setReal(vr[i], value[i]);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2SetReal() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


fmi2Status fmi2SetInteger(void* c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			modelInstance->setInteger(vr[i], value[i]);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2SetInteger() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


fmi2Status fmi2SetBoolean(void* c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			modelInstance->setBoolean(vr[i], value[i]==1);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2SetBoolean() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


fmi2Status fmi2SetString(void* c, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	for (size_t i=0; i<nvr; ++i) {
		try {
			modelInstance->setString(vr[i], value[i]);
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			modelInstance->logger(fmi2Error, "error",
								  IBK::FormatString("Error in fmi2SetString() for vr=%1 and value=%2.")
								  .arg(vr[i]).arg(value[i]).str().c_str());
			return fmi2Error;
		}
	}
	return fmi2OK;
}


/* Getting and setting the internal FMU state */

fmi2Status fmi2GetFMUstate(void* c, fmi2FMUstate* FMUstate) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);

	if (modelInstance->m_fmuStateSize == 0) {
		modelInstance->logger(fmi2Error, "error", "fmi2GetFMUstate is called though FMU was not yet completely set up "
							  "or serialization is not supported by this FMU.");
		return fmi2Error;
	}

	// check if new alloc is needed
	if (*FMUstate == NULL) {
		// alloc new memory
		fmi2FMUstate fmuMem = malloc(modelInstance->m_fmuStateSize);
		// remember this memory array
		modelInstance->m_fmuStates.insert(fmuMem);
		// store size of memory in first 8 bytes of fmu memory
		*(size_t*)(fmuMem) = modelInstance->m_fmuStateSize;
		// return newly created FMU mem
		*FMUstate = fmuMem;
	}
	else {
		// check if FMUstate is in list of stored FMU states
		if (modelInstance->m_fmuStates.find(*FMUstate) == modelInstance->m_fmuStates.end()) {
			modelInstance->logger(fmi2Error, "error", "fmi2GetFMUstate is called with invalid FMUstate (unknown or already released pointer).");
			return fmi2Error;
		}
	}

	// now copy FMU state into memory array
	modelInstance->serializeFMUstate(*FMUstate);

	return fmi2OK;
}


fmi2Status fmi2SetFMUstate(void* c, fmi2FMUstate FMUstate) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);

	// check if FMUstate is in list of stored FMU states
	if (modelInstance->m_fmuStates.find(FMUstate) == modelInstance->m_fmuStates.end()) {
		modelInstance->logger(fmi2Error, "error", "fmi2SetFMUstate is called with invalid FMUstate (unknown or already released pointer).");
		return fmi2Error;
	}

	// now copy FMU state into memory array
	modelInstance->deserializeFMUstate(FMUstate);

	return fmi2OK;
}


fmi2Status fmi2FreeFMUstate(void* c, fmi2FMUstate* FMUstate) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);

	if (FMUstate == NULL) {
		// similar to "delete NULL" this is a no-op
		return fmi2OK;
	}

	// check if FMUstate is in list of stored FMU states
	if (modelInstance->m_fmuStates.find(*FMUstate) == modelInstance->m_fmuStates.end()) {
		modelInstance->logger(fmi2Error, "error", "fmi2FreeFMUstate is called with invalid FMUstate (unknown or already released pointer).");
		return fmi2Error;
	}

	// free memory
	free(*FMUstate);
	// and remove pointer from list of own fmu state pointers
	modelInstance->m_fmuStates.erase(*FMUstate);
	*FMUstate = NULL; // set pointer to zero

	return fmi2OK;
}


fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate FMUstate, size_t* s) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);

	// check if FMUstate is in list of stored FMU states
	if (modelInstance->m_fmuStates.find(FMUstate) == modelInstance->m_fmuStates.end()) {
		modelInstance->logger(fmi2Error, "error", "fmi2FreeFMUstate is called with invalid FMUstate (unknown or already released pointer).");
		return fmi2Error;
	}

	// if the state of stored previously, then we must have a valid fmu size
	FMI_ASSERT(modelInstance->m_fmuStateSize != 0);

	// store size of memory to copy
	*s = modelInstance->m_fmuStateSize;

	return fmi2OK;
}


fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t s) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);

	// check if FMUstate is in list of stored FMU states
	if (modelInstance->m_fmuStates.find(FMUstate) == modelInstance->m_fmuStates.end()) {
		modelInstance->logger(fmi2Error, "error", "fmi2FreeFMUstate is called with invalid FMUstate (unknown or already released pointer).");
		return fmi2Error;
	}

	// if the state of stored previously, then we must have a valid fmu size
	FMI_ASSERT(modelInstance->m_fmuStateSize != 0);

	// copy memory
	std::memcpy(serializedState, FMUstate, modelInstance->m_fmuStateSize);

	return fmi2OK;
}


fmi2Status fmi2DeSerializeFMUstate(void* c, const char serializedState[], size_t s, fmi2FMUstate*  FMUstate) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);

	// check if FMUstate is in list of stored FMU states
	if (modelInstance->m_fmuStates.find(FMUstate) == modelInstance->m_fmuStates.end()) {
		modelInstance->logger(fmi2Error, "error", "fmi2FreeFMUstate is called with invalid FMUstate (unknown or already released pointer).");
		return fmi2Error;
	}

	// if the state of stored previously, then we must have a valid fmu size
	FMI_ASSERT(modelInstance->m_fmuStateSize == s);

	// copy memory
	std::memcpy(*FMUstate, serializedState, modelInstance->m_fmuStateSize);

	return fmi2OK;
}



/* Getting partial derivatives */

// 33
// optional possibility to evaluate partial derivatives for the FMU
fmi2Status fmi2GetDirectionalDerivative(void* c, const unsigned int[], size_t,
																const unsigned int[], size_t,
																const double[], double[])
{
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);

	modelInstance->logger(fmi2Warning, "warning", "fmi2GetDirectionalDerivative is called but not implemented");
	return fmi2Warning;
}



/* Enter and exit the different modes */

// Model-Exchange only
fmi2Status fmi2EnterEventMode(void* c){
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);
	std::string text = "fmi2EnterEventMode: Enter into event mode.";
	modelInstance->logger(fmi2OK, "progress", text.c_str());
	return fmi2OK;
}


// Model-Exchange only
fmi2Status fmi2NewDiscreteStates(void* c, fmi2EventInfo* eventInfo) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);
	eventInfo->newDiscreteStatesNeeded = false;
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2NewDiscreteStates: at current time: %1").arg(modelInstance->m_tInput));
	return fmi2OK;
}


// Model-Exchange only
fmi2Status fmi2EnterContinuousTimeMode(void* c) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", "fmi2EnterContinuousTimeMode: Enter into continuous mode.");
	return fmi2OK;
}


// Model-Exchange only
fmi2Status fmi2CompletedIntegratorStep (void* c, fmi2Boolean,
										fmi2Boolean* enterEventMode, fmi2Boolean* terminateSimulation)
{
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);

	// Currently, we never enter Event mode
	*enterEventMode = false;

	modelInstance->logger(fmi2OK, "progress", "Integrator step completed.");
	try {
		modelInstance->completedIntegratorStep();
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		modelInstance->logger(fmi2Error, "error", "Error in fmi2CompletedIntegratorStep().");
		*terminateSimulation = true;
		return fmi2Error;
	}

	*terminateSimulation = false;

	return fmi2OK;
}



/* Providing independent variables and re-initialization of caching */

// Sets a new time point
// Model-Exchange only
fmi2Status fmi2SetTime (void* c, fmi2Real time) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2SetTime: Set time point: %1").arg(time));
	// cache new time point
	modelInstance->m_tInput = time;
	return fmi2OK;
}


// Model-Exchange only
fmi2Status fmi2SetContinuousStates(void* c, const fmi2Real x[], size_t nx) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);

	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2SetContinuousStates: "
		"Set continuous states with size %1 with model size %2").arg(nx).arg(modelInstance->m_yInput.size()));
	FMI_ASSERT(nx == modelInstance->m_yInput.size());

	// cache input Y vector
	std::memcpy( &(modelInstance->m_yInput[0]), x, nx*sizeof(double) );
	return fmi2OK;
}



/* Evaluation of the model equations */


// Model-Exchange only
fmi2Status fmi2GetDerivatives(void* c, fmi2Real derivatives[], size_t nx) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);

	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetDerivatives: "
		"Getting derivatives with size %1 with model size %2").arg(nx).arg(modelInstance->m_ydot.size()));

	// Update model state if any of the inputs have been modified.
	// Does nothing, if the model state is already up-to-date after a previous call
	// to updateIfModified().
	try {
		modelInstance->updateIfModified();
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		modelInstance->logger(fmi2Error, "error", IBK::FormatString("fmi2GetDerivatives: "
			"Exception while updating model: %1").arg(ex.what()));
		return fmi2Error;
	}

	// return derivatives currently cached in model
	std::memcpy( derivatives, &(modelInstance->m_ydot[0]), nx * sizeof(double) );
	return fmi2OK;
}


// Model-Exchange only
fmi2Status fmi2GetEventIndicators (void* c, fmi2Real[], size_t ni){
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetEventIndicators: size %1").arg(ni));
	return fmi2OK;
}


// Model-Exchange only
fmi2Status fmi2GetContinuousStates(void* c, fmi2Real x[], size_t nx) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);

	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetContinuousStates: "
		"Getting continuous states with size %1 with model size %2").arg(nx).arg(modelInstance->m_yInput.size()));
	FMI_ASSERT(nx == modelInstance->m_yInput.size());

	std::memcpy( x, &(modelInstance->m_yInput[0]), nx * sizeof(double) );
	return fmi2OK;
}


// Model-Exchange only
fmi2Status fmi2GetNominalsOfContinuousStates(void* c, fmi2Real[], size_t nx) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetNominalsOfContinuousStates: size: %1").arg(nx));
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2SetRealInputDerivatives(void* c,	const fmi2ValueReference vr[], size_t nvr,
										const fmi2Integer order[], const fmi2Real value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2SetRealInputDerivatives: size: %1").arg(nvr));
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2GetRealOutputDerivatives(void* c, const fmi2ValueReference vr[], size_t nvr,
										const fmi2Integer order[], fmi2Real value[]) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetRealOutputDerivatives: size: %1").arg(nvr));
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2DoStep(void* c, double currentCommunicationPoint, double communicationStepSize,
					  int noSetFMUStatePriorToCurrentPoint)
{
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	MessageHandlerSwapper handlerSwap(modelInstance->m_messageHandlerPtr); (void)handlerSwap;
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);

	// signal model that we have started a new communication interval, and also pass information whether we still need to iterate over last
	// interval or not
	modelInstance->model()->startCommunicationInterval(currentCommunicationPoint, (noSetFMUStatePriorToCurrentPoint == fmi2True));
	//modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2DoStep: %1 += %2").arg(currentCommunicationPoint).arg(communicationStepSize));

	// if currentCommunicationPoint < current time of integrator, restore
	try {
		modelInstance->integrateTo(currentCommunicationPoint + communicationStepSize);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		modelInstance->logger(fmi2Error, "error", IBK::FormatString("fmi2DoStep: "
			"Exception while integrating model: %1").arg(ex.what()));
		return fmi2Error;
	}
	// tell model that we are finished with communication interval
	modelInstance->model()->completeCommunicationInterval();
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2CancelStep(void* c) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2CancelStep: cancel current step."));
	return fmi2OK;
}



// CoSim only
fmi2Status fmi2GetStatus(void* c, const fmi2StatusKind s,
						fmi2Status* value) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetStatus: get current status."));
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2GetRealStatus(void* c, const fmi2StatusKind s,
						fmi2Real* value) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetRealStatus: get real status."));
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2GetIntegerStatus(void* c, const fmi2StatusKind s,
						fmi2Integer* value) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetIntegerStatus: get integer status."));
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2GetBooleanStatus(void* c, const fmi2StatusKind s,
						fmi2Boolean* value) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetBooleanStatus: get boolean status."));
	return fmi2OK;
}


// CoSim only
fmi2Status fmi2GetStringStatus(void* c, const fmi2StatusKind s,
						fmi2String* value) {
	InstanceData * modelInstance = static_cast<InstanceData*>(c);
	FMI_ASSERT(modelInstance != NULL);
	FMI_ASSERT(!modelInstance->m_modelExchange);
	modelInstance->logger(fmi2OK, "progress", IBK::FormatString("fmi2GetStringStatus: get string status."));
	return fmi2OK;
}
