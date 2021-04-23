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
	unsigned int nResults = 0;
	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_variables) {
		if(variable.m_inputVariable)
			++nResults;
	}
	// resize results vector
	m_results.resize(nResults);
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
	if(m_fmiDescription->m_variables.empty())
		return nullptr;

	// search for value reference inside fmi descriptions:
	// we create a FMIVariable and search for an identic type inside fmi descriptions
	NANDRAD::FMIVariableDefinition compVariable;
	// we only request inputs
	compVariable.m_inputVariable = true;
	// copy data from input reference
	compVariable.m_objectID = valueRef.m_id;
	compVariable.m_varName = valueRef.m_name.m_name;
	// copy index
	if(valueRef.m_name.m_index != -1)
		compVariable.m_vectorIndex = (unsigned int) valueRef.m_name.m_index;

	// search inside container
	unsigned int index = 0;
	for(const NANDRAD::FMIVariableDefinition &variable : m_fmiDescription->m_variables) {
		if(variable.sameModelVarAs(compVariable)) {
			break;
		}
		// update index
		if(variable.m_inputVariable)
			++index;
	}

	// not found
	if(index == m_fmiDescription->m_variables.size())
		return nullptr;

	// copy quantity description
	quantityDesc.m_id = valueRef.m_id;
	quantityDesc.m_name = valueRef.m_name.m_name;
	quantityDesc.m_referenceType = valueRef.m_referenceType;

	// return suitable value reference
	return &m_results[index];
}


void FMIInputOutput::inputReferences(std::vector<InputReference> & inputRefs) const {

	/// \todo implement
}


void FMIInputOutput::initInputReferences(const std::vector<AbstractModel *> &) {

}


void FMIInputOutput::setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) {
	m_valueRefs = resultValueRefs;
	/// \todo process result descriptions and check if they match FMI output variable specs
}



} // namespace NANDRAD_MODEL
