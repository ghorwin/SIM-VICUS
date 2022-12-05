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

#ifndef VICUS_EpdCategroySetH
#define VICUS_EpdCategroySetH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"


#include <IBK_Parameter.h>
namespace VICUS {

/*!
	An EPD Category Set defines a material EPD, that needs to be combined by severel different
	sub-EPDs for each Category (A,B,C,D).
*/
class EpdCategroySet {

public:
	VICUS_READWRITE
	VICUS_COMP(EpdCategroySet)

	EpdCategroySet() {}

	EpdCategroySet(unsigned int idCatA, unsigned int idCatB, unsigned int idCatC, unsigned int idCatD):
		m_idCategoryA(idCatA),
		m_idCategoryB(idCatB),
		m_idCategoryC(idCatC),
		m_idCategoryD(idCatD)
	{}

	inline bool isEmpty() const {
		return m_idCategoryA == INVALID_ID && m_idCategoryB == INVALID_ID &&
				m_idCategoryC == INVALID_ID && m_idCategoryD == INVALID_ID;
	}


	/* Id of Category A .*/
	unsigned int				m_idCategoryA = INVALID_ID;					// XML:A
	/* Id of Category B .*/
	unsigned int				m_idCategoryB = INVALID_ID;					// XML:A
	/* Id of Category C .*/
	unsigned int				m_idCategoryC = INVALID_ID;					// XML:A
	/* Id of Category D .*/
	unsigned int				m_idCategoryD = INVALID_ID;					// XML:A


};

inline bool EpdCategroySet::operator!=(const EpdCategroySet & other) const {
	if (m_idCategoryA != other.m_idCategoryA) return true;
	if (m_idCategoryB != other.m_idCategoryB) return true;
	if (m_idCategoryC != other.m_idCategoryC) return true;
	if (m_idCategoryD != other.m_idCategoryD) return true;
	return false;
}

}
#endif // VICUS_EpdCategroySetH
