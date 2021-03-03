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
	if (m_type != other.m_type)
		return true;
	if (m_spline != other.m_spline)
		return true;

	return false;
}


} // namespace VICUS
