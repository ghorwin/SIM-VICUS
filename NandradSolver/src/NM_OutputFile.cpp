#include "NM_OutputFile.h"

#include <NANDRAD_ObjectList.h>

namespace NANDRAD_MODEL {

void OutputFile::stepCompleted(double t) {
	// for output variables with time integration rule, do time integration here

}


void OutputFile::setInputValueRef(const InputReference & inputRef, const double * resultValueRef) {
	FUNCID(OutputFile::setInputValueRef);
	// For now we assume that the framework will tell us the result value refs in the same
	// order as we have published the input references.
	// We do some checking, though.

	unsigned int currentValueRefIndex = m_valueRefs.size();
	if (m_inputRefs[currentValueRefIndex] != inputRef)
		throw IBK::Exception(IBK::FormatString("Input reference for provided value reference does not match original order."), FUNC_ID);

	m_valueRefs.push_back(resultValueRef);
}



void OutputFile::createInputReferences() {
	// process all output definitions
	for (const NANDRAD::OutputDefinition & od : m_outputDefinitions) {
		// get the associated object list
		const NANDRAD::ObjectList * ol = od.m_objectListRef;

		// special handling for Location ref type
		if (ol->m_referenceType == NANDRAD::ModelInputReference::MRT_LOCATION) 	{
			InputReference inref;
			inref.m_id = 0;
			inref.m_constant = true;
			inref.m_required = false;
			inref.m_sourceName = od.m_quantity;
			inref.m_referenceType = ol->m_referenceType;
			m_inputRefs.push_back(inref);
			continue;
		}
		// process all IDs in the object list's id group
		for (unsigned int id : ol->m_filterID.m_ids) {
			InputReference inref;
			inref.m_id = id;
			inref.m_constant = true;
			inref.m_required = false;
			inref.m_sourceName = od.m_quantity;
			inref.m_referenceType = ol->m_referenceType;
			m_inputRefs.push_back(inref);
		}
	}
}


} // namespace NANDRAD_MODEL
