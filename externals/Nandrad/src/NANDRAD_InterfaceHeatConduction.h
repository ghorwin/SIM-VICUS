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

#ifndef NANDRAD_InterfaceHeatConductionH
#define NANDRAD_InterfaceHeatConductionH

#include <IBK_Parameter.h>
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Contains parameters for convenctive heat exchange between walls and
	zones/ambient climate.
*/
class InterfaceHeatConduction  {
	NANDRAD_READWRITE_PRIVATE
public:
	/*! Model types supported by this model. */
	enum modelType_t {
		/*! Constant transfer coefficient. */
		MT_Constant,				// Keyword: Constant		'Constant heat exchange coefficient'
		NUM_MT						// Keyword: None			'No convective heat exchange'
	};

	/*! Parameters. */
	enum para_t {
		/*! Constant heat transfer coefficient [W/m2K] */
		P_HeatTransferCoefficient,	// Keyword: HeatTransferCoefficient [W/m2K]		'Convective heat transfer coefficient'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(InterfaceHeatConduction)
	NANDRAD_COMP(InterfaceHeatConduction)

	/*! Checks for valid parameters (value ranges). */
	void checkParameters() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;					// XML:A:required
	/*! List of constant parameters. */
	IBK::Parameter						m_para[NUM_P];							// XML:E

}; // InterfaceHeatConduction

} // namespace NANDRAD

#endif // NANDRAD_InterfaceHeatConductionH
