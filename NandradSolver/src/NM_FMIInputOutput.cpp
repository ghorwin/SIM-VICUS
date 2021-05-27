/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NM_FMIInputOutput.h"

#include "NANDRAD_FMIVariableDefinition.h"
#include "NANDRAD_Project.h"

#include <IBK_StringUtils.h>

namespace NANDRAD_MODEL {


void FMIInputOutput::setup(const NANDRAD::Project & prj) {
	// store pointer to fmi description
	m_fmiDescription = &prj.m_fmiDescription;

	// process all FMI input variables, create an element in the m_FMIInputValues map for each
	// *different* value reference and set start value
	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_inputVariables) {
		std::map<unsigned int, double>::const_iterator valIt = m_FMIInputValues.find(variable.m_fmiValueRef);
		// if already an input var with this value ref exists in the map (more than one
		// input variable with the same reference id are allowed), skip it
		if (valIt != m_FMIInputValues.end())
			continue;

		// insert into map with its start value given in project file
		m_FMIInputValues[variable.m_fmiValueRef] = variable.m_fmiStartValue;
	}
}


int FMIInputOutput::setTime(double /*t*/) {
	// if interpolation of input variables is enabled, calculate
	// value in integration interval based on Taylor series expansion rule
	// and store in m_FMIInputValues
	return 0; // signal success
}


const double * FMIInputOutput::resolveResultReference(const NANDRAD_MODEL::InputReference & valueRef,
													  QuantityDescription & quantityDesc) const
{
	IBK_ASSERT(m_fmiDescription != nullptr);
	// no fmi variables
	if (m_fmiDescription->m_inputVariables.empty())
		return nullptr;

	// we create a FMIVariable and search for an identic type inside FMI descriptions
	NANDRAD::FMIVariableDefinition variable;
	// copy data from input reference
	variable.m_objectID = valueRef.m_id;
	variable.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", valueRef.m_referenceType);
	variable.m_varName += std::string(".") + valueRef.m_name.m_name;
	// set an invalid fmi id
	variable.m_fmiValueRef = NANDRAD::INVALID_ID;
	// copy index
	if (valueRef.m_name.m_index != -1)
		variable.m_vectorIndex = (unsigned int) valueRef.m_name.m_index;

	// translate given value reference into an existing FMI variable
	// (note that different value references may share the same fmi input quantity)
	for (const NANDRAD::FMIVariableDefinition &compVar : m_fmiDescription->m_inputVariables) {
		if (variable.sameModelVarAs(compVar)) {
			// copy missing FMI reference information
			variable = compVar;
			break;
		}
	}
	// Note: alternative way of implementing this: use classic iterator loop, break when found and then
	//       compare iterator with end-iterator to check if found -> later access iterator directly -> no copy necessary

	// we dot not find a suitable fmi input variable
	if (variable.m_fmiValueRef == NANDRAD::INVALID_ID)
		return nullptr;

	// copy quantity description
	quantityDesc.m_id = valueRef.m_id;
	quantityDesc.m_name = valueRef.m_name.m_name;
	quantityDesc.m_referenceType = valueRef.m_referenceType;
	quantityDesc.m_unit = variable.m_unit;
	quantityDesc.m_constant = true; // with respect to other models, this is a constant value during integration

	// find suitable value reference (access via FMI reference ids)
	std::map<unsigned int, double>::const_iterator valIt = m_FMIInputValues.find(variable.m_fmiValueRef);
	// value reference must! exist
	IBK_ASSERT(valIt != m_FMIInputValues.end());

	return &(valIt->second);
}


void FMIInputOutput::inputReferences(std::vector<InputReference> & inputRefs) const {

	IBK_ASSERT(m_fmiDescription != nullptr);
	// no FMI variables
	if (m_fmiDescription->m_outputVariables.empty())
		return;

	// set all output quantities as input references
	// order of inputRefs matches order of m_outputVariables
	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_outputVariables) {
		// fill input reference
		InputReference inputRef;
		// copy data from input reference
		inputRef.m_id = variable.m_objectID;
		// split variable name into name and reference type
		std::vector<std::string> tokens;
		IBK::explode_in2(variable.m_varName, tokens, '.');
		IBK_ASSERT(tokens.size() == 2);
		// set name and reference type
		inputRef.m_name.m_name = tokens[1];
		inputRef.m_referenceType = (NANDRAD::ModelInputReference::referenceType_t)
				NANDRAD::KeywordList::Enumeration("ModelInputReference::referenceType_t", tokens[0]);
		// copy index
		if (variable.m_vectorIndex != NANDRAD::INVALID_ID)
			inputRef.m_name.m_index = (int) variable.m_vectorIndex;
		// store reference
		inputRefs.push_back( inputRef );
	}
}



void FMIInputOutput::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/, const std::vector<const double *> & resultValueRefs) {
	IBK_ASSERT(resultValueRefs.size() == m_fmiDescription->m_outputVariables.size());
	// value refs are sorted in same order as output variables -> we store them in map based on fmiValueRef
	for (unsigned int i=0; i<resultValueRefs.size(); ++i)
		m_FMIOutputValueRefs[m_fmiDescription->m_outputVariables[i].m_fmiValueRef] = resultValueRefs[i];

}


void FMIInputOutput::setFMIInputValue(unsigned int varID, double value) {
	std::map<unsigned int, double>::iterator inputVarIt = m_FMIInputValues.find(varID);
	// non-existent variable id is a progammer error (error in FMU export)
	IBK_ASSERT(inputVarIt != m_FMIInputValues.end());
	// set value
	inputVarIt->second = value;
}


void FMIInputOutput::getFMIOutputValue(unsigned int varID, double & value) const {
	std::map<unsigned int, const double*>::const_iterator outputVarIt = m_FMIOutputValueRefs.find(varID);
	// non-existent variable id is a progammer error (error in FMU export)
	IBK_ASSERT(outputVarIt != m_FMIOutputValueRefs.end());
	// get value
	value = *outputVarIt->second;
}



} // namespace NANDRAD_MODEL
