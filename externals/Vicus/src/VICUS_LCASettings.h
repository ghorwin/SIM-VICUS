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

#ifndef VICUS_LCASettingsH
#define VICUS_LCASettingsH

#include <IBK_Flag.h>

#include <IBKMK_Vector3D.h>
#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

/*! Stores general settings about LCA Options. */
class LCASettings {
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	enum para_t {
		P_TimePeriod,					// Keyword: TimePeriod					[a]		'Time period for consideration [a].'
		P_PriceIncrease,				// Keyword: PriceIncrease				[%]		'Yearly price increase [%].'
		NUM_P
	};


	VICUS_READWRITE_IFNOTEMPTY(LCASettings)
	VICUS_COMP(LCASettings)

	/*! Constructor. */
	LCASettings() {
		initDefaults();
	}


	/*! Init default values. */
	void initDefaults();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Time Period for LCA/LCC consideration. */
	IBK::Parameter						m_para[NUM_P];			// XML:E

};

inline bool LCASettings::operator!=(const LCASettings & other) const {
	if (m_para[P_TimePeriod] != other.m_para[P_TimePeriod]) return true;
	if (m_para[P_PriceIncrease] != other.m_para[P_PriceIncrease]) return true;
	return false;
}


} // namespace VICUS


#endif // VICUS_LCASettingsH
