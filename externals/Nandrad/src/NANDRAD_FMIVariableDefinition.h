/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NANDRAD_FMIVariableDefinitionH
#define NANDRAD_FMIVariableDefinitionH

#include <string>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {


/*! Definition of an FMI input/output variable and the corresponding value reference info for NANDRAD.
*/
class FMIVariableDefinition {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! The variable name as it appears in the FMI model description. */
	std::string m_fmiVarName;																// XML:A:required
	/*! The variable variable type as it appears in the FMI model description. */
	std::string m_fmiTypeName;																// XML:E
	/*! The unqiue variable reference number for the FMI model description. */
	unsigned int m_fmiValueRef;																// XML:A:required

	/*! The reference type of the object providing/requesting variable (see ModelInputReference::referenceType_t). */
	std::string m_objectType;																// XML:E:required
	/*! The ID of the referenced object. */
	unsigned int m_objectID = NANDRAD::INVALID_ID;											// XML:E:required
	/*! The variable name for the variable reference in NANDRAD. */
	std::string m_varName;																	// XML:E:required
	unsigned int m_varID = NANDRAD::INVALID_ID;												// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_FMIVariableDefinitionH
