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

#ifndef NANDRAD_FMIDescriptionH
#define NANDRAD_FMIDescriptionH

#include <vector>

#include <IBK_Path.h>

#include "NANDRAD_FMIVariableDefinition.h"

namespace NANDRAD {


/*! Contains all data to generate a modelDescription.xml and configure the NANDRAD FMI Slave.

*/
class FMIDescription {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	/*! Tests if a variable for this model quantity exists already in the variable list.
		Only the NANDRAD model variable properties (name, objectID, vector index/id) are compared.
	*/
	bool hasVariable(const FMIVariableDefinition & var) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! The FMI model name. */
	std::string							m_modelName;							// XML:E

	/*! Path to target directory, where FMU file shall be exported to.
		FMU file path will be automatically generated from model name and FMU path.
		This is stored so that it is not necessary to specify the FMU path again and again when
		exporting new project variants.
	*/
	IBK::Path							m_FMUPath;								// XML:E

	/*! Holds all variable definitions. */
	std::vector<FMIVariableDefinition>	m_variables;							// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_FMIDescriptionH
