#ifndef VICUS_OUTPUTS_H
#define VICUS_OUTPUTS_H

#include "VICUS_CodeGenMacros.h"
#include <NANDRAD_Outputs.h>

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
*/
class Outputs {
	VICUS_READWRITE_PRIVATE
public:

	/*! Flags. */
	enum flag_t {
		OF_BinaryOutputs,				// Keyword: BinaryOutputs		'If true, output files are written in binary format (the default, if flag is missing).'
		OF_CreateDefaultZoneOutputs,	// Keyword: CreateDefaultZoneOutputs	'If true, default output definitions for zones are created.'
		NUM_OF
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_IFNOTEMPTY(Outputs)
	VICUS_COMP(Outputs)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! List with output (file) definitions. */
	std::vector<NANDRAD::OutputDefinition>				m_definitions;				// XML:E

	/*! List with output grids. */
	std::vector<NANDRAD::OutputGrid>					m_grids;					// XML:E

	/*! (optional) The time unit to be used in all output files.
		If not set (undefined unit), the time unit is selected automatically
		based on the simulation duration.
	*/
	IBK::Unit											m_timeUnit;					// XML:E

	/*! (optional) If true, output files are written in binary format (the default, if flag is missing). */
	IBK::Flag											m_flags[NUM_OF];			// XML:E
};

} // namespace VICUS

#endif // VICUS_OUTPUTS_H
