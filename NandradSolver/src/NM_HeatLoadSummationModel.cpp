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

#include "NM_HeatLoadSummationModel.h"

#include <IBK_Exception.h>

#include <NANDRAD_HeatLoadSummationModel.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {

void HeatLoadSummationModel::setup(const NANDRAD::HeatLoadSummationModel & model,
									 const std::vector<NANDRAD::ObjectList> & objLists) {
	FUNCID(HeatLoadSummationModel::setup);

	m_zoneCoolingLoad = model.m_zoneCoolingLoad;
	// all models require an object list with indication of zones that this model applies to
	if (model.m_objectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  model.m_objectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined object list '%1'.")
							 .arg(model.m_objectList), FUNC_ID);
	m_objectList = &(*oblst_it);

	// attribute 'useZoneCoolingLoad is only allowed for zone load summation
	if(m_zoneCoolingLoad && m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_ZONE) {
		throw IBK::Exception(IBK::FormatString("Attribute 'useZoneCoolingLoad' can only be set for object list reference type 'Zone'.")
							 , FUNC_ID);
	}

	// reserve storage memory for results
	m_results.resize(NUM_R, 0);
}


const NANDRAD::ObjectList & HeatLoadSummationModel::objectList() const{
	return *m_objectList;
}


void HeatLoadSummationModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those zones were added, that are actually parameterized
	// so we can rely on the existence of zones whose IDs are in our object list and we do not need to search
	// through all the models

	// it may be possible, that an object list does not contain a valid id, for example, when the
	// requested IDs did not exist - in this case a warning was already printed, so we can just bail out here
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// For each of the zones in the object list we generate results as defined
	// in the type Results.
	for (int varIndex=0; varIndex<NUM_R; ++varIndex) {
		// store name, unit and description of the vector quantity
		const std::string &quantityName = KeywordList::Keyword("HeatLoadSummationModel::Results", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("HeatLoadSummationModel::Results", varIndex );
		const std::string &quantityDescription = KeywordList::Description("HeatLoadSummationModel::Results", varIndex );
		// vector-valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription,false) );
	}
}


const double * HeatLoadSummationModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	for (unsigned int varIndex = 0; varIndex<NUM_R; ++varIndex) {
		if (KeywordList::Keyword("HeatLoadSummationModel::Results", (Results)varIndex ) == quantityName.m_name)
			return &m_results[varIndex];
	}
	// no suitable quantity found
	return nullptr;
}


void HeatLoadSummationModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // no valid zones in object list -> nothing to do
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());
	NANDRAD::ModelInputReference::referenceType_t refType = m_objectList->m_referenceType;

	std::string quantity;
	// check name of input quantity
	switch(refType) {
		case NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE:
			quantity = "ActiveLayerThermalLoad";
		break;
		case NANDRAD::ModelInputReference::MRT_ZONE: {
			// zhone only provides ideal load
			if(m_zoneCoolingLoad == true)
				quantity = "IdealCoolingLoad";
			else
				quantity = "IdealHeatingLoad";
		}
		break;
		case NANDRAD::ModelInputReference::MRT_NETWORKELEMENT:
			quantity = "FlowElementHeatLoss";
		break;
		default: break;
	}

	// no quantity found -> return without input references
	if(quantity.empty())
		return;

	// heat load is optional
	// in the case, no layer is active, load is ignored
	for (unsigned int id : indexKeys) {
		InputReference r;
		r.m_id = id;
		r.m_referenceType = refType;
		r.m_name.m_name = quantity;
		r.m_required = false;
		m_inputRefs.push_back(r);
	}
}


void HeatLoadSummationModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void HeatLoadSummationModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	if (m_objectList->m_filterID.m_ids.empty())
		return; // no valid zones in object list -> nothing to do

	// add all nojn-zero entries to value reference vector

	for (const double *ref : resultValueRefs) {
		if(ref == nullptr)
			continue;
		m_valueRefs.push_back(ref);
	}
}


void HeatLoadSummationModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {

	// our heating loads depend on heating control values, and cooling loads depend on cooling control values
	for (const double *ref : m_valueRefs) {
		resultInputValueReferences.push_back(
					std::make_pair(&m_results[R_TotalHeatLoad], ref) );
	}
}


int HeatLoadSummationModel::update() {
	// nothiung to do
	if(m_valueRefs.empty())
		return 0;

	double totalLoad = 0.0;
	// sum up all valid load entries
	for (const double *ref : m_valueRefs) {
		totalLoad += *ref;
	}
	// for network elements invert flux direction
	if(m_objectList->m_referenceType == NANDRAD::ModelInputReference::MRT_NETWORKELEMENT ||
		m_zoneCoolingLoad)
		totalLoad *= -1.0;

	m_results[R_TotalHeatLoad] = totalLoad;

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
