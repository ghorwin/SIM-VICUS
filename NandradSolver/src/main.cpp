#include <iostream>

#include <IBK_MessageHandler.h>
#include <IBK_MessageHandlerRegistry.h>
#include <IBK_messages.h>

// include solver control framework and integrator
#include <SOLFRA_SolverControlFramework.h>

// include header for command line argument parser
#include <NANDRAD_ArgsParser.h>
#include <NANDRAD_Project.h>

// include model implementation class
#include "NM_NandradModel.h"

#define SERIALIZATION_TEST
#ifdef SERIALIZATION_TEST
#include <NANDRAD_SerializationTest.h>
#include <NANDRAD_Utilities.h>
#include <tinyxml.h>
#endif // SERIALIZATION_TEST

const char * const PROGRAM_INFO =
	"NANDRAD Solver\n"
	"All rights reserved.\n\n"
	"The NANDRAD Development Team:\n"
	"Anne Paepcke, Andreas Nicolai\n"
	"Contact: \n"
	"  anne.paepcke [at] tu-dresden.de\n"
	"  andreas.nicolai [at] tu-dresden.de\n\n";


void createSim01(NANDRAD::Project &prj){
	//create a zone
	NANDRAD::Zone zone;
	zone.m_id = 1;
	zone.m_displayName = "TestZone01";
	zone.m_type = NANDRAD::Zone::ZT_ACTIVE;
	zone.m_para[NANDRAD::Zone::ZP_AREA].set("Area", 10, IBK::Unit("m2"));
	zone.m_para[NANDRAD::Zone::ZP_VOLUME].set("Volume", 30, IBK::Unit("m3"));
	zone.m_para[NANDRAD::Zone::ZP_TEMPERATURE].set("Temperature", 5, IBK::Unit("C"));
	//add zone to prj
	prj.m_zones[zone.m_id] = zone;

	prj.m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].set("StartYear", 2015);
	prj.m_solverParameter.initDefaults();

	prj.m_location.m_climateFileName = IBK::Path("climate\testClimate.epw");
	prj.m_location.m_para[NANDRAD::Location::LP_LATITUDE].set("Latitude", 51, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::LP_LONGITUDE].set("Longitude",13, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::LP_ALTITUDE].set("Altitude",100, IBK::Unit("m"));
	prj.m_location.m_para[NANDRAD::Location::LP_ALBEDO].set("Albedo", 0.2, IBK::Unit("-"));

	NANDRAD::ConstructionInstance conInsta;
	conInsta.m_id = 2;
	conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
	conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
	conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));

	conInsta.m_displayName = "South Wall";

	//create interface
	NANDRAD::Interface interface;
	interface.m_id = 3;
	interface.m_zoneId = 1;
	interface.m_location = NANDRAD::Interface::IT_A;
	interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
	interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
	//add first interface
	conInsta.m_interfaces.push_back(interface);

	//add second interface
	interface.m_location = NANDRAD::Interface::IT_B;
	interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
	interface.m_zoneId = 0;
	interface.m_id = 4;
	//add second interface
	conInsta.m_interfaces.push_back(interface);

	conInsta.m_constructionTypeId = 10001;
	//add construction instance
	prj.m_constructionInstances[conInsta.m_id] = conInsta;

	//outputs

	NANDRAD::OutputGrid grid;
	grid.m_name = "hourly";
	NANDRAD::Interval intVal;
	intVal.m_para[NANDRAD::Interval::IP_END].set("End", 1, IBK::Unit("h"));
	intVal.m_para[NANDRAD::Interval::IP_STEPSIZE].set("StepSize", 1, IBK::Unit("h"));
	grid.m_intervals.push_back(intVal);

	NANDRAD::Outputs outputs;
	outputs.m_grids.push_back(grid);
	prj.m_outputs.m_grids.push_back(grid);

	NANDRAD::OutputDefinition outDef;
	outDef.m_quantity = "Temperature";

	prj.m_outputs.m_outputDefinitions.push_back(outDef);

}

int main(int argc, char * argv[]) {
	FUNCID(main);

	NANDRAD::Project prj;
	// parameter setzen

	prj.writeXML(IBK::Path("SimQuality1.xml"));

#ifdef SERIALIZATION_TEST
	NANDRAD::SerializationTest st;
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "NandradProject" );
	doc.LinkEndChild(root);

	// write all project parts
	st.writeXML(root);
	IBK::Path filenamePath("serializationtest.xml");
	doc.SaveFile( filenamePath.str() );

	// now read in the file in separate object and compare

	NANDRAD::SerializationTest st2;
	TiXmlDocument doc2;
	std::map<std::string,IBK::Path> placeholders;
	TiXmlElement * xmlElem = NANDRAD::openXMLFile(placeholders, filenamePath, "NandradProject", doc2);
	if (!xmlElem)
		return 1; // empty project, this means we are using only defaults

	// we read our subsections from this handle
	TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

	try {
		// Project Info
		xmlElem = xmlRoot.FirstChild("SerializationTest").Element();
		st2.readXML(xmlElem);
	}
	catch (IBK::Exception &ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message(IBK::FormatString("Error in line %1 of project file '%2'.")
			.arg(xmlElem->Row()).arg(filenamePath), IBK::MSG_ERROR, FUNC_ID);
	}

	return 0;

#endif // SERIALIZATION_TEST

	try {
		// a stopwatch to measure time needed for solver initialization
		IBK::StopWatch initWatch;

		// *** Command line parsing ***
		NANDRAD::ArgsParser args;
		args.parse(argc, argv);
		// handle default arguments like help and man-page requests, which are printed to std::cout
		if (args.handleDefaultFlags(std::cout))
			// stop if help/man-page requested
			return EXIT_SUCCESS;
		if (args.flagEnabled(IBK::SolverArgsParser::DO_VERSION)) {
			std::cout << PROGRAM_INFO << std::endl;
			NANDRAD_MODEL::NandradModel::printVersionStrings();
			SOLFRA::SolverControlFramework::printVersionInfo();
			IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			return EXIT_SUCCESS;
		}

		// check if errors are present, error messages are written to std::cerr
		if (args.handleErrors(std::cerr))
			return EXIT_FAILURE;

		// *** create main model instance ***
		NANDRAD_MODEL::NandradModel model;

		// *** create directory structure ***
		model.setupDirectories(args);
		// now we have a log directory and can write our messages to the log file

		// *** setup message handler ***

		unsigned int verbosityLevel = IBK::string2val<int>(args.option(IBK::SolverArgsParser::DO_VERBOSITY_LEVEL));
		IBK::MessageHandler * messageHandlerPtr = IBK::MessageHandlerRegistry::instance().messageHandler();
		messageHandlerPtr->setConsoleVerbosityLevel(verbosityLevel);
		messageHandlerPtr->setLogfileVerbosityLevel(verbosityLevel);
		messageHandlerPtr->m_contextIndentation = 48;
		std::string errmsg;

		IBK::Path logfile = model.dirs().m_logDir / "screenlog.txt";
		bool success = messageHandlerPtr->openLogFile(logfile.str(), args.m_restart, errmsg);
		if (!success) {
			IBK::IBK_Message(errmsg, IBK::MSG_WARNING, FUNC_ID);
			IBK::IBK_Message("Cannot create log file, outputs will only be printed on screen.", IBK::MSG_WARNING, FUNC_ID);
		}

		// *** write program/copyright info ***
		IBK::IBK_Message(PROGRAM_INFO, IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		NANDRAD_MODEL::NandradModel::printVersionStrings();
		SOLFRA::SolverControlFramework::printVersionInfo();
		IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// *** Initialize model. ***

		// init model (first read project, then initialize model)
		model.init(args);
		IBK::IBK_Message( IBK::FormatString("Model initialization complete, duration: %1\n\n").arg(initWatch.diff_str()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// *** Run model through solver control framework ***
		IBK::IBK_Message("Creating solver framework\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::SolverControlFramework solver(&model);
		solver.m_useStepStatistics = args.flagEnabled(IBK::SolverArgsParser::DO_STEP_STATS);
		solver.m_logDirectory = model.dirs().m_logDir;
		solver.m_stopAfterSolverInit = args.flagEnabled(IBK::SolverArgsParser::GO_TEST_INIT);
		solver.m_restartFilename = model.dirs().m_varDir / "restart.bin";

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

