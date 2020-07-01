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

#include <iostream>
#include <sstream>
#include <algorithm>

#include "IBK_ArgParser.h"

#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"
#include "IBK_Exception.h"
#include "IBK_assert.h"
#include "IBK_messages.h"
#include "IBK_FormatString.h"

using namespace std;

namespace IBK {

/*! Local helper function for ArgParser. */
std::string composeFlagString(const char shortVersion, const std::string & longVersion) {
	string flagstr;
	if (shortVersion != 0)
		flagstr = string("-") + shortVersion;
	if (!longVersion.empty()) {
		if (!flagstr.empty())
			flagstr += ", ";
		flagstr += "--" + longVersion;
	}
	return flagstr;
}

/*! Local helper function for ArgParser. */
std::string composeOptionString(const char shortVersion, const std::string & longVersion, std::string optionDesc) {
	string flagstr;
	if (optionDesc.empty()) {
		optionDesc = "...";
	}
	if (shortVersion != 0)
		flagstr = string("-") + shortVersion + "=<" + optionDesc + ">";
	if (!longVersion.empty()) {
		if (!flagstr.empty())
			flagstr += ", --" + longVersion + "=<...>";
		else
			flagstr += "--" + longVersion + "=<" + optionDesc + ">";
	}
	return flagstr;
}

// *** ArgParser ***

ArgParser::ArgParser() {
	addFlag(0, "help", "Prints the help page.");
	addFlag(0, "man-page", "Prints the man page (source version) to std output.");
	addFlag(0, "cmd-line", "Prints the command line as it was understood by the command line parser.");
	addFlag(0, "options-left", "Prints all options that are unknown to the command line parser.");
	m_appname = "<application>";
}

void ArgParser::setAppName(const std::string & appname) {
	m_appname = appname;
}

void ArgParser::parse(int argc, const char* const argv[]) {

	int argcount = 0;
	std::string appName = argv[0];

	m_args.push_back(appName); // store application name

	while (++argcount < argc) {

		// check for options starting with '-'
		if (argv[argcount][0]=='-') {
			// check if we have an option or an =
			string arg( argv[argcount] );
			string option;
			string::size_type pos = arg.find('=');
			if (pos != string::npos) {
				option = arg.substr(pos+1);
				IBK::trim(option, " \t");
				arg = arg.substr(1, pos-1);
			}
			else
				arg = arg.substr(1);
			// "-" or "--" is not allowed
			if (arg.empty()) {
				m_optionsLeft.push_back(argv[argcount]);
				continue;
			}
			// now scan for --
			if (arg[0] == '-') {
				// check for long versions
				arg = arg.substr(1);
				if (arg.empty()) {
					m_optionsLeft.push_back(argv[argcount]);
					continue;
				}
				unsigned int idx = findOption(0, arg);
				if (idx != m_knownOptions.size()) {
					// special handling of flags: if user simply specifies
					// flag, we store 'true' instead of an empty given value
					if (option.empty() && m_knownOptions[idx].m_isFlag) {
						option = "true";
					}
					m_knownOptions[idx].m_givenValue = option;
				}
				else {
					// remember unknown option in options-left vector
					m_optionsLeft.push_back(argv[argcount]);
				}
			}
			else {
				// check for short versions
				unsigned int idx = findOption(arg[0], "");
				if (idx != m_knownOptions.size()) {
					// special handling of flags: if user simply specifies
					// flag, we store 'true' instead of an empty given value
					if (option.empty() && m_knownOptions[idx].m_isFlag) {
						option = "true";
					}
					m_knownOptions[idx].m_givenValue = option;
				}
				else {
					// special handling, if first character after - is a number or a ., then treat it as positional argument
					if (std::string("0123456789.").find(arg[0]) != std::string::npos)
						m_args.push_back(argv[argcount]); // remember as positional argument
					else
						m_optionsLeft.push_back(argv[argcount]); // invalid/unknown flag
				}
			}
		}
		else {
			// no option, must be an argument
			m_args.push_back(argv[argcount]);
		}
	} // while
}


void ArgParser::addOption(const char shortVersion, const std::string & longVersion, const std::string & description,
						  const std::string & descValue, const std::string & defaultValue)
{
	const char * const FUNC_ID = "[ArgParser::addOption]";
	ArgParser::OptionType ft(shortVersion, longVersion, description, descValue, defaultValue);
	// is this a flag?
	if (defaultValue == "true" || defaultValue == "false")
		ft.m_isFlag = true;
	// check if option already exists
	unsigned int idx = findOption(shortVersion, longVersion);
	if (idx != m_knownOptions.size()) {
		IBK::IBK_Message( IBK::FormatString("Overwriting existing option '%1' / '%2' not allowed.").arg(shortVersion).arg(longVersion),
						  IBK::MSG_ERROR, FUNC_ID);
		exit(0); // kill application, throwing an exception here is useless, since argparser is typically not wrapped by a try-catch-clause.
	}
	// we disallow overwriting of existing options because of following problems:
	// - accidental overwriting of options
	// - accidental definition of two options:
	//   suppose an option "-x" exists
	//   and another option "--exclude" is added
	//   later, someone changes code to have -x also as argument alternative to --exclude -> no the first
	//   option is overwritten by accident.
	m_knownOptions.push_back(ft);
}


void ArgParser::setOption( const char shortVersion, const std::string & value ){
	// find ID
	unsigned int idx = findOption( shortVersion, "" );
	if (idx == m_knownOptions.size())
		throw IBK::Exception(IBK::FormatString("Unknown option '-%1'.").arg(shortVersion), "[ArgParser::setOption]");

	// set value if it is not a flag
	m_knownOptions[idx].m_givenValue = value;
}

void ArgParser::setOption( const std::string & longVersion, const std::string & value ){
	// find ID
	unsigned int idx = findOption( 0, longVersion );
	if (idx == m_knownOptions.size())
		throw IBK::Exception(IBK::FormatString("Unknown option '--%1'.").arg(longVersion), "[ArgParser::setOption]");

	// set value
	m_knownOptions[idx].m_givenValue = value;
}

bool ArgParser::hasOption(const char shortVersion) const {
	// try to find the index of the flag in the vector with known flags
	unsigned int idx = findOption(shortVersion, "");
	if (idx == m_knownOptions.size()) {
		throw IBK::Exception(IBK::FormatString("Option '%1' has not been defined with addOption().")
			.arg(std::string(1, shortVersion)), "[ArgParser::hasOption]");
	}
	// flag/option given?
	return !m_knownOptions[idx].m_givenValue.empty();
}

bool ArgParser::hasOption(const std::string & longVersion) const {
	// try to find the index of the flag in the vector with known flags
	unsigned int idx = findOption(0, longVersion);
	if (idx == m_knownOptions.size()) {
		throw IBK::Exception(IBK::FormatString("Option '%1' has not been defined with addOption().")
			.arg(longVersion), "[ArgParser::hasOption]");
	}
	// flag/option given?
	return !m_knownOptions[idx].m_givenValue.empty();
}

void ArgParser::removeOption(const std::string & longVersion) {
	// try to find the index of the flag in the vector with known flags
	unsigned int idx = findOption(0, longVersion);
	if (idx == m_knownOptions.size()) {
		throw IBK::Exception(IBK::FormatString("Option '%1' has not been defined with addOption().")
			.arg(longVersion), "[ArgParser::removeOption]");
	}
	// remove entry
	m_knownOptions.erase(m_knownOptions.begin()+idx);
}

const std::string & ArgParser::option(const char shortVersion) const {
	// try to find the index of the flag in the vector with known flags
	unsigned int idx = findOption(shortVersion, "");
	if (idx == m_knownOptions.size()) {
		throw IBK::Exception(IBK::FormatString("Option '%1' has not been defined with addOption().")
			.arg(std::string(1, shortVersion)), "[ArgParser::hasOption]");
	}
	// return default value if given value is empty
	if (m_knownOptions[idx].m_givenValue.empty())
		return m_knownOptions[idx].m_defaultValue;
	else
		return m_knownOptions[idx].m_givenValue;
}

const std::string & ArgParser::option(const std::string & longVersion) const {
	// try to find the index of the flag in the vector with known flags
	unsigned int idx = findOption(0, longVersion);
	if (idx == m_knownOptions.size()) {
		throw IBK::Exception(IBK::FormatString("Option '%1' has not been defined with addOption().")
			.arg(longVersion), "[ArgParser::hasOption]");
	}
	// return default value if given value is empty
	if (m_knownOptions[idx].m_givenValue.empty())
		return m_knownOptions[idx].m_defaultValue;
	else
		return m_knownOptions[idx].m_givenValue;
}

void ArgParser::printHelp(std::ostream & out) const {
	out << "Syntax: " << m_appname << " [flags] [options] arg1 arg2 ...\n\n";
	const unsigned int TEXT_WIDTH = 79;
	const unsigned int TAB_WIDTH = 20;
	printFlags(out,TEXT_WIDTH,TAB_WIDTH);
	printOptions(out,TEXT_WIDTH,TAB_WIDTH);
}

void ArgParser::printManPage(std::ostream & out) const {
	// header line: .TH [name of program] [section number] [center footer] [left footer] [center header]
	// example:     .TH foo 1 "14 May 1999" "version 1.0"
	out << ".TH \"" << m_appname << "\" 1 \"" << __DATE__ << "\" \"version " << m_manVersionString << "\"\n";
	// .SH NAME
	// foo - my own text editor
	out << ".SH NAME\n";
	out << m_appname << " - " << m_manShortDescription << "\n";
}


bool ArgParser::handleDefaultFlags(std::ostream & out) const {
	try {
		if (flagEnabled("cmd-line")) {
			out << "Command line: " << commandLine() << endl;
		}

		if (flagEnabled("options-left") && !m_optionsLeft.empty()) {
			out << "Options left:\n";
			for (unsigned int i=0; i<m_optionsLeft.size(); ++i)
				out << m_optionsLeft[i] << " ";
			out << endl;
		}

		if (flagEnabled("help")) {
			printHelp(out);
			return true;
		}

		if (flagEnabled("man-page")) {
			printManPage(out);
			return true;
		}
	}
	catch (std::exception & ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "Exception while processing default command-line arguments." << std::endl;
	}
	return false;
}


std::string ArgParser::commandLine() const {
	string cmdline;
	// first the executable path
	cmdline += m_args[0];
	// finally all remaining arguments
	for (unsigned int i=1; i<m_args.size(); ++i) {
		cmdline += " " + m_args[i];
	}
	// now all flags, in short format
	for ( unsigned int i = 0; i< m_knownOptions.size(); ++i) {
		// only print flags for now
		const OptionType & o = m_knownOptions[i];
		if (!o.m_isFlag) continue;
		// only print if value was given
		if (!o.m_givenValue.empty()) {
			if (o.m_shortVersion == 0)	cmdline += "\n --" + o.m_longVersion;
			else						cmdline += string("\n -") + o.m_shortVersion;
		}
	}
	// now all the options
	for ( unsigned int i = 0; i< m_knownOptions.size(); ++i) {
		// only print option
		const OptionType & o = m_knownOptions[i];
		if (o.m_isFlag) continue;
		if (!o.m_givenValue.empty()) {
			if (o.m_shortVersion == 0)	cmdline += "\n --" + o.m_longVersion + "=" + o.m_givenValue;
			else						cmdline += string("\n -") + o.m_shortVersion + "="  + o.m_givenValue;
		}
	}
	return cmdline;
}


std::string ArgParser::commandLineForOptions() const {
	// first the executable path
	std::string cmdline = m_appname;

	// add all flags to command line
	for ( unsigned int i = 0; i< m_knownOptions.size(); ++i) {
		// only print flags for now
		const OptionType & o = m_knownOptions[i];
		// only print if value was given
		if (o.m_givenValue.empty())
			continue;
		// for flags, simply write the argument
		if (o.m_isFlag) {
			if (o.m_shortVersion == 0)	cmdline += " --" + o.m_longVersion;
			else						cmdline += std::string(" -") + o.m_shortVersion;
		}
		else {
			if (o.m_shortVersion == 0)	cmdline += " --" + o.m_longVersion + "=" + o.m_givenValue;
			else						cmdline += string(" -") + o.m_shortVersion + "="  + o.m_givenValue;
		}
	}

	return cmdline;
}


unsigned int ArgParser::findOption(const char shortVersion, const std::string & longVersion) const {
	// search for known flags
	std::vector<OptionType>::const_iterator it;
	if (shortVersion != 0)
		it = find(m_knownOptions.begin(), m_knownOptions.end(), shortVersion);
	else
		it = find(m_knownOptions.begin(), m_knownOptions.end(), longVersion);
	unsigned int idx = (unsigned int)(std::distance(m_knownOptions.begin(), it) );
	return idx;
}

void ArgParser::printFlags(std::ostream & out, unsigned int TEXT_WIDTH, unsigned int TAB_WIDTH) const {
	// collect flags
	vector<string> flags;
	for (unsigned int i=0; i<m_knownOptions.size(); ++i) {
		if (!m_knownOptions[i].m_isFlag) continue;
		string line = string(2, ' ');
		line += composeFlagString(m_knownOptions[i].m_shortVersion, m_knownOptions[i].m_longVersion);
		line += string(2, ' ');
		vector<string> lines;
		formatLines(line, m_knownOptions[i].m_description, lines, TAB_WIDTH, TEXT_WIDTH);
		flags.insert(flags.end(), lines.begin(), lines.end());
	}

	if (!flags.empty()) {
		out << "Flags:\n";
		for (vector<string>::const_iterator it = flags.begin(); it != flags.end(); ++it)
			out << *it << endl;
		out << endl;
	}
}

void ArgParser::printOptions(std::ostream & out, unsigned int TEXT_WIDTH, unsigned int TAB_WIDTH) const {
	// collect options
	vector<string> options;
	for (unsigned int i=0; i<m_knownOptions.size(); ++i) {
		if (m_knownOptions[i].m_isFlag) continue;
		string line = string(2, ' ');
		line += composeOptionString(m_knownOptions[i].m_shortVersion, m_knownOptions[i].m_longVersion, m_knownOptions[i].m_descValue);
		line += string(2, ' ');
		vector<string> lines;
		formatLines(line, m_knownOptions[i].m_description, lines, TAB_WIDTH, TEXT_WIDTH);
		options.insert(options.end(), lines.begin(), lines.end());
	}
	if (!options.empty()) {
		out << "Options:\n";
		for (vector<string>::const_iterator it = options.begin(); it != options.end(); ++it)
			out << *it << endl;
		out << endl;
	}
}

void ArgParser::formatLines(const std::string & firstToken, const std::string & str, std::vector<std::string> & lines, unsigned int tabWidth, unsigned int textWidth) const {
	std::list<std::string> sections;
	IBK::explode(str, sections, '\n');
	std::string line;
	// wrap all lines after either \n or when text width is reached
	std::string token;
	line = firstToken;
	lines.clear();
	if (line.size() < tabWidth-1){
		line += std::string(tabWidth-1 - line.size(), ' ');
	} else {
		lines.push_back( line );
		line.clear();
	}
	for (std::list<std::string>::const_iterator it = sections.begin(); it != sections.end(); ++it) {
		std::stringstream strm(*it);
		while (strm >> token) {
			std::string longerline;
			if (line.empty())
				longerline = std::string(tabWidth, ' ') + token;
			else
				longerline = line + " " + token;
			if (longerline.size() > textWidth && line.size() != 0) {
				lines.push_back(line);
				line = std::string(tabWidth, ' ') + token;
				continue;
			}
			line.swap(longerline);
		}
		lines.push_back(line);
		line.clear();
	}
}




} // namespace IBK

