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

#include <IBKMK_Vector3D.h>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_EpdModuleDataset.h"


namespace VICUS {

/*! Stores general settings about LCA Options. */
class LccSettings {
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	enum para_t {
		P_PriceIncreaseGeneral,				// Keyword: PriceIncreaseGeneral			[%]		'Yearly general price increase [%].'
		P_PriceIncreaseEnergy,				// Keyword: PriceIncreaseEnergy				[%]		'Yearly energy price increase [%].'
		P_DiscountingInterestRate,			// Keyword: DiscountingInterestRate			[%]		'Interest rate for discounting [%].'
		P_CoalConsumption,					// Keyword: CoalConsumption					[kWh/a]	'Coal consumption [kWh/a].'
		P_GasConsumption,					// Keyword: GasConsumption					[kWh/a]	'Gas consumption [kWh/a].'
		P_ElectricityConsumption,			// Keyword: ElectricityConsumption			[kWh/a]	'Electricity consumption [kWh/a].'
		NUM_P
	};

	// TODO : refactor to allow double values as prices are defined in xx,xx ct/kWh in data tables
	enum intPara_t {
		IP_CoalPrice,						// Keyword: CoalPrice					'Price of Coal in ct per kWh.'
		IP_GasPrice,						// Keyword: GasPrice					'Price of Gas in ct per kWh.'
		IP_ElectricityPrice,				// Keyword: ElectricityPrice			'Price of Electricity in ct per kWh.'
		NUM_IP
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
	bool isLcaCategoryDefined(EpdModuleDataset::Module mod) const;


	// TODO : add checkParameters() function, to be called after reading project data to check for correct units
	//        otherwise users may specify "ElectricityConsumption = 50 C" and the software/UI would crash

	// *** PUBLIC MEMBER VARIABLES ***

	/*! IBK::Parameters. */
	IBK::Parameter						m_para[NUM_P];								// XML:E

	/*! IBK::IntParas. */
	IBK::IntPara						m_intPara[NUM_IP];							// XML:E
};

inline bool LccSettings::operator!=(const LccSettings & other) const {
	for(unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i])
			return true;

	return false;
}


} // namespace VICUS


#endif // VICUS_LcaSettingsH
