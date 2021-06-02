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

#include "VICUS_ZoneIdealHeatingCooling.h"

#include "VICUS_KeywordList.h"

namespace VICUS {



bool ZoneIdealHeatingCooling::isValid() const
{
	if(m_id == INVALID_ID)
		return false;

	try {
		if(!m_para[P_HeatingLimit].empty())
			m_para[P_HeatingLimit].checkedValue(VICUS::KeywordList::Keyword("ZoneIdealHeatingCooling::para_t", P_HeatingLimit),
							 "W/m2", "W/m2", 0, true, 1000, true, nullptr);
		if(!m_para[P_CoolingLimit].empty())
			m_para[P_CoolingLimit].checkedValue(VICUS::KeywordList::Keyword("ZoneIdealHeatingCooling::para_t", P_CoolingLimit),
							 "W/m2", "W/m2", 0, true, 1000, true, nullptr);

	}  catch (...) {
		return false;
	}

	return true;
}

AbstractDBElement::ComparisonResult ZoneIdealHeatingCooling::equal(const AbstractDBElement *other) const{
	const ZoneIdealHeatingCooling * otherHC = dynamic_cast<const ZoneIdealHeatingCooling*>(other);
	if (otherHC == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherHC->m_para[i])
			return Different;
	}
		//check meta data

	if(m_displayName != otherHC->m_displayName ||
			m_color != otherHC->m_color ||
			m_dataSource != otherHC->m_dataSource ||
			m_notes != otherHC->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}

}
