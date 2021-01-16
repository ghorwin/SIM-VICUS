/*	FMI Interface Data Structure for the Delphin Solver
*/

/*! Model identifier to constuct dll function names.*/
#define MODEL_IDENTIFIER DelphinFMI

#include "fmi2common/fmi2Functions.h"
#include "fmi2common/fmi2FunctionTypes.h"
#include "InstanceData.h"

#ifdef _WIN32

#undef UNICODE
#include <Windows.h>
#include <ShlObj.h>  // need to include definitions of constants

#endif // _WIN32

#include <memory>

#include <IBK_assert.h>
#include <IBK_messages.h>
#include <IBK_MessageHandlerRegistry.h>

#include <SOLFRA_SolverControlFramework.h>
#include <SOLFRA_IntegratorSundialsCVODE.h>
#include <SOLFRA_LESDense.h>
#include <SOLFRA_PrecondInterface.h>
#include <SOLFRA_JacobianInterface.h>
#include <SOLFRA_OutputScheduler.h>
#include <SOLFRA_ModelInterface.h>

#include <NANDRAD_ArgsParser.h>
#include <NANDRAD_Project.h>

#include <NM_Directories.h>
#include <NM_NandradModel.h>

#include <IBK_Time.h>


const char * const InstanceDataCommon::GUID = "{471a3b52-4923-44d8-ab4b-fcdb813c8465}";

const char * const PROGRAM_INFO =
	"NANDRAD Solver Functional Mock-Up Interface\n"
	"All rights reserved.\n\n"
	"The NANDRAD/SIM-VICUS Development Team.\n"
	"Core developers:\n"
	"Andreas Nicolai and Anne Paepcke\n"
	"Contact: \n"
	"  andreas.nicolai [at] tu-dresden.de\n"
	"  anne.paepcke [at] tu-dresden.de\n\n";

std::string with_replaced_spaces(const std::string & text) {
	return IBK::replace_string(text, " ", "_");
}

// Constructor, initializes common implementation by passing pointer to model
// that implements the SOLFRA::FMUModelInterface interface.
InstanceData::InstanceData() :
	InstanceDataCommon(&m_model),
	m_tStart(0)
{
}


InstanceData::~InstanceData() {
	// wait for OpenMPs threads to spin down
	// before we destruct the object
	IBK::StopWatch w;
	while (w.difference() < 100);
}

// create a model instance
void InstanceData::init() {
	const char * const FUNC_ID = "[InstanceData::init]";

	logger(fmi2OK, "progress", "Starting initialization.");

	// TODO

	// Model Init
	try {

		// TODO

		logger(fmi2OK, "progress", "Initialization complete.");
		IBK::IBK_Message("Model initialization finished.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	catch (IBK::Exception & ex) {
		logger(fmi2Error, "error", IBK::FormatString("Exception caught: %1").arg(ex.what()));
		throw IBK::Exception(ex, "Exception caught.", FUNC_ID);
	}
	catch (std::exception & ex) {
		logger(fmi2Error, "error", IBK::FormatString("Exception caught: %1").arg(ex.what()));
		throw IBK::Exception(ex, IBK::FormatString("Exception caught: %1").arg(ex.what()), FUNC_ID);
	}

}


void InstanceData::integrateTo(double tCommunicationIntervalEnd) {
	const char * const FUNC_ID = "[InstanceData::integrateTo]";

//	// ask all components of the integration framework for size
//	SOLFRA::ModelInterface * modelInterface = m_model.modelInterface();
//	// update self pointer - needed when multiple instances of the solver used concurrently in a model
//	NANDRAD_MODEL::NandradModel::m_self = dynamic_cast<NANDRAD_MODEL::NandradModel*>(modelInterface);

	try {
//		double tCommunicationIntervalStart = modelInterface->integratorInterface()->t();

		// TODO

		// only for forward time steppping: otherwise in terminate
		m_model.completeCommunicationInterval();
	}
	catch (IBK::Exception & ex) {
		logger(fmi2Error, "error", IBK::FormatString("Exception caught: %1").arg(ex.what()));
		throw IBK::Exception(ex, "Exception caught.", FUNC_ID);
	}
	catch (std::exception & ex) {
		logger(fmi2Error, "error", IBK::FormatString("Exception caught: %1").arg(ex.what()));
		throw IBK::Exception(ex, IBK::FormatString("Exception caught: %1").arg(ex.what()), FUNC_ID);
	}

}


void InstanceData::computeFMUStateSize() {
	const char * const FUNC_ID = "[InstanceData::computeFMUStateSize]";
	IBK_ASSERT(!m_modelExchange);

#if 0

	// ask all components of the integration framework for size
	SOLFRA::ModelInterface * modelInterface = m_model.modelInterface();

	SOLFRA::IntegratorInterface *integrator = modelInterface->integratorInterface();
	SOLFRA::LESInterface *lesSolver = modelInterface->lesInterface();
	SOLFRA::PrecondInterface  *precond  = modelInterface->preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = modelInterface->jacobianInterface();

	m_fmuStateSize = 8; // 8 bytes for leading size header

	size_t s = integrator->serializationSize();
	if (s == 0)
		throw IBK::Exception("Integrator does not support serialization.", FUNC_ID);
	m_fmuStateSize += s;

	if (lesSolver != NULL) {
		s = lesSolver->serializationSize();
		if (s == 0)
			throw IBK::Exception("LES solver does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	if (precond != NULL) {
		s = precond->serializationSize();
		if (s == 0)
			throw IBK::Exception("Preconditioner does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	if (jacobian != NULL) {
		s = jacobian->serializationSize();
		if (s == 0)
			throw IBK::Exception("Jacobian matrix generator does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	s = modelInterface->serializationSize();
	// we allow s == 0
	m_fmuStateSize += s;
#endif
}


void InstanceData::serializeFMUstate(void * FMUstate) {
	IBK_ASSERT(!m_modelExchange);
#if 0
	// ask all components of the integration framework for size
	SOLFRA::ModelInterface * modelInterface = m_model.modelInterface();
	// update self pointer - needed when multiple instances of the solver used concurrently in a model
	NANDRAD_MODEL::NandradModel::m_self = dynamic_cast<NANDRAD_MODEL::NandradModel*>(modelInterface);
	SOLFRA::IntegratorInterface *integrator = modelInterface->integratorInterface();
	SOLFRA::LESInterface *lesSolver = modelInterface->lesInterface();
	SOLFRA::PrecondInterface  *precond  = modelInterface->preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = modelInterface->jacobianInterface();

	void * dataStart = (char*)FMUstate + 8;
	integrator->serialize(dataStart);
	if (lesSolver != NULL)
		lesSolver->serialize(dataStart);
	if (precond != NULL)
		precond->serialize(dataStart);
	if (jacobian != NULL)
		jacobian->serialize(dataStart);
	modelInterface->serialize(dataStart);
#endif
}


void InstanceData::deserializeFMUstate(void * FMUstate) {
	IBK_ASSERT(!m_modelExchange);
#if 0
	// ask all components of the integration framework for size
	SOLFRA::ModelInterface * modelInterface = m_model.modelInterface();
	// update self pointer - needed when multiple instances of the solver used concurrently in a model
	NANDRAD_MODEL::NandradModel::m_self = dynamic_cast<NANDRAD_MODEL::NandradModel*>(modelInterface);
	SOLFRA::IntegratorInterface *integrator = modelInterface->integratorInterface();
	SOLFRA::LESInterface *lesSolver = modelInterface->lesInterface();
	SOLFRA::PrecondInterface  *precond  = modelInterface->preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = modelInterface->jacobianInterface();

	void * dataStart = (char*)FMUstate + 8;
	integrator->deserialize(dataStart);
	if (lesSolver != NULL)
		lesSolver->deserialize(dataStart);
	if (precond != NULL)
		precond->deserialize(dataStart);
	if (jacobian != NULL)
		jacobian->deserialize(dataStart);
	modelInterface->deserialize(dataStart);
#endif
}


void InstanceData::clearBuffers() {
	IBK_ASSERT(!m_modelExchange);
	// write and clear output buffers
	m_model.clearOutputBuffer();
}
