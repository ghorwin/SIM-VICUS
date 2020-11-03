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

namespace NANDRAD {

/*!	WindowGlazingSystem defines the glazing to be used in a window. */
class WindowGlazingSystem  {
public:

	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_Simple,						// Keyword: Simple			'Simple globbed-layers model.'
		NUM_MT
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;						// XML:A:required

}; // WindowGlazingSystem

} // namespace NANDRAD

#endif // NANDRAD_WindowGlazingSystemH
