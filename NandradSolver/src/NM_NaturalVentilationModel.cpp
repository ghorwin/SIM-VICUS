#include "NM_NaturalVentilationModel.h"

#include <IBK_Exception.h>

#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_NaturalVentilationModel.h>
#include <NANDRAD_ObjectList.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {



void NaturalVentilationModel::setup(const NANDRAD::NaturalVentilationModel & ventilationModel,
									const NANDRAD::SimulationParameter & simPara,
									const std::vector<NANDRAD::ObjectList> & objLists)
{
	FUNCID(NaturalVentilationModel::setup);

	m_ventilationModel = &ventilationModel;
	m_moistureBalanceEnabled = simPara.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled();

	// check for mandatory parameters
	switch (m_ventilationModel->m_model) {
		case NANDRAD::NaturalVentilationModel::M_Constant :
			if (m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_VentilationRate].empty())
				throw IBK::Exception(IBK::FormatString("Missing parameter 'VentilationRate'."), FUNC_ID);
			m_ventilationRate = m_ventilationModel->m_para[NANDRAD::NaturalVentilationModel::P_VentilationRate].checkedValue("1/s",
				"1/h", 0, false, std::numeric_limits<double>::max(), false, "Invalid parameter.");
		break;

		case NANDRAD::NaturalVentilationModel::M_Scheduled : {
		} break;

		default:
			throw IBK::Exception(IBK::FormatString("Unknown/undefined model ID."), FUNC_ID);
	}

	// all models require an object list with indication of ventilated zones
	if (m_ventilationModel->m_zoneObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ZoneObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  m_ventilationModel->m_zoneObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined ZoneObjectList '%1'.")
							 .arg(m_ventilationModel->m_zoneObjectList).arg(m_ventilationModel->m_id), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_ZONE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'Zone'.")
							 .arg(m_objectList->m_name), FUNC_ID);


	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


void NaturalVentilationModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those zones were added, that are actual parameterized
	// so we can rely on the existence of zones whose IDs are in our object list and we do not need to search
	// through all the models

	// it may be possible, that an object list does not contain a valid id, for example, when the
	// requested IDs did not exist - in this case a warning was already printed, so we can just bail out here
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// for each of the zones in the object list, we generate vector-valued results as defined in the type VectorValuedResults
	// retrieve index information from vector valued results
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	const char * const cat = "NaturalVentilationModel::VectorValuedResults";
	for (int varIndex=0; varIndex<NUM_VVR; ++varIndex) {
		// store name, unit and description of the vector quantity
		const std::string &quantityName = KeywordList::Keyword(cat, varIndex );
		const std::string &quantityUnit = KeywordList::Unit( cat, varIndex );
		const std::string &quantityDescription = KeywordList::Description( cat, varIndex );
		// vector-valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription,
			false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
	}
}


const double * NaturalVentilationModel::resultValueRef(const QuantityName & quantityName) const {
	return nullptr;

}


void NaturalVentilationModel::initResults(const std::vector<AbstractModel *> &) {
}


void NaturalVentilationModel::initInputReferences(const std::vector<AbstractModel *> & models) {

}


void NaturalVentilationModel::inputReferences(std::vector<InputReference> & inputRefs) const {

}

void NaturalVentilationModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const
{

}

void NaturalVentilationModel::setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs)
{

}

int NaturalVentilationModel::update() {
	return 0; // signal success
}

} // namespace NANDRAD_MODEL
