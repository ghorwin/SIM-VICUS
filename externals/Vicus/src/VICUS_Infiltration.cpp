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

#include "VICUS_Infiltration.h"

#include "VICUS_KeywordList.h"

namespace VICUS {

bool Infiltration::isValid() const {
	if (m_airChangeType == NUM_AC)
		return false;

	try {
		m_para[P_AirChangeRate].checkedValue(VICUS::KeywordList::Keyword("Infiltration::para_t", P_AirChangeRate),
							 "1/h", "1/h", 0, true, 100, true, nullptr);

	}  catch (...) {
		return false;
	}

	if (m_airChangeType == AC_n50){
		try {
			m_para[P_ShieldingCoefficient].checkedValue(VICUS::KeywordList::Keyword("Infiltration::para_t", P_ShieldingCoefficient),
														"-", "-", 0, true, 10, true, nullptr);

		}  catch (...) {
			return false;
		}
	}
	return true;
}


AbstractDBElement::ComparisonResult Infiltration::equal(const AbstractDBElement *other) const{
	const Infiltration * otherInf = dynamic_cast<const Infiltration*>(other);
	if (otherInf == nullptr)
		return Different;

	// check parameters
	if (m_airChangeType != otherInf->m_airChangeType)
		return Different;
	for (unsigned int i=0; i<NUM_P; ++i){
		if (m_para[i] != otherInf->m_para[i])
			return Different;
	}

	//check meta data

	if (m_displayName != otherInf->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}

} // namespace VICUS
