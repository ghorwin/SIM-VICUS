/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

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
	addOption(0, "fmu-export",
		"(optional) Generate FMU based on project file specification.",
		"<FMU file name>",
		"");
	addFlag(0, "fmu-modelica-wrapper",
		"(optional) If set to 'true' a Modelica wrapper for fmu exported file is created. Only in combination with 'fmu-export'");
//	addOption(0, "fmu-report-format",
//		"(optional) Special format for FMU export report.",
//		"<FMU report extension>",
//		"");
//	addFlag(0, "designday",
//		"(optional) Calculate a design day based on project file specification:  true = design day calculation| false = usual calculation.");
//	addFlag(0,  "report",
//		"(optional) Requests a building energy report: true = report is generated| false = report is only generated if requested from project file.");
//	addOption(0, "write-project",
//		"(optional) Write project file including all preprocessor directives and shared output/schedule references.",
//		"<output file name>",
//		"");
//	addOption(0, "output-format",
//		"(optional) Specify data format for model outputs.",
//		"<output file extension>",
//		"d6o");
}

} // namespace NANDRAD
