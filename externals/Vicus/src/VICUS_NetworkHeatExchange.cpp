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
	for (unsigned int i=0; i<NUM_ID; ++i){
		if (m_idReferences[i] != other.m_idReferences[i])
			return true;
	}
	for (unsigned int i=0; i<NUM_SPL; ++i){
		if (m_splPara[i] != other.m_splPara[i])
			return true;
	}
	if (m_modelType != other.m_modelType)
		return true;

	return false;
}


} // namespace VICUS
