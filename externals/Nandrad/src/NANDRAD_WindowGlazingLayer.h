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
		T_Air,					// Keyword: Air						'Air layer'
		T_Glass,				// Keyword: Glass					'Glass layer'
		NUM_T
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	type_t						m_type = NUM_T;						// XML:A

	// TODO : Stephan, add other properties for layers here

};

} // namespace NANDRAD

#endif // NANDRAD_WindowGlazingLayerH
