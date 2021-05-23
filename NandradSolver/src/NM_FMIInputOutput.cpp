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

	// check size of results vector
	unsigned int nResults = m_fmiDescription->m_inputVariables.size();
	// resize results vector
	m_results.resize(nResults);
	// resize input value references
	// set all output quantities as input references
	unsigned int resultIndex = 0;

	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_inputVariables) {
		// declare FMI input value references as results
		m_FMIInputValueRefs[variable.m_fmiValueRef] = &m_results[resultIndex];
		// store input value
		m_results[resultIndex] = variable.m_fmiStartValue;
		// inputs are stored as results inside container
		++resultIndex;
	}
}


int FMIInputOutput::setTime(double /*t*/) {
	// if interpolation of input variables is enabled, calculate
	// value in integration interval based on Taylor series expansion rule
	// and store in m_results
	return 0; // signal success
}


const double * FMIInputOutput::resolveResultReference(const NANDRAD_MODEL::InputReference & valueRef,
													  QuantityDescription & quantityDesc) const
{
	IBK_ASSERT(m_fmiDescription != nullptr);
	// no fmi variables
	if(m_fmiDescription->m_inputVariables.empty())
		return nullptr;

	// search for value reference inside fmi descriptions:
	// we create a FMIVariable and search for an identic type inside fmi descriptions
	NANDRAD::FMIVariableDefinition compVariable;
	// copy data from input reference
	compVariable.m_objectID = valueRef.m_id;
	compVariable.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", valueRef.m_referenceType);
	compVariable.m_varName += std::string(".") + valueRef.m_name.m_name;
	// copy index
	if(valueRef.m_name.m_index != -1)
		compVariable.m_vectorIndex = (unsigned int) valueRef.m_name.m_index;

	// search inside container
	unsigned int resultIndex = 0;
	std::string fmiUnit;
	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_inputVariables) {
		if(variable.sameModelVarAs(compVariable)) {
			fmiUnit = variable.m_unit;
			break;
		}
		// update index
		++resultIndex;
	}

	// not found
	if(resultIndex == m_fmiDescription->m_inputVariables.size())
		return nullptr;

	// copy quantity description
	quantityDesc.m_id = valueRef.m_id;
	quantityDesc.m_name = valueRef.m_name.m_name;
	quantityDesc.m_referenceType = valueRef.m_referenceType;
	quantityDesc.m_unit = fmiUnit;
	quantityDesc.m_constant = true; // with respect to other models, this is a constant value during integration


	// return suitable value reference
	return &m_results[resultIndex];
}


void FMIInputOutput::inputReferences(std::vector<InputReference> & inputRefs) const {

	IBK_ASSERT(m_fmiDescription != nullptr);
	// no fmi variables
	if(m_fmiDescription->m_outputVariables.empty())
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
		if(variable.m_vectorIndex != NANDRAD::INVALID_ID)
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


void FMIInputOutput::setFMIInputValue(unsigned int varID, double value)
{
	std::map<unsigned int, double*>::iterator inputVarIt =
			m_FMIInputValueRefs.find(varID);
	// non-existent variable id is a progammer error (error in FMU export)
	IBK_ASSERT(inputVarIt != m_FMIInputValueRefs.end());
	IBK_ASSERT(inputVarIt->second != nullptr);
	// set value
	*inputVarIt->second = value;
}


void FMIInputOutput::getFMIOutputValue(unsigned int varID, double & value) const
{
	std::map<unsigned int, const double*>::const_iterator outputVarIt =
			m_FMIOutputValueRefs.find(varID);
	// non-existent variable id is a progammer error (error in FMU export)
	IBK_ASSERT(outputVarIt != m_FMIOutputValueRefs.end());
	// get value
	value = *outputVarIt->second;
}



} // namespace NANDRAD_MODEL
