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

#ifndef NANDRAD_WindowGlazingSystemH
#define NANDRAD_WindowGlazingSystemH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_WindowGlazingLayer.h"

namespace NANDRAD {

/*!	WindowGlazingSystem defines the glazing to be used in a window. */
class WindowGlazingSystem  {
public:

	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_Simple,						// Keyword: Simple								'Standard globbed-layers model.'
		MT_Detailed,					// Keyword: Detailed							'Detailed window model with layers.'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_ThermalTransmittance,			// Keyword: ThermalTransmittance		[W/m2K]		'Thermal transmittance'
		NUM_P
	};

	/*! Enum type with all possible glazing system spline parameters.*/
	enum splinePara_t {
		SP_SHGC,		// Keyword: SHGC		[---]		'Short wave transmittance at outside directed surface.'
		NUM_SP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number for this ventilation rate model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;					// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string							m_displayName;								// XML:A
	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;						// XML:A:required

	/*! Model parameters. */
	IBK::Parameter						m_para[NUM_P];								// XML:E

	/*! Parameter for angle-dependent SHGC value. */
	LinearSplineParameter				m_splinePara[NUM_SP];						// XML:E

	/*! Parameters for the detailed multi-layer model. */
	std::vector<WindowGlazingLayer>		m_layers;									// XML:E

}; // WindowGlazingSystem

} // namespace NANDRAD

#endif // NANDRAD_WindowGlazingSystemH
