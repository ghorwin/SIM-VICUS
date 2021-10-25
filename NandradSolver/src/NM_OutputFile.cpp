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

#include "NM_OutputFile.h"

#include <fstream>

#include <IBK_messages.h>
#include <IBK_Path.h>
#include <IBK_assert.h>
#include <IBK_UnitList.h>
#include <IBK_FileUtils.h>

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

	// shift states
	// m_tCurrentStep has the value of t at the *last* call to stepCompleted(), so now this
	// is actually out "last step's time point"
	double dt = t - m_tCurrentStep;  // integration interval length in [s]
	// special handling for initial call
	if (m_tLastStep == -1) {
		// here we end up on first call the step completed, usually t = 0, except in restart case
		/// \todo think of a way to restore integral values in case of restarting
		m_tLastStep = t;
		m_tCurrentStep = t;
		return; // we have initialized our time points, so let's bail out here... nothing to integrate so far
	}
	// update our time point values
	m_tLastStep = m_tCurrentStep;
	m_tCurrentStep = t;

	// loop over all *available* variables and handle those with OTT_MEAN or OTT_INTEGRAL
	unsigned int col=0; // storage column index
	for (unsigned int i=0; i<m_numCols; ++i) {
		if (m_outputVarInfo[i].m_timeType != NANDRAD::OutputDefinition::OTT_NONE) {

			m_integrals[0][col] = m_integrals[1][col];
			// now retrieve value
			double val = *m_outputVarInfo[i].m_valueRef;

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

	IBK_ASSERT(m_outputVarInfo.empty());

	// process all variables and cache units
	m_haveIntegrals = false;
	for (unsigned int i=0; i<resultValueRefs.size(); ++i) {
		if (resultValueRefs[i] == nullptr)  continue; // skip not available outputs; Note: Error/warning message is shown at the end.

		// append data information entry about this variable
		m_outputVarInfo.push_back(OutputFileVarInfo());
		OutputFileVarInfo & outputVarInfo = m_outputVarInfo.back(); // readability-improvement

		// store data
		outputVarInfo.m_valueRef = resultValueRefs[i];
		outputVarInfo.m_quantityDesc = resultDescriptions[i];

		try {
			// check result unit
			outputVarInfo.m_resultUnit = IBK::Unit(resultDescriptions[i].m_unit);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Invalid/unknown unit provided for quantity %1 ('%2').")
								 .arg(outputVarInfo.m_quantityDesc.m_name).arg(outputVarInfo.m_quantityDesc.m_description), FUNC_ID);
		}
		// in case of integral quantities, generate time integral quantity from value
		const NANDRAD::OutputDefinition & od = m_outputDefinitions[ m_outputDefMap[i].first ];
		outputVarInfo.m_timeType = od.m_timeType; // cache time-type of output value
		// target unit conversion in case of time-type=Integral
		if (outputVarInfo.m_timeType == NANDRAD::OutputDefinition::OTT_INTEGRAL) {
			IBK::Unit u = IBK::UnitList::instance().integralQuantity(outputVarInfo.m_resultUnit, false, true);
			// special handling for energy units J and J/m2 - we rather want kWh and kWh/m2 as outputs
			if (u.name() == "J/m2")
				u.set("kWh/m2");
			else if (u.name() == "J")
					u.set("kWh");
			// replace value unit
			outputVarInfo.m_resultUnit = u;
		}
		// For MEAN and INTEGRAL we need to create additional storage containers
		if (od.m_timeType != NANDRAD::OutputDefinition::OTT_NONE)
			m_haveIntegrals = true;

		// check that scalar and vector-valued variables are not mixed
		if (m_inputRefs[i].m_name.m_index == -1 && resultDescriptions[i].m_size != 1)
			throw IBK::Exception(IBK::FormatString("Requested scalar output quantity '%1', but model "
												   "generates vector-valued quantity with this name.").arg(m_inputRefs[i].m_name.m_name), FUNC_ID);
		if (m_inputRefs[i].m_name.m_index != -1 && resultDescriptions[i].m_indexKeyType == NANDRAD_MODEL::VectorValuedQuantityIndex::NUM_IndexKeyType)
			throw IBK::Exception(IBK::FormatString("Requested vector-valued output quantity '%1', but model "
												   "generates a scalar quantity with this name.").arg(m_inputRefs[i].m_name.encodedString()), FUNC_ID);

		// *** compose column header ***

		// quantity suffix depends on time type
		std::string quantitySuffix;
		if (outputVarInfo.m_timeType == NANDRAD::OutputDefinition::OTT_MEAN)
			quantitySuffix = "-average";
		else if (outputVarInfo.m_timeType == NANDRAD::OutputDefinition::OTT_INTEGRAL)
			quantitySuffix = "-integral";

		std::string quantityString = m_inputRefs[i].m_name.m_name;
		if (m_inputRefs[i].m_name.m_index != -1)
			quantityString += "(id=" + IBK::val2string(m_inputRefs[i].m_name.m_index) + ")";

		std::string variableName;
		if (m_inputRefs[i].m_id != 0)
			variableName = IBK::FormatString("%1(id=%2).%3%4")
					.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
					.arg(m_inputRefs[i].m_id)
					.arg(quantityString)
					.arg(quantitySuffix).str();
		else
			variableName = IBK::FormatString("%1.%2%3")
					.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
					.arg(quantityString)
					.arg(quantitySuffix).str();

		// append unit to variable name and add to header
		outputVarInfo.m_columnHeader = variableName + " [" + outputVarInfo.m_resultUnit.name() + "]";
	}

	// initialize our cache vectors

	// we go through all variables and count the number of non-zero values
	m_numCols = m_outputVarInfo.size();
	// if we have integral values, initialize integral data store with 0
	if (m_haveIntegrals) {
		m_integrals[0].resize(m_numCols, 0.0);
		m_integrals[1].resize(m_numCols, 0.0);
		m_integralsAtLastOutput.resize(m_numCols, 0.0);
		// time points of -1 mean "uninitialized" - the simulation may be continued from later time points
		m_tLastStep = -1;
		m_tCurrentStep = -1;
	}


	// Now generate warnings for all requested outputs that could not be generated.

	// Note : when requesting vector-valued results from models using a wildcard for the model selection,
	//        it may be the case that two model instances provide the same vector valued-result variable.
	//        Yet, these vectors define different subsets of model objects, i.e. results for different zones. #
	//        Since input references are generated for all models, but only
	//        one model delivers results, a warning will be issued for all other models, even though the requested
	//        variable was found.
	//        We need an additional check that if the requested quantity was a vector-valued quantity, and it was
	//        found in _any_ of the models in the object list, the warning will not be printed.
	//        We can check that in the end, once all variables have been collected and we check this specific situation:
	//        - if the QuantityDescription indicates a vector-valued quantity, and it is present already in our collected
	//          m_quantityDescs, we skip the warnings

	for (unsigned int i=0; i<resultValueRefs.size(); ++i) {

		if (resultValueRefs[i] == nullptr) {

			// Case ObjectList -> FilterID="*" -> Model 102 doesn't have output -> ignore warnings
			// Case ObjectList -> FilterID="101,102" -> Model 102 doesn't have output -> issue warning

			bool modelIDFromWildcard = m_outputDefMap[i].second;

			// If from wildcard, we look for any variable that matches the name but is provided from a different model
			// then, the output request was fulfilled (at least once) and we need to output a warning.

			if (modelIDFromWildcard) {
				// search for already satisfied request by different model
				const InputReference & requested = m_inputRefs[i];
				bool found = false;
				for (const OutputFileVarInfo & outputVars : m_outputVarInfo) {
					const QuantityDescription & available = outputVars.m_quantityDesc;
					if (requested.m_referenceType != available.m_referenceType) continue;
					if (requested.m_name.m_name != available.m_name) continue;
					if (requested.m_name.m_index == -1 && available.m_indexKeyType != VectorValuedQuantityIndex::NUM_IndexKeyType)
						continue; // requested scalar - provided vector
					if (requested.m_name.m_index != -1) {
						if (available.m_indexKeyType == VectorValuedQuantityIndex::NUM_IndexKeyType)
							continue; // requested vector-valued - provided scalar
						// compare index
						if (std::find(available.m_indexKeys.begin(), available.m_indexKeys.end(), requested.m_name.m_index) ==
								available.m_indexKeys.end())
							continue;
					}
					found = true;
					break;
				}
				if (found)
					continue; // skip, no warning needed
			}

			if (m_inputRefs[i].m_name.m_index == -1)
				IBK::IBK_Message(IBK::FormatString("Output for %1(id=%2).%3 not available, skipped.")
								 .arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
								 .arg(m_inputRefs[i].m_id)
								 .arg(m_inputRefs[i].m_name.m_name), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			else
				IBK::IBK_Message(IBK::FormatString("Output for %1(id=%2).%3(id=%4) not available, skipped.")
								 .arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_inputRefs[i].m_referenceType))
								 .arg(m_inputRefs[i].m_id)
								 .arg(m_inputRefs[i].m_name.m_name)
								 .arg(m_inputRefs[i].m_name.m_index), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		}
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
			m_outputDefMap.push_back( std::make_pair(i, false) );
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
			m_outputDefMap.push_back(std::make_pair(i, ol->m_filterID.m_allIDs) );
		}
	}
}


void OutputFile::createFile(bool restart, bool binary, const std::string & timeColumnLabel, const IBK::Path * outputPath,
							const std::map<std::string, std::string> & varSubstitutionMap)
{
	FUNCID(OutputFile::createFile);

	m_binary = binary;

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
			if (binary)
				m_ofstream = IBK::create_ofstream(outFilePath, std::ios_base::binary | std::ios_base::app);
			else
				m_ofstream = IBK::create_ofstream(outFilePath, std::ios_base::app);
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
	if (binary)
		m_ofstream = IBK::create_ofstream(outFilePath, std::ios_base::binary | std::ios_base::trunc);
	else
		m_ofstream = IBK::create_ofstream(outFilePath);

	if (!m_ofstream)
		throw IBK::Exception(IBK::FormatString("Error creating file %1.").arg(outFilePath), FUNC_ID);

	IBK::IBK_Message(IBK::FormatString("%1 : %2 values\n").arg(m_filename,20,std::ios_base::left).arg(m_numCols), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// now write header
	std::vector<std::string> headerLabels;

	headerLabels.push_back( timeColumnLabel );
	// add time column header
	for (unsigned int i=0; i<m_numCols; ++i)
		headerLabels.push_back(m_outputVarInfo[i].m_columnHeader);

	// modify headers using variable substitution map
	if (!varSubstitutionMap.empty()){
		std::vector<std::string> tokens;
		for (unsigned int i=0; i<m_numCols; ++i) {
			// Mind: headerLabels have size m_numCols + 1 because of the first time column
			//       hence we add +1 in the index access to headerLabels[] below
			IBK::explode(headerLabels[i+1], tokens, ".", IBK::EF_NoFlags); // we need to split element name and quantity
			std::map<std::string, std::string>::const_iterator varSubst = varSubstitutionMap.find(tokens[0]);
			if (varSubst != varSubstitutionMap.end() )
				headerLabels[i+1] = varSubst->second + "." + tokens[1]; // and merge quantity with substituted element name
		}
	}

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
	for (unsigned int i=0; i<m_numCols; ++i) {
		unsigned int col=i+1; // Mind: column 0 is the time column
		switch (m_outputVarInfo[i].m_timeType) {
			case NANDRAD::OutputDefinition::OTT_NONE :
			default :
				// retrieve value for this variable and store in cache vector
				vals[col] = *m_outputVarInfo[i].m_valueRef;
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
				if (m_outputVarInfo[i].m_timeType == NANDRAD::OutputDefinition::OTT_MEAN) {

					// special handling for first output value: we store the current values
					if (m_tLastStep == m_tCurrentStep) {
						vals[col] = *m_outputVarInfo[i].m_valueRef;
						m_integralsAtLastOutput[i] = 0; // initialize last output values with 0
					}
					else {
						// we first compute the change in integral values between integral value at last output and
						// the current interval value stored in vals[col]
						double deltaValue = vals[col] - m_integralsAtLastOutput[i];
						double deltaTime = t_out - m_tLastOutput;
						IBK_ASSERT(deltaTime > 0);
						// store current integral value
						m_integralsAtLastOutput[i] = vals[col];
						// compute and store average value
						vals[col] = deltaValue/deltaTime;
					}
				}
			}
			break;
		} // switch

		// perform target unit conversion
		IBK::UnitList::instance().convert(m_outputVarInfo[i].m_resultUnit.base_unit(), m_outputVarInfo[i].m_resultUnit, vals[col]);
	}
	// finally update last outputs time point
	m_tLastOutput = t_out;

	m_cache.emplace_back(vals); // like push-back, but without re-allocation
}


unsigned int OutputFile::cacheSize() const {
	unsigned int cache = m_numCols * m_cache.size() * sizeof(double);
	return cache;
}


void OutputFile::flushCache() {
	// no outputs - nothing to do
	if (m_numCols == 0 || m_ofstream == nullptr)
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
