/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
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
	std::vector<OutputDefinition>				m_outputDefinitions;		// XML:E

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

#endif // OutputsH
