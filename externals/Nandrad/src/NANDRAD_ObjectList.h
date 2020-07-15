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

#ifndef NANDRAD_ObjectListH
#define NANDRAD_ObjectListH

#include <string>
#include <map>
#include <vector>

#include "NANDRAD_IDGroup.h"
#include "NANDRAD_CodeGenMacros.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class ObjectList

	An ObjectList selects implicit or explicit models of one equal reference type (filter type).
	That may be zones, interfaces,	construction instances, embedded objects or explicit models.
	It uses an id group or a name list to filter the model instances that should be selected.

	The space type filter is only temporary. It selects models of type 'Zone' that are all
	of one (or more than one) space type.

	The object list name can be referenced from other structures.
	Use the object list name reference for the definition of outputs, model references and implicit model
	feedback:
	\code
	<ObjectList>objectListName<\ObjectList>
*/
class ObjectList {
public:

	/*! Default constructor. */
	ObjectList();

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	//void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	//void writeXML(TiXmlElement * parent) const;

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-Name of the object list. */
	std::string									m_name;						// XML:E

	/*! Encodes the referenced model IDs. */
	IDGroup										m_filterID;
};

} // namespace NANDRAD

#endif // NANDRAD_ObjectListH
