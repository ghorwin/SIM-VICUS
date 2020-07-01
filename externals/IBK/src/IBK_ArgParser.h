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

#ifndef IBK_ArgParserH
#define IBK_ArgParserH

#include <iomanip>
#include <string>
#include <vector>
#include <set>

namespace IBK {

/*! Provides command line argument parsing functionality.

	The following example illustrates the use of the parser:
	\code
	int main(int argc, char * argv[]) {
		IBK::ArgParser args;
		// add flags and options, flags are characterized
		// through
		args.addOption('x',"close-on-exit",
			"Closes application window when program closes.\n(Windows only).",
			"true|false",
			"false");
		args.addOption('v',
			"verbosity",
			"Sets the verbosity level. -v=0 or --verbosity=0 means minimal output.",
			"0..4",
			"1");

		// now parse the command line arguments
		args.parse(argc, argv);
		// the h option is defined by default, we can print the help page if needed
		if (args.flagEnabled('h')) {
			args.printHelp(cout);
			return 0;
		}
		// query option value
		string verbosity = args.option('v');
		// same as
		verbosity = args.option("verbosity");
		// casting into appropriate value is done by user code, not by the ArgParser class

		return EXIT_SUCCESS;
	}
	\endcode

	Alternatively, you will derive from ArgParser and set the appropriate flags and options
	in the constructor of your derived class. You can then override the parse function
	and after successfull call to ArgParser::parse() you can retrieve and implement your
	conversion functions for given options.

	For console solvers consider using the IBK::SolverArgsParser implementation which
	already provides a lot of common functionality.

	/// \todo : Discuss convertion from ANSI to utf8 in Windows: shoul this be a basic functionality of
	ArgsParser?

*/
class ArgParser {
public:
	/*! Default constructor, initializes flags based on passed command line options. */
	ArgParser();

	/*! Virtual destructor, so that derived classes can have own members. */
	virtual ~ArgParser() {}

	/*! Set the application/executable name (printed in printHelp())
		\param appname Application name
		\deprecated
	*/
	void setAppName(const std::string & appname);

	/*! Parses command line arguments.
		\param argc Number of command line arguments
		\param argv List of command line arguments
	*/
	virtual void parse(int argc, const char * const argv[]);

	/*! Adds a new option to the list of known options.
		\param shortVersion	A single character, can be a 0 if no short version exists.
		\param longVersion	A string with the long version, preferably of format "<token>-<token>".
		\param description	An optional description used for the man and help pages.
		\param descValue	The value shown in the help/man page as option argument, for example 'FULL PATH' as
							in "--install-dir=<FULL PATH>". For flags use 'true|false'.
		\param defaultValue	A default value to be used when the option is not set. Flags must have
							either 'true' or 'false' as default arguments.

		An option is given on the command line in form
		\code
		-v=1                // single character version
		--verbosity=1       // long version of the flag
		--install-dir="C:\Program Files\MyApp"    // string arguments with spaces must be enclosed in quotes
		\endcode

		A flag is given on the command line with either 'true' or 'false' as argument
		\code
		-d=true                  // a single character flag
		--enable-debugging=true  // long version of the flag
		\endcode
		Alternatively use the short syntax: when given, option/flag is set to given value, if missing
		option is set to default value
		\code
		-d                       // a single character flag
		--enable-debugging       // long version of the flag
		\endcode

		There must not be spaces between the option, =, and value. String arguments must be
		in double-quotes when they contain whitespaces.

		Whether the user has specified an option can be queried with hasOption(). The value of the
		option (either user argument or default value) can be queried with option().

		\warning An assertion is raised when the same option (short or long version) has previously defined.
	*/
	void addOption(const char shortVersion, const std::string & longVersion, const std::string & description,
		const std::string & descValue, const std::string & defaultValue);

	/*! Convenience function for adding flags (options with true/false arguments and "false" as default).
		\code
		// adding a flag
		args.addFlag('v', "version", "Show solver version info.");
		// is actually the same as
		args.addOption('v', "version", "Show solver version info.", "true|false", "false");
		\endcode
	*/
	void addFlag(const char shortVersion, const std::string & longVersion, const std::string & description, const std::string & defaultValue = "false") {
		addOption(shortVersion, longVersion, description, "true|false", defaultValue);
	}

	/*! Set an option value, this can be used to set programmatically an option value
		as if it was parsed from the command-line.
		To unset a flag (so it doesn't show up when printing commandLine() ) set
		an empty string as value.
		\param shortVersion	The shortversion identifier of an option.
		\param value		The value set for this option. For flags use 'true' or 'false' as arguments.
	*/
	void setOption( const char shortVersion, const std::string & value);

	/*! Set an option value, this can be used to set programmatically an option value
		as if it was parsed from the command-line.
		To unset a flag (so it doesn't show up when printing commandLine() ) set
		an empty string as value.
		\param longVersion	The longversion identifier of an option.
		\param value		The value set for this option. For flags use 'true' or 'false' as arguments.
	*/
	void setOption( const std::string & longVersion, const std::string & value );

	/*! Returns true, if user has specified this option on the command line.
		\param shortVersion	The shortversion identifier of an option.
		\return			False if option isn't found, true otherwise.
	*/
	bool hasOption(const char shortVersion) const;

	/*! Checks if user has specified this option on the command line.
		\param longVersion	The longversion identifier of an option.
		\return			False if option isn't found, true otherwise.
	*/
	bool hasOption(const std::string & longVersion) const;

	/*! Removes an option again, useful for customizing predefined argument lists.
		Throws an exception if the requested option does not exist.
	*/
	void removeOption(const std::string & longVersion);

	/*!  Gives access to a command line option by the short version string identifier.
		\param shortVersion	The shortversion identifier of an option.
		\return			Value of the option, either the default passed to addOption() or the
		command-line value if provided by the user.
	*/
	const std::string & option(const char shortVersion) const;

	/*! Gives access to a command line option by the long version string identifier.
		\param longVersion	The longversion identifier of an option.
		\return			Value of the option, either the default passed to addOption() or the
		command-line value if provided by the user.
	*/
	const std::string & option(const std::string & longVersion) const;

	/*! Convenience function, same as checking option(..) for 'true' result.
		\param shortVersion	The shortversion identifier of a flag.
		\return			Value of the flag, 'true' if flag is enabled and
		'false' otherwise
	*/
	bool flagEnabled(const char shortVersion) const { return option(shortVersion) == "true"; }

	/*! Convenience function, same as checking option(..) for 'true' result.
		\param longVersion	The longversion identifier of a flag.
		\return			Value of the flag, 'true' if flag is enabled and
		'false' otherwise
	*/
	bool flagEnabled(const std::string & longVersion) const { return option(longVersion) == "true"; }

	/*! Prints help page to std-output.
		\param out	Stream buffer of the help output.
	*/
	virtual void printHelp(std::ostream & out) const;

	/*! Prints man page to std-output.
		\param out	Stream buffer of the manual output.
	*/
	virtual void printManPage(std::ostream & out) const;

	/*! Handles the default flags -h, --man-page, --cmd-line and --options-left and calls appropriate
		functions printHelp(), printManPage().
		\param out Output stream to direct output of help/man-page printing routines to.
		\return Returns true if either help-page or man-page was printed.
	*/
	virtual bool handleDefaultFlags(std::ostream & out) const;

	/*! Returns unused/unknown options/flags.
		\return List of option or name strings.
	*/
	const std::vector<std::string> & optionsLeft() const { return m_optionsLeft; }

	/*! Returns arguments.
		\return List of all extracted command line argument strings.
	*/
	const std::vector<std::string> & args() const { return m_args; }

	/*! Returns command line as it was understood by the parser (excluding all unknown options).
		\return Superset string of all correctly read command line arguments.
	*/
	std::string commandLine() const;

	/*! Returns command line as it was set by the user.
		\return Original command line string.
	*/
	std::string commandLineForOptions() const;

	// Public attributes, used to define output of help and man pages. */

	/*! Application name, used in the syntax line of printHelp(). */
	std::string								m_appname;

	/*! Short description of the program. */
	std::string								m_manVersionString;
	/*! Short description of the program. */
	std::string								m_manShortDescription;
	/*! Long description of the program. */
	std::string								m_manLongDescription;
	/*! Author info to show in the man page. */
	std::string								m_manAuthor;


protected:
	/*! Holds information about a known flag/option. */
	struct OptionType {
		/*! Default constructor. */
		OptionType() {}
		/*! Constructor for only the short and long argument strings. */
		OptionType(const char shortVersion, const std::string & longVersion,
			const std::string & description) :
			m_shortVersion(shortVersion),
			m_longVersion(longVersion),
			m_description(description),
			m_isFlag(true),
			m_isSet(false)
		{
		}

		/*! Constructor with all attributes of an option.

		*/
		OptionType(const char shortVersion, const std::string & longVersion,
			const std::string & description, const std::string & descValue, const std::string & defaultValue) :
			m_shortVersion(shortVersion),
			m_longVersion(longVersion),
			m_description(description),
			m_defaultValue(defaultValue),
			m_descValue(descValue),
			m_isFlag(false),
			m_isSet(false)
		{
		}

		/*! Comparison operator for short option names. */
		bool operator==(const char & shortVersion) const { return m_shortVersion == shortVersion; }
		/*! Comparison operator for long option names. */
		bool operator==(const std::string & longVersion) const { return m_longVersion == longVersion; }

		char		m_shortVersion;		///< The short version character.
		std::string m_longVersion;		///< The long version string.
		std::string m_description;		///< A description for the option.
		std::string m_defaultValue;		///< The default value.
		std::string m_givenValue;		///< Holds the value parsed from the command line if option is given.
		std::string m_descValue;		///< The value placeholder in help/man page (\sa addOption)
		bool		m_isFlag;			///< True, if this is a flag.
		bool		m_isSet;			///< True, if flag is set/enabled.
	};

	/*! Finds a string requested flag or option in the set of all registered option
		values (return m_knownOptions.size() if not found).
		\param shortVersion The shortversion identifier of the option/flag.
		\param longVersion  The longversion identifier of the option/flag.
		\return index of flag/option in vector m_knownOptions
	*/
	unsigned int findOption(const char shortVersion, const std::string & longVersion) const;

	/*! Prints flags-section of help page.
		\param out			Stream buffer for direct output of the flag section.
		\param TEXT_WIDTH	Text width for the actual lines.
		\param TAB_WIDTH	Tab width for the actual lines.
	*/
	void printFlags(std::ostream & out, unsigned int TEXT_WIDTH, unsigned int TAB_WIDTH) const;

	/*! Prints options-section of help page.
		\param out			Stream buffer for direct output of the options section.
		\param TEXT_WIDTH	Text width for the actual lines.
		\param TAB_WIDTH	Tab width for the actual lines.
	*/
	void printOptions(std::ostream & out, unsigned int TEXT_WIDTH, unsigned int TAB_WIDTH) const;

	/*! Formats a help line with command and description and scale it to the actual text width.
		\param firstToken	Command or flag.
		\param str			Description for the firstToken.
		\param lines		Holds the formated lines.
		\param tabWidth		Tab width for the actual lines.
		\param textWidth	Text width for the actual lines.
	*/
	void formatLines(const std::string & firstToken,
					 const std::string & str,
					 std::vector<std::string> & lines,
					 unsigned int tabWidth,
					 unsigned int textWidth) const;

	/*! Stores options not accepted/known to the parser.
		These options can be queried with optionsLeft()
	*/
	std::vector<std::string>				m_optionsLeft;

	/*! List of all arguments extracted from the command line in the order they were received. */
	std::vector<std::string>				m_args;

	/*! Stores list of known flags/options. */
	std::vector<OptionType>					m_knownOptions;
};

} //namespace IBK

/*! \file IBK_ArgParser.h
	\brief Contains the declaration of the class ArgParser.

	\example ArgParser.cpp
	This small example demonstrates the use of the IBK::ArgParser.
*/

#endif // IBK_ArgParserH
