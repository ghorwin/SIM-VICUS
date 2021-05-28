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

#ifndef NANDRAD_OutputDefinitionH
#define NANDRAD_OutputDefinitionH

#include <string>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

class ObjectList;
class OutputGrid;

/*!	The output definition class selects quantities to be logged. They are specified
	by a unique variable name and an object list with models containing
	the requested quantity.
	In the case of a vector-valued quantity the id or index must be given in brackets.
	\code
	quantity = name[<id or index>]
	\endcode
	The extracted quantity will be treated like a scalar quantity.
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

	NANDRAD_READWRITE
	NANDRAD_COMP(OutputDefinition)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Filename (if empty then filename is generated automatically). */
	std::string								m_fileName;					// XML:E

	/*! Quantity id (for example,  "AirTemperature"). */
	std::string								m_quantity;					// XML:E:required

	/*! Time output type (defaults to OTT_NONE). */
	timeType_t								m_timeType = NUM_OTT;		// XML:E

	/*! Name of the object list. */
	std::string								m_objectListName;			// XML:E:required

	/*! Rerefence name of output grid (corresponds to OutputGrid::m_name). */
	std::string								m_gridName;					// XML:E:required


	// *** Variables used only during simulation ***

	/*! Pointer to output grid, assigned during project initialization within solver for fast access. */
	const OutputGrid						*m_gridRef = nullptr;

	/*! Pointer to object list, assigned during project initialization within solver for fast access. */
	const ObjectList						*m_objectListRef = nullptr;
};

} // namespace NANDRAD

#endif // NANDRAD_OutputDefinitionH
