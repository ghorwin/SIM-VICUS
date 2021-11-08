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

#ifndef VICUS_OutputDefinitionH
#define VICUS_OutputDefinitionH

#include <NANDRAD_Outputs.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

	struct SourceObject {

		SourceObject(){}

		SourceObject(unsigned int id, std::string displayName):
			m_id(id),
			m_displayName(displayName)
		{}

		/*! Indicates whether source object is set active */
		bool						m_isActive = false;

		/*! ID of Source Object */
		unsigned int				m_id;
		/*! Display Name of source object */
		std::string					m_displayName;

	};

/*! Defines an output definition.
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
class OutputDefinition {
	VICUS_READWRITE_PRIVATE
public:

	/*! Different options to handle time averaging/integration. */
	enum timeType_t {
		/*! Write outputs as calculated at output time points. */
		OTT_NONE,		// Keyword: None			'Write values as calculated at output times.'
		/*! Average value in last output interval. */
		OTT_MEAN,		// Keyword: Mean			'Average values in time (mean value in output step).'
		/*! Time integral of output value. */
		OTT_INTEGRAL,	// Keyword: Integral		'Integrate values in time.'
		NUM_OTT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_IFNOTEMPTY(OutputDefinition)
	VICUS_COMP(OutputDefinition)


	unsigned int									m_id = INVALID_ID;				// XML:A

	/*! Name of output */
	std::string										m_name;							// XML:A

	/*! Type of output */
	std::string										m_type;							// XML:A

	/*! Unit of output definition */
	IBK::Unit										m_unit;							// XML:E

	/*! Description of output definition */
	std::string										m_description;

	/*! Time Type of output definition */
	timeType_t										m_timeType;						// XML:E

	/*! Vector of all Vector indexes/ids */
	std::vector<unsigned int>						m_vectorIds;					// XML:E

	/*! Vector of all Vector Source object id(s) */
	std::vector<unsigned int>						m_sourceObjectIds;				// XML:E

	/*! Vector of all active Vector Source object id(s) */
	std::vector<unsigned int>						m_activeSourceObjectIds;		// XML:E

	/*! Map that holds all data to source objects
		key is the id of the sourceObject;
	*/
	std::map<unsigned int, SourceObject>			m_idToSourceObject;

	// ===================== ONLY NEEDED IN WIDGET ===================================

	/*! Pointer to output grid */
	NANDRAD::OutputGrid						*m_outputGrid = nullptr;
};

} // namespace VICUS

#endif // VICUS_OutputDefinitionH
