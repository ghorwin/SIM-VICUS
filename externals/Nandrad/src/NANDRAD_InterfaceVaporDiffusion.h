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

#ifndef NANDRAD_InterfaceVaporDiffusionH
#define NANDRAD_InterfaceVaporDiffusionH

#include <IBK_Parameter.h>
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Contains parameters for vapor diffusion flux calculation. */
class InterfaceVaporDiffusion {
	NANDRAD_READWRITE_PRIVATE
public:

	/*! Parameters to be defined for the various window model types. */
	enum para_t {
		P_VaporTransferCoefficient,		// Keyword: VaporTransferCoefficient		[s/m]	'Vapor Transfer Coefficient.'
		NUM_P
	};
	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_Constant,					// Keyword: Constant		'Constant model.'
		NUM_MT
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(InterfaceVaporDiffusion)
	NANDRAD_COMP(InterfaceVaporDiffusion)

	/*! Checks for valid parameters (value ranges). */
	void checkParameters() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;					// XML:A:required
	/*! List of constant parameters.*/
	IBK::Parameter						m_para[NUM_P];							// XML:E

}; // InterfaceVaporDiffusion

} // namespace NANDRAD

#endif // NANDRAD_InterfaceVaporDiffusionH
