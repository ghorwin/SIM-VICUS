#include "NM_AbstractModel.h"
#include "NM_AbstractTimeDependency.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD_MODEL {

// Dummy implementations to silence the out-of-line virtual function compiler warning

AbstractModel::~AbstractModel() {

}

AbstractTimeDependency::~AbstractTimeDependency() {

}

AbstractStateDependency::~AbstractStateDependency() {

}

} // namespace NANDRAD_MODEL
