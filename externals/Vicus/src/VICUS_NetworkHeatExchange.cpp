#include "VICUS_NetworkHeatExchange.h"

namespace VICUS {

NANDRAD::HydraulicNetworkHeatExchange NetworkHeatExchange::toNandradHeatExchange() const{

	NANDRAD::HydraulicNetworkHeatExchange hx;
	hx.m_modelType = (NANDRAD::HydraulicNetworkHeatExchange::ModelType) m_modelType;
	for (unsigned int i=0; i<NANDRAD::HydraulicNetworkHeatExchange::NUM_P; ++i)
		hx.m_para[i]  = m_para[i];
	for (unsigned int i=0; i<NANDRAD::HydraulicNetworkHeatExchange::NUM_SPL; ++i)
		hx.m_splPara[i] = m_splPara[i];
	for (unsigned int i=0; i<NANDRAD::HydraulicNetworkHeatExchange::NUM_ID; ++i)
		hx.m_idReferences[i]  = m_idReferences[i];

	return hx;
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
