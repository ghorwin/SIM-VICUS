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
		"(optional) If given, a Modelica wrapper for the exported FMU is created. Only works in combination with 'fmu-export'.");

	// adjust options for les-solver, integrator and precond to show only the options available for NANDRAD
	for (OptionType & ot : m_knownOptions) {
		if (ot.m_longVersion == "integrator") {
			ot.m_description = "auto|CVode|ImplicitEuler|ExplicitEuler";
		}
		else if (ot.m_longVersion == "les-solver") {
			ot.m_description = "auto|Dense|KLU|GMRES|BiCGStab";
		}
		else if (ot.m_longVersion == "precond") {
			ot.m_description = "auto|ILU";
		}
	}
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
		"Running with 8 threads\n"
		"> "<< m_appname << " -p=8 <project file>\n\n"
		"Starting solver with different LES solver\n"
		"> "<< m_appname << " --les-solver=GMRES <project file>\n\n"
		"Starting solver with BiCGStab iterative solver, Krylov subspace limit of 50 and ILU preconditioner\n"
		"> "<< m_appname << " --les-solver=BiCGStab(50) --precond=ILU <project file>\n\n";
}

} // namespace NANDRAD
