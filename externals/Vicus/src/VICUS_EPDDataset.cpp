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

#include "VICUS_EpdDataset.h"

namespace VICUS {

VICUS::EpdDataset VICUS::EpdDataset::scaleByFactor(const double & factor) const {
	VICUS::EpdDataset epd = *this;
	for(unsigned int i=0; i<epd.m_epdCategoryDataset.size(); ++i) {
		epd.m_epdCategoryDataset[i].scaleByFactor(factor);
	}
	return epd;
}

bool EpdDataset::behavesLike(const EpdDataset & other) const {

	if(m_referenceUnit != other.m_referenceUnit ||
			m_referenceQuantity != other.m_referenceQuantity)
		return false;

	if(m_category != other.m_category)
		return false;

	if(m_epdCategoryDataset.size() != other.m_epdCategoryDataset.size())
		return false;

	for(unsigned int i = 0; i<m_epdCategoryDataset.size(); ++i) {
		const EpdCategoryDataset &ds      = m_epdCategoryDataset[i];
		const EpdCategoryDataset &dsOther = other.m_epdCategoryDataset[i];

		for (unsigned int i=0; i<EpdCategoryDataset::NUM_P; ++i) {
			EpdCategoryDataset::para_t t = static_cast<EpdCategoryDataset::para_t>(i);
			if(ds.m_para[t].empty() && dsOther.m_para[t].empty())
				continue;
			if(ds.m_para[t] != dsOther.m_para[t])
				return false;
		}
	}

	return true;

}

AbstractDBElement::ComparisonResult EpdDataset::equal(const AbstractDBElement *other) const {
	const EpdDataset * otherEPD = dynamic_cast<const EpdDataset*>(other);
	if (otherEPD == nullptr)
		return Different;

	//first check critical data
	if(m_epdCategoryDataset.size() != otherEPD->m_epdCategoryDataset.size())
		return Different;

	//check parameters
	for(unsigned int i=0; i<m_epdCategoryDataset.size(); ++i){
		for(unsigned int j=0; j<EpdCategoryDataset::NUM_P; ++j){
			if(m_epdCategoryDataset[i].m_para[j] != otherEPD->m_epdCategoryDataset[i].m_para[j])
				return Different;
		}
	}
	if(m_uuid != otherEPD->m_uuid ||
			m_referenceUnit != otherEPD->m_referenceUnit||
			m_referenceQuantity != otherEPD->m_referenceQuantity ||
			m_type != otherEPD->m_type ||
			m_category != otherEPD->m_category ||
			m_expireYear != otherEPD->m_expireYear)
		return Different;

	//check meta data

	if(m_displayName != otherEPD->m_displayName ||
			m_color != otherEPD->m_color ||
			m_dataSource != otherEPD->m_dataSource ||
			m_manufacturer != otherEPD->m_manufacturer ||
			m_notes != otherEPD->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}

EpdDataset EpdDataset::operator+(const EpdDataset & epd) {
	VICUS::EpdDataset addedEpd = *this;
	for(unsigned int j=0; j<m_epdCategoryDataset.size(); ++j) {
		for(unsigned int i=0; i<EpdCategoryDataset::NUM_P; ++i)
			addedEpd.m_epdCategoryDataset[j].m_para[i].value += epd.m_epdCategoryDataset[j].m_para[i].value;
	}
	return addedEpd;
}

void EpdDataset::operator+=(const EpdDataset & epd) {
//	for(unsigned int i=0; i<NUM_P; ++i) {
//		m_para[i].value += epd.m_para[i].value;
//	}
}

}
