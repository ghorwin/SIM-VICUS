#include "NANDRAD_HydraulicNetworkPipeProperties.h"

namespace NANDRAD {

bool HydraulicNetworkPipeProperties::operator!=(const HydraulicNetworkPipeProperties &other) const {
	for (unsigned n=0; n<NUM_P; ++n){
		if (m_para[n] != other.m_para[n])
			return true;
	}
	return false;
}

} // namespace NANDRAD
