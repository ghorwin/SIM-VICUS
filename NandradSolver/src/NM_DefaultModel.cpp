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

#include "NM_DefaultModel.h"

#include <algorithm>
#include <limits>
#include <stddef.h>

#include <IBK_assert.h>
#include <IBK_messages.h>

#include <NANDRAD_KeywordList.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {

const unsigned int DefaultModel::InvalidVectorIndex = std::numeric_limits<unsigned int>::max();

void DefaultModel::initResults(const std::vector<AbstractModel*> & /* models */) {
	FUNCID(DefaultModel::initResults);

	// retrieve all result quantities from quantity description list
	std::vector<QuantityDescription> resDesc;
	resultDescriptions(resDesc);

	// now check the generate
	for (unsigned int i = 0; i < resDesc.size(); ++i) {

		const QuantityDescription &desc = resDesc[i];

		// check for correct unit
		IBK::Unit valueUnit;
		try {
			valueUnit = IBK::Unit(desc.m_unit);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Quantity description '%1' has invalid/undefined unit.")
								 .arg(desc.m_unit), FUNC_ID);
		}

		// distinguish between scalar results (no keys and size = 1)
		// and vector-valued results
		if (desc.m_size == 1 && desc.m_indexKeys.empty()) {
			// scalar results are stored as plain double, with the convention that the value is always stored in base SI unit
			m_results.push_back(0);
		}
		else  {
			// Size of vector valued results depends on specific model capabilities
			// and cannot be resized here. In many cases, the size can only be specified
			// in a call to initResults() when all model objects are known.
			// This specific code, however, has to be implemented in derived models.
			VectorValuedQuantity quantity(1);
			// if index keys are given, resize quantity
			if (!desc.m_indexKeys.empty()) {
				IBK_ASSERT(desc.m_indexKeyType != VectorValuedQuantityIndex::NUM_IndexKeyType);
				quantity = VectorValuedQuantity(desc.m_indexKeys);
			}

			// store result
			m_vectorValuedResults.push_back(quantity);
		}
	}
	// After call of this routine all result vectors are prepared.
	// All vector valued result quantities must now be resized from
	// the inherited model itself. Prepare a set of indices either containing
	// numbers or IDs and use this set for resizing.
}


void DefaultModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	FUNCID(DefaultModel::resultDescriptions);

	// fill result discriptions with informations from the keyword list
	std::string category = std::string(ModelIDName()) + "::Results";

	try {
		if (KeywordList::CategoryExists(category.c_str()) ) {
			for (int varIndex = 0; varIndex <= KeywordList::MaxIndex(category.c_str()); ++varIndex) {
				bool constant = false;
				resDesc.push_back( QuantityDescription(
					KeywordList::Keyword( category.c_str(), varIndex ),
					KeywordList::Unit( category.c_str(), varIndex ),
					KeywordList::Description( category.c_str(), varIndex ),
					constant) );
			}
		}
		// The m_vectorValuedResults vector may either be empty or filled already. If
		// the vector is filled it contains all information about the vector elements
		// including indices that are occupied.
		category = std::string(ModelIDName()) + "::VectorValuedResults";
		if (KeywordList::CategoryExists(category.c_str()) ) {
			for (int varIndex = 0; varIndex <= KeywordList::MaxIndex(category.c_str()); ++varIndex) {
				bool constant = false;
				// retrieve index information from vector valued results
				std::vector<unsigned int> indexKeys;
				// store name, unit and description of the vector quantity
				const std::string &quantityName = KeywordList::Keyword( category.c_str(), varIndex );
				const std::string &quantityUnit = KeywordList::Unit( category.c_str(), varIndex );
				const std::string &quantityDescription = KeywordList::Description( category.c_str(), varIndex );
				// vector-valued quantity descriptions store the description
				// of the quantity itself as well as key strings and descriptions
				// for all vector elements
				resDesc.push_back( QuantityDescription(
					quantityName, quantityUnit, quantityDescription,
					constant, VectorValuedQuantityIndex::IK_Index, indexKeys) );
			}
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing result descriptions for model %1 with id #%2")
			.arg(ModelIDName()).arg(id()), FUNC_ID);
	}
}


void DefaultModel::resultValueRefs(std::vector<const double *> &res) const {

	res.clear();
	// fill with all results and vector valued results

	for (unsigned int i = 0; i < m_results.size(); ++i) {
		res.push_back(&m_results[i]);
	}
#if 0
	for (unsigned int i = 0; i < m_vectorValuedResults.size(); ++i) {
		// loop over all vector valued results
		for (std::vector<double>::const_iterator valueIt = m_vectorValuedResults[i].begin();
			valueIt != m_vectorValuedResults[i].end(); ++valueIt)
		{
			res.push_back(&(*valueIt));
		}
	}
#endif
}


const double * DefaultModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
//	FUNCID(DefaultModel::resultValueRef);

	// find corresponding quantity description
	int resultsIndex = decodeResultType(quantityName.m_name);
	// scalar results
	if (resultsIndex != -1 && quantityName.m_index == -1) {

		// check that the index does not exceed available storage memory location
		if (resultsIndex >= (int)m_results.size())
			return nullptr;

		return &m_results[(size_t)resultsIndex];
	}

	// a vector valued result
	resultsIndex = decodeVectorValuedResultType(quantityName.m_name);

	// invalid quantity
	if (resultsIndex == -1)
		return nullptr;

	/// \todo check this condition: Is this even possible when decodeVectorValuedResultType() returns a valid
	/// index?
	if (resultsIndex >= (int)m_vectorValuedResults.size())
		return nullptr;

	const VectorValuedQuantity & vecResults  = m_vectorValuedResults[(size_t)resultsIndex]; // reading improvement

	// if vector is created but still empty, we have no result to return
	if (vecResults.size() == 0)
		return nullptr;

	// no index is given (requesting entire vector?)
	if (quantityName.m_index == -1) {
		// return access to the first vector element
		return &vecResults.data()[0];
	}
	// index definition
	else {
		return &vecResults[(size_t)quantityName.m_index];
	}
	// Note: function may throw an IBK::Exception
}


int DefaultModel::decodeResultType(const std::string &quantity) const {

	// first check results
	std::string category = ModelIDName() + std::string("::Results");

	if (KeywordList::CategoryExists(category.c_str()) &&
		KeywordList::KeywordExists(category.c_str(), quantity))

		return (int)KeywordList::Enumeration(category.c_str(), quantity);

	// find corresponding quantity description
	std::vector<QuantityDescription> resDesc;
	resultDescriptions(resDesc);

	// check if we have a quantity defined
	std::vector<QuantityDescription>::iterator resDescIt =
		std::find(resDesc.begin(), resDesc.end(), quantity);
	// no quantity defined
	if (resDescIt == resDesc.end()) {
		return -1;
	}

	// vector results are tested for in decodeVectorValuedResultType()
	if (resDescIt->m_size != 1 || !resDescIt->m_indexKeys.empty())
		return -1;

	int resultsVectorIndex = (int) (resDescIt - resDesc.begin());

	return resultsVectorIndex;
}


int DefaultModel::decodeVectorValuedResultType(const std::string &quantity) const {

	// first check results
	std::string category = ModelIDName() + std::string("::VectorValuedResults");

	if (KeywordList::CategoryExists(category.c_str()) &&
		KeywordList::KeywordExists(category.c_str(), quantity))

		return (int)KeywordList::Enumeration(category.c_str(), quantity);

	// find corresponding quantity description inside description vector
	std::vector<QuantityDescription> resDesc;
	resultDescriptions(resDesc);

	// check if we have a quantity defined
	std::vector<QuantityDescription>::iterator resDescIt =
		std::find(resDesc.begin(), resDesc.end(), quantity);
	// no quantity defined
	if (resDescIt == resDesc.end()) {
		return -1;
	}

	// a scalar result
	if (resDescIt->m_size == 1 && resDescIt->m_indexKeys.empty()) {
		return -1;
	}

	int vectorValuedResultsIndex = (int)(resDescIt - resDesc.begin());
	// vector results are listed after scalar results
	vectorValuedResultsIndex -= (int)m_results.size();

	return vectorValuedResultsIndex; // this is the index in vector m_vectorValuedResults
}


} // namespace NANDRAD_MODEL

