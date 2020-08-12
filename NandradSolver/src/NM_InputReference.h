/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
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

#ifndef NM_InputReferenceH
#define NM_InputReferenceH

#include <string>
#include <IBK_Unit.h>

#include "NM_QuantityName.h"

#include <NANDRAD_ModelInputReference.h>

namespace NANDRAD_MODEL {

/*! This data type is used to formulate an input variable requirement from point of view of a model object.
	It contains the information referenceType and id to address an object, and a quantity name (which basically
	is the variable name with optional vector index/id information) to lookup the variable.

	Further, it expresses whether this variables is to be treated as constant, and whether it is required by the model,
	or can be missing.
*/
class InputReference {
public:
	/*! Comparison operator. */
	bool operator==(const InputReference & other) const {
		if (m_referenceType != other.m_referenceType) return false;
		if (m_id != other.m_id) return false;
		if (m_name != other.m_name) return false;
		// note: required is additional information, not needed for comparison (i.e. input references are considered
		//       the same, if refType, id and name match.
		return true;
	}

	/*! Inequality operator. */
	bool operator!=(const InputReference & other) const {
		return !operator==(other);
	}


	/*! ReferenceType: zone, construction instance, interface, etc. */
	NANDRAD::ModelInputReference::referenceType_t	m_referenceType = NANDRAD::ModelInputReference::NUM_MRT;
	/*! ID of the referenced source model/parametrization entity.*/
	unsigned int									m_id = 0;
	/*! Name of the result quantity inside the referenced model.*/
	QuantityName									m_name;
	/*! For input references indicates, that the model requires this input and cannot work (correctly) without.
		For loads/fluxes, a missing input may be treated as 0 flux, and hence required may be false for
		such quantities.
	*/
	bool											m_required = true;
};

} // namespace NANDRAD_MODEL

#endif // NM_InputReferenceH
