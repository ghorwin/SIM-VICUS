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

#include "NANDRAD_EmbeddedObject.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace NANDRAD {

EmbeddedObject::objectType_t EmbeddedObject::objectType() const {

#if 0
	if ( m_window.m_modelType != EmbeddedObjectWindow::NUM_MT )
		return OT_WINDOW;

	if ( m_door.m_modelType != EmbeddedObjectDoor::NUM_MT )
		return OT_DOOR;

	if ( m_hole.m_modelType != EmbeddedObjectHole::NUM_MT )
		return OT_HOLE;

#define MISSING_OT_GIVES_WARNING
#ifdef MISSING_OT_GIVES_WARNING
	IBK::IBK_Message(
		IBK::FormatString("Missing model parametrization for Embedded Object with id %1.").arg(m_id), IBK::MSG_WARNING, "[EmbeddedObject::objectType]");
	return NUM_OT;
#else // MISSING_OT_GIVES_WARNING
	throw IBK::Exception(
		IBK::FormatString("Missing model parametrization for Embedded Object with id %1. Please check your embedded objects in project file.").arg(m_id),
				"[EmbeddedObject::objectType]"
				);
#endif

#endif
	return EmbeddedObject::NUM_OT;
}


} // namespace NANDRAD

