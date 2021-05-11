#include "NM_IdealPipeRegisterModel.h"

#include <IBK_Exception.h>
#include <IBK_physics.h>

#include <NANDRAD_IdealPipeRegisterModel.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Thermostat.h>
#include <NANDRAD_Zone.h>

#include "NM_Physics.h"

#include "NM_KeywordList.h"
#include "NM_ThermostatModel.h"

namespace NANDRAD_MODEL {

void IdealPipeRegisterModel::setup(const NANDRAD::IdealPipeRegisterModel & model,
									 const std::vector<NANDRAD::ObjectList> & objLists,
									 const std::vector<NANDRAD::Zone> & zones)
{
	FUNCID(IdealPipeRegisterModel::setup);

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
	m_maxMassFlow = model.m_para[NANDRAD::IdealPipeRegisterModel::P_MaxMassFlow].value;
	// compute fluid volume
	m_fluidCrossSection = IBK::PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length;

	// resolve thermostat zone
	std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(),
																  zones.end(),
																  model.m_thermostatZoneID);

	if (zone_it == zones.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermsotatZoneId.")
							 .arg(model.m_thermostatZoneID), FUNC_ID);

	// store zone
	m_thermostatZoneId = model.m_thermostatZoneID;

	// reserve storage memory for results
	m_vectorValuedResults.resize(NUM_VVR);

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


const NANDRAD::ObjectList & IdealPipeRegisterModel::objectList() const{
	return *m_objectList;
}


void IdealPipeRegisterModel::initResults(const std::vector<AbstractModel *> &) {
	// no model IDs, nothing to do (see explanation in resultDescriptions())
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of referenced zones
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());
	// resize result vectors accordingly
	for (unsigned int varIndex=0; varIndex<NUM_VVR; ++varIndex)
		m_vectorValuedResults[varIndex] = VectorValuedQuantity(indexKeys);
}


void IdealPipeRegisterModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those zones were added, that are actually parameterized
	// so we can rely on the existence of zones whose IDs are in our object list and we do not need to search
	// through all the models

	// it may be possible, that an object list does not contain a valid id, for example, when the
	// requested IDs did not exist - in this case a warning was already printed, so we can just bail out here
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// For each of the constructions in the object list we generate vector-valued results as defined
	// in the type Results.
	for (int varIndex=0; varIndex<NUM_VVR; ++varIndex) {
		// store name, unit and description of the quantity
		const std::string &quantityName = KeywordList::Keyword("IdealPipeRegisterModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("IdealPipeRegisterModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("IdealPipeRegisterModel::VectorValuedResults", varIndex );
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription, false) );
	}
}


const double * IdealPipeRegisterModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// determine variable enum index
	unsigned int varIndex=0;
	for (; varIndex<NUM_VVR; ++varIndex) {
		if (KeywordList::Keyword("IdealPipeRegisterModel::VectorValuedResults", (VectorValuedResults)varIndex ) == quantityName.m_name)
			break;
	}
	if (varIndex == NUM_VVR)
		return nullptr;
	// now check the index
	if (quantityName.m_index == -1) // no index - not allowed
		return &m_vectorValuedResults[varIndex][0];
	// search for index
	try {
		const double & valRef = m_vectorValuedResults[varIndex][(unsigned int)quantityName.m_index];
		return &valRef;
	} catch (...) {
		// exception is thrown when index is not available - return nullptr
		return nullptr;
	}
}


void IdealPipeRegisterModel::initInputReferences(const std::vector<AbstractModel *> & models) {

	if (m_objectList->m_filterID.m_ids.empty())
		return;

	// set an input reference to each layer temperature
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	InputReference l;
	l.m_name.m_name = "ActiveLayerTemperature";
	l.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	l.m_required = true;
	// keys code construction id
	for(unsigned int key : indexKeys) {
		l.m_id = key;
		// store reference
		m_inputRefs.push_back(l);
	}

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


void IdealPipeRegisterModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	inputRefs = m_inputRefs;
}


void IdealPipeRegisterModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	FUNCID(IdealPipeRegisterModel::setInputValueRefs);

	if (m_objectList->m_filterID.m_ids.empty())
		return;

	// we fill layer temperatures
	unsigned int index = 0;
	for( ; index < m_objectList->m_filterID.m_ids.size(); ++index)
		m_activeLayerTemperatureRefs.push_back(resultValueRefs[index]);

	// we now must ensure, that for each zone there is exactly one matching control signal

	IBK_ASSERT(m_thermostatModelObjects == resultValueRefs.size());

	for (unsigned int i=0; i<m_thermostatModelObjects; ++i, ++index) {
		// heating control value
		if (resultValueRefs[index] != nullptr) {
			// do we have already a result value for this zone?
			if (m_thermostatValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate heating control value result generated by different thermostats "
													   "for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
			m_thermostatValueRef = resultValueRefs[index];
		}
	}

	// check that we have indeed value refs for all zones
	if (m_thermostatValueRef == nullptr)
		throw IBK::Exception(IBK::FormatString("Missing heating control value for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
}


void IdealPipeRegisterModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	IBK_ASSERT(!m_objectList->m_filterID.m_ids.empty());

	// our heating loads depend on heating control values, and cooling loads depend on cooling control values
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {
		// pair: result - input

		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_SurfaceHeatingLoad].dataPtr() + i, m_thermostatValueRef) );
	}
}


int IdealPipeRegisterModel::update() {
	// get control values
	double heatingControlValue = *m_thermostatValueRef;
	// clip
	heatingControlValue = std::max(0.0, std::min(1.0, heatingControlValue));
	double massFlow = heatingControlValue * m_maxMassFlow;
	double supplyTemperature = *m_supplyTemperatureRef;

	// calculate inner heat transfer
	double velocity = std::fabs(massFlow)/(m_fluidVolume * m_fluidDensity);
	double viscosity = m_fluidViscosity.value(supplyTemperature);
	double reynolds = ReynoldsNumber(velocity, viscosity, m_innerDiameter);
	double prandtl = PrandtlNumber(viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	double nusselt = NusseltNumber(reynolds, prandtl, m_length, m_innerDiameter);
	double innerHeatTransferCoefficient = nusselt * m_fluidConductivity /
											m_innerDiameter;

	// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
	double UAValueTotal = m_length /
			(
				  1.0 / (innerHeatTransferCoefficient * m_innerDiameter * IBK::PI)
				+ 1.0 / m_UValuePipeWall
			);

	// calculate heat load
	double *surfaceHeatingLoadPtr = m_vectorValuedResults[VVR_SurfaceHeatingLoad].dataPtr();
	unsigned int nTargets = (unsigned int) m_objectList->m_filterID.m_ids.size();
	for(unsigned int i = 0; i < nTargets; ++i) {
		// Q in [W] = DeltaT * UAValueTotal
		double layerTemperature = *m_activeLayerTemperatureRefs[i];
		// calculate heat loss with given (for steady state model we interpret mean temperature as
		// outflow temperature and calculate a corresponding heat flux)
		*(surfaceHeatingLoadPtr + i) = massFlow * m_fluidHeatCapacity *
				(supplyTemperature - layerTemperature) *
				(1. - std::exp(-UAValueTotal / (std::fabs(massFlow) * m_fluidHeatCapacity )) );
	}

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
