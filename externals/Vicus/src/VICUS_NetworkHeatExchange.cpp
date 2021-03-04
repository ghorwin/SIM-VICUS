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
	for (unsigned int i=0; i<NUM_IP; ++i){
		if (m_intPara[i] != other.m_intPara[i])
			return true;
	}
	if (m_modelType != other.m_modelType)
		return true;
	if (m_heatExchangeSpline != other.m_heatExchangeSpline)
		return true;

	return false;
}


} // namespace VICUS
