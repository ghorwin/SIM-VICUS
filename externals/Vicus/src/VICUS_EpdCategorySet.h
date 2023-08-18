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

#ifndef VICUS_EpdCategorySetH
#define VICUS_EpdCategorySetH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <IBK_Parameter.h>
namespace VICUS {
/*!
	An EPD Category Set defines a material EPD, that needs to be combined by severel different
	sub-EPDs for each Category (A,B,C,D).
*/
class EpdCategorySet {

public:

	enum Category {
		C_IDCategoryA,				// Keyword: IDCategoryA				[-]	'ID of Catefory A'
		C_IDCategoryB,				// Keyword: IDCategoryB				[-]	'ID of Catefory B'
		C_IDCategoryC,				// Keyword: IDCategoryC				[-]	'ID of Catefory C'
		C_IDCategoryD,				// Keyword: IDCategoryD				[-]	'ID of Catefory D'
		NUM_C
	};

    VICUS_COMP(EpdCategorySet)

	EpdCategorySet() {
		m_idCategory[C_IDCategoryA] = VICUS::INVALID_ID;
		m_idCategory[C_IDCategoryB] = VICUS::INVALID_ID;
		m_idCategory[C_IDCategoryC] = VICUS::INVALID_ID;
		m_idCategory[C_IDCategoryD] = VICUS::INVALID_ID;
	}


	EpdCategorySet(unsigned int idCatA, unsigned int idCatB, unsigned int idCatC, unsigned int idCatD) 	{
		m_idCategory[C_IDCategoryA] = idCatA;
		m_idCategory[C_IDCategoryB] = idCatB;
		m_idCategory[C_IDCategoryC] = idCatC;
		m_idCategory[C_IDCategoryD] = idCatD;
	}

	inline bool isEmpty() const {
		return m_idCategory[C_IDCategoryA] == INVALID_ID && m_idCategory[C_IDCategoryB] == INVALID_ID &&
				m_idCategory[C_IDCategoryC] == INVALID_ID && m_idCategory[C_IDCategoryD] == INVALID_ID;
	}

	void readXML(const TiXmlElement * element); \
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/* Id of Category .*/
	IDType				m_idCategory[NUM_C];

};


inline bool EpdCategorySet::operator!=(const EpdCategorySet & other) const {
	if (m_idCategory[C_IDCategoryA] != other.m_idCategory[C_IDCategoryA]) return true;
	if (m_idCategory[C_IDCategoryB] != other.m_idCategory[C_IDCategoryB]) return true;
	if (m_idCategory[C_IDCategoryC] != other.m_idCategory[C_IDCategoryC]) return true;
	if (m_idCategory[C_IDCategoryD] != other.m_idCategory[C_IDCategoryD]) return true;
	return false;
}

}
#endif // VICUS_EpdCategorySetH
