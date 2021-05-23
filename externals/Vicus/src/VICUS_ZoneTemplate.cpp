/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "VICUS_ZoneTemplate.h"



namespace VICUS {

ZoneTemplate::ZoneTemplate() {
	for (int i = 0; i<NUM_ST; ++i)
		m_idReferences[i] = VICUS::INVALID_ID;
}


bool ZoneTemplate::isValid() const {

	// TODO : Implement
	if(m_id ==  INVALID_ID)
		return false;

	///TODO check the sub templates in SV-project with isValid()

	return true;
}


unsigned int ZoneTemplate::subTemplateCount() const {

	unsigned int count = 0;
	for (int i=0; i<NUM_ST; ++i)
		if (m_idReferences[i] != VICUS::INVALID_ID) ++count;
	return count;

}


ZoneTemplate::SubTemplateType ZoneTemplate::usedReference(unsigned int index) const {
	int count = 0;
	int i=0;
	// Example  : index = 0 and we have the first id set
	//            loop 1:   i = 0, count = 1 -> return i=0
	// Example 2: index = 1 and we have the first and third id set:
	//            loop 1:   i = 0, count = 1
	//            loop 2:   i = 1, count = 1
	//            loop 3:   i = 2, count = 2  -> return i=1
	for (; i<NUM_ST; ++i) {
		// increase count for each used id reference
		if (m_idReferences[i] != VICUS::INVALID_ID)
			++count;
		if (count > (int)index)
			break;
	}
	return (ZoneTemplate::SubTemplateType)i; // if index > number of used references, we return NUM_ST here
}

AbstractDBElement::ComparisonResult ZoneTemplate::equal(const AbstractDBElement *other) const{
	const ZoneTemplate * otherEPD = dynamic_cast<const ZoneTemplate*>(other);
	if (otherEPD == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_ST; ++i){
		if(m_idReferences[i] != otherEPD->m_idReferences[i])
			return Different;
	}
	//check meta data

	if(m_displayName != otherEPD->m_displayName ||
			m_color != otherEPD->m_color ||
			m_dataSource != otherEPD->m_dataSource ||
			m_notes != otherEPD->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}



} // namespace VICUS
