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

#ifndef NANDRAD_ProjectInfoH
#define NANDRAD_ProjectInfoH

#include <string>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Contains meta-information about the project. */
class ProjectInfo  {
public:

	NANDRAD_READWRITE
	NANDRAD_COMP(ProjectInfo)

	/*! Comments about the project. */
	std::string							m_comment;				// XML:E
	/*! Time stamp when the project file was created. */
	std::string							m_created;				// XML:E
	/*! Time stamp, when the project was last modified. */
	std::string							m_lastEdited;			// XML:E
};

inline bool ProjectInfo::operator!=(const ProjectInfo & other) const {
	if (m_comment != other.m_comment) return true;
	if (m_created != other.m_created) return true;
	if (m_lastEdited != other.m_lastEdited) return true;
	return false;
}

} // namespace NANDRAD

#endif // NANDRAD_ProjectInfoH
