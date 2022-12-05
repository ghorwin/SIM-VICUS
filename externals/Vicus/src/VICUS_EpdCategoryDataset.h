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

#ifndef VICUS_EpdCategroyDatasetH
#define VICUS_EpdCategroyDatasetH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <IBK_Parameter.h>

namespace VICUS {

/*!
	An EPD Category Set defines a material EPD, that needs to be combined by severel different
	sub-EPDs for each Category (A,B,C,D).
*/
class EpdCategoryDataset {

public:
	VICUS_READWRITE
	VICUS_COMP(EpdCategoryDataset)

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Mass of material in dependence to the area. */
		P_AreaDensity,				// Keyword: AreaDensity				[kg/m2]	'mass of material in dependence of the area.'
		/*! Dry density of the material. */
		P_DryDensity,				// Keyword: Density					[kg/m3]	'Dry density of the material.'
		/*! Global Warming Potential. */
		P_GWP,						// Keyword: GWP						[kg]	'Global Warming Potential.'
		/*! Depletion potential of the stratospheric ozone layer . */
		P_ODP,						// Keyword: ODP						[kg]	'Depletion potential of the stratospheric ozone layer.'
		/*! Photochemical Ozone Creation Potential . */
		P_POCP,						// Keyword: POCP					[kg]	'Photochemical Ozone Creation Potential.'
		/*! Acidification potential . */
		P_AP,						// Keyword: AP						[kg]	'Acidification potential.'
		/*! Eutrophication potential. */
		P_EP,						// Keyword: EP						[kg]	'Eutrophication potential.'
		/*! Total use of non-renewable primary energy resources. */
		P_PENRT,					// Keyword: PENRT					[W/mK]	'Total use of non-renewable primary energy resources.'
		/*! Total use of renewable primary energy resources . */
		P_PERT,						// Keyword: PERT					[W/mK]	'Total use of renewable primary energy resources .'

		NUM_P
	};

	//the most frequently used categories
	enum Module {
		M_A1,								// Keyword: A1
		M_A2,								// Keyword: A2
		M_A1_A2,							// Keyword: A1-A2
		M_A3,								// Keyword: A3
		M_A1_A3,							// Keyword: A1-A3
		M_A4,								// Keyword: A4
		M_A5,								// Keyword: A5
		M_B1,								// Keyword: B1
		M_B2,								// Keyword: B2
		M_B3,								// Keyword: B3
		M_B4,								// Keyword: B4
		M_B5,								// Keyword: B5
		M_B6,								// Keyword: B6
		M_B7,								// Keyword: B7
		M_C1,								// Keyword: C1
		M_C2,								// Keyword: C2
		M_C2_C3,							// Keyword: C2-C3
		M_C2_C4,							// Keyword: C2-C4
		M_C3,								// Keyword: C3
		M_C3_C4,							// Keyword: C3-C4
		M_C4,								// Keyword: C4
		M_D,								// Keyword: D
		NUM_M
	};

	EpdCategoryDataset() {}

	EpdCategoryDataset(const Module &module) :
		m_module(module)
	{}

	~EpdCategoryDataset();

	/*! Returns an EPD with all Parameters scaled by the defined factor. */
	EpdCategoryDataset scaleByFactor(const double &factor) const;


	/*! Category type A1, A2, ... */
	Module							m_module = NUM_M;						// XML:E:required

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E


};

}
#endif // VICUS_EpdCategroyDatasetH
