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

	FUNCID(FMIDescription::checkParameters);
	// check input and output variable definition
	for (FMIVariableDefinition &var : m_inputVariables) {
		var.checkParameters();
	}
	for (FMIVariableDefinition &var : m_outputVariables) {
		var.checkParameters();
	}


	// create a uniqueness struct for input variables:
	// input variables allow duplicate fmi variable ids under the condition,
	// that fmi variable name and unit match
	std::map<std::string, std::pair<unsigned int, std::string> > usedFmiInputVars;
	// check uniqueness of all ids for all defined vars
	std::set<unsigned int> usedFmiInputIds;
	// check uniqueness of output variable names and ids
	std::set<std::string> usedFmiOutputNames;
	std::set<unsigned int> usedFmiOutputIds;

	// check for uniqueness of value references in all variables (mind valueref 42)
	usedFmiInputIds.insert(42);

	for (FMIVariableDefinition &var : m_inputVariables) {
		// check if id is used already
		std::map<std::string, std::pair<unsigned int, std::string> >::const_iterator
				refIt = usedFmiInputVars.find(var.m_fmiVarName);

		// name is unused
		if(refIt == usedFmiInputVars.end()) {
			usedFmiInputVars[var.m_fmiVarName] = std::make_pair
					(var.m_fmiValueRef, var.m_unit);
			// error: duplicate id
			if(usedFmiInputIds.find(var.m_fmiValueRef) != usedFmiInputIds.end()) {
				throw IBK::Exception(IBK::FormatString("Duplicate id %1 for FMI input variable '%2'!")
						.arg(var.m_fmiValueRef).arg(var.m_fmiVarName), FUNC_ID);
			}
			// store id
			usedFmiInputIds.insert(var.m_fmiValueRef);
		}
		// check for matching fmi definitions
		else {
			// wrong id
			unsigned int fmiId =refIt->second.first;
			if(fmiId != var.m_fmiValueRef) {
				throw IBK::Exception(IBK::FormatString("Duplicate definition of FMI input variable '%1' with id %2 instead of id %3!")
									 .arg(var.m_fmiVarName).arg(var.m_fmiValueRef).arg(fmiId), FUNC_ID);
			}
			// wrong unit
			std::string fmiUnit = refIt->second.second;
			if(fmiUnit != var.m_unit) {
				throw IBK::Exception(IBK::FormatString("Duplicate definition of FMI input variable '%1' with unit '%2' instead of 'id '%3'!")
									 .arg(var.m_fmiVarName).arg(var.m_unit).arg(fmiUnit), FUNC_ID);
			}
		}
	}
	for (FMIVariableDefinition &var : m_outputVariables) {
		// error: duplicate name
		if(usedFmiOutputNames.find(var.m_fmiVarName) != usedFmiOutputNames.end()) {
			throw IBK::Exception(IBK::FormatString("Duplicate FMI output variable '%1'!")
					.arg(var.m_fmiVarName), FUNC_ID);
		}
		// store name
		usedFmiOutputNames.insert(var.m_fmiVarName);
		// error: duplicate id
		if(usedFmiOutputIds.find(var.m_fmiValueRef) != usedFmiOutputIds.end()) {
			throw IBK::Exception(IBK::FormatString("Duplicate id %1 for FMI output variable '%2'!")
					.arg(var.m_fmiValueRef).arg(var.m_fmiVarName), FUNC_ID);
		}
		// store id
		usedFmiOutputIds.insert(var.m_fmiValueRef);
	}
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

