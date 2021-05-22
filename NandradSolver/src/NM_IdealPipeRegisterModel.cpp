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
									 const std::vector<NANDRAD::ObjectList> & objLists)
{
	FUNCID(IdealPipeRegisterModel::setup);

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

	// retrieve reference to supply temperature parameter
	if (model.m_modelType == NANDRAD::IdealPipeRegisterModel::MT_Constant)
		m_supplyTemperatureRef = &model.m_para[NANDRAD::IdealPipeRegisterModel::P_SupplyTemperature].value;


	// retrieve model type
	m_modelType = (int) model.m_modelType;

	// retrieve all parameters (have been checked already)
	m_maxMassFlow = model.m_para[NANDRAD::IdealPipeRegisterModel::P_MaxMassFlow].value;
	// pipe properties
	m_innerDiameter = model.m_para[NANDRAD::IdealPipeRegisterModel::P_PipeInnerDiameter].value;
	m_length = model.m_para[NANDRAD::IdealPipeRegisterModel::P_PipeLength].value;
	m_UValuePipeWall = model.m_para[NANDRAD::IdealPipeRegisterModel::P_UValuePipeWall].value;
	// integers
	m_nParallelPipes = (unsigned int) model.m_intPara[NANDRAD::IdealPipeRegisterModel::IP_NumberParallelPipes].value;
	// fluid properties
	m_fluidHeatCapacity = model.m_fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidDensity = model.m_fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidConductivity = model.m_fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = model.m_fluid.m_kinematicViscosity.m_values;

	// compute fluid cross section and volume
	m_fluidCrossSection = IBK::PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length;

	// store zone
	m_thermostatZoneId = model.m_thermostatZoneID;

	// reserve storage memory for vector valued results
	m_vectorValuedResults.resize(NUM_VVR);

	// the rest of the initialization can only be done when the object lists have been initialized, i.e. this happens in resultDescriptions()
}


void IdealPipeRegisterModel::initResults(const std::vector<AbstractModel *> &) {
	// no construction instance IDs, nothing to do
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return
	// get IDs of referenced constructions
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());
	// resize result vectors accordingly
	for (unsigned int varIndex=0; varIndex<NUM_VVR; ++varIndex)
		m_vectorValuedResults[varIndex] = VectorValuedQuantity(indexKeys);
}


void IdealPipeRegisterModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// during initialization of the object lists, only those construction instances were added,
	// that are actually parameterized
	// so we can rely on the existence of construction instances whose IDs are in our object list and we
	// do not need to search through all the models
	if (m_objectList->m_filterID.m_ids.empty())
		return; // nothing to compute, return

	// Retrieve index information from vector valued results.
	std::vector<unsigned int> indexKeys(m_objectList->m_filterID.m_ids.begin(), m_objectList->m_filterID.m_ids.end());

	// For each of the constructions in the object list we generate vector-valued results as defined
	// in the type Results.
	for (int varIndex=0; varIndex<NUM_VVR; ++varIndex) {
		// store name, unit and description of the quantity
		const std::string &quantityName = KeywordList::Keyword("IdealPipeRegisterModel::VectorValuedResults", varIndex );
		const std::string &quantityUnit = KeywordList::Unit("IdealPipeRegisterModel::VectorValuedResults", varIndex );
		const std::string &quantityDescription = KeywordList::Description("IdealPipeRegisterModel::VectorValuedResults", varIndex );
		resDesc.push_back( QuantityDescription(
			quantityName, quantityUnit, quantityDescription, false, VectorValuedQuantityIndex::IK_ModelID, indexKeys) );
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

	InputReference tRef;
	tRef.m_name.m_name = "ActiveLayerTemperature";
	tRef.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	tRef.m_required = true;
	// keys code construction id
	for(unsigned int key : indexKeys) {
		tRef.m_id = key;
		// store reference
		m_inputRefs.push_back(tRef);
	}

	// retrieve reference to scheduled supply temperature parameter
	if(m_modelType == (int) NANDRAD::IdealPipeRegisterModel::MT_Scheduled) {
		tRef.m_name.m_name = "SupplyTemperature";
		tRef.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		tRef.m_id = m_thermostatZoneId;
		tRef.m_required = true;
		// store reference
		m_inputRefs.push_back(tRef);
	}

	// loop over all models, pick out the Thermostat-models and request input for a single zone. Only
	// one (and exactly) ome request per zone must be fulfilled!
	for (AbstractModel * model : models) {
		// ignore all models that are not thermostat models
		if (model->referenceType() != NANDRAD::ModelInputReference::MRT_MODEL)
			continue;
		ThermostatModel * mod = dynamic_cast<ThermostatModel *>(model);
		if (mod != nullptr) {
			// create an input reference to heating and cooling control values for all the constructions that we heat
			InputReference r;
			r.m_id = model->id();
			r.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
			r.m_name.m_name = "HeatingControlValue";
			r.m_name.m_index = (int) m_thermostatZoneId; // vector gets ID of requested zone
			r.m_required = false;
			m_inputRefs.push_back(r);
			r.m_name.m_name = "CoolingControlValue";
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

	// retrieve reference to scheduled supply temperature parameter
	if(m_modelType == (int) NANDRAD::IdealPipeRegisterModel::MT_Scheduled) {
		m_supplyTemperatureRef = resultValueRefs[index++];
	}
	// we now must ensure, that for each zone there is exactly one matching control signal

	IBK_ASSERT(2 * m_thermostatModelObjects + index == resultValueRefs.size());

	for (unsigned int i=0; i<m_thermostatModelObjects; ++i) {
		// heating control value
		// heating control value
		if (resultValueRefs[2 * i] != nullptr) {
			// we check only for heating control value if maximum power is > 0
			// perform check for each zone in order to avoid duplicate definition
			if(m_heatingThermostatValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate heating control value result generated by different thermostats "
													   "for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
			m_heatingThermostatValueRef = resultValueRefs[2 * i + index];
		}
		if (resultValueRefs[2 * i + 1] != nullptr) {
			// we check only for cooling control value if maximum power is > 0
			// perform check for each zone in order to avoid duplicate definition
			if( m_coolingThermostatValueRef != nullptr)
				throw IBK::Exception(IBK::FormatString("Duplicate cooling control value result generated by different thermostats "
													   "for zone id=%1.").arg(m_thermostatZoneId), FUNC_ID);
			m_coolingThermostatValueRef = resultValueRefs[2 * i + index + 1];
		}
	}

	// check that we have indeed value refs for all zones
	if (m_heatingThermostatValueRef == nullptr && m_coolingThermostatValueRef == nullptr)
		throw IBK::Exception(IBK::FormatString("Missing heating or cooling control value for zone id=%1.")
							 .arg(m_thermostatZoneId), FUNC_ID);
}


void IdealPipeRegisterModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// our heating loads depend on heating control values, and cooling loads depend on cooling control values
	for (unsigned int i=0; i<m_objectList->m_filterID.m_ids.size(); ++i) {

		// we have heating defined
		if(m_heatingThermostatValueRef != nullptr) {
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_MassFlux].dataPtr() + i, m_heatingThermostatValueRef) );
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_ActiveLayerThermalLoad].dataPtr() + i, m_heatingThermostatValueRef) );
		}
		// we operate in cooling mode
		if(m_coolingThermostatValueRef != nullptr){
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_MassFlux].dataPtr() + i, m_coolingThermostatValueRef) );
			resultInputValueReferences.push_back(
						std::make_pair(m_vectorValuedResults[VVR_ActiveLayerThermalLoad].dataPtr() + i, m_coolingThermostatValueRef) );
		}
		// add supply temperature reference
		resultInputValueReferences.push_back(
					std::make_pair(m_vectorValuedResults[VVR_ActiveLayerThermalLoad].dataPtr() + i, m_supplyTemperatureRef) );
	}
}


int IdealPipeRegisterModel::update() {

	if (m_objectList->m_filterID.m_ids.empty())
		return 0; // nothing to compute, return

	double heatingMassFlow = 0.0;
	double coolingMassFlow = 0.0;

	// get control value for heating
	if(m_heatingThermostatValueRef != nullptr) {
		double heatingControlValue = *m_heatingThermostatValueRef;
		// clip
		heatingControlValue = std::max(0.0, std::min(1.0, heatingControlValue));
		heatingMassFlow = heatingControlValue * m_maxMassFlow;
	}
	// get control value for cooling
	if(m_coolingThermostatValueRef != nullptr) {
		double coolingControlValue = *m_coolingThermostatValueRef;
		// clip
		coolingControlValue = std::max(0.0, std::min(1.0, coolingControlValue));
		coolingMassFlow = coolingControlValue * m_maxMassFlow;
	}

	// retrieve supply temperature
	double supplyTemperature = *m_supplyTemperatureRef;
	// calculate viscosity and prandtl number (independent from mass flow)
	double viscosity = m_fluidViscosity.value(supplyTemperature);
	double prandtl = PrandtlNumber(viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);

	// store number of active layers heated by current model
	unsigned int nTargets = (unsigned int) m_objectList->m_filterID.m_ids.size();

	// access all vector quantities
	double *massFluxPtr = m_vectorValuedResults[VVR_MassFlux].dataPtr();
	double *surfaceLoadPtr = m_vectorValuedResults[VVR_ActiveLayerThermalLoad].dataPtr();

	// set initial heat load and mass flux to 0
	std::memset(massFluxPtr, 0.0, nTargets * sizeof (double) );
	std::memset(surfaceLoadPtr, 0.0, nTargets * sizeof (double) );

	// both heating and cooling may request a positive mass flux
	// anyhow, we demand for a supply temperature > layer temperature in the case
	// of heating and a supply temperature < layer temperature in the case of cooling
	// both demands never will be fulfilled together and therefor mass flux is uniquely defined

	// heating is requested
	if(heatingMassFlow > 0.0) {
		// calculate inner heat transfer
		double velocity = std::fabs(heatingMassFlow)/(m_fluidVolume * m_fluidDensity);
		double reynolds = ReynoldsNumber(velocity, viscosity, m_innerDiameter);
		double nusselt = NusseltNumber(reynolds, prandtl, m_length, m_innerDiameter);
		double innerHeatTransferCoefficient = nusselt * m_fluidConductivity /
												m_innerDiameter;

		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		double UAValueTotal = m_length /
				( 1.0 / (innerHeatTransferCoefficient * m_innerDiameter * IBK::PI)
				+ 1.0 / m_UValuePipeWall );

		// compute exponential decay of thermal mass (analytical soluation for heat
		// loss over a pipe with constant environmental temperature
		double thermalMassDecay = heatingMassFlow * m_fluidHeatCapacity *
				(1. - std::exp(-UAValueTotal / (std::fabs(heatingMassFlow) * m_fluidHeatCapacity ) ) );

		// distribute load for all targets
		for(unsigned int i = 0; i < nTargets; ++i) {
			// only accept layers whose temperature < supply temperature
			double layerTemperature = *m_activeLayerTemperatureRefs[i];
			if(layerTemperature >= supplyTemperature) {
				continue;
			}
			// store mass flow
			*(massFluxPtr + i) = heatingMassFlow;
			// calculate heat gain in [W]
			*(surfaceLoadPtr + i) = (supplyTemperature - layerTemperature) * thermalMassDecay;
		}
	}
	// cooling is requested
	if(coolingMassFlow > 0.0) {
		// calculate inner heat transfer
		double velocity = std::fabs(coolingMassFlow)/(m_fluidVolume * m_fluidDensity);
		double reynolds = ReynoldsNumber(velocity, viscosity, m_innerDiameter);
		double nusselt = NusseltNumber(reynolds, prandtl, m_length, m_innerDiameter);
		double innerHeatTransferCoefficient = nusselt * m_fluidConductivity /
												m_innerDiameter;

		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		double UAValueTotal = m_length /
				( 1.0 / (innerHeatTransferCoefficient * m_innerDiameter * IBK::PI)
				+ 1.0 / m_UValuePipeWall );

		// compute exponential decay of thermal mass
		double thermalMassDecay = coolingMassFlow * m_fluidHeatCapacity *
				(1. - std::exp(-UAValueTotal / (std::fabs(coolingMassFlow) * m_fluidHeatCapacity ) ) );

		// distribute load for all targets
		for(unsigned int i = 0; i < nTargets; ++i) {
			// only accept layers whose temperature > supply temperature
			double layerTemperature = *m_activeLayerTemperatureRefs[i];
			if(layerTemperature <= supplyTemperature) {
				continue;
			}
			// store mass flow
			*(massFluxPtr + i) = coolingMassFlow;
			// calculate heat loss in [W]
			*(surfaceLoadPtr + i) = (supplyTemperature - layerTemperature) * thermalMassDecay;
		}
	}

	return 0; // signal success
}

} // namespace NANDRAD_MODEL
