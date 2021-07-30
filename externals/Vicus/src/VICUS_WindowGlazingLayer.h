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

#ifndef VICUS_WindowGlazingLayerH
#define VICUS_WindowGlazingLayerH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <QString>
#include <vector>

#include <IBK_Parameter.h>

#include <NANDRAD_LinearSplineParameter.h>

namespace VICUS {

/*! Window glazing layer is a child of WindowGlazingSystem, but not a stand-alone
	database element.

	TODO Review and remove if not needed anylonger.

*/
class WindowGlazingLayer {
public:

	/*! Model types supported by the window model. */
	enum type_t {
		T_Gas,					// Keyword: Gas						'Gas layer'
		T_Glass,				// Keyword: Glass					'Glass layer'
		NUM_T
	};

	/*! Basic parameters. */
	enum para_t {
		/*! Dry density of the material. */
		P_Thickness,				// Keyword: Thickness					[m]			'Thickness of the window layer.'
		/*! Thermal conductivity of the dry material. */
		P_Conductivity,				// Keyword: Conductivity				[W/mK]		'Thermal conductivity of the window layer.'
		/*! Mass density of gas layer. */
		P_MassDensity,				// Keyword: MassDensity					[kg/m3]		'Mass density of the fill-in gas.'
		/*! Height of detailed window (needed for Convection in Cavity).  */
		P_Height,					// Keyword: Height						[m]			'Height of the detailed window.'
		/*! Width of detailed window (needed for Convection in Cavity).  */
		P_Width,					// Keyword: Width						[m]			'Width of the detailed window.'
		/*! Emissivity of surface facing outside. */
		P_LongWaveEmissivityInside,	// Keyword: LongWaveEmissivityInside	[---]		'Emissivity of surface facing outside.'
		/*! Emissivity of surface facing inside. */
		P_LongWaveEmissivityOutside,// Keyword: LongWaveEmissivityOutside [---]		'Emissivity of surface facing inside.'

		NUM_P
	};


	/*! Enum type with all possible layer spline parameters.*/
	enum splinePara_t {
		SP_ShortWaveTransmittance,		// Keyword: ShortWaveTransmittance		[---]		'Short wave transmittance at outside directed surface.'
		SP_ShortWaveReflectanceOutside,	// Keyword: ShortWaveReflectanceOutside	[---]		'Short wave reflectance of surface facing outside.'
		SP_ShortWaveReflectanceInside,	// Keyword: ShortWaveReflectanceInside	[---]		'Short wave reflectance of surface facing inside.'
		SP_Conductivity,				// Keyword: Conductivity				[W/mK]		'Thermal conductivity of the gas layer.'
		SP_DynamicViscosity,			// Keyword: DynamicViscosity			[kg/ms]		'Dynamic viscosity of the gas layer.'
		SP_HeatCapacity,				// Keyword: HeatCapacity				[J/kgK]		'Specific heat capacity of the gas layer.'
		NUM_SP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	/*! Comparison operator. */
	bool operator!=(const WindowGlazingLayer &other)const {
		if (m_type != other.m_type)
			return true;

		for(unsigned int i=0; i<NUM_P; ++i)
			if(m_para[i] != other.m_para[i])
				return true;

		for(unsigned int i=0; i<NUM_SP; ++i)
			if(m_splinePara[i] != other.m_splinePara[i])
				return true;

		return false;
	}

	/*! Comparison operator. */
	bool operator==(const WindowGlazingLayer &other) const { return !(*this != other);}

	bool isValid() const {
		///TODO Stephan implement valid function
		return true;
	}

	// *** PUBLIC MEMBER VARIABLES ***

	type_t							m_type = NUM_T;							// XML:A:required

	/*! Basic parameters of the window layer  */
	IBK::Parameter					m_para[NUM_P];							// XML:E

	// Layer Data in LinearSpline

	NANDRAD::LinearSplineParameter	m_splinePara[NUM_SP];					// XML:E

};

} // namespace VICUS


#endif // VICUS_WindowGlazingLayerH
