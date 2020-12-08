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

#ifndef IBK_SolverArgsParserH
#define IBK_SolverArgsParserH

#include "IBK_ArgParser.h"
#include "IBK_Path.h"

namespace IBK {

/*! A specialized version of the IBK::ArgParser that defines commonly used solver flags and the command line arguments.
	This class should be used by numerical integrator applications that support a consistent set of
	command line arguments and flags.

	Instead of querying command line arguments using their string representations, users of this class may use
	the defined enumeration values instead.
*/
class SolverArgsParser : public IBK::ArgParser {
public:

	/*! Options and flags that can only be specified on the command-line. */
	enum DebugOptions {
		DO_VERSION,
		DO_DISABLE_PERIODIC_HEADERS,
		DO_STEP_STATS,
		DO_CLOSE_ON_EXIT,
		DO_VERBOSITY_LEVEL,
		DO_RESTART_INFO, // shows information about restart file
		NUM_DebugOptions
	};

	/*! Generic options that will be set at each solver run. */
	enum GenericOptions {
		GO_RESTART = NUM_DebugOptions,
		GO_RESTART_FROM,
		GO_DISABLE_PERIODIC_RESTART_FILE_WRITING,
		GO_TEST_INIT,
		GO_OUTPUT_DIR,
		GO_PARALLEL_THREADS,
		NUM_GenericOptions
	};

	/*! Options that can be used to override properties that are stored in the project file.
		These are labeled override-options.
	*/
	enum OverrideOptions {
		OO_INTEGRATOR = NUM_GenericOptions,
		OO_LES_SOLVER,
		OO_PRECONDITIONER,
		NUM_OverrideOptions
	};

	/*! Default constructor, initializes flags based on passed command line options. */
	SolverArgsParser();


	/*! Returns a unique character for a command-line argument, or 0 if none is given.
		\param index An index in the range of all defined keywords.
	*/
	virtual char keywordChar( int index ) const;

	/*! Returns a unique keyword for a command-line argument.
		\param index An index in the range of all defined keywords.
	*/
	virtual std::string keyword( int index ) const;

	/*! Returns a description for a command-line argument.
		\param index An index in the range of all defined keywords.
	*/
	virtual std::string description( int index ) const;

	/*! Returns a descriptive value for a command-line argument.
		\param index An index in the range of all defined keywords.
	*/
	virtual std::string descriptionValue( int index ) const;

	/*! Returns a default value for a command-line argument.
		\param index An index in the range of all defined keywords.
		\return Returns a default value. An empty value means that an argument is required for this option.
	*/
	virtual std::string defaultValue( int index ) const;

	/*! Options available for each command-line argument.
		These options are used in the user interface to provide the user
		with choices. An empty vector means the user must input a string
		or a specialized implementation is required.
		\param index An index in the range of all defined keywords.
	*/
	virtual std::vector< std::string > options( int index ) const;

	/*! Re-implementation of ArgParser::parse function that extracts relevant arguments
		and stores them in member variables for easy access. */
	virtual void parse(int argc, const char * const argv[]);

	/*! Prints help page to std-output. */
	virtual void printHelp(std::ostream & out) const;

	/*! Prints man page to std-output. */
	virtual void printManPage(std::ostream & out) const;

	/*! Convenience function similar to IBK::ArgParser::hasFlag() taking enumeration
		value rather than string/character.
		\param index An index in the range of all defined keywords.
		\note This function shadows IBK::ArgParser::flagEnabled(char) so if you need the original function,
			  remember to prefix the function call with IBK::ArgParser.
	*/
	bool flagEnabled(int index) const;

	/*! Convenience function similar to IBK::ArgParser::hasOption() taking enumeration
		value rather than string/character.
		\param index An index in the range of all defined keywords.
		\note This function shadows IBK::ArgParser::hasOption(char) so if you need the original function,
			  remember to prefix the function call with IBK::ArgParser.
	*/
	bool hasOption(int index) const;

	/*! Convenience function similar to IBK::ArgParser::option() taking enumeration
		value rather than string/character.
		\param index An index in the range of all defined keywords.
		\note This function shadows IBK::ArgParser::option(char) so if you need the original function,
			  remember to prefix the function call with IBK::ArgParser.
	*/
	const std::string & option(int index) const;

	/*! Checks for errors in command line and writes corresponding error message
		to output stream.
		\return Returns true, if errors were present.
	*/
	bool handleErrors(std::ostream & errstrm);

	/*! The executable path or installation directory. */
	IBK::Path		m_executablePath;

	/*! The full path to the project file (absolute file path or relative to working directory). */
	IBK::Path		m_projectFile;

	/*! True if any form of restart was requested. */
	bool			m_restart;

	/*! True if restart of the solver from some specified time was requested. */
	bool			m_restartFrom;

	/*! Time point in [s] to restart the simulation from if m_restartFrom is true. */
	double			m_restartTime;

	/*! Parallel thread count. */
	int				m_numParallelThreads;


	/*! Holds LES solver name without optional parenthesis.*/
	std::string		m_lesSolverName;
	/*! Holds integer value passed as optional argument to les-solver (-1 for not specified) */
	unsigned int	m_lesSolverOption;
	/*! Holds preconditioner name without optional parenthesis.*/
	std::string		m_preconditionerName;
	/*! Holds integer value passed as optional argument to precond (-1 for not specified) */
	unsigned int	m_preconditionerOption;
};

} // namespace IBK

/*! \file IBK_SolverArgsParser.h
	\brief Contains the declaration of class SolverArgsParser.
*/

#endif // IBK_SolverArgsParserH
