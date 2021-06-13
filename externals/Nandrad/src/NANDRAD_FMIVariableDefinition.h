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

	/*! Checks variable names. */
	void checkParameters();

	// *** PUBLIC MEMBER VARIABLES ***

	bool operator<(const FMIVariableDefinition & other) const {
		// Note: compare properties starting with fast comparisons (integers) before strings.
		if (m_objectId < other.m_objectId) return true;
		if (m_objectId > other.m_objectId) return false;

		if (m_vectorIndex < other.m_vectorIndex) return true;
		if (m_vectorIndex > other.m_vectorIndex) return false;

		if (m_fmiValueRef < other.m_fmiValueRef) return true;
		if (m_fmiValueRef > other.m_fmiValueRef) return false;

		if (m_varName < other.m_varName) return true;
		if (m_varName > other.m_varName) return false;

		if (m_fmiVarName < other.m_fmiVarName) return true;
		if (m_fmiVarName > other.m_fmiVarName) return false;

		return m_fmiTypeName < other.m_fmiTypeName;
	}


	/*! Compares NANDRAD model variable definitions (not FMI variables). */
	bool sameModelVarAs(const FMIVariableDefinition & other) const {
		return m_objectId == other.m_objectId &&
				m_vectorIndex == other.m_vectorIndex &&
				m_varName == other.m_varName;
	}

	/*! The variable name as it appears in the FMI model description. */
	std::string m_fmiVarName;																// XML:A:required
	/*! Optional description of the variable. */
	std::string m_fmiVarDescription;														// XML:E
	/*! Unit of the variable. */
	std::string	m_unit;																		// XML:A:required
	/*! The variable variable type as it appears in the FMI model description. */
	std::string m_fmiTypeName;																// XML:E
	/*! The unqiue variable reference number for the FMI model description. */
	IDType m_fmiValueRef = NANDRAD::INVALID_ID;												// XML:A:required
	/*! The start value to be used for this variable. */
	double m_fmiStartValue;																	// XML:E:required

	/*! The variable name for the variable reference in NANDRAD.
		This is an encoded name in format <objectRefType>.<variableName>, for example
		'Zone.AirTemperature'.
	*/
	std::string m_varName;																	// XML:E:required
	/*! The ID of the referenced object. */
	IDType m_objectId = NANDRAD::INVALID_ID;												// XML:E:required
	/*! Vector ID/Index for vector valued quantities. */
	IDType m_vectorIndex = NANDRAD::INVALID_ID;												// XML:E
};


} // namespace NANDRAD

#endif // NANDRAD_FMIVariableDefinitionH
