/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
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
		/*! Mass Density of Gas Layer. */
		P_MassDensity,				// Keyword: MassDensity					[kg/m3]		'Mass density of the fill-in gas.'
		/*! Height of  detailed Window (needed for Convection in Cavity.  */
		P_Height,					// Keyword: MassDensity					[m]			'Height of the detailed window.'
		/*! Mass Density of Gas Layer. */
		P_Width,					// Keyword: MassDensity					[m]			'height of the detailed window.'
		/*! Emissivity of surface facing outside. */
		P_LongWaveEmissivityInside,	// Keyword: LongWaveEmissivityInside	[---]		'Emissivity of surface facing outside.'
		/*! Emissivity of surface facing inside. */
		P_LongWaveEmissivityOutside,// Keyword: P_LongWaveEmissivityOutside [---]		'Emissivity of surface facing inside.'

		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	type_t						m_type = NUM_T;						// XML:A:required

	// TODO : Stephan, add other properties for layers here

	/*! Unique ID-number for this window layer. */
	unsigned int				m_id;								// XML:A:required

	/*! Display name of layer. */
	std::string					m_displayName;						// XML:A

	/*! Basic parameters of the window layer  */
	IBK::Parameter				m_para[NUM_P];						// XML:E

	// Layer Data

	/*! Parameter for temperature-dependent conductivity of gas layers. */
	LinearSplineParameter		m_conductivity;						// XML:E

	/*! Parameter for temperature-dependent dynamic viscosity of gas layers. */
	LinearSplineParameter		m_dynamicViscosity;					// XML:E

	/*! Parameter for temperature-dependent conductivity of gas layers. */
	LinearSplineParameter		m_heatCapacity;						// XML:E

	// Short Wave for Glasing Layers

	/*! Short wave transmittance. */
	LinearSplineParameter		m_shortWaveTransmittance;			// XML:E

	/*! Short Wave Reflectance of surface facing inside. */
	LinearSplineParameter		m_shortWaveReflectanceInside;		// XML:E

	/*! Short Wave Reflectance of surface facing outside. */
	LinearSplineParameter		m_shortWaveReflectanceOutside;		// XML:E

	//LinearSplineParameter		m_test[10];


};

} // namespace NANDRAD

#endif // NANDRAD_WindowGlazingLayerH
