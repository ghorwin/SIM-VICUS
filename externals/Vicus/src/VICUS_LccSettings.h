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

#ifndef VICUS_LccSettingsH
#define VICUS_LccSettingsH

#include <IBK_Flag.h>

#include <IBKMK_Vector3D.h>
#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_EpdCategoryDataset.h"


namespace VICUS {

/*! Stores general settings about LCA Options. */
class LccSettings {
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	enum para_t {
        P_PriceIncreaseGeneral,				// Keyword: PriceIncreaseGeneral			[%]		'Yearly general price increase [%].'
        P_PriceIncreaseEnergy,				// Keyword: PriceIncreaseEnergy             [%]		'Yearly energy price increase [%].'
        P_CalculationInterestRate,			// Keyword: CalculationInterestRate         [%]		'Interest rate for calculation [%].'
        NUM_P
	};

    VICUS_READWRITE_IFNOTEMPTY(LccSettings)
    VICUS_COMP(LccSettings)

	/*! Constructor. */
    LccSettings() {
		initDefaults();
	}


	/*! Init default values. */
	void initDefaults();

	/*! Returns whether a Category is defined in LCA Settings. */
	bool isLcaCategoryDefined(EpdCategoryDataset::Module mod) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Time Period for LCA/LCC consideration. */
	IBK::Parameter						m_para[NUM_P];								// XML:E



};

inline bool LccSettings::operator!=(const LccSettings & other) const {


	return false;
}


} // namespace VICUS


#endif // VICUS_LcaSettingsH
