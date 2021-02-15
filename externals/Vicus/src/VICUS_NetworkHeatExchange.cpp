#include "VICUS_NetworkHeatExchange.h"

namespace VICUS {

NetworkHeatExchange::NetworkHeatExchange()
{

}

bool NetworkHeatExchange::operator!=(const NetworkHeatExchange &other) const{

	for (unsigned int i=0; i<NUM_P; ++i){
		if (m_para[i] != other.m_para[i])
			return true;
	}
	if (m_dataFile != other.m_dataFile)
		return true;
	if (m_fmuFile != other.m_fmuFile)
		return true;

	return false;
}


} // namespace VICUS
