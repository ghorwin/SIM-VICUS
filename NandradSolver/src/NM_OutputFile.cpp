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

#include "NM_OutputFile.h"

#include <fstream>

#include <IBK_messages.h>
#include <IBK_Path.h>
#include <IBK_assert.h>
#include <IBK_UnitList.h>

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_KeywordList.h>

namespace NANDRAD_MODEL {

OutputFile::~OutputFile() {
	delete m_ofstream;
}


void OutputFile::stepCompleted(double t) {
	// for output variables with time integration/averaging rule, do time integration here

	if (!m_haveIntegrals)
		return; // nothing to do

	/// \todo we should add a minimum delay for executing this function - either here or in the framework itself
	///       to avoid excessive overhead when evaluating stepCompleted() calls in tiny steps

	// loop over all *available* variables and handle those with OTT_MEAN or OTT_INTEGRAL
	unsigned int col=0; // storage column index
	for (unsigned int i=0; i<m_valueRefs.size(); ++i) {
		if (m_valueRefs[i] == nullptr) continue; // skip unavailable vars
		const NANDRAD::OutputDefinition & od = m_outputDefinitions[ m_outputDefMap[i] ];
		if (od.m_timeType != NANDRAD::OutputDefinition::OTT_NONE) {

			// shift states
			double dt = t - m_tCurrentStep;  // integration interval length in [s]
			// special handling for initial call
			if (m_tLastStep == -1) {
				// on first call the step completed, usually t = 0, except in restart case
				/// \todo think of a way to restore integral values in case of restarting
				m_tLastStep = t;
				m_tCurrentStep = t;
				continue;
			}

			m_tLastStep = m_tCurrentStep;
			m_tCurrentStep = t;
			m_integrals[0][col] = m_integrals[1][col];
			// now retrieve value
			double val = *m_valueRefs[i];

			// integrate over interval, using simple rectangular rule
			double dVal = val*dt;
			// add add to integral
			m_integrals[1][col] = dVal + m_integrals[0][col];
		}
		++col;
		}
}


void OutputFile::setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions,
								   const std::vector<const double *> & resultValueRefs)
{
	FUNCID(OutputFile::setInputValueRefs);

	unsigned int currentValueRefIndex = m_valueRefs.size();

	m_valueRefs = resultValueRefs;
	m_quantityDescs = resultDescriptions;

	// process all variables and cache units
	for (unsigned int i=0; i<m_valueRefs.size(); ++i) {
		const double * resultValueRef = m_valueRefs[i];
		if (resultValueRef != nullptr) {
			try {
				m_valueUnits.push_back(IBK::Unit(m_quantityDescs[i].m_unit));
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Invalid/unknown unit provided for quantity %1 ('%2').")
									 .arg(m_quantityDescs[i].m_name).arg(m_quantityDescs[i].m_description), FUNC_ID);
			}
			// in case of integral quantities, generate time integral quantity from value
			const NANDRAD::OutputDefinition & od = m_outputDefinitions[ m_outputDefMap[currentValueRefIndex] ];
			if (od.m_timeType == NANDRAD::OutputDefinition::OTT_INTEGRAL) {
				IBK::Unit u = IBK::UnitList::instance().integralQuantity(m_valueUnits.back(), false, true);
				// special handling for energy units J and J/m2 - we rather want kWh and kWh/m2 as outputs
				if (u.name() == "J/m2")
					u.set("kWh/m2");
				else if (u.name() == "J")
						u.set("kWh");
				m_valueUnits.back() = u;
			}
		}
		else {
			// add dummy unit, since result is not available
			m_valueUnits.push_back(IBK::Unit());
		}
	}

	// initialize our cache vectors

	// we go through all variables and count the number of non-zero values
	m_numCols = 0;
	m_haveIntegrals = false;
	for (unsigned int i=0; i<m_valueRefs.size(); ++i) {
		if (m_valueRefs[i] == nullptr) {
			IBK::IBK_Message(IBK::FormatString("Output for %1(id=%2).%3 not available, skipped.\n")
							 .arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
							 .arg(m_inputRefs[i].m_id)
							 .arg(m_inputRefs[i].m_name.m_name), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		}
		else {
			++m_numCols;
			// decide integration based on original output definition
			const NANDRAD::OutputDefinition & od = m_outputDefinitions[ m_outputDefMap[i] ];
			if (od.m_timeType != NANDRAD::OutputDefinition::OTT_NONE)
				m_haveIntegrals = true;
		}
	}

	// if we have integral values, initialize integral data store with 0
	if (m_haveIntegrals) {
		m_integrals[0].resize(m_numCols, 0.0);
		m_integrals[1].resize(m_numCols, 0.0);
		m_integralsAtLastOutput.resize(m_numCols, 0.0);
		// time points of -1 mean "uninitialized" - the simulation may be continued from later time points
		m_tLastStep = -1;
		m_tCurrentStep = -1;
	}

}



void OutputFile::createInputReferences() {
	m_haveIntegrals = false;
	// process all output definitions
	for (unsigned int i = 0; i < m_outputDefinitions.size(); ++i) {
		 const NANDRAD::OutputDefinition & od = m_outputDefinitions[i];
		// get the associated object list
		const NANDRAD::ObjectList * ol = od.m_objectListRef;

		if (od.m_timeType != NANDRAD::OutputDefinition::OTT_NONE)
			m_haveIntegrals = true;

		// special handling for Location ref type
		if (ol->m_referenceType == NANDRAD::ModelInputReference::MRT_LOCATION) 	{
			InputReference inref;
			inref.m_id = 0;
			inref.m_required = false;
			inref.m_name.fromEncodedString(od.m_quantity);
			inref.m_referenceType = ol->m_referenceType;
			m_inputRefs.push_back(inref);
			m_outputDefMap.push_back(i);
			continue;
		}
		// process all IDs in the object list's id group
		for (unsigned int id : ol->m_filterID.m_ids) {
			InputReference inref;
			inref.m_id = id;
			inref.m_required = false;
			inref.m_name.fromEncodedString(od.m_quantity);
			inref.m_referenceType = ol->m_referenceType;
			m_inputRefs.push_back(inref);
			m_outputDefMap.push_back(i);
		}
	}
}


void OutputFile::createFile(bool restart, bool binary, const std::string & timeColumnLabel, const IBK::Path * outputPath) {
	FUNCID(OutputFile::createFile);

	m_binary = binary;

	// write warning messages for unavailable output quantities
	for (unsigned int i=0; i<m_valueRefs.size(); ++i) {
		if (m_valueRefs[i] == nullptr) {
			IBK::IBK_Message(IBK::FormatString("Output for %1(id=%2).%3 not available, skipped.\n")
							 .arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
							 .arg(m_inputRefs[i].m_id)
							 .arg(m_inputRefs[i].m_name.m_name), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
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
		std::wstring wfname = outFilePath.wstr();
		m_ofstream = new std::ofstream(wfname.c_str(), std::ios_base::binary);
	#else
		std::string filenameAnsi = IBK::WstringToANSI(outFilePath.wstr(), false);
		m_ofstream = new std::ofstream(filenameAnsi.c_str(), std::ios_base::binary);
	#endif
#else
		m_ofstream = new std::ofstream(outFilePath.c_str(), std::ios_base::binary);
#endif
	}
	else {
#if defined(_WIN32)
	#if defined(_MSC_VER)
		std::wstring wfname = outFilePath.wstr();
		m_ofstream = new std::ofstream(wfname.c_str());
	#else
		std::string filenameAnsi = IBK::WstringToANSI(outFilePath.wstr(), false);
		m_ofstream = new std::ofstream(filenameAnsi.c_str());
	#endif
#else
		m_ofstream = new std::ofstream(outFilePath.c_str());
#endif
	}

	if (!m_ofstream) {
		throw IBK::Exception(IBK::FormatString("Error creating file %1.").arg(outFilePath), FUNC_ID);
	}

	IBK::IBK_Message(IBK::FormatString("%1 : %2 values\n").arg(m_filename,20,std::ios_base::left).arg(m_numCols), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// now write header

	std::vector<std::string> headerLabels;

	// add time column header
	headerLabels.push_back( timeColumnLabel );

	unsigned int col=1; // Mind: column 0 is the time column
	for (unsigned int i=0; i<m_valueRefs.size(); ++i) {
		if (m_valueRefs[i] == nullptr) continue; // skip unavailable vars

		IBK::Unit u(m_valueUnits[i]);

		// compose column title
		std::string header;
		std::string quantityString = m_inputRefs[i].m_name.m_name;
		if (m_inputRefs[i].m_name.m_index != -1)
			quantityString += "(id=" + IBK::val2string(m_inputRefs[i].m_name.m_index) + ")";

		if (m_inputRefs[i].m_id != 0)
			header = IBK::FormatString("%1(id=%2).%3 [%4]")
					.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
					.arg(m_inputRefs[i].m_id)
					.arg(quantityString)
					.arg(u.name()).str();
		else
			header = IBK::FormatString("%1.%2 [%3]")
					.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
					.arg(quantityString)
					.arg(u.name()).str();
		headerLabels.push_back(header);
		++col; // increase var counter
	}
	IBK_ASSERT(col == m_numCols+1);

	// now we have the header completed, and the first row's values and we write to file
	if (binary) {
		/// \todo implement binary format writing
	}
	else {
		// header
		for (unsigned int i=0; i<headerLabels.size(); ++i) {
			if (i != 0)
				*m_ofstream << "\t";
			*m_ofstream << headerLabels[i];
		}
		*m_ofstream << '\n';
	}
}


void OutputFile::cacheOutputs(double t_out, double t_timeOfYear) {
	// no outputs - nothing to do
	if (m_numCols == 0)
		return;

	// NOTE: t_out is already converted to output time unit!!!

	// append data to cache
	std::vector<double> vals(m_numCols+1);
	vals[0] = t_timeOfYear;
	unsigned int col=1; // Mind: column 0 is the time column
	for (unsigned int i=0; i<m_valueRefs.size(); ++i) {
		if (m_valueRefs[i] == nullptr) continue; // skip unavailable vars

		const NANDRAD::OutputDefinition & od = m_outputDefinitions[ m_outputDefMap[i] ];
		switch (od.m_timeType) {
			case NANDRAD::OutputDefinition::OTT_NONE :
			default :
				// retrieve value for this variable and store in cache vector
				vals[col] = *m_valueRefs[i];
			break;

			case NANDRAD::OutputDefinition::OTT_MEAN :
			case NANDRAD::OutputDefinition::OTT_INTEGRAL : {
				// interpolate linearly in interval [t_mLast, t_mCurrent]
				IBK_ASSERT(m_tLastStep <= t_out);
				IBK_ASSERT(t_out <= m_tCurrentStep);
				// special handling when m_tLast == m_tCurrent = t_start
				if (m_tLastStep == m_tCurrentStep)
					vals[col] = 0;
				else {
					IBK_ASSERT(m_tLastStep < m_tCurrentStep);
					double alpha = (t_out-m_tLastStep)/(m_tCurrentStep - m_tLastStep);
					// NOTE: the column index in the target vector 'vals' starts with 1 for the first value
					//       since column 0 is the time column. In the m_integrals vector, however, the
					//       first value is in column 0. Hence, we need to shift the column when retrieving the integral value.
					vals[col] = m_integrals[1][col-1]*alpha + m_integrals[0][col-1]*(1-alpha);
				}

				// next part only for MEAN
				if (od.m_timeType == NANDRAD::OutputDefinition::OTT_MEAN) {

					// special handling for first output value: we store the current values
					if (m_tLastStep == m_tCurrentStep) {
						vals[col] = *m_valueRefs[i];
						m_integralsAtLastOutput[col] = 0; // initialize last output values with 0
					}
					else {
						// we first compute the change in integral values between integral value at last output and
						// the current interval value stored in vals[col]
						double deltaValue = vals[col] - m_integralsAtLastOutput[col];
						double deltaTime = t_out - m_tLastOutput;
						IBK_ASSERT(deltaTime > 0);
						// store current integral value
						m_integralsAtLastOutput[col] = vals[col];
						// compute and store average value
						vals[col] = deltaValue/deltaTime;
					}
					m_tLastOutput = t_out;
				}
			}
			break;
		} // switch

		// perform target unit conversion
		IBK::UnitList::instance().convert(m_valueUnits[i].base_unit(), m_valueUnits[i], vals[col]);

		++col;
	}

	m_cache.emplace_back(vals); // like push-back, but without re-allocation
}


unsigned int OutputFile::cacheSize() const {
	unsigned int cache = m_numCols * m_cache.size() * 2 * 8;
	return cache;
}


void OutputFile::flushCache() {
	// no outputs - nothing to do
	if (m_numCols == 0)
		return;

	// dump all rows of the cache into file
	for (std::vector<double> & vals : m_cache) {
		if (m_binary) {
			/// \todo implement binary file writing
		}
		else {
			// dump vector in ascii mode
			// first values
			for (unsigned int i=0; i<vals.size(); ++i) {
				if (i != 0)
					*m_ofstream << "\t";
				*m_ofstream << vals[i];
			}
			*m_ofstream << '\n';
		}
	}
	// flush stream
	m_ofstream->flush();
	// and clear cache
	m_cache.clear();
}


} // namespace NANDRAD_MODEL
