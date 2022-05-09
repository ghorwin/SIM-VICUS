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

#include "VICUS_SurfaceHeating.h"

#include "VICUS_KeywordList.h"

namespace VICUS {



bool SurfaceHeating::isValid(const Database<NetworkPipe> & pipeDB) const {
	if(m_id == INVALID_ID)
		return false;

	switch (m_type) {
		case VICUS::SurfaceHeating::T_Ideal:{
			try {
				m_para[P_HeatingLimit].checkedValue(VICUS::KeywordList::Keyword("SurfaceHeating::para_t", P_HeatingLimit),
																				"W/m2", "W/m2", 0, true, 10000, true, nullptr);
				m_para[P_CoolingLimit].checkedValue(VICUS::KeywordList::Keyword("SurfaceHeating::para_t", P_CoolingLimit),
													"W/m2", "W/m2", 0, true, 10000, true, nullptr);

			}  catch (...) {
				return false;
			}
		}
		break;
		case VICUS::SurfaceHeating::T_PipeRegister:{
			try {
				m_para[P_TemperatureDifferenceSupplyReturn].checkedValue(VICUS::KeywordList::Keyword("SurfaceHeating::para_t", P_TemperatureDifferenceSupplyReturn),
																				"K", "K", 1, true, 80, true, nullptr);
				m_para[P_MaxFluidVelocity].checkedValue(VICUS::KeywordList::Keyword("SurfaceHeating::para_t", P_MaxFluidVelocity),
													"m/s", "m/s", 0, false, 10, true, nullptr);
				m_para[P_PipeSpacing].checkedValue(VICUS::KeywordList::Keyword("SurfaceHeating::para_t", P_PipeSpacing),
														"m", "m", 0, false, 10, true, nullptr);
			}  catch (...) {
				return false;
			}

			const NetworkPipe * pipe = pipeDB[m_idPipe];
			if(pipe == nullptr || !pipe->isValid())
				return false;

			if(m_heatingCoolingCurvePoints.m_values.size() != 2)
				return false;
			if(m_heatingCoolingCurvePoints.m_values.find("Tsupply") == m_heatingCoolingCurvePoints.m_values.end() ||
					m_heatingCoolingCurvePoints.m_values.find("Tout") == m_heatingCoolingCurvePoints.m_values.end())
				return false;
			if(m_heatingCoolingCurvePoints.valueVector("Tsupply").size() != 4 ||
					m_heatingCoolingCurvePoints.valueVector("Tout").size() != 4)
				return false;
		}
		break;
		case VICUS::SurfaceHeating::NUM_T:
			return false;
	}
	return true;
}


AbstractDBElement::ComparisonResult SurfaceHeating::equal(const AbstractDBElement *other) const{
	const SurfaceHeating * surfHeat = dynamic_cast<const SurfaceHeating*>(other);
	if (surfHeat == nullptr)
		return Different;

	//first check critical data

	//check parameters
//	for(unsigned int i=0; i<NUM_P; ++i){
//		if(m_para[i] != surfHeat->m_para[i])
//			return Different;
//	}
	if(m_type != surfHeat->m_type)
		return Different;

	if(m_type == T_Ideal){
		if(m_para[P_HeatingLimit] != surfHeat->m_para[P_HeatingLimit] ||
				m_para[P_CoolingLimit] != surfHeat->m_para[P_CoolingLimit])
			return Different;
	}
	else if(m_type == T_PipeRegister){
		if(m_idPipe != surfHeat->m_idPipe)
			return Different;
		if(m_para[P_TemperatureDifferenceSupplyReturn] != surfHeat->m_para[P_TemperatureDifferenceSupplyReturn] ||
				m_para[P_PipeSpacing] != surfHeat->m_para[P_PipeSpacing] ||
				m_para[P_MaxFluidVelocity] != surfHeat->m_para[P_MaxFluidVelocity])
			return Different;
		if(m_heatingCoolingCurvePoints != surfHeat->m_heatingCoolingCurvePoints)
			return Different;
	}
	//check meta data

	if(m_displayName != surfHeat->m_displayName ||
			m_color != surfHeat->m_color ||
			m_notes != surfHeat->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}

} // namespace VICUS
