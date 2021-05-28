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

#include "VICUS_WindowGlazingSystem.h"
#include "VICUS_KeywordList.h"

#include "IBK_physics.h"

namespace VICUS {

AbstractDBElement::ComparisonResult WindowGlazingSystem::equal(const AbstractDBElement *other) const {
	const WindowGlazingSystem * otherWindow = dynamic_cast<const WindowGlazingSystem*>(other);
	if (otherWindow == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherWindow->m_para[i])
			return Different;
	}

	for(unsigned int i=0; i<NUM_SP; ++i){
		if(m_splinePara[i] != otherWindow->m_splinePara[i])
			return Different;
	}

	if(m_layers != otherWindow->m_layers)
		return Different;

	//check meta data

	if(m_displayName != otherWindow->m_displayName ||
			m_color != otherWindow->m_color ||
			m_dataSource != otherWindow->m_dataSource ||
			m_manufacturer != otherWindow->m_manufacturer ||
			m_notes != otherWindow->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}

double WindowGlazingSystem::uValue() {
	switch (m_modelType) {
		case VICUS::WindowGlazingSystem::MT_Simple:
			return m_para[P_ThermalTransmittance].get_value();

		case VICUS::WindowGlazingSystem::MT_Detailed:
			///TODO Dirk implement calculation u-value detailed window model
			///TODO Stephan implement calculation u-value detailed window model
		return -1;
		case VICUS::WindowGlazingSystem::NUM_MT:
		return -1;
	}

	return -1;
}

double WindowGlazingSystem::SHGC() {
	switch (m_modelType) {
		case VICUS::WindowGlazingSystem::MT_Simple:
			return m_splinePara[SP_SHGC].m_values.value(0);
		case VICUS::WindowGlazingSystem::MT_Detailed:
			///TODO Dirk implement calculation SHGC detailed window model
			///TODO Stephan implement calculation SHGC detailed window model
		return -1;
		case VICUS::WindowGlazingSystem::NUM_MT:
		return -1;
	}

	return -1;
}

bool WindowGlazingSystem::isValid() const {
	if(m_id == INVALID_ID ||
	   m_modelType == NUM_MT)
		return false;
	switch (m_modelType) {
		case MT_Simple:{
			try {
				m_para[P_ThermalTransmittance].checkedValue("ThermalTransmittance", "W/m2K", "W/m2K", 0, false, 2000, true, nullptr);
				/// TODO Stephan
				//m_splinePara[SP_SHGC].checkAndInitialize("SHGC", IBK::Unit("Deg"), IBK::Unit("---"), IBK::Unit("Deg"), 0, true, 1, true, nullptr);
			}  catch (...) {

			}
		}
		break;
		case MT_Detailed:{
			if(m_layers.empty())
				return false;
			for(unsigned int i=0; i<m_layers.size(); ++i)
				if(!m_layers[i].isValid())
					return false;
		}
		break;
		case NUM_MT:
		return false;
	}



	return true;
}

} // namespace VICUS
