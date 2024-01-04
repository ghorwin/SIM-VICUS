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

#ifndef VICUS_OutputsH
#define VICUS_OutputsH

#include "VICUS_CodeGenMacros.h"
#include <NANDRAD_Outputs.h>

#include <VICUS_OutputDefinition.h>

namespace VICUS {

/*! Contains output definitions/specifications.
	The VICUS output definition is very similar to the NANDRAD Outputs data structure, but
	contains some more properties needed for the user interface.

	If CreateDefaultZoneOutputs is true, default output definitions for zones are created.
		These are:
		- zone air temperatures for all zones
		- wall surface temperatures for all constructions
		- wall heat flux towards zones
		- wall heat flux towards ambient
		- total heat conduction load in zone

	Also, default object lists are being created.

	Similarly, the flag "CreateDefaultNetworkOutputs" defines, whether default outputs for network objects are being created.

	Additionally, custom output definitions can be specified.
*/
class Outputs {
	VICUS_READWRITE_PRIVATE
public:

	/*! Flags. */
	enum flag_t {
		F_BinaryFormat,				// Keyword: BinaryFormat					'If true, output files are written in binary format (the default, if flag is missing).'
		F_CreateDefaultZoneOutputs,		// Keyword: CreateDefaultZoneOutputs	'If true, default output definitions for zones are created.'
		F_CreateDefaultNetworkOutputs,	// Keyword: CreateDefaultNetworkOutputs	'If true, default output definitions for networks are created.'
		F_CreateDefaultNetworkSummationModels,	// Keyword: CreateDefaultNetworkSummationModels	'If true, default summation models and according output definitions for networks are created.'
		NUM_F
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_IFNOTEMPTY(Outputs)
	VICUS_COMP(Outputs)

	void initDefaults();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! List with user-defined output definitions. */
	std::vector<VICUS::OutputDefinition>				m_definitions;				// XML:E

	/*! List with output grids. */
	std::vector<NANDRAD::OutputGrid>					m_grids;					// XML:E

	/*! Hash code (MD5) of output variables file 'output_reference_list.txt' */
	std::string											m_checkSum;					// XML:A

	/*! (optional) The time unit to be used in all output files.
		If not set (undefined unit), the time unit is selected automatically
		based on the simulation duration.
	*/
	IBK::Unit											m_timeUnit;					// XML:E

	/*! (optional) If true, output files are written in binary format (the default, if flag is missing). */
	IBK::Flag											m_flags[NUM_F];				// XML:E
};

} // namespace VICUS

#endif // VICUS_OutputsH
