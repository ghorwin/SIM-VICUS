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

#ifndef NANDRAD_ObjectListH
#define NANDRAD_ObjectListH

#include <string>

#include "NANDRAD_ModelInputReference.h"
#include "NANDRAD_IDGroup.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	An ObjectList is used to select model instances based on reference type and ids.
	That may be zones, interfaces,	construction instances, embedded objects etc.
	It uses an id group to filter the model instances that should be selected.

	The object list name can be referenced from other structures.
	Use the object list name reference for the definition of outputs and model references.
*/
class ObjectList {
	NANDRAD_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_NAME

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-Name of the object list. */
	std::string									m_name;						// XML:A:required

	/*! The type of model that we reference data from. */
	ModelInputReference::referenceType_t		m_referenceType;

	/*! Encodes the referenced model IDs. */
	IDGroup										m_filterID;
};

} // namespace NANDRAD

#endif // NANDRAD_ObjectListH
