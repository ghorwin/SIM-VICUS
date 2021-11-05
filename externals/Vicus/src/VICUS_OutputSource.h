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

#ifndef VICUS_OutputSourceH
#define VICUS_OutputSourceH

#include "VICUS_CodeGenMacros.h"
#include "string"

namespace VICUS {

/*! Contains a vicus output definition.
	The VICUS output definition is very similar to the NANDRAD Outputs data structure, but
	contains some more properties needed for the user interface.

	Also, default object lists are being created.
*/
class OutputSource {
	VICUS_READWRITE_PRIVATE
public:


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_IFNOTEMPTY(OutputSource)
	VICUS_COMP(OutputSource)

	OutputSource();

	OutputSource(unsigned int id, std::string displayName);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Indicates whether source object is set active */
	bool						m_isActive = false;		// XML:A

	/*! ID of Source Object */
	unsigned int				m_id;					// XML:A

	/*! Display Name of source object */
	std::string					m_displayName;			// XML:A




};

} // namespace VICUS

#endif // VICUS_OutputSourceH
