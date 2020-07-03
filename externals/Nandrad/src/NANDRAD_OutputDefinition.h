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

#ifndef NANDRAD_OutputDefinitionH
#define NANDRAD_OutputDefinitionH

#include <string>
#include <map>

#include "NANDRAD_IDGroup.h"
//#include "NANDRAD_ModelInputReference.h"
#include "NANDRAD_ObjectList.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class OutputDefinition

	The output definition class selects quantities that are written in one
	output file (in the case of a scalar quantity). They are specified
	by a unique variable name and an object list with models containing
	the requested quantity.
	In the case of a vector valued quantity two definitions are valid:

	1.) The complete vector valued quantity, use the name of the quantity
	quantity = name
	For the variable of each model a single output file will be
	constructed.

	2.) A single vector value:
	quantity = name[<idstring>]
	The quantity will be treated like a scalar quantity.

	The encoding of the ID string is documented for the class IDGroup.
*/

class OutputDefinition {
public:
	// ***KEYWORDLIST-START***
	enum fileType_t {
		FT_DATAIO,                  // Keyword: DataIO						'Write DataIO output format (*.d6o or *.d6b)'
		FT_REPORT                   // Keyword: Report						'Write report output format (*.csv)'
	};

	enum timeType_t {
		OTT_NONE,                  // Keyword: None							'Write values as calculated at output times'
		OTT_MEAN,                  // Keyword: Mean							'Average values in time (mean value in output step)'
		OTT_INTEGRAL,              // Keyword: Integral						'Integrate values in time'
		NUM_OTT
	};

	// ***KEYWORDLIST-END***

	/*! Default constructor. */
	OutputDefinition();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	/*! Comparison operator by value. */
	bool operator==(const OutputDefinition & other) const { return !operator!=(other); }

	/*! Not-equal comparison operator by value. */
	bool operator!=(const OutputDefinition & other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Output file type: DataIO per default. */
	fileType_t								m_fileType;

	/*! Time output type. */
	timeType_t								m_timeType;

	/*! Name of the object list. */
	std::string								m_objectListName;

	/*! Time unit. Only supported for type 'Report'*/
	std::string								m_timeUnit;

	/*! Optional quantity name: needed for file name generation. */
	std::string								m_quantityName;

	/*! Quantity string for model output (may include more than one quantities for report output). */
	std::string								m_quantity;

	/*! Rerefence name of output grid, corresponds to OutputGrid::m_name. */
	std::string								m_gridName;

	/*! Pointer to object list, assigned during project initialization within solver for fast access. */
	const ObjectList						*m_objectListRef;
};

} // namespace NANDRAD

#endif // ModelH
