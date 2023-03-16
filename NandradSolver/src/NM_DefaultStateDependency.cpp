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

#include "NM_AbstractModel.h"
#include "NM_DefaultStateDependency.h"
#include "NM_KeywordList.h"
#include "NM_QuantityName.h"

#include <NANDRAD_KeywordList.h>

#include <limits>
#include <algorithm>
#include <stddef.h>

#include <IBK_assert.h>

namespace NANDRAD_MODEL {

DefaultStateDependency::DefaultStateDependency(unsigned int modelTypeID) {
	AbstractStateDependency::m_modelTypeId = modelTypeID;
}

#if 0
void DefaultStateDependency::inputReferenceDescriptions(std::vector<QuantityDescription> & refDesc) const {

	FUNCID(DefaultStateDependency::inputReferenceDescriptions);
	// No state dependend model object without the model properties!!!
	const AbstractModel* model = dynamic_cast<const AbstractModel*>(this);
	IBK_ASSERT(model != nullptr);

	// Retreive index information using the keyword list.
	std::string category = std::string(model->ModelIDName()) + "::InputReferences";

	try {
		if(KeywordList::CategoryExists(category.c_str()) )
		{
			// of all defined parameters
			if (KeywordList::CategoryExists(category.c_str()))
			{
				unsigned int maxIndex = (unsigned int)KeywordList::MaxIndex(category.c_str());
				for (unsigned int refIndex = 0; refIndex <= maxIndex; ++refIndex) {
					std::vector<VectorValuedQuantityIndex> indexKeys;
					std::vector<std::string> indexKeyDescriptions;
					bool constant = true;
					refDesc.push_back(QuantityDescription(
						KeywordList::Keyword(category.c_str(), refIndex),
						KeywordList::Unit(category.c_str(), refIndex),
						KeywordList::Description(category.c_str(), refIndex),
						constant));
				}
			}

			//// we have filled our some input references already
			//std::vector<InputReference> inputRefs;
			//inputReferences(inputRefs);

			//if (!inputRefs.empty())
			//{
			//	for (std::vector<InputReference>::const_iterator it = inputRefs.begin();
			//		it != inputRefs.end(); ++it)
			//	{
			//		std::vector<QuantityDescription>::const_iterator
			//			quantIt = std::find_if(refDesc.begin(), refDesc.end(),
			//				NANDRAD::FindByName<QuantityDescription>(it->m_targetName.name()));

			//		// unregistered references
			//		if (quantIt == refDesc.end())
			//			continue;

			//		IBK_ASSERT(KeywordList::KeywordExists(category.c_str(), it->m_targetName.name()));
			//		int quantityType = KeywordList::Enumeration(category.c_str(), it->m_targetName.name());
			//		int index = it->m_targetName.index();

			//		// unregistered references
			//		if ((unsigned int)quantityType >= refDesc.size())
			//			break;

			//		// now register current reference
			//		if (index == -1) {

			//			// we recicle the scalar target
			//			QuantityDescription &quantityDesc = refDesc[quantityType];
			//			// scalar quantity
			//			IBK_ASSERT(quantityDesc.m_indexKeys.empty());
			//			// quantity must! be empty
			//			IBK_ASSERT(quantityDesc.m_size == 1);
			//			quantityDesc.m_constant = it->m_constant;
			//		}
			//		// vector valued quantity
			//		else {
			//			// add key values to last quantity description
			//			QuantityDescription &quantityDesc = refDesc[quantityType];
			//			// add index key, key description and correct size
			//			VectorValuedQuantityIndex indexKey;
			//			// set key type to 'Id' per default
			//			indexKey.m_keyType = VectorValuedQuantityIndex::IK_ModelID;
			//			indexKey.m_keyValue = index;
			//			// add new index
			//			// find insert position
			//			std::vector<VectorValuedQuantityIndex>::const_iterator
			//				insertIt = std::lower_bound(quantityDesc.m_indexKeys.begin(),
			//					quantityDesc.m_indexKeys.end(), indexKey);

			//			// position not occupied already
			//			if (insertIt == quantityDesc.m_indexKeys.end() ||
			//				insertIt->m_keyValue != indexKey.m_keyValue) {

			//				// find position inside description vector
			//				int pos = insertIt - quantityDesc.m_indexKeys.begin();
			//				IBK_ASSERT(pos <= quantityDesc.m_indexKeyDescriptions.size());
			//				// insert key and decription
			//				quantityDesc.m_indexKeys.insert(insertIt, indexKey);
			//				quantityDesc.m_indexKeyDescriptions.insert(
			//					quantityDesc.m_indexKeyDescriptions.begin() + pos,
			//					indexKey.toString());
			//			}
			//			//sstore values
			//			quantityDesc.m_size = quantityDesc.m_indexKeys.size();
			//		}
			//	}
			//}
		}
	}
	catch(IBK::Exception &ex)
	{
		throw IBK::Exception(ex, IBK::FormatString("Error initializing input reference description for Model #%1 with id #%2")
			.arg(model->ModelIDName()).arg(model->id()), FUNC_ID);
	}
}
#endif

void DefaultStateDependency::inputReferences(std::vector<InputReference>  & /*inputRefs*/) const {

	// copy references into vector
//	inputRefs.insert(inputRefs.end(), m_inputReferences.begin(),m_inputReferences.end() );
}

void DefaultStateDependency::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/, const std::vector<const double *> & /*resultValueRefs*/) {
#if 0
	FUNCID(DefaultStateDependency::setInputValueRef);
	// for the first call we need to resize all input value references
	if(m_inputValueRefs.empty() && !m_inputReferences.empty())
		m_inputValueRefs.resize(m_inputReferences.size(), nullptr);

	// find corresponding quantity description
	std::vector<QuantityDescription> refDesc;
	inputReferenceDescriptions(refDesc);

	// if not done already construct the offset vector
	if (m_inputValueOffset.empty())
	{
		// resize and fill offset vector
		m_inputValueOffset.resize(refDesc.size());

		unsigned int offset = 0;
		for (unsigned int k = 0; k < refDesc.size(); ++k)
		{
			if (refDesc[k].m_indexKeys.empty()) {
				// ignore empty vector valued references
				if (refDesc[k].m_size > 0) {

					IBK_ASSERT(refDesc[k].m_size == 1);
					// find target quantity in input reference vector using special
					// find functions
					std::vector<InputReferenceToVectorValuedTarget>::iterator it =
						std::find_if(m_inputReferences.begin(), m_inputReferences.end(),
							FindInputReferenceInsertPosition((int)k, -1));

					// ups, no corresponding input reference is defined
					if (it == m_inputReferences.end() || it->m_quantityType != (int) k) {
						throw IBK::Exception(IBK::FormatString("Input reference to target name %1 is undefined!")
							.arg(refDesc[k].m_name),
							FUNC_ID);
					}
					// check that no vector quantity is defined
					if (it->m_index != -1) {
						throw IBK::Exception(IBK::FormatString("Invalid key #%2 for "
							"input reference to target name '%1'! Target is of scalar type.")
							.arg(refDesc[k].m_name).arg(it->m_index),
							FUNC_ID);
					}
				}
			}
			else {
				IBK_ASSERT(refDesc[k].m_size == refDesc[k].m_indexKeys.size());
				// find target quantity in input reference vector using special
				// find functions
				std::vector<InputReferenceToVectorValuedTarget>::iterator it =
					std::find_if(m_inputReferences.begin(), m_inputReferences.end(),
						FindInputReference((int)k, refDesc[k].m_indexKeys.front().m_keyValue));

				for (unsigned int i = 0; i < refDesc[k].m_indexKeys.size(); ++i, ++it) {

					const VectorValuedQuantityIndex &indexKey = refDesc[k].m_indexKeys[i];
					// ups, no corresponding input reference is defined
					if (it == m_inputReferences.end() ||
						it->m_quantityType != (int) k ||
						it->m_index > (int) indexKey.m_keyValue) {
						throw IBK::Exception(IBK::FormatString("Input reference to target name '%1' with key #%2 is undefined!")
							.arg(refDesc[k].m_name).arg(indexKey.m_keyValue),
							FUNC_ID);
					}
					if (it->m_index < (int) indexKey.m_keyValue) {
						throw IBK::Exception(IBK::FormatString("Invalid key #%2 for "
							"input reference to target name '%1'!")
							.arg(refDesc[k].m_name).arg(it->m_index),
							FUNC_ID);
					}
				}
				// select all index references witrh wrong key
				if(it != m_inputReferences.end() && it->m_quantityType == (int) k) {
					throw IBK::Exception(IBK::FormatString("Invalid key #%2 for "
						"input reference to target name '%1'!")
						.arg(refDesc[k].m_name).arg(it->m_index),
						FUNC_ID);
				}
			}
			m_inputValueOffset[k] = offset;
			offset += refDesc[k].m_size;
		}
	}

	try {
		// check if we have a quantity defined
		std::vector<QuantityDescription>::iterator refDescIt =
			std::find_if(refDesc.begin(), refDesc.end(),
				NANDRAD::FindByName<QuantityDescription>(targetName.name()));
		// no quantity defined
		if (refDescIt == refDesc.end()) {
			throw IBK::Exception(IBK::FormatString("Error encoding target name %1!")
				.arg(targetName.name()),
				FUNC_ID);
		}

		std::ptrdiff_t pos = refDescIt - refDesc.begin();

		// find target quantity in input reference vector using special
		// find functions
		std::vector<InputReferenceToVectorValuedTarget>::iterator it =
			std::find_if(m_inputReferences.begin(), m_inputReferences.end(),
			FindInputReference((int) pos, targetName.index()) );
		// ups, no corresponding input reference is defined
		if(it == m_inputReferences.end())  {
			throw IBK::Exception(IBK::FormatString( "Input reference to target name %1 is undefined!")
												.arg(targetName.name()),
												FUNC_ID);
		}

		// calculate a vector index from iterator position
		unsigned int vecIndex = (unsigned int)std::distance(m_inputReferences.begin(), it);

		// an input value refs vector of smaller size than the m_inputValueReferences
		// vector is a programmer error
		IBK_ASSERT(vecIndex < m_inputValueRefs.size());
		// return the input value reference at the same index position
		m_inputValueRefs[vecIndex] = resultValueRef;
	}
	catch(IBK::Exception &ex)
	{
		// cast to abstract model
		const AbstractModel* model = dynamic_cast<const AbstractModel*>(this);
		IBK_ASSERT(model != nullptr);

		throw IBK::Exception(ex, IBK::FormatString( "Error retrieving reference to target %1 "
											"of model %2 with id %3!")
											.arg(targetName.name())
											.arg(model->ModelIDName())
											.arg(model->id()),
											FUNC_ID);
	}
#endif
}

void DefaultStateDependency::stateDependencies(std::vector< std::pair<const double *, const double *> >	&resultInputValueReferences) const {
	// No state dependend model object without the model properties!!!
//	const AbstractModel* model = dynamic_cast<const AbstractModel*>(this);
//	IBK_ASSERT(model != nullptr);

	// clear pattern
	if(!resultInputValueReferences.empty() )
		resultInputValueReferences.clear();

	// retrieve vector of pointer adresses to the value references
	std::vector<const double *> valueRefs;

	// retrieve input value references
	std::vector<const double *> inputRefs;
	inputValueRefs(inputRefs);

	// now we add a dependency between each input value and each result value
	for(unsigned int i = 0; i < valueRefs.size(); ++i)
	{
		const double *resultValue = valueRefs[i];
		for(unsigned int j = 0; j < inputRefs.size(); ++j)
		{
			const double *inputValue = inputRefs[j];
			// store the pair of adresses of input value and result
			// inside pattern list (the pattern is dense)
			resultInputValueReferences.push_back(
				std::make_pair(resultValue, inputValue) );
		}
	}
}


void DefaultStateDependency::constraints(std::map< const double *,
	std::pair<double, double> > &constraintsPerValueRef) const
{
#if 0
	if (!constraintsPerValueRef.empty())
		constraintsPerValueRef.clear();

	const AbstractModel *model = dynamic_cast<const AbstractModel*>(this);
	// each state depenmdency also is a model
	IBK_ASSERT(model != nullptr);

	// check all result references for constraints
	std::vector<QuantityDescription> resDescs;
	model->resultDescriptions(resDescs);

	for (unsigned int i = 0; i < resDescs.size(); ++i) {
		// redability improvement
		const QuantityDescription &resDesc = resDescs[i];
		// skip descriptions without limits
		if (resDesc.m_minMaxValue.first == -std::numeric_limits<double>::max() &&
			resDesc.m_minMaxValue.second == std::numeric_limits<double>::max())
			continue;

		// scalar quantity
		if (resDesc.m_indexKeys.empty() && resDesc.m_size == 1) {
			const double *valueRef = model->resultValueRef(resDesc.m_name);
			IBK_ASSERT(valueRef != nullptr);
			// add constraint
			constraintsPerValueRef[valueRef] = resDesc.m_minMaxValue;
		}
		// vector quantity with index notation
		else if (!resDesc.m_indexKeys.empty()) {
			// loop over all indexes
			for (unsigned int j = 0; j < resDesc.m_indexKeys.size(); ++j) {
				// store local index
				const VectorValuedQuantityIndex &index = resDesc.m_indexKeys[j];
				// convert name into reference name
				const QuantityName quantityName(resDesc.m_name, index.m_keyValue);
				// search value reference
				const double *valueRef = model->resultValueRef(quantityName);
				IBK_ASSERT(valueRef != nullptr);
				// add constraint
				constraintsPerValueRef[valueRef] = resDesc.m_minMaxValue;
			}
		}
		// vector quantity without index notation
		else if (resDesc.m_size > 0) {
			// loop over all indexes
			for (unsigned int j = 0; j < resDesc.m_size; ++j) {
				// store local index
				const VectorValuedQuantityIndex index(VectorValuedQuantityIndex::IK_Index,j);
				// convert name into reference name
				const QuantityName quantityName(resDesc.m_name, index.m_keyValue);
				// search value reference
				const double *valueRef = model->resultValueRef(quantityName);
				IBK_ASSERT(valueRef != nullptr);
				// add constraint
				constraintsPerValueRef[valueRef] = resDesc.m_minMaxValue;
			}
		}
	}
#endif
}


#if 0
InputReference & DefaultStateDependency::inputReference(int quantityType, int index) {
	// first seach the lower bound/ equal element
	std::vector<InputReferenceToVectorValuedTarget>::iterator it =
		std::find_if(m_inputReferences.begin(), m_inputReferences.end(),
		FindInputReferenceInsertPosition(quantityType, index) );

	// special case: input reference exists already
	// -> the element matches quantity and key (equality, *it == inputRef(quantityType, indexKey))
	if(it != m_inputReferences.end() && it->m_quantityType == quantityType && it->m_index == index)
		return *it;

	// otherwise add a new element (inequality, *it > inputRef(quantityType, indexKey) )
	InputReferenceToVectorValuedTarget inputRef;
	inputRef.m_quantityType = quantityType;
	inputRef.m_index = index;

	// special case: end of vector
	if(it == m_inputReferences.end())
	{
		// push back a new element and return its value
		m_inputReferences.push_back(inputRef);
		return m_inputReferences.back();
	}

	// Insert the input reference at the current position of the next larger element
	// and return the corresponding iterator. The larger element will be shifted
	// for one vector position.
	return *m_inputReferences.insert(it,inputRef);
}


const InputReference & DefaultStateDependency::inputReference(int quantityType, int index) const {
	FUNCID(DefaultStateDependency::inputReference);
	// find the input reference matching quantity type and index
	std::vector<InputReferenceToVectorValuedTarget>::const_iterator it =
		std::find_if(m_inputReferences.begin(), m_inputReferences.end(),
		FindInputReference(quantityType, index) );
	// element exists
	if(it != m_inputReferences.end())
		return *it;
	// exception: no element exists
	const AbstractModel* model = dynamic_cast<const AbstractModel*>(this);
	IBK_ASSERT(model != nullptr);

	std::string category = std::string(model->ModelIDName()) + "::InputReferences";
	// construct error message
	std::string errmsg = std::string("Error accessing input reference");
	if(KeywordList::MaxIndex(category.c_str()) >= quantityType)
	{
		errmsg += IBK::FormatString(" for variable %1").arg(KeywordList::Keyword(category.c_str(),quantityType)).str();
	}
	if(index != -1)
	{
		errmsg += IBK::FormatString(" and index %1").arg(index).str();
	}
	errmsg += std::string("! Input reference does not exist.");
	throw IBK::Exception(errmsg, FUNC_ID);
}
#endif


std::vector<const double *>::const_iterator DefaultStateDependency::inputValueRefs(int /*quantityType*/) const {
#if 0
	IBK_ASSERT(dynamic_cast<const AbstractModel*>(this) != nullptr);
	// error: wrong quantity type is requested
	IBK_ASSERT_X( quantityType >= 0 &&
				  quantityType < (int) KeywordList::Count( (std::string(dynamic_cast<const AbstractModel*>(this)->ModelIDName()) + "::InputReferences").c_str()),
			IBK::FormatString("Error accessing input value reference: quantity type %1 does not exist!").arg(quantityType));

	// programmer checks
	IBK_ASSERT(!m_inputReferences.empty());
	IBK_ASSERT(!m_inputValueRefs.empty());
	IBK_ASSERT(m_inputReferences.size() == m_inputValueRefs.size());
	IBK_ASSERT(quantityType < (int) m_inputValueOffset.size());

	// return an iterator to the first occurance of the current quantity inside
	// input reference vector
	unsigned int offset = m_inputValueOffset[quantityType];
	if (offset >= m_inputValueRefs.size())
		return m_inputValueRefs.end();

	return m_inputValueRefs.begin() + offset;
#endif
	return m_inputValueRefs.begin();
}


const double *DefaultStateDependency::inputValueRef(int /*quantityType*/) const {
#if 0

	IBK_ASSERT(dynamic_cast<const AbstractModel*>(this) != nullptr);
	// error: wrong quantity type is requested
	IBK_ASSERT_X( quantityType >= 0 &&
				  quantityType < (int) KeywordList::Count( (std::string(dynamic_cast<const AbstractModel*>(this)->ModelIDName()) + "::InputReferences").c_str()),
			IBK::FormatString("Error accessing input value reference: quantity type %1 does not exist!").arg(quantityType));

	// programmer checks
	IBK_ASSERT(!m_inputReferences.empty());
	IBK_ASSERT(!m_inputValueRefs.empty());
	IBK_ASSERT(m_inputReferences.size() == m_inputValueRefs.size());
	IBK_ASSERT(quantityType < (int) m_inputValueOffset.size());

	// find the value to the requested quantity type in the input value vector
	unsigned int offset = m_inputValueOffset[quantityType];
	if (offset >= m_inputValueRefs.size())
		return nullptr;

	return m_inputValueRefs[offset];
#endif
	return nullptr;
}


int DefaultStateDependency::decodeInputReferenceTargeType(const std::string &targetName) const {
	const AbstractModel * model = dynamic_cast<const AbstractModel*>(this);
	IBK_ASSERT(model != nullptr);

	std::string category = model->ModelIDName() + std::string("::InputReferences");

	if (!KeywordList::CategoryExists(category.c_str()))
		return -1;

	if (!KeywordList::KeywordExists(category.c_str(), targetName))
		return -1;

	return (int)KeywordList::Enumeration(category.c_str(), targetName);
}

} // namespace NANDRAD_MODEL

