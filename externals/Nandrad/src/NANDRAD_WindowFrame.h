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

#ifndef NANDRAD_WindowFrameH
#define NANDRAD_WindowFrameH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*!	WindowFrame defines the frame of a window.

	\todo Extend this class to hold alternative ways for defining a frame.
*/
class WindowFrame  {
	NANDRAD_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(WindowFrame)
	NANDRAD_COMP(WindowFrame)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Material used for frame (INVALID_ID for no frame). */
	unsigned int	m_materialID = INVALID_ID;						// XML:E:required
	/*! Cross section occupied by all frame elements in this window (required when m_materialID is given). */
	IBK::Parameter	m_area;											// XML:E:required

}; // WindowFrame


inline bool WindowFrame::operator!=(const WindowFrame & other) const {
	if (m_materialID != other.m_materialID) return true;
	if (m_area != other.m_area) return true;
	return false;
}

} // namespace NANDRAD

#endif // NANDRAD_WindowFrameH
