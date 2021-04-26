#include "NM_IdealHeatingCoolingModel.h"

#include <IBK_Exception.h>

#include <NANDRAD_IdealHeatingCoolingModel.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"
#include "NM_ThermostatModel.h"

namespace NANDRAD_MODEL {

void IdealHeatingCoolingModel::setup(const NANDRAD::IdealHeatingCoolingModel & model,
									 const std::vector<NANDRAD::ObjectList> & objLists,
									 const std::vector<NANDRAD::Zone> & zones)
{
	FUNCID(IdealHeatingCoolingModel::setup);

	m_zones = &zones;

	// all models require an object list with indication of zones that this model applies to
	if (model.m_zoneObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ZoneObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  model.m_zoneObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined object list '%1'.")
							 .arg(model.m_zoneObjectList), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_ZONE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'Zone'.")
							 .arg(m_objectList->m_name), FUNC_ID);

	// parameters have been checked already
	m_maxHeatingPower = model.m_para[NANDRAD::IdealHeatingCoolingModel::P_MaxHeatingPowerPerArea].value;
	m_maxCoolingPower = model.m_para[NANDRAD::IdealHeatingCoolingModel::P_MaxCoolingPowerPerArea].value;

	// reserve storage memory for results
	m_vectorValuedResults.resize(NUM_VVR);

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


const NANDRAD::ObjectList & IdealHeatingCoolingModel::objectList() const{
	return *m_objectList;
}


void IdealHeatingCoolingModel::initResults(const std::vector<AbstractModel *> &) {
	FUNCID(IdealHeatingCoolingModel::initResults);

	// no model IDs, nothing to do (see explanation in resultDescriptions())
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of referenced zones
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());
	// resize result vectors accordingly
	for (unsigned int varIndex=0; varIndex<NUM_VVR; ++varIndex)
		m_vectorValuedResults[varIndex] = VectorValuedQuantity(indexKeys);

	// we also cache the zonal volumes for faster computation
	for (unsigned int id : indexKeys) {
		// find zone by ID
		const std::vector<NANDRAD::Zone>::const_iterator it = std::find(m_zones->begin(), m_zones->end(), id);
		if (it == m_zones->end())
			throw IBK::Exception(IBK::FormatString("Zone with id '%1' is referenced in object list '%2' but does not exist "
								 "(error in object list init code).").arg(id).arg(m_objectList->m_name), FUNC_ID);

		// check existence and validity of zone area
		m_zoneAreas.push_back(it->m_para[NANDRAD::Zone::P_Area].checkedValue("Area", "m2",
			"m2", 0, true, std::numeric_limits<double>::max(), true,
			"Zone area must be >= 0 W/m2.") );

	}
}


void IdealHeatingCoolingModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those zones were added, that are actually parameterized
	// so we can rely on the existence of zones whose IDs are in our object list and we do not need to search
	// through all the models

	// it may be possible, that an object list does not contain a valid id, for example, when the
	// requested IDs did not exist - in this case a warning was already printed, so we can just bail out here
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// Retrieve index information from vector valued results.
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	// For each of the zones in the object list we generate vector-valued results as defined
	// in the type VectorValuedResults.
	for (int varIndex=0; varIndex<NUM_VVR; ++varIndex) {
		// store name, unit and description of the vector quantity
		const std::string &quantityName = KeywordList::Keyword("IdealHeatingCoolingModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("IdealHeatingCoolingModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("IdealHeatingCoolingModel::VectorValuedResults", varIndex );
		// vector-valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription,
			false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
	}
}


const double * IdealHeatingCoolingModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_VVR; ++varIndex) {
		if (KeywordList::Keyword("IdealHeatingCoolingModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
			break;
	}
	if (varIndex == NUM_VVR)
		return nullptr;
	// now check the index
	if (quantityName.m_index == -1) // no index - return start of vector
		return m_vectorValuedResults[varIndex].dataPtr();
	// search for index
	try {
		const double & valRef = m_vectorValuedResults[varIndex][(unsigned int)quantityName.m_index];
		return &valRef;
	} catch (...) {
		// exception is thrown when index is not available - return nullptr
		return nullptr;
	}
}


void IdealHeatingCoolingModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // no valid zones in object list -> nothing to do
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	// we need control values for heating and cooling

	// These can be provided by HVACControlModels as dedicated 'IdealHeatingControlValue' and
	// 'IdealCoolingControlValue'.
	// TODO Andreas/Dirk

	// Alternatively, we can source a thermostat control value directly. Thermostat-modells provide
	// 'HeatingControlValue' and 'CoolingControlValue'

	// We loop over all models, pick out the Thermostat-models and request input for our zones. Only
	// one (and exactly) one request per zone must be fulfilled!

	// search all models for construction models that have an interface to this zone
	for (AbstractModel * model : models) {
		// ignore all models that are not thermostat models
		if (model->referenceType() != NANDRAD::ModelInputReference::MRT_MODEL)
			continue;
		ThermostatModel * mod = dynamic_cast<ThermostatModel *>(model);
		if (mod != nullptr) {
			// create an input reference to heating and cooling control values for all the zones that we heat
			for (unsigned int zoneId : indexKeys) {
				InputReference r;
				r.m_id = model->id();
				r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
				r.m_name.m_name = "HeatingControlModel";
				r.m_name.m_index = (int)zoneId; // vector gets ID of current zone
				r.m_required = false;
				m_inputRefs.push_back(r);
				r.m_name.m_name = "CoolingControlModel";
				m_inputRefs.push_back(r);
			}
			++m_thermostatModelObjects;
		}
	}
}


void IdealHeatingCoolingModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void IdealHeatingCoolingModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	FUNCID(IdealHeatingCoolingModel::setInputValueRefs);
	if (m_objectList->m_filterID.m_ids.empty())
		return; // no valid zones in object list -> nothing to do
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	// we now must ensure, that for each zone there is exactly one matching control signal
	// TODO : first look for unique results from HVACControlValues, then search for thermostat results and ignore
	//        search for all zones, where already a valid HVACControlValue was found

	IBK_ASSERT(m_thermostatModelObjects*2*indexKeys.size() == resultValueRefs.size());
	std::vector<const double *> thermostatValueRefs(2*indexKeys.size(), nullptr); // [0 = heatingControl first zone, 1 - coolingControl first zone, 2 - heating control second zone ...]
	for (unsigned int i=0; i<m_thermostatModelObjects; ++i) {
		for (unsigned int j=0; j<indexKeys.size(); ++j) {
			// heating control value
			if (resultValueRefs[i*m_thermostatModelObjects + j*2] != nullptr) {
				// do we have already a result value for this zone?
				unsigned int zoneID = indexKeys[j];
				if (thermostatValueRefs[j*2] != nullptr)
					throw IBK::Exception(IBK::FormatString("Duplicate heating control value result generated by different thermostats "
														   "for zone id=%1.").arg(zoneID), FUNC_ID);
				thermostatValueRefs[j*2] = resultValueRefs[i*m_thermostatModelObjects +j*2];
			}
			// cooling control value
			if (resultValueRefs[i*m_thermostatModelObjects + j*2 + 1] != nullptr) {
				// do we have already a result value for this zone?
				unsigned int zoneID = indexKeys[j];
				if (thermostatValueRefs[j*2 + 1] != nullptr)
					throw IBK::Exception(IBK::FormatString("Duplicate cooling control value result generated by different thermostats "
														   "for zone id=%1.").arg(zoneID), FUNC_ID);
				thermostatValueRefs[j*2 + 1] = resultValueRefs[i*m_thermostatModelObjects +j*2];
			}
		}
	}

	// check that we have indeed value refs for all zones
	for (unsigned int j=0; j<indexKeys.size(); ++j) {
		unsigned int zoneID = indexKeys[j];
		if (thermostatValueRefs[j*2] == nullptr)
			throw IBK::Exception(IBK::FormatString("Missing heating control value for zone id=%1.").arg(zoneID), FUNC_ID);
		if (thermostatValueRefs[j*2 + 1] == nullptr)
			throw IBK::Exception(IBK::FormatString("Missing cooling control value for zone id=%1.").arg(zoneID), FUNC_ID);
	}

	m_valueRefs = thermostatValueRefs;
}


void IdealHeatingCoolingModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// our heating loads depend on heating control values, and cooling loads depend on cooling control values
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		// pair: result - input

		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_IdealHeatingLoad].dataPtr() + i, m_valueRefs[i*2]) );
		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_IdealCoolingLoad].dataPtr() + i, m_valueRefs[i*2 + 1]) );
	}
}


int IdealHeatingCoolingModel::update() {
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		// retrieve zone area
		double area = m_zoneAreas[i];
		// get control values
		double heatingControlValue = *m_valueRefs[i*2];
		double coolingControlValue = *m_valueRefs[i*2 + 1];

		// clip
		heatingControlValue = std::max(0.0, std::min(0.0, heatingControlValue));
		coolingControlValue = std::max(0.0, std::min(0.0, coolingControlValue));

		*(m_vectorValuedResults[VVR_IdealHeatingLoad].dataPtr() + i) = heatingControlValue*area*m_maxHeatingPower;
		*(m_vectorValuedResults[VVR_IdealCoolingLoad].dataPtr() + i) = coolingControlValue*area*m_maxCoolingPower;
	}

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
