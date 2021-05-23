/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>
	  
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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
