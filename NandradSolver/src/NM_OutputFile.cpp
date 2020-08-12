#include "NM_OutputFile.h"

#include <fstream>

#include <IBK_messages.h>
#include <IBK_Path.h>
#include <IBK_assert.h>

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_KeywordList.h>

namespace NANDRAD_MODEL {

OutputFile::~OutputFile() {
	delete m_ofstream;
}


void OutputFile::stepCompleted(double t) {
	// for output variables with time integration rule, do time integration here
	(void)t;
}


void OutputFile::setInputValueRef(const InputReference & inputRef, const QuantityDescription & resultDesc, const double * resultValueRef) {
	FUNCID(OutputFile::setInputValueRef);
	// For now we assume that the framework will tell us the result value refs in the same
	// order as we have published the input references.
	// We do some checking, though.

	unsigned int currentValueRefIndex = m_valueRefs.size();
	if (m_inputRefs[currentValueRefIndex] != inputRef)
		throw IBK::Exception(IBK::FormatString("Input reference for provided value reference does not match original order."), FUNC_ID);

	m_valueRefs.push_back(resultValueRef);
	m_quantityDescs.push_back(resultDesc);
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
			inref.m_required = false;
			inref.m_name = od.m_quantity;
			inref.m_referenceType = ol->m_referenceType;
			m_inputRefs.push_back(inref);
			continue;
		}
		// process all IDs in the object list's id group
		for (unsigned int id : ol->m_filterID.m_ids) {
			InputReference inref;
			inref.m_id = id;
			inref.m_required = false;
			inref.m_name = od.m_quantity;
			inref.m_referenceType = ol->m_referenceType;
			m_inputRefs.push_back(inref);
		}
	}
}


void OutputFile::createFile(double t_secondsOfYear, bool restart, bool binary, const IBK::Path * outputPath) {
	FUNCID(OutputFile::createFile);

	IBK_ASSERT(m_inputRefs.size() == m_valueRefs.size());
	// we go through all variables and count the number of non-zero values
	m_numCols = 0;
	bool haveIntegrals = false;
	for (unsigned int i=0; i<m_valueRefs.size(); ++i) {
		if (m_valueRefs[i] == nullptr) {
			IBK::IBK_Message(IBK::FormatString("Output for %1[%2].%3 not available, skipped.")
							 .arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
							 .arg(m_inputRefs[i].m_id)
							 .arg(m_inputRefs[i].m_name.m_name), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		}
		else {
			++m_numCols;
		}
	}
	// if we have no outputs in this file, we do nothing
	if (m_numCols == 0) {
		IBK::IBK_Message(IBK::FormatString("%1 : No output variables available, skipped\n")
						 .arg(m_filename,20,std::ios_base::left), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		return;
	}
	// compose final file path
	IBK::Path outFilePath = *outputPath / m_filename;
	if (restart) {
		if (outFilePath.exists()) {
			// try to re-open the file
			if (binary) {
#if defined(_WIN32)
	#if defined(_MSC_VER)
				m_ofstream = new std::ofstream(outFilePath.wstr().c_str(), std::ios_base::binary | std::ios_base::app);
	#else
				std::string filenameAnsi = IBK::WstringToANSI(outFilePath.wstr(), false);
				m_ofstream = new std::ofstream(filenameAnsi.c_str(), std::ios_base::binary | std::ios_base::app);
	#endif
#else
				m_ofstream = new std::ofstream(outFilePath.str().c_str(), std::ios_base::binary | std::ios_base::app);
#endif
			}
			else {
#if defined(_WIN32)
	#if defined(_MSC_VER)
				m_ofstream = new std::ofstream(outFilePath.wstr().c_str(), std::ios_base::app);
	#else
				std::string filenameAnsi = IBK::WstringToANSI(outFilePath.wstr(), false);
				m_ofstream = new std::ofstream(filenameAnsi.c_str(), std::ios_base::app);
	#endif
#else
				m_ofstream = new std::ofstream(outFilePath.str().c_str(), std::ios_base::app);
#endif
			}
			if (!m_ofstream) {
				throw IBK::Exception(IBK::FormatString("Error re-opening file %1 for writing.").arg(outFilePath), FUNC_ID);
			}

			IBK::IBK_Message(IBK::FormatString("%1 : %2 values\n").arg(m_filename,20,std::ios_base::left).arg(m_numCols), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// all ok, bail out here
			return;
		}
		else {
			IBK::IBK_Message(IBK::FormatString("Missing file '%1' during restart of simulation. The file will be created now.")
							 .arg(m_filename), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);

			// fall-through to regular "create new file" code
		}
	}

	// create file now and write header
	if (binary) {
#if defined(_WIN32)
	#if defined(_MSC_VER)
		m_ofstream = new std::ofstream(outFilePath.wstr().c_str(), std::ios_base::binary);
	#else
		std::string filenameAnsi = IBK::WstringToANSI(outFilePath.wstr(), false);
		m_ofstream = new std::ofstream(filenameAnsi.c_str(), std::ios_base::binary);
	#endif
#else
		m_ofstream = new std::ofstream(outFilePath.str().c_str(), std::ios_base::binary);
#endif
	}
	else {
#if defined(_WIN32)
	#if defined(_MSC_VER)
		m_ofstream = new std::ofstream(outFilePath.wstr().c_str());
	#else
		std::string filenameAnsi = IBK::WstringToANSI(outFilePath.wstr(), false);
		m_ofstream = new std::ofstream(filenameAnsi.c_str());
	#endif
#else
		m_ofstream = new std::ofstream(outFilePath.str().c_str());
#endif
	}

	if (!m_ofstream) {
		throw IBK::Exception(IBK::FormatString("Error creating file %1.").arg(outFilePath), FUNC_ID);
	}

	IBK::IBK_Message(IBK::FormatString("%1 : %2 values\n").arg(m_filename,20,std::ios_base::left).arg(m_numCols), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// now write header

	std::vector<double> vals(m_numCols,0);
	for (unsigned int i=0, j=0; i<m_valueRefs.size(); ++i) {
		if (m_valueRefs[i] == nullptr) continue; // skip unavailable vars

		// retrieve (initial) value for this variable
		vals[j] = *m_valueRefs[i];

		// compose column title



		++j; // increase var counter
	}
}


void OutputFile::writeOutputs(double t_secondsOfYear) {
	// no outputs - nothing to do
	if (m_numCols == 0)
		return;
}


unsigned int OutputFile::cacheSize() const {
	unsigned int cache = m_numCols * m_cache.size() * 2 * 8;
	return cache;
}


void OutputFile::flushCache() {
	// no outputs - nothing to do
	if (m_numCols == 0)
		return;

	// if
}


} // namespace NANDRAD_MODEL
