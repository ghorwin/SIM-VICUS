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

#ifndef NANDRAD_InterfaceSolarAbsorptionH
#define NANDRAD_InterfaceSolarAbsorptionH

#include <IBK_Parameter.h>
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	\brief Declaration for class InterfaceSolarAbsorption

	An embedded object generally defines a wall opening (a window or a door).
	That means, the calculation radiant heat fluxes and heat fluxes by heat transmission are performed
	by a window model or door model. The embedded oject	stores an exchangable parameter model
	that defines the name for the calculation model and constant model parameters.
*/
class InterfaceSolarAbsorption {
	NANDRAD_READWRITE_PRIVATE
public:

	/*! Parameters to be defined for the various window model types. */
	enum para_t {
		P_AbsorptionCoefficient,	// Keyword: AbsorptionCoefficient [---]			'Constant Absorption coefficient [0,...,1].'
		NUM_P
	};
	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_CONSTANT,				// Keyword: Constant		'Constant model.'
		NUM_MT
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(InterfaceSolarAbsorption)
	NANDRAD_COMP(InterfaceSolarAbsorption)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;					// XML:A:required
	/*! List of constant parameters.*/
	IBK::Parameter						m_para[NUM_P];							// XML:E

}; // InterfaceSolarAbsorption

} // namespace NANDRAD

#endif // NANDRAD_InterfaceSolarAbsorptionH
