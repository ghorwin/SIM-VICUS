/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NANDRAD_ArgsParser.h"

namespace NANDRAD {

ArgsParser::ArgsParser() {
	setAppName("NandradSolver");
	addOption(0, "fmu-export",
		"(optional) Generate FMU based on project file specification.",
		"FMU file name",
		"");
	addFlag(0, "fmu-modelica-wrapper",
		"(optional) If set to 'true' a Modelica wrapper for fmu exported file is created. Only in combination with 'fmu-export'");

	/// \todo adjust options for les-solver, integrator and precond to show only the options available for NANDRAD
}


void ArgsParser::printHelp(std::ostream & out) const {
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
		"Starting solver with GMRES(30) and ILUT preconditioner\n"
		"> "<< m_appname << " --les-solver=BiCGStab(50) --precond=ILUT <project file>\n\n";
}

} // namespace NANDRAD
