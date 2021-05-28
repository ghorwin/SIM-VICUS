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

	/*! Checks all input and output variables. */
	void checkParameters();

	/*! Tests if an input variable for this model quantity exists already in the variable list.
		Only the NANDRAD model variable properties (name, objectID, vector index/id) are compared.
	*/
	bool hasInputVariable(const FMIVariableDefinition & var) const;

	/*! Tests if an output variable for this model quantity exists already in the variable list.
		Only the NANDRAD model variable properties (name, objectID, vector index/id) are compared.
	*/
	bool hasOutputVariable(const FMIVariableDefinition & var) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! The FMI model name. */
	std::string							m_modelName;							// XML:E

	/*! Holds all input variable definitions.
		Note: There may be several input variables with same valueRef and name, yet different
			  NANDRAD variable names. In this case, only one FMI input variable is mapped to
			  all NANDRAD variables.
	*/
	std::vector<FMIVariableDefinition>	m_inputVariables;							// XML:E
	/*! Holds all output variable definitions. */
	std::vector<FMIVariableDefinition>	m_outputVariables;							// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_FMIDescriptionH
