/*	NANDRAD Solver Framework and Model Implementation.

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

#include "NM_FMIInputOutput.h"

#include "NANDRAD_FMIVariableDefinition.h"
#include "NANDRAD_Project.h"

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
		// inputs are stored as results inside container
		++resultIndex;
	}
	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_outputVariables) {
		// store reference
		m_FMIOutputValueRefs[variable.m_fmiValueRef] = nullptr;
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
	compVariable.m_varName = valueRef.m_name.m_name;
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


	// return suitable value reference
	return &m_results[resultIndex];
}


void FMIInputOutput::inputReferences(std::vector<InputReference> & inputRefs) const {

	IBK_ASSERT(m_fmiDescription != nullptr);
	// no fmi variables
	if(m_fmiDescription->m_outputVariables.empty())
		return;

	// input references sorted via id
	std::map<unsigned int, InputReference> inputRefsMap;

	// set all output quantities as input references
	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_outputVariables) {
		// fill input reference
		InputReference inputRef;
		// copy data from input reference
		inputRef.m_id = variable.m_objectID;
		inputRef.m_name.m_name = variable.m_varName;
		// copy index
		if(variable.m_vectorIndex != NANDRAD::INVALID_ID)
			inputRef.m_name.m_index = (int) variable.m_vectorIndex;
		// store reference
		inputRefsMap[variable.m_fmiValueRef] = inputRef;
	}

	// copy input references
	for (const std::pair<unsigned int, InputReference> &inputRef : inputRefsMap)
		inputRefs.push_back(inputRef.second);
}


void FMIInputOutput::initInputReferences(const std::vector<AbstractModel *> &) {

}


void FMIInputOutput::setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) {

	FUNCID(FMIInputOutput::setInputValueRefs);

	// fill value references according to id
	IBK_ASSERT(m_FMIOutputValueRefs.size() == resultValueRefs.size());
	std::map<unsigned int, const double*>::iterator valueRefIt = m_FMIOutputValueRefs.begin();

	for(unsigned int i = 0; i < resultValueRefs.size(); ++i, ++valueRefIt)
		valueRefIt->second = resultValueRefs[i];

	// process result descriptions and check if they match FMI output variable specs
	for(const QuantityDescription &resDesc : resultDescriptions) {
		// multiple index is not allowed index
		if(!resDesc.m_indexKeys.empty())
			throw IBK::Exception(IBK::FormatString("Malformed variable '%1' in FMI definitions!").
								 arg(resDesc.m_name), FUNC_ID);

		bool found = false;

		for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_outputVariables) {
			// mismatching name
			if(variable.m_varName != resDesc.m_name)
				continue;
			found = true;
			break;
		}
		if(!found) {
			throw IBK::Exception(IBK::FormatString("Mismatching variable '%1' in FMI definitions!").
								 arg(resDesc.m_name), FUNC_ID);
		}
	}
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
	IBK_ASSERT(outputVarIt->second != nullptr);
	// get value
	value = *outputVarIt->second;
}



} // namespace NANDRAD_MODEL
