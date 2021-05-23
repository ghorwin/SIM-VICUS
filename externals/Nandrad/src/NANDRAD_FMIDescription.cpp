/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_FMIDescription.h"

namespace NANDRAD {


void FMIDescription::checkParameters() {
	// check input and output variable definition
	for (FMIVariableDefinition &var : m_inputVariables) {
		var.checkParameters();
	}
	for (FMIVariableDefinition &var : m_outputVariables) {
		var.checkParameters();
	}
	// TODO : check for uniqueness of value references in all variables (mind valueref 42)
}

bool FMIDescription::hasInputVariable(const FMIVariableDefinition & var) const {
	for (const FMIVariableDefinition & v : m_inputVariables)
		if (v.sameModelVarAs(var)) return true;
	return false;
}


bool FMIDescription::hasOutputVariable(const FMIVariableDefinition & var) const {
	for (const FMIVariableDefinition & v : m_outputVariables)
		if (v.sameModelVarAs(var)) return true;
	return false;
}


} // namespace NANDRAD

