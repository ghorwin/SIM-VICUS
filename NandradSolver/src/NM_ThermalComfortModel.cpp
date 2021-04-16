#include "NM_ThermalComfortModel.h"

#include <IBK_Exception.h>
#include <IBK_physics.h>

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Zone.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {



void ThermalComfortModel::setup() {
	FUNCID(ThermalComfortModel::setup);

}


void ThermalComfortModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	QuantityDescription result;
	result.m_constant = true;
	result.m_description = NANDRAD_MODEL::KeywordList::Description("ThermalComfortModel::Results", R_OperativeTemperature);
	result.m_name = NANDRAD_MODEL::KeywordList::Keyword("ThermalComfortModel::Results", R_OperativeTemperature);
	result.m_displayName = m_displayName;
	result.m_unit = NANDRAD_MODEL::KeywordList::Unit("ThermalComfortModel::Results", R_OperativeTemperature);

	resDesc.push_back(result);
}


const double * ThermalComfortModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	IBK_ASSERT(quantityName.m_name == NANDRAD_MODEL::KeywordList::Description("ThermalComfortModel::Results", R_OperativeTemperature));
	return &m_operativeTemperature;
}


void ThermalComfortModel::inputReferences(std::vector<InputReference> & inputRefs) const {



}


void ThermalComfortModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
												const std::vector<const double *> & resultValueRefs)
{
	// simply store and check value references
	m_valueRefs = resultValueRefs;
}


void ThermalComfortModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// operative temperature depends on all input variables
	for (const double * valRef : m_valueRefs) {
		// dependency on room air temperature of corresponding zone
		resultInputValueReferences.push_back(std::make_pair(&m_operativeTemperature, valRef) );
	}
}


int ThermalComfortModel::update() {

	// compute radiant temperature as area-weighted mean temperature
	// add room air temperature



	return 0; // signal success
}


} // namespace NANDRAD_MODEL
