/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_SolverArgsParser.h"

#include <fstream> // to check if project file exists and can be opened
#include <iostream>
#include <sstream>
#include <list>
#include <cstring>

#if defined(_OPENMP)
#include <omp.h>
#endif // _OPENMP

#include "IBK_StringUtils.h"
#include "IBK_UnitList.h"
#include "IBK_Unit.h"
#include "IBK_Constants.h"

using namespace std;

namespace IBK {

SolverArgsParser::SolverArgsParser() :
	m_restart(false),
	m_restartFrom(false),
	m_restartTime(-1),
	m_numParallelThreads(1)
{
	// automatically add flags and options based on defined keywords
	for (int i=0; i<NUM_OverrideOptions; ++i) {
		addOption(keywordChar(i),
			keyword(i),
			description(i),
			descriptionValue(i),
			defaultValue(i));
	}
}

void SolverArgsParser::parse(int argc, const char * const argv[]) {
	const char * const FUNC_ID = "[SolverArgsParser::parse]";

	// parse parent
	IBK::ArgParser::parse(argc, argv);

	// now extract arguments
	if (args().size() > 0) {
		m_executablePath = args()[0];
	}

	if (args().size() > 1) {
		m_projectFile = args()[1];
	}

	// check restart option
	if (flagEnabled(GO_RESTART)) {
		m_restart = true;
	}

	if (findOption(0, "restart-from") != m_knownOptions.size() && hasOption(GO_RESTART_FROM)) {
		m_restart = true;
		m_restartFrom = true;
		// parse argument
		std::stringstream strm(option(GO_RESTART_FROM));
		// extract value and unit
		string ustr;
		double val;
		if (strm >> val >> ustr) {
			// convert into seconds
			try {
				IBK::Unit u(ustr);
				if (u.base_id() == IBK::Unit("s").base_id()) {
					IBK::UnitList::instance().convert(u, IBK::Unit("s"), val);
					m_restartTime = val;
				}
				else
					throw IBK::Exception( IBK::FormatString("Expected time unit for restart time parameter."), FUNC_ID);

			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex,  IBK::FormatString("Invalid value to '--%1' option.").arg(keyword(GO_RESTART_FROM)), FUNC_ID);
			}
		}
	}

	// use num parallel threads variable
	if (hasOption(GO_PARALLEL_THREADS)) {
		// try to convert argument to integer
		try {
			int numP = IBK::string2val<int>(option(GO_PARALLEL_THREADS)); // might throw when invalid number is given
			if (numP < 0)
				throw IBK::Exception(IBK::FormatString("Invalid value to '--%1' option.").arg(keyword(GO_PARALLEL_THREADS)), FUNC_ID);
			// store user-defined number of threads value
			m_numParallelThreads = numP;
#if defined(_OPENMP)
			// and set number of threads globally
			omp_set_num_threads(numP);
#endif
		}
		catch (...) {
			throw IBK::Exception(IBK::FormatString("Invalid value to '--%1' option.").arg(keyword(GO_PARALLEL_THREADS)), FUNC_ID);
		}
	} // if (hasOption(GO_PARALLEL_THREADS))
	else {

#if defined(_OPENMP)
#pragma omp parallel
{

		// no-user argument, initialize numParallelThreads with environment variable
#pragma omp master
		m_numParallelThreads = omp_get_num_threads();
}
#else // defined(_OPENMP)
		m_numParallelThreads = 1; // sequential code
#endif // defined(_OPENMP)

	} // else (hasOption(GO_PARALLEL_THREADS))

	// special cases les-solver and preconditioner
	m_lesSolverName.clear();
	m_lesSolverOption = (unsigned int)-1;
	m_preconditionerName.clear();
	m_preconditionerOption = (unsigned int)-1;

	if (hasOption(OO_LES_SOLVER)) {
		std::string lesOption = option(OO_LES_SOLVER);
		// we may have a parenthesis option
		int val;
		std::string solverName;
		IBK::ExtractResultType ertRes = IBK::extractFromParenthesis<int>(lesOption, solverName, val);
		switch (ertRes) {
			case IBK::ERT_BadNumber :
			case IBK::ERT_NoNumber :
				throw IBK::Exception( IBK::FormatString("Invalid format of --les-solver option, argument was '%1'.").arg(lesOption), FUNC_ID);
			default : ;
		}
		m_lesSolverName = solverName;
		if (ertRes == IBK::ERT_Success) {
			if (val <= 0)
				throw IBK::Exception( IBK::FormatString("Invalid integer value used in --les-solver option (value in parenthesis must be a positive integer), argument was '%1'.").arg(lesOption), FUNC_ID);
			m_lesSolverOption = val;
		}
	}

	if (hasOption(OO_PRECONDITIONER)) {
		std::string preOption = option(OO_PRECONDITIONER);
		// we may have a parenthesis option
		int val;
		std::string solverName;
		IBK::ExtractResultType ertRes = IBK::extractFromParenthesis<int>(preOption, solverName, val);
		switch (ertRes) {
			case IBK::ERT_BadNumber :
			case IBK::ERT_NoNumber :
				throw IBK::Exception( IBK::FormatString("Invalid format of --precond option, argument was '%1'.").arg(preOption), FUNC_ID);
			default : ;
		}
		m_preconditionerName = solverName;
		if (ertRes == IBK::ERT_Success) {
			if (val < 0)
				throw IBK::Exception( IBK::FormatString("Invalid integer value used in --precond option (value in parenthesis must not be negative), argument was '%1'.").arg(preOption), FUNC_ID);
			m_preconditionerOption = val;
		}
	}

	// convert all path-related arguments to UTF8
#if defined(_WIN32)

	std::string pathStr = m_projectFile.str();
	std::wstring wPrjFile = IBK::ANSIToWstring(pathStr, false);
	m_projectFile = IBK::WstringToUTF8(wPrjFile);

	if (hasOption(GO_OUTPUT_DIR)) {
		unsigned int oIdx = findOption(keywordChar(GO_OUTPUT_DIR), keyword(GO_OUTPUT_DIR));
		pathStr = m_knownOptions[oIdx].m_givenValue;
		wPrjFile = IBK::ANSIToWstring(pathStr, false);
		m_knownOptions[oIdx].m_givenValue = IBK::WstringToUTF8(wPrjFile);
	}

#endif // defined(_WIN32)

}


void SolverArgsParser::printHelp(std::ostream & out) const {
	// re-use implementation of original help function
	out << "Syntax: " << m_appname << " [options] <project file>\n\n";
	const unsigned int TEXT_WIDTH = 79;
	const unsigned int TAB_WIDTH = 26;
	printFlags(out,TEXT_WIDTH,TAB_WIDTH);
	printOptions(out,TEXT_WIDTH,TAB_WIDTH);
	// write examples
	out << "\nExamples:\n\n"
		"Starting solver\n"
		"> "<< m_appname << " <project file>\n\n"
		"Starting solver with different LES solver\n"
		"> "<< m_appname << " --les-solver=GMRES <project file>\n\n"
		"Starting solver with GMRES(30) and band preconditioner\n"
		"> "<< m_appname << " --les-solver=GMRES(30) --precond=Band <project file>\n\n"
		"Starting solver with GMRES(30) and band preconditioner with user-defined half-bandwidth\n"
		"> "<< m_appname << " --les-solver=GMRES(30) --precond=Band(1) <project file>\n";
}


void SolverArgsParser::printManPage(std::ostream & out) const {
	// first the default
	IBK::ArgParser::printManPage(out);
	// now append DELPHIN-specific examples
}


bool SolverArgsParser::flagEnabled(int index) const {
	if (keywordChar(index) != 0) {
		return IBK::ArgParser::flagEnabled( keywordChar(index) );
	}
	else if (!keyword(index).empty()) {
		return IBK::ArgParser::flagEnabled( keyword(index) );
	}
	return false;
}


bool SolverArgsParser::hasOption(int index) const {
	if (keywordChar(index) != 0) {
		return IBK::ArgParser::hasOption( keywordChar(index) );
	}
	else {
		return IBK::ArgParser::hasOption( keyword(index) );
	}
}


const std::string & SolverArgsParser::option(int index) const {
	if (keywordChar(index) != 0) {
		return IBK::ArgParser::option( keywordChar(index) );
	}
	else {
		return IBK::ArgParser::option( keyword(index) );
	}
}


bool SolverArgsParser::handleErrors(std::ostream & errstrm) {
	const char * const FUNC_ID = "[SolverArgsParser::handleErrors]";
	// check for valid project file
	try {
		if (!m_projectFile.isValid())
			throw IBK::Exception("Missing or invalid project file argument.", FUNC_ID);
		if (m_restartFrom && flagEnabled(GO_RESTART))
			throw IBK::Exception( IBK::FormatString("Cannot have both '--%1' and '--%2' options.")
				.arg(keyword(GO_RESTART)).arg(keyword(GO_RESTART_FROM)), FUNC_ID);
		if (m_restartFrom && m_restartTime == -1) {
			throw IBK::Exception( IBK::FormatString("Invalid or missing time-unit argument for option '--%1'.")
				.arg(keyword(GO_RESTART_FROM)), FUNC_ID);
		}
		int v;
		try { v = IBK::string2val<int>(option(DO_VERBOSITY_LEVEL)); }
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Invalid argument for option '--%1'.").arg(keyword(DO_VERBOSITY_LEVEL)), FUNC_ID);
		}
		if (v < 0 || v > 4) {
			throw IBK::Exception(IBK::FormatString("Invalid argument for option '--%1', number must be in the range 0..4.")
				.arg(keyword(DO_VERBOSITY_LEVEL)), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		IBK::set_console_text_color(IBK::CF_BRIGHT_RED);
		ex.writeMsgStackToError();
		IBK::IBK_Message( IBK::FormatString("Use '%1 --help' for help.").arg(m_appname), IBK::MSG_ERROR, FUNC_ID);
		IBK::set_console_text_color(IBK::CF_GREY);
		return true;
	}
	catch (std::exception & ex) {
		IBK::set_console_text_color(IBK::CF_BRIGHT_RED);
		errstrm << ex.what() << endl;
		errstrm << "\nUse '" << m_appname << " --help' for help." << endl;
		IBK::set_console_text_color(IBK::CF_GREY);
		return true;
	}
	return false;
}


char SolverArgsParser::keywordChar( int index ) const {
	switch( index ) {
		case DO_VERSION						: return 'v';
		case DO_CLOSE_ON_EXIT				: return 'x';
		case GO_RESTART						: return 'r';
		case GO_TEST_INIT					: return 't';
		case GO_OUTPUT_DIR					: return 'o';
		case GO_PARALLEL_THREADS			: return 'p';
	}
	return 0; // no short version available for this option
}


std::string SolverArgsParser::keyword( int index ) const {
	switch( index ) {
		case DO_VERSION						: return "version";
		case DO_STEP_STATS					: return "step-stats";
		case DO_VERBOSITY_LEVEL				: return "verbosity-level";
		case DO_DISABLE_PERIODIC_HEADERS	: return "disable-headers";
		case DO_CLOSE_ON_EXIT				: return "close-on-exit";
		case DO_RESTART_INFO				: return "restart-info";
		case GO_RESTART						: return "restart";
		case GO_RESTART_FROM				: return "restart-from";
		case GO_DISABLE_PERIODIC_RESTART_FILE_WRITING	: return "disable-periodic-restart-file-writing";
		case GO_TEST_INIT					: return "test-init";
		case GO_OUTPUT_DIR					: return "output-dir";
		case GO_PARALLEL_THREADS			: return "parallel-threads";
		case OO_INTEGRATOR					: return "integrator";
		case OO_LES_SOLVER					: return "les-solver";
		case OO_PRECONDITIONER				: return "precond";
		default								: return "";
	}
}


std::string SolverArgsParser::description( int index ) const {
	switch( index ) {
		case DO_VERSION						: return "Show solver version info.";
		case DO_STEP_STATS					: return "Enable statistics outputs after each completed solver step.";
		case DO_VERBOSITY_LEVEL				: return "Level of output detail (0-3).";
		case DO_DISABLE_PERIODIC_HEADERS	: return "Disable periodically printed headers in console output.";
		case DO_CLOSE_ON_EXIT				: return "Close console window after finishing simulation.";
		case DO_RESTART_INFO				: return "Prints information about the restart file (if available).";
		case GO_RESTART						: return "Continue stopped simulation from last restart check-point.";
		case GO_RESTART_FROM				: return "Continue stopped simulation from the given restart time.";
		case GO_DISABLE_PERIODIC_RESTART_FILE_WRITING	: return "Disable periodic restart file writing (do not write restart file during simulation, only at end of simulation).";
		case GO_TEST_INIT					: return "Run the solver initialization and stop.";
		case GO_OUTPUT_DIR					: return "Writes solver output to different base directory.";
		case GO_PARALLEL_THREADS			: return "Number of threads to use by the solver, 0 means use of OMP_NUM_THREADS environment variable.";
		// specific solver implementations should provide overloads for this function and provide a more
		// detailed description of the next two arguments.
		case OO_INTEGRATOR					: return "Specify an alternative integrator engine.";
		case OO_LES_SOLVER					: return "Specify an alternative linear equation system solver.";
		case OO_PRECONDITIONER				: return "Specify an alternative preconditioner for iterative solver.";
		default :
			throw IBK::Exception("Missing implementation.", "[SolverArgsParser::description]");
	}
}


std::string SolverArgsParser::descriptionValue( int index ) const {
	switch( index ) {
		case DO_VERSION						: return "true|false";
		case DO_STEP_STATS					: return "true|false";
		case DO_VERBOSITY_LEVEL				: return "0-4";
		case DO_DISABLE_PERIODIC_HEADERS	: return "true|false";
		case DO_CLOSE_ON_EXIT				: return "true|false";
		case DO_RESTART_INFO				: return "true|false";
		case GO_RESTART						: return "true|false";
		case GO_RESTART_FROM				: return "value unit";
		case GO_DISABLE_PERIODIC_RESTART_FILE_WRITING	: return "true|false";
		case GO_TEST_INIT					: return "true|false";
		case GO_OUTPUT_DIR					: return "directory";
		case GO_PARALLEL_THREADS			: return "0...MaxNumThreads";
		case OO_LES_SOLVER					: return "auto|BTridiag|Band|GMRES|BiCGStab|TFQMR";
		case OO_PRECONDITIONER				: return "auto|Band|ILU";
		case OO_INTEGRATOR					: return "auto|CVode|ImplicitEuler";
		default :
			throw IBK::Exception("Missing implementation.", "[SolverArgsParser::descriptionValue]");
	}
}


std::string SolverArgsParser::defaultValue( int index ) const {
	// an empty default value means user must provide a value when command-line argument is specified
	switch( index ) {
		case DO_VERSION						: return "false";
		case DO_STEP_STATS					: return "false";
		case DO_VERBOSITY_LEVEL				: return "1";
		case DO_DISABLE_PERIODIC_HEADERS	: return "false";
		case DO_CLOSE_ON_EXIT				: return "false";
		case DO_RESTART_INFO				: return "false";
		case GO_RESTART						: return "false";
		case GO_RESTART_FROM				: return "";
		case GO_DISABLE_PERIODIC_RESTART_FILE_WRITING	: return "false";
		case GO_TEST_INIT					: return "false";
		case GO_OUTPUT_DIR					: return "";
		case GO_PARALLEL_THREADS			: return "1"; // by default sequential run
		case OO_INTEGRATOR					: return "auto";
		case OO_LES_SOLVER					: return "auto";
		case OO_PRECONDITIONER				: return "auto";
		default :
			throw IBK::Exception("Missing implementation.", "[SolverArgsParser::defaultValue]");
	}
}


std::vector< std::string > SolverArgsParser::options( int index ) const {
	std::vector< std::string > vec;

	switch( index ) {
		case DO_DISABLE_PERIODIC_HEADERS :
		case DO_CLOSE_ON_EXIT :
		case GO_RESTART :
		case GO_TEST_INIT :
		case DO_STEP_STATS :
		case DO_RESTART_INFO :
		case GO_DISABLE_PERIODIC_RESTART_FILE_WRITING :
		case DO_VERSION  :
				vec.push_back( "true");
				vec.push_back( "false");
		break;

		case DO_VERBOSITY_LEVEL :
				vec.push_back( "0" );
				vec.push_back( "1" );
				vec.push_back( "2" );
				vec.push_back( "3" );
		break;

		case OO_INTEGRATOR :
			vec.push_back( std::string("auto") );
			vec.push_back( std::string("CVode") );
			vec.push_back( std::string("ImplicitEuler") );
			vec.push_back( std::string("RK") );
		break;

		case OO_LES_SOLVER :
			vec.push_back( std::string("auto") );
			vec.push_back( std::string("BTridiag") );
			vec.push_back( std::string("Band") );
			vec.push_back( std::string("GMRES") );
			vec.push_back( std::string("BiCGStab") );
			vec.push_back( std::string("TFQMR") );
		break;

		case OO_PRECONDITIONER :
			vec.push_back( std::string("auto") );
			vec.push_back( std::string("Band") );
			vec.push_back( std::string("ILU") );
		break;

		default :
		break;
	}

	return vec;
}


} // namespace IBK


