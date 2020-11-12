#include "NM_WindowModel.h"

namespace NANDRAD_MODEL {

void WindowModel::setup(const NANDRAD::EmbeddedObjectWindow & windowModelPara,
						const NANDRAD::SimulationParameter & simPara,
						const NANDRAD::ConstructionInstance & conInst)
{

}

void WindowModel::initResults(const std::vector<AbstractModel *> &)
{

}

void WindowModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const
{

}

const double * WindowModel::resultValueRef(const QuantityName & quantityName) const
{

}

void WindowModel::inputReferences(std::vector<InputReference> & inputRefs) const
{

}

void WindowModel::setInputValueRefs(const std::vector<QuantityDescription> &, const std::vector<const double *> & resultValueRefs)
{

}

void WindowModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const
{

}

int WindowModel::update()
{

}



} // namespace NANDRAD_MODEL
