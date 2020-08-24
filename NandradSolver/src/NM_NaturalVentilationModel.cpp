#include "NM_NaturalVentilationModel.h"

#include <NANDRAD_SimulationParameter.h>

namespace NANDRAD_MODEL {



void NaturalVentilationModel::setup(const NANDRAD::NaturalVentilationModel & ventilationModel,
									const NANDRAD::SimulationParameter & simPara)
{
	m_ventilationModel = &ventilationModel;
	m_moistureBalanceEnabled = simPara.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled();

}


void NaturalVentilationModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

}

const double * NaturalVentilationModel::resultValueRef(const QuantityName & quantityName) const {
	return nullptr;

}

void NaturalVentilationModel::initResults(const std::vector<AbstractModel *> &)
{

}

void NaturalVentilationModel::initInputReferences(const std::vector<AbstractModel *> & models)
{

}

void NaturalVentilationModel::inputReferences(std::vector<InputReference> & inputRefs) const
{

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
