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

#include "VICUS_NetworkPipe.h"

#include "VICUS_KeywordList.h"

bool VICUS::NetworkPipe::isValid() const
{
	if (m_id == INVALID_ID)
		return false;

	for (unsigned int i=0; i<NUM_P; ++i){
		bool zeroAllowed = para_t(i) == P_ThicknessInsulation;
		int ii = (int)i;
		try {
			m_para[i].checkedValue(KeywordList::Keyword("NetworkPipe::para_t", ii), KeywordList::Unit("NetworkPipe::para_t", ii),
								   KeywordList::Unit("NetworkPipe::para_t", ii), 0, zeroAllowed, std::numeric_limits<double>::max(),
								   false, nullptr);
		} catch (...) {
			return false;
		}
	}

	return true;
}

VICUS::AbstractDBElement::ComparisonResult VICUS::NetworkPipe::equal(const VICUS::AbstractDBElement *other) const {
	const NetworkPipe * otherNetPipe = dynamic_cast<const NetworkPipe*>(other);
	if (otherNetPipe == nullptr)
		return Different;

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherNetPipe->m_para[i])
			return Different;
	}

	//check meta data
	if(m_displayName != otherNetPipe->m_displayName || m_color != otherNetPipe->m_color
			|| m_categoryName != otherNetPipe->m_categoryName)
		return OnlyMetaDataDiffers;

	return Equal;
}
