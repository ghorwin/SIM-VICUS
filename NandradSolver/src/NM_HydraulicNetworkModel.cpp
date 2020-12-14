#include "NM_HydraulicNetworkModel.h"

#include <NANDRAD_HydraulicNetwork.h>

namespace NANDRAD_MODEL {

void HydraulicNetworkModel::setup(const NANDRAD::HydraulicNetwork & nw) {

}


void HydraulicNetworkModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// no results for now
}


void HydraulicNetworkModel::resultValueRefs(std::vector<const double *> & res) const {
	// no results for now
}


const double * HydraulicNetworkModel::resultValueRef(const QuantityName & quantityName) const {
	return nullptr;
}


void HydraulicNetworkModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// no inputs for now
}


void HydraulicNetworkModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// no inputs for now
}


void HydraulicNetworkModel::setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) {
	// no inputs for now
}


void HydraulicNetworkModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// no state dependencies for now
}



int HydraulicNetworkModel::update() {
	// re-compute hydraulic network


	return 0; // signal success
}



} // namespace NANDRAD_MODEL
