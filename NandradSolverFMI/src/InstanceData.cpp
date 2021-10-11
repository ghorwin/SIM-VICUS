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
#include <SOLFRA_LESKLU.h>
#include <SOLFRA_PrecondInterface.h>
#include <SOLFRA_JacobianInterface.h>
#include <SOLFRA_OutputScheduler.h>
#include <SOLFRA_ModelInterface.h>

#include <NANDRAD_ArgsParser.h>
#include <NANDRAD_Project.h>

#include <NM_Directories.h>
#include <NM_NandradModel.h>

#include <IBK_Time.h>


const char * const InstanceDataCommon::GUID ="{471a3b52-4923-44d8-ab4b-fcdb813c1244}";
		//"{471a3b52-4923-44d8-ab4b-fcdb813c8465}";

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

	// Model Init
	try {

		// Generate project filename
		IBK::Path projectFile = IBK::Path::fromURI(m_resourceLocation);
		projectFile /= "Project.nandrad"; // append project file, always the same within FMU container

		// Create and customize NANDRAD::ArgsParser
		NANDRAD::ArgsParser args;
		args.m_projectFile = projectFile.str();

		IBK::Path outputDirectory = IBK::Path(m_resultsRootDir);
		// error: master didn't specify the ResultsRootDir parameter or no directory is defined
		if (m_resultsRootDir.empty()) {
			 //m_resultsRootDir now holds the default/fall-back output root dir
			 throw IBK::Exception( IBK::FormatString("No output directory is specified inside string parameter "
				"'resultRootDir'."), FUNC_ID);
		}

		// clear path from 'IBK/Nandrad'
		IBK::Path resultsParentDir = outputDirectory;
		if(resultsParentDir.filename() == "Nandrad") {
			try {
				if(resultsParentDir.parentPath().filename() == "IBK") {
					resultsParentDir = resultsParentDir.parentPath().parentPath();
				}
			}
			catch(IBK::Exception &ex) {
				throw IBK::Exception(ex, IBK::FormatString("Invalid results root directory '%1'.").arg(outputDirectory.str()),
									  FUNC_ID);
			}
		}

		// check if base directory exists
		if(!resultsParentDir.exists()) {
			throw IBK::Exception( IBK::FormatString("Invalid results parent directory '%1'.").arg(outputDirectory.str()),
								  FUNC_ID);
		}

		// append "results" for the actual output files
		outputDirectory /= "results";
		if (!IBK::Path::makePath(outputDirectory)) {
			throw IBK::Exception( IBK::FormatString("Cannot create output directory '%1'.").arg(outputDirectory.str()),
								  FUNC_ID);
		}

		args.setOption(args.keyword(IBK::SolverArgsParser::GO_OUTPUT_DIR), m_resultsRootDir);

		// *** create directory structure ***
		try {
			IBK::Path executablePath = args.m_projectFile.parentPath().parentPath() / "binaries";
			m_model.setupDirectories(args);
			// *** create message handler and log file ***
			setupMessageHandler(m_model.dirs().m_logDir / "screenlog.txt");
			// now we have a log directory and can write our messages to the log file
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, "Error creating project/ output directories.", FUNC_ID);
		}

		// *** write program/copyright info ***
		IBK::IBK_Message(PROGRAM_INFO, IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		NANDRAD_MODEL::NandradModel::printVersionStrings();
		SOLFRA::SolverControlFramework::printVersionInfo();
		IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// *** initialize model ***
		try {
			m_model.init(args);

			// set start time
			if (m_tStart != 0.0) {
				IBK::IBK_Message( IBK::FormatString("Settings simulation start time to t=%1 s (= %2)\n").arg(m_tStart).arg(IBK::Time::format_time_difference(m_tStart)),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
//				const_cast<NANDRAD::Project&>(m_model).m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value = m_tStart;
			}
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, "Model initialization error.", FUNC_ID);
		}

//		// resize all internal quantities for model exchange
//		if (m_modelExchange) {
//			// resize all internal quantities
//			m_yInput.resize(m_nandradModel.n());
//			std::memcpy(&m_yInput[0], m_nandradModel.y0(), m_nandradModel.n() * sizeof(double) );
//			m_ydot.resize(m_nandradModel.n());
//		}
		// initialize integrator for co-simulation
//		else {
		IBK_ASSERT(!m_modelExchange);
		// retrieve integrator, les-solver, ... from model
		SOLFRA::IntegratorInterface *integrator = m_model.integratorInterface();
		IBK_ASSERT(integrator != NULL);

		SOLFRA::LESInterface *lesSolver = m_model.lesInterface();
		SOLFRA::PrecondInterface  *precond  = m_model.preconditionerInterface();
		SOLFRA::JacobianInterface *jacobian = m_model.jacobianInterface();

		// initialize integrator with read solution
		try {
			integrator->init(&m_model, m_model.t0(), m_model.y0(), lesSolver, precond, jacobian);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Initialization of Integrator failed.", FUNC_ID);
		}
		// update model with all initial states and time
		try {
			m_model.setTime(m_model.t0());
			m_model.setY(m_model.y0());
			m_model.ydot(nullptr);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Calculation of initial states failed.", FUNC_ID);
		}
//		}
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
		double tCommunicationIntervalStart = m_model.integratorInterface()->t();
		// reset is not implemented, yet
		if(tCommunicationIntervalStart > tCommunicationIntervalEnd) {
			throw IBK::Exception(IBK::FormatString("Error performing integration step: "
				"reset from integrator time point %1 to the new communication interval end %2 is not "
				" supported, yet!")
				.arg(tCommunicationIntervalStart)
				.arg(tCommunicationIntervalEnd),
				FUNC_ID);
		}

		/* 1.) Set a new tStop value (= tCommunicationIntervalEnd) in integrator*/
		SOLFRA::IntegratorInterface *integrator		 = m_model.integratorInterface();

		SOLFRA::IntegratorErrorControlled* integratorErrorControlled =
			dynamic_cast<SOLFRA::IntegratorErrorControlled*> (integrator);
		// stop value is still only defined for error controled integrators
		if(  integratorErrorControlled != nullptr)
			integratorErrorControlled->m_stopTime = tCommunicationIntervalEnd;

		/* 3.) integration loop until communication interval has been reached:*/
		SOLFRA::OutputScheduler		*outputScheduler = m_model.outputScheduler();


		// outputs are only written if we start the simulation from begin, therefore
		// we pass t0 and y0 for the initial model evaluation within writeOutputs()
		if (tCommunicationIntervalStart == m_model.t0()) {
			m_model.setTime(m_model.t0());
			m_model.setY(m_model.y0());
			m_model.ydot(nullptr);
			m_model.writeOutputs( m_model.t0(), m_model.y0());
		}
		else {
			// reset outputs if necessary
			m_model.resetOutputBuffer(tCommunicationIntervalStart);
		}

		m_model.startCommunicationInterval(tCommunicationIntervalStart);
		// integration loop
		double t = tCommunicationIntervalStart;
		double tOutput = outputScheduler->nextOutputTime(t);

		while (t < tCommunicationIntervalEnd) {

			// (contains parallel code)
			SOLFRA::IntegratorInterface::StepResultType res = integrator->step();
			if (res != SOLFRA::IntegratorInterface::StepSuccess) {
				throw IBK::Exception("Integrator step failed!", FUNC_ID);
			}

			SOLFRA::IntegratorSundialsCVODE * cvodeIntegrator = dynamic_cast<SOLFRA::IntegratorSundialsCVODE *>(integrator);
			if (cvodeIntegrator != nullptr)
				// reset linear setup to default value
				cvodeIntegrator->setLinearSetupFrequency(0);

			// get new time point and time step size
			t = integrator->t();

			// notify model of completed step (contains parallel code)
			const double * y_current = integrator->yOut( t );

			m_model.stepCompleted(t, y_current);

			while (tOutput <= t) {

				// retrieve interpolated states at output time point (contains parallel code)
				const double * yOutput = integrator->yOut(tOutput);
				double tOutputNext = 0.0;

				// tell m_model to write outputs
				m_model.setTime(tOutput);
				m_model.setY(yOutput);
				m_model.ydot(nullptr);
				m_model.writeOutputs(tOutput, yOutput);

				// retrieve new output time point
				tOutputNext = outputScheduler->nextOutputTime(tOutput);
				if (tOutputNext <= tOutput)
					throw IBK::Exception(IBK::FormatString("Output scheduler returned output time %1, which is <= last output time %2.")
						.arg(tOutputNext).arg(tOutput), FUNC_ID);

				tOutput = tOutputNext;

				//// if t_outNext is > t_end (including some rounding error compensation), we break the loop
				//if (tOutputNext > tCommunicationIntervalEnd*(1+1e-10))
				//	break;
			} // while (tOutput <= t) {

		} // while (t < tCommunicationIntervalEnd)

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

	SOLFRA::IntegratorInterface *integrator = m_model.integratorInterface();
	SOLFRA::LESInterface *lesSolver = m_model.lesInterface();
	SOLFRA::PrecondInterface  *precond  = m_model.preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = m_model.jacobianInterface();

	IBK_ASSERT(sizeof(size_t) == 8);
	m_fmuStateSize = 8; // 8 bytes for leading size header

	// ask all components of the integration framework for size

	size_t s = integrator->serializationSize();
	if (s == SOLFRA::NON_SUPPORTED_FUNCTION)
		throw IBK::Exception("Integrator does not support serialization.", FUNC_ID);
	m_fmuStateSize += s;

	if (lesSolver != nullptr) {
		s = lesSolver->serializationSize();
		if (s == SOLFRA::NON_SUPPORTED_FUNCTION)
			throw IBK::Exception("LES solver does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	if (precond != nullptr) {
		s = precond->serializationSize();
		if (s == SOLFRA::NON_SUPPORTED_FUNCTION)
			throw IBK::Exception("Preconditioner does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	if (jacobian != nullptr) {
		s = jacobian->serializationSize();
		if (s == SOLFRA::NON_SUPPORTED_FUNCTION)
			throw IBK::Exception("Jacobian matrix generator does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	s = m_model.serializationSize();
	// we allow s == 0
	m_fmuStateSize += s;
}


void InstanceData::serializeFMUstate(void * FMUstate) {
	IBK_ASSERT(!m_modelExchange);

	SOLFRA::IntegratorInterface *integrator = m_model.integratorInterface();
	SOLFRA::LESInterface *lesSolver = m_model.lesInterface();
	SOLFRA::PrecondInterface  *precond  = m_model.preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = m_model.jacobianInterface();

	// let all components copy their data into our memory
	void * dataStart = (char*)FMUstate + 8; // add space for the initial size_t that holds total sizes
	integrator->serialize(dataStart);
	if (lesSolver != nullptr)
		lesSolver->serialize(dataStart);
	if (precond != nullptr)
		precond->serialize(dataStart);
	if (jacobian != nullptr)
		jacobian->serialize(dataStart);
	m_model.serialize(dataStart);
}


void InstanceData::deserializeFMUstate(void * FMUstate) {
	IBK_ASSERT(!m_modelExchange);

	SOLFRA::IntegratorInterface *integrator = m_model.integratorInterface();
	SOLFRA::LESInterface *lesSolver = m_model.lesInterface();
	SOLFRA::PrecondInterface  *precond  = m_model.preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = m_model.jacobianInterface();

	// copy component memory back from state memory
	void * dataStart = (char*)FMUstate + 8; // add space for the initial size_t that holds total sizes
	integrator->deserialize(dataStart);
	if (lesSolver != nullptr)
		lesSolver->deserialize(dataStart);
	if (precond != nullptr)
		precond->deserialize(dataStart);
	if (jacobian != nullptr)
		jacobian->deserialize(dataStart);
	m_model.deserialize(dataStart);
}


void InstanceData::finish() {
	m_model.writeFinalOutputs();
}


void InstanceData::clearBuffers() {
	IBK_ASSERT(!m_modelExchange);
	// write and clear output buffers
	m_model.clearOutputBuffer();
}
