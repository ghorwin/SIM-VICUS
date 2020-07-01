#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <memory>

#include <IBK_StringUtils.h>
#include <IBK_Exception.h>
#include <IBK_MessageHandler.h>
#include <IBK_MessageHandlerRegistry.h>
#include <IBK_messages.h>

// include solver control framework and integrator
#include <SOLFRA_SolverControlFramework.h>
#include <SOLFRA_IntegratorSundialsCVODE.h>

// include model implementation class
#include "NM_NandradModel.h"

// include project class (holds input data)
#include <NANDRAD_Project.h>
// include header for command line argument parser
#include <NANDRAD_ArgsParser.h>


const char * const PROGRAM_INFO =
	"NANDRAD Solver\n"
	"All rights reserved.\n\n"
	"The NANDRAD Development Team:\n"
	"Anne Paepcke, Andreas Nicolai, Stefan Vogelsang\n"
	"Contact: \n"
	"  anne.paepcke [at] tu-dresden.de\n"
	"  andreas.nicolai [at] tu-dresden.de\n\n";


int main(int argc, char * argv[]) {
	const char * const FUNC_ID = "[main]";

	NANDRAD_MODEL::NandradModel model;

	NANDRAD::ArgsParser args;

	IBK::StopWatch initWatch;

	try {
		// *** Command line parsing ***
#ifdef WIN32
		args.setAppName("NandradSolver");
#else
		args.setAppName("NandradSolver");
#endif
		args.parse(argc, argv);
		// handle default arguments like help and man-page requests, which are printed to std::cout
		if (args.handleDefaultFlags(std::cout))
			// stop if help/man-page requested
			return EXIT_SUCCESS;
		if (args.flagEnabled(IBK::SolverArgsParser::DO_VERSION)) {
			std::cout << PROGRAM_INFO << std::endl;
			NANDRAD_MODEL::NandradModel::printVersionStrings();
			SOLFRA::SolverControlFramework::printVersionInfo();
			return EXIT_SUCCESS;
		}

		// check if errors are present, error messages are written to std::cerr
		if (args.handleErrors(std::cerr))
			return EXIT_FAILURE;

		// *** create directory structure ***
		model.setupDirectories(args, args.m_projectFile);
		// now we have a log directory and can write our messages to the log file


		// *** setup message handler ***

		unsigned int verbosityLevel = IBK::string2val<int>(args.option(IBK::SolverArgsParser::DO_VERBOSITY_LEVEL));
		IBK::MessageHandler * messageHandlerPtr = IBK::MessageHandlerRegistry::instance().messageHandler();
		messageHandlerPtr->setConsoleVerbosityLevel(verbosityLevel);
		messageHandlerPtr->setLogfileVerbosityLevel(verbosityLevel);
		messageHandlerPtr->m_contextIndentation = 48;
		std::string errmsg;
//#define NO_LOGFILE
#ifndef NO_LOGFILE
		IBK::Path logfile = model.dirs().m_logDir / "screenlog.txt";
		bool success = messageHandlerPtr->openLogFile(logfile.str(), args.m_restart, errmsg);
		if (!success) {
			IBK::IBK_Message(errmsg, IBK::MSG_WARNING, FUNC_ID);
			IBK::IBK_Message("Cannot create log file, outputs will only be printed on screen.", IBK::MSG_WARNING, FUNC_ID);
		}
#endif // NO_LOGFILE

		// *** write program/copyright info ***
		IBK::IBK_Message(PROGRAM_INFO, IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		NANDRAD_MODEL::NandradModel::printVersionStrings();
		SOLFRA::SolverControlFramework::printVersionInfo();
		IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// *** Initialize model. ***

		// init model (first read project, then initialize model)
		model.init(args);
		IBK::IBK_Message( IBK::FormatString("Model initialization complete, duration: %1\n\n").arg(initWatch.diff_str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// *** if designday option is set, additionally calculate design data
		if (static_cast<IBK::ArgParser>(args).flagEnabled("designday")) {
			// create a new file including design parameters
			model.createHeatingDesignDayCalculationProject();
			model.createCoolingDesignDayCalculationProject();
			// create a new model for design calculation
			NANDRAD::ArgsParser argsDesign(args);
			argsDesign.m_projectFile = model.dirs().m_heatingDesignRootDir;
			argsDesign.m_projectFile.addExtension("nandrad");
			// set output directory
			argsDesign.options(IBK::SolverArgsParser::GO_OUTPUT_DIR).push_back(model.dirs().m_heatingDesignResultsDir.str());
			NANDRAD_MODEL::NandradModel modelHeatingDesignCalc;
			// set all directories
			modelHeatingDesignCalc.setupDirectories(argsDesign, argsDesign.m_projectFile);
			// initialize model
			modelHeatingDesignCalc.init(argsDesign);
			// and run model
			IBK::IBK_Message("Precalulate heating design day\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			SOLFRA::SolverControlFramework solverHeatingDesignCalc(&modelHeatingDesignCalc);
			solverHeatingDesignCalc.m_useStepStatistics = argsDesign.flagEnabled(IBK::SolverArgsParser::DO_STEP_STATS);
			solverHeatingDesignCalc.m_logDirectory = modelHeatingDesignCalc.dirs().m_logDir.str();
			solverHeatingDesignCalc.run();
			// write results file

			// the same with cooling
			argsDesign.m_projectFile = model.dirs().m_coolingDesignRootDir;
			argsDesign.m_projectFile.addExtension("nandrad");
			argsDesign.options(IBK::SolverArgsParser::GO_OUTPUT_DIR).clear();
			argsDesign.options(IBK::SolverArgsParser::GO_OUTPUT_DIR).push_back(model.dirs().m_coolingDesignResultsDir.str());
			NANDRAD_MODEL::NandradModel modelCoolingDesignCalc;
			// set all directories
			modelCoolingDesignCalc.setupDirectories(argsDesign, argsDesign.m_projectFile);
			// initialize model
			modelCoolingDesignCalc.init(argsDesign);
			// and run model
			IBK::IBK_Message("Precalulate cooling design day\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			SOLFRA::SolverControlFramework solverCoolingDesignCalc(&modelCoolingDesignCalc);
			solverCoolingDesignCalc.m_useStepStatistics = argsDesign.flagEnabled(IBK::SolverArgsParser::DO_STEP_STATS);
			solverCoolingDesignCalc.m_logDirectory = modelCoolingDesignCalc.dirs().m_logDir.str();
			solverCoolingDesignCalc.run();

			// write results file
			model.writeDesignDaySummary( );
			return EXIT_SUCCESS;
		}

		// *** if FMU option is set, generate FMU container
		if (static_cast<IBK::ArgParser>(args).hasOption("fmu-export")) {
			// generate FMU
			std::string targetFileStr = static_cast<IBK::ArgParser>(args).option("fmu-export");
			// special format for fmu report
			if (targetFileStr.empty()) {
				throw IBK::Exception("Missing target file for option 'fmu-export'!", "[main]");
			}
			IBK::Path targetFile(targetFileStr);

			// special format for fmu report
			if(static_cast<IBK::ArgParser>(args).hasOption("fmu-report-format")) {
				std::string reportExtension = static_cast<IBK::ArgParser>(args).option("fmu-report-format");
				model.export2FMU(args.m_executablePath.parentPath(), args.m_projectFile, targetFile, reportExtension);
			}
			else {
				model.export2FMU(args.m_executablePath.parentPath(), args.m_projectFile, targetFile);
			}
			// create wrapper file for modelica environment
			if(static_cast<IBK::ArgParser>(args).flagEnabled("fmu-modelica-wrapper")) {
				// generate wrapper file
				model.createModelicaFMUAdapterAndWrapper(IBK::Path(targetFile));
			}

			return EXIT_SUCCESS;
		}

		// *** if write-project option is set, additionally write project file with all included directives
		// and references
		if (static_cast<IBK::ArgParser>(args).hasOption("write-project")) {
			std::string targetFileStr = static_cast<IBK::ArgParser>(args).option("write-project");
			// special format for fmu report
			if (targetFileStr.empty()) {
				throw IBK::Exception("Missing target file for option 'write-project'!", "[main]");
			}
			IBK::Path targetFile(targetFileStr);

			IBK::Path projectDirectory(targetFile);
			projectDirectory = projectDirectory.absolutePath();
			projectDirectory = projectDirectory.parentPath();

			if (!projectDirectory.isDirectory()) {
				IBK::IBK_Message(IBK::FormatString("Creating project output '%1'\n").arg(projectDirectory.absolutePath()),
					IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				IBK::Path::makePath(projectDirectory);
			}

			NANDRAD::Project project = model.project();
			project.m_outputsReference.clear();
			project.m_schedulesReference.clear();
			// get path of project file output
			project.writeXML(targetFile);

			return EXIT_SUCCESS;
		}

		// *** Run model through solver control framework ***
		IBK::IBK_Message("Creating solver framework\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::SolverControlFramework solver(&model);
		solver.m_useStepStatistics = args.flagEnabled(IBK::SolverArgsParser::DO_STEP_STATS);
		solver.m_logDirectory = model.dirs().m_logDir.str();
		solver.m_stopAfterSolverInit = args.flagEnabled(IBK::SolverArgsParser::GO_TEST_INIT);
		solver.m_restartFilename = model.dirs().m_varDir.str() + "/restart.bin";

		// depending on the restart settings, either run from start or continue simulation
		if (args.m_restartFrom) {
			IBK::IBK_Message("Continuing computation from selected check point\n\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			solver.restartFrom(args.m_restartTime);
		}
		else if (args.m_restart) {
			IBK::IBK_Message("Continuing computation from last recorded check point\n\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			solver.restart();
		}
		else {
			solver.run();
			if (!solver.m_stopAfterSolverInit)
				solver.writeMetrics();
			else {
				IBK::IBK_Message( IBK::FormatString("Total initialization time: %1\n").arg(initWatch.diff_str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("Critical error, simulation aborted.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}
	catch (std::exception& ex) {
		IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		IBK::IBK_Message("Critical error, simulation aborted.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

