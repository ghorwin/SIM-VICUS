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

#ifndef VICUS_EpdModuleDatasetH
#define VICUS_EpdModuleDatasetH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_AbstractDBElement.h"

#include <IBK_Parameter.h>

namespace VICUS {

/*!
	An EPD Category Set defines a material EPD, that needs to be combined by severel different
	sub-EPDs for each Category (A,B,C,D).
*/
class EpdModuleDataset {
public:
	VICUS_COMP(EpdModuleDataset)

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Mass of material in dependence to the area. */
		P_AreaDensity,				// Keyword: AreaDensity				[kg/m2]	'mass of material in dependence of the area.'
		/*! Dry density of the material. */
		P_DryDensity,				// Keyword: DryDensity				[kg/m3]	'Dry density of the material.'
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
		M_A1,		// Keyword: A1					'A1: Raw material procurement'
		M_A2,		// Keyword: A2					'A2: Transport'
		M_A3,		// Keyword: A3					'A3: Production'
		M_A4,		// Keyword: A4					'A4: Transport'
		M_A5,		// Keyword: A5					'A5: Construction / Installation'
		M_B1,		// Keyword: B1					'B1: Usage'
		M_B2,		// Keyword: B2					'B2: Maintenance'
		M_B3,		// Keyword: B3					'B3: Maintenance'
		M_B4,		// Keyword: B4					'B4: Replacement'
		M_B5,		// Keyword: B5					'B5: Modernisation'
		M_B6,		// Keyword: B6					'B6: Energy consumption in operation'
		M_B7,		// Keyword: B7					'B7: Water consumption in operation'
		M_C1,		// Keyword: C1					'C1: deconstruction / demolition'
		M_C2,		// Keyword: C2					'C2: Transport'
		M_C3,		// Keyword: C3					'C3: Waste recycling'
		M_C4,		// Keyword: C4					'C4: Disposal'
		M_D,		// Keyword: D					'D: Potential for reuse, recovery and recycling'
		NUM_M
	};

	EpdModuleDataset() {}

	EpdModuleDataset(const std::vector<Module> &modules) :
		m_modules(modules)
	{}

	~EpdModuleDataset();

	/*! Checks if all parameters of Category dataset. */
	bool isValid() const;

	/*! Returns an EPD with all Parameters scaled by the defined factor. */
	EpdModuleDataset scaleByFactor(const double &factor) const;

	/*! Defines Operator += .*/
	const EpdModuleDataset& operator +=(const EpdModuleDataset& otherEpd);

	void operator/(const EpdModuleDataset& otherEpd);


	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;


	/*! Category type A1, A2, ... */
	std::vector<Module>				m_modules;

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];

};

}
#endif // VICUS_EpdModuleDatasetH
