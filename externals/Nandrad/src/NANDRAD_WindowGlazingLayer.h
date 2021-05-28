/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#ifndef NANDRAD_WindowGlazingLayerH
#define NANDRAD_WindowGlazingLayerH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_LinearSplineParameter.h"

namespace NANDRAD {

/*!	WindowGlazingLayer defines a layer of a glazing system (air or glass). */
class WindowGlazingLayer  {
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
		P_LongWaveEmissivityOutside,// Keyword: P_LongWaveEmissivityOutside [---]		'Emissivity of surface facing inside.'

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

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	type_t						m_type = NUM_T;						// XML:A:required

	/*! Unique ID-number for this window layer. */
	unsigned int				m_id = INVALID_ID;					// XML:A:required

	/*! Display name of layer. */
	std::string					m_displayName;						// XML:A

	/*! Basic parameters of the window layer  */
	IBK::Parameter				m_para[NUM_P];						// XML:E

	// Layer Data in LinearSpline

	LinearSplineParameter		m_splinePara[NUM_SP];				// XML:E


};

} // namespace NANDRAD

#endif // NANDRAD_WindowGlazingLayerH
