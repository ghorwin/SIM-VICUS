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


bool VICUS::NetworkPipe::isValid() const
{
	if (m_id == INVALID_ID)
		return false;
	if (m_para[VICUS::NetworkPipe::P_DiameterOutside].value <= 0 || m_para[VICUS::NetworkPipe::P_ThicknessWall].value <= 0
			|| m_para[VICUS::NetworkPipe::P_RoughnessWall].value <= 0 || m_para[VICUS::NetworkPipe::P_ThermalConductivityWall].value <= 0)
		return false;
	if (m_para[VICUS::NetworkPipe::P_ThicknessInsulation].value < 0 || m_para[VICUS::NetworkPipe::P_ThermalConductivityInsulation].value < 0)
		return false;
	return true;
}

VICUS::AbstractDBElement::ComparisonResult VICUS::NetworkPipe::equal(const VICUS::AbstractDBElement *other) const {
	const NetworkPipe * otherNetPipe = dynamic_cast<const NetworkPipe*>(other);
	if (otherNetPipe == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherNetPipe->m_para[i])
			return Different;
	}

	if (m_categoryName != otherNetPipe->m_categoryName)
		return Different;

	//check meta data

	if(m_displayName != otherNetPipe->m_displayName ||
			m_color != otherNetPipe->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}
