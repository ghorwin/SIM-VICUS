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

#include <string>
#include <vector>

#include <NANDRAD_IDVectorMap.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"


namespace VICUS {

/*! Defines an output definition.
	The VICUS output definition is very similar to the NANDRAD::OutputDefinition data structure, but
	contains some more properties needed for the user interface.

	In NANDRAD, the output definition contains a variable name that encodes also the index in case
	of vector-valued results. The source object is identified via flexibly defined object lists.

	The object lists are not available in the VICUS user interface - currently, there is no support for
	object lists defined by the user. Instead, object lists are created on-the-fly when needed.

	Hence, we need to store information about the source object separetely, so that during the NANDRAD project
	export we can create object lists for similarly referenced output source objects.

	Example:
	- we define a zonal output "AirTemperature"
	- the user selects zones 1,5 and 10 as object sources

	The data for output definitions matches that from the output_reference_list.txt generated during NANDRAD solver init.

	Note: OutputDefinitions are globbed during NANDRAD project export into groups sorted by used grid, time type,
		  source object type and quantity, since these define a unique NANDRAD::OutputDefinition.
		  Within the VICUS project, such definitions should be stored uniquely, yet, it does not harm if there
		  are duplicates, since these will be merged during the grouping process.
*/
class OutputDefinition {
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

	VICUS_READWRITE
	VICUS_COMP(OutputDefinition)

	/*! Name of result variable, for example 'AirTemperature'.
		\note While NANDRAD::OutputDefinition stores IDs of vector-valued quantities in [], for example
			  'ThermalLoad[2]', VICUS::OutputDefinition holds only the quantity name itself in member m_quantity,
			  and the vector IDs separately in member m_vectorIds.
	*/
	std::string										m_quantity;						// XML:A:required

	/*! Time type of output definition. If missing, defaults to OTT_NONE. */
	timeType_t										m_timeType = NUM_OTT;			// XML:E

	/*! Type of the source object (this is already a NANDRAD reference type and does not match VICUS object types). */
	std::string										m_sourceObjectType;				// XML:A:required

	/*! Vector of all vector source object id(s). If this vector has more than one ID, corresponding vectorIds will be stored in m_vectorIdMap. */
	std::vector<unsigned int>						m_sourceObjectIds;				// XML:E:required

	/*! Stores the vector ids for each source object id. */
	NANDRAD::IDVectorMap<unsigned int>				m_vectorIdMap;					// XML:E

	/*! Rerefence name of output grid (corresponds to OutputGrid::m_name). */
	std::string										m_gridName;						// XML:E:required
};

} // namespace VICUS

#endif // VICUS_OutputDefinitionH
