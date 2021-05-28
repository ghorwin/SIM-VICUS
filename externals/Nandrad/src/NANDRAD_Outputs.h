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

#ifndef NANDRAD_OutputsH
#define NANDRAD_OutputsH

#include <string>
#include <vector>

#include <IBK_Unit.h>
#include <IBK_Flag.h>

#include "NANDRAD_OutputGrid.h"
#include "NANDRAD_OutputDefinition.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Stores vectors with OutputGrid and OutputDefinition data. */
class Outputs {
	NANDRAD_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(Outputs)
	NANDRAD_COMP(Outputs)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! List with output (file) definitions. */
	std::vector<OutputDefinition>				m_definitions;				// XML:E

	/*! List with output grids. */
	std::vector<OutputGrid>						m_grids;					// XML:E

	/*! (optional) The time unit to be used in all output files.
		If not set (undefined unit), the time unit is selected automatically
		based on the simulation duration.
	*/
	IBK::Unit									m_timeUnit;					// XML:E

	/*! (optional) If true, output files are written in binary format (the default, if flag is missing). */
	IBK::Flag									m_binaryFormat;				// XML:E

};


} // namespace NANDRAD

#endif // NANDRAD_OutputsH
