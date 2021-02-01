#include "VICUS_NetworkComponent.h"

namespace VICUS {

void NetworkComponent::readXML(const TiXmlElement * element) {
	// TODO: Andreas, first read NANDRAD::HydraulicNetworkComponent::readXML,
	//       then call readXMLprivate()
}

TiXmlElement * NetworkComponent::writeXML(TiXmlElement * parent) const {
	// TODO: Andreas, first call NANDRAD::HydraulicNetworkComponent::writeXML,
	//       then call readXMLprivate()
	return nullptr;
}


bool NetworkComponent::isValid() const {
	// TODO : Hauke, implement

	return true;
}


} // namespace VICUS
