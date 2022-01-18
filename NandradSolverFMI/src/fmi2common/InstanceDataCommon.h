/* Common implementation of the FMI Interface Data Structure used by IBK solvers.
	Copyright 2016 Andreas Nicolai, LGPL
*/

#ifndef InstanceDataCommonH
#define InstanceDataCommonH

#include <vector>
#include <set>

#include <IBK_Path.h>
#include <IBK_FormatString.h>

#include <SOLFRA_FMUModelInterface.h>

#include "fmi2FunctionTypes.h"

namespace IBK {
	class MessageHandler;
}


/*! This class wraps data needed for FMUs and implements common functionality.

	In order to use this in your own InstanceData structure, you must inherit this
	file and implement the following functions:

	\code
	class InstanceData : public InstanceDataCommon {
	public:
		// Initializes InstanceData
		void init();

		// Re-implement if you want ModelExchange support
		virtual void updateIfModified();
		// Re-implement if you want CoSim support
		virtual void integrateTo(double tCommunicationIntervalEnd);
	};
	\endcode

	Also, you must define the symbol
	\code
	const char * const InstanceDataCommon::GUID = "{471a3b52-4923-44d8-ab4a-fcdb813c7322}";
	\endcode
	in your InstanceData.cpp file.
*/
class InstanceDataCommon {
public:

	/*! Enumeration values that control output behavior of FMU. */
	enum OutputBehaviour {
		OB_Disabled,
		OB_EveryCompletedStep,
		OB_WithThreashold,
		OB_ToleranceBased
	};


	/*! Initializes empty instance. */
	InstanceDataCommon(SOLFRA::FMUModelInterface * model);

	/*! Destructor, resource cleanup. */
	virtual ~InstanceDataCommon();

	/*! This function triggers a state-update of the embedded model whenever our cached input
		data differs from the input data in the model (must be implemented in derived classes).
	*/
	virtual void updateIfModified();

	/*! Called from fmi2DoStep(). */
	virtual void integrateTo(double tCommunicationIntervalEnd);

	/*! Provides access to internally stored model. */
	SOLFRA::FMUModelInterface * model() { return m_model; }

	/*! Send a logging message to FMU environment if logger is present.*/
	void logger(fmi2Status, fmi2String, const IBK::FormatString &);
	/*! Send a logging message to FMU environment if logger is present.*/
	void logger(fmi2Status state, fmi2String category, fmi2String msg) {
		logger(state, category, IBK::FormatString(msg));
	}

	/*! Create message handler object and opens log file.
		Directory structure is created if not existing yet.
		Function throws an IBK::Exception when log file cannot be created or
		if message handler object exists already.
	*/
	void setupMessageHandler(const IBK::Path & logfile);

	/*! Sets a new input parameter of type double. */
	void setReal(int varID, double value);

	/*! Sets a new input parameter of type int. */
	void setInteger(int varID, int value);

	/*! Sets a new input parameter of type string. */
	void setString(int varID, fmi2String value);

	/*! Sets a new input parameter of type bool. */
	void setBoolean(int varID, bool value);

	/*! Retrieves an output parameter of type double. */
	void getReal(int varID, double & value);

	/*! Retrieves an output parameter of type int. */
	void getInteger(int varID, int & value);

	/*! Retrieves an output parameter of type string. */
	void getString(int varID, fmi2String & value);

	/*! Retrieves an output parameter of type bool. */
	void getBoolean(int varID, bool & value);

	/*! Called from fmi2CompletedIntegratorStep(): only ModelExchange. */
	void completedIntegratorStep();

	/*! Re-implement for getFMUState()/setFMUState() support.
		This function computes the size needed for full serizalization of
		the FMU and stores the size in m_fmuStateSize.
		\note The size includes the leading 8byte for the 64bit integer size
		of the memory array (for testing purposes).
		If serialization is not supported, the function will set an fmu size of 0.
	*/
	virtual void computeFMUStateSize() { m_fmuStateSize = 0; } // default implementation sets zero size = no serialization

	/*! Re-implement for getFMUState() support.
		Copies the internal state of the FMU to the memory array pointed to by FMUstate.
		Memory array always has size m_fmuStateSize.
	*/
	virtual void serializeFMUstate(void * FMUstate) { (void)FMUstate; }

	/*! Re-implement for setFMUState() support.
		Copies the content of the memory array pointed to by FMUstate to the internal state of the FMU.
		Memory array always has size m_fmuStateSize.
	*/
	virtual void deserializeFMUstate(void * FMUstate) { (void)FMUstate; }

	/*! Called from fmi2FreeInstance() in CoSimulation at the end of simulation.
		Write outputs here.
	*/
	virtual void finish() {}

	/*! Global unique ID that identifies this FMU.
		Must match the GUID in the ModelDescription file.
		\note This GUID is model-specific, so you must define this static symbol
			  in the cpp file of your derived class.
	*/
	static const char * const GUID;

	/*! Stores the FMU callback functions for later use.
		It is usable between fmi2Instantiate and fmi2Terminate.*/
	const fmi2CallbackFunctions*	m_callbackFunctions;

	/*! True if in initialization mode. */
	bool							m_initializationMode;

	/*! Name of the instance inside the FMU environment.*/
	std::string						m_instanceName;

	/*! Resource root path as set via fmi2Instantiate(). */
	std::string						m_resourceLocation;

	/*! Logging enabled flag as set via fmi2Instantiate(). */
	bool							m_loggingOn;

	/*! Logging categories supported by the master. */
	std::vector<std::string>		m_loggingCategories;

	/*! If true, this is a ModelExchange FMU. */
	bool							m_modelExchange;

	/*! Base directory for storing FMU specific results.
		Set via setStringParameter() with parameter name "ResultsRootDir" with fixed ID 42.
		String is an UTF8 encoded path.
	*/
	std::string						m_resultsRootDir;

	/*! Time point in [s] received by last call to fmi2SetTime(). */
	double							m_tInput;

	/*! Model state vector as received by last call to fmi2SetContinuousStates(). */
	std::vector<double>				m_yInput;

	/*! Model derivatives vector as updated by last call to updateIfModified(). */
	std::vector<double>				m_ydot;

	/*! Signals that one of the real parameter inputs have been changed.
		This flag is reset whenever updateIfModified() has been called.
		The flag is set in any of the setXXXParameter() functions.
	*/
	bool							m_externalInputVarsModified;

	/*! Cached pointer to IBK message handler (NULL by default, which means - not using IBK messages or relying on default message handler). */
	IBK::MessageHandler				*m_messageHandlerPtr;


	/*! Holds the size of the FMU when serialized in memory.
		This value does not change after full initialization of the solver so it can be cached.
		Initially it will be zero so functions can check if initialization is properly done.
	*/
	size_t							m_fmuStateSize;
	/*! Holds pointers to all currently stored FMU states.
		Pointers get added in function fmi2GetFMUstate(), and removed in fmi2FreeFMUstate().
		Unreleased memory gets deallocated in destructor.
	*/
	std::set<void*>					m_fmuStates;

	/*! Controls output behavior of FMUs. */
	OutputBehaviour					m_outputBehavior;

private:
	/*! Read/write pointer to the model implementing the FMI interface functions. */
	SOLFRA::FMUModelInterface		*m_model;

}; // class InstanceDataCommon

#endif // InstanceDataCommonH
