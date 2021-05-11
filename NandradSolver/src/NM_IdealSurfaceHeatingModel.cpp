#include "NM_IdealSurfaceHeatingModel.h"

#include <IBK_Exception.h>

#include <NANDRAD_IdealSurfaceHeatingModel.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Thermostat.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"
#include "NM_ThermostatModel.h"

namespace NANDRAD_MODEL {

void IdealSurfaceHeatingModel::setup(const NANDRAD::IdealSurfaceHeatingModel & model,
									 const std::vector<NANDRAD::ObjectList> & objLists,
									 const std::vector<NANDRAD::Zone> & zones)
{
	FUNCID(IdealSurfaceHeatingModel::setup);

	// all models require an object list with indication of zones that this model applies to
	if (model.m_constructionObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ConstructionObjectList' parameter."), FUNC_ID);
	// check and resolve reference to object list
	std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(objLists.begin(),
																		  objLists.end(),
																		  model.m_constructionObjectList);
	if (oblst_it == objLists.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined object list '%1'.")
							 .arg(model.m_constructionObjectList), FUNC_ID);
	m_objectList = &(*oblst_it);
	// ensure correct reference type of object list
	if (m_objectList->m_referenceType != NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE)
		throw IBK::Exception(IBK::FormatString("Invalid reference type in object list '%1', expected type 'ConstructionInstance'.")
							 .arg(m_objectList->m_name), FUNC_ID);

	// parameters have been checked already
	m_maxHeatingPower = model.m_para[NANDRAD::IdealSurfaceHeatingModel::P_MaxHeatingPowerPerArea].value;

	// resolve thermostat zone
	std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(),
																  zones.end(),
																  model.m_thermostatZoneID);

	if (zone_it == zones.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermsotatZoneId.")
							 .arg(model.m_thermostatZoneID), FUNC_ID);

	// store zone
	m_thermostatZoneId = model.m_thermostatZoneID;
	// store zone are (checked already)
	m_thermostatZoneArea = zone_it->m_para[NANDRAD::Zone::P_Area].value;

	// resize result vector
	m_results.resize(NUM_R);
	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


const NANDRAD::ObjectList & IdealSurfaceHeatingModel::objectList() const{
	return *m_objectList;
}


void IdealSurfaceHeatingModel::initResults(const std::vector<AbstractModel *> &) {
}


void IdealSurfaceHeatingModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those zones were added, that are actually parameterized
	// so we can rely on the existence of zones whose IDs are in our object list and we do not need to search
	// through all the models

	// it may be possible, that an object list does not contain a valid id, for example, when the
	// requested IDs did not exist - in this case a warning was already printed, so we can just bail out here
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// For each of the zones in the object list we generate vector-valued results as defined
	// in the type Results.
	for (int varIndex=0; varIndex<NUM_R; ++varIndex) {
		// store name, unit and description of the quantity
		const std::string &quantityName = KeywordList::Keyword("IdealSurfaceHeatingModel::Results", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("IdealSurfaceHeatingModel::Results", varIndex );
		const std::string &quantityDescription = KeywordList::Description("IdealSurfaceHeatingModel::Results", varIndex );
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription, false) );
	}
}


const double * IdealSurfaceHeatingModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_R; ++varIndex) {
		if (KeywordList::Keyword("IdealSurfaceHeatingModel::Results", (Results)varIndex ) == quantityName.m_name)
			break;
	}
	if (varIndex == NUM_R)
		return nullptr;
	// now check the index
	if (quantityName.m_index == -1) // no index - not allowed
		return nullptr;
	// search for index = construction instance id (must be contained inside object list)
	if(!m_objectList->m_filterID.contains((unsigned int) quantityName.m_index) )
		// exception is thrown when index is not available - return nullptr
		return nullptr;

	// one result quantitiy is stored for all referencing construction instances
	return &m_results[varIndex];
}


void IdealSurfaceHeatingModel::initInputReferences(const std::vector<AbstractModel *> & models) {

	IBK_ASSERT (!m_objectList->m_filterID.m_ids.empty());
	// set reference to zone air temperature

	// loop over all models, pick out the Thermostat-models and request input for a single zone. Only
	// one (and exactly) one request per zone must be fulfilled!
	for (AbstractModel * model : models) {
		// ignore all models that are not thermostat models
		if (model->referenceType() != NANDRAD::ModelInputReference::MRT_MODEL)
			continue;
		ThermostatModel * mod = dynamic_cast<ThermostatModel *>(model);
		if (mod != nullptr) {
			// create an input reference to heating and cooling control values for all the zones that we heat
			InputReference r;
			r.m_id = model->id();
			r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
			r.m_name.m_name = "HeatingControlValue";
			r.m_name.m_index = (int) m_thermostatZoneId; // vector gets ID of requested zone
			r.m_required = false;
			m_inputRefs.push_back(r);
			++m_thermostatModelObjects;
		}
	}
}


void IdealSurfaceHeatingModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void IdealSurfaceHeatingModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	FUNCID(IdealSurfaceHeatingModel::setInputValueRefs);
	IBK_ASSERT (!m_objectList->m_filterID.m_ids.empty());

	// we now must ensure, that for each zone there is exactly one matching control signal

	IBK_ASSERT(m_thermostatModelObjects == resultValueRefs.size());

	for (unsigned int i=0; i<m_thermostatModelObjects; ++i) {
		// heating control value
		if (resultValueRefs[i] != nullptr) {
			// do we have already a result value for this zone?
			if (m_thermostatValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate heating control value result generated by different thermostats "
													   "for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
			m_thermostatValueRef = resultValueRefs[i];
		}
	}

	// check that we have indeed value refs for all zones
	if (m_thermostatValueRef == nullptr)
		throw IBK::Exception(IBK::FormatString("Missing heating control value for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
}


void IdealSurfaceHeatingModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	IBK_ASSERT(!m_objectList->m_filterID.m_ids.empty());

	resultInputValueReferences.push_back(
					std::make_pair(&m_results[R_IdealSurfaceHeatingLoad], m_thermostatValueRef) );
}


int IdealSurfaceHeatingModel::update() {
	// retrieve zone area
	double area = m_thermostatZoneArea;
	// get control values
	double heatingControlValue = *m_thermostatValueRef;
	// clip
	heatingControlValue = std::max(0.0, std::min(1.0, heatingControlValue));
	m_results[R_IdealSurfaceHeatingLoad] = heatingControlValue*area*m_maxHeatingPower;

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
