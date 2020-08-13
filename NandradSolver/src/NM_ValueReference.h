/*	NANDRAD Solver Framework and Model Implementation.

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

#ifndef NM_ValueReferenceH
#define NM_ValueReferenceH

#include <string>

#include <NANDRAD_ModelInputReference.h>
#include "NM_QuantityDescription.h"

namespace NANDRAD_MODEL {


/*! Data type that uniquely identifies a (result) variable.

	Reference type (m_referenceType) and id (m_id) identify the object that generates the data.
	Variable name (m_name) identifies the name of the variable to retrieve (may be a vector-valued variable name, without
	index of course).

	This data structure is used to determine the object a variable is requested from. The results
	from all objects/model instances are stored in a map (from variable reference to object), hence
	the operator< in this class. It is actually an extended QuantityDescription, but with additional comparison
	operator, that allows its efficient use in a map.

	When looking up requested input variables from models, we simply search for this value reference, and hereby determine
	object and id and eventually get the object instance that provides the variable we need. Then, we can ask
	the object instance to give us a persistent pointer to the result variable's memory location.
*/
class ValueReference : public QuantityDescription {
public:
	/*! ReferenceType: zone, model or constructionInstance */
	NANDRAD::ModelInputReference::referenceType_t	m_referenceType;
	/*! ID of the referenced source model/parametrization entity.*/
	unsigned int									m_id;

	/*! Comparison operator for sorting result quantities. */
	bool operator<(const ValueReference &other) const {
		if (m_referenceType < other.m_referenceType)
			return true;
		if (m_referenceType > other.m_referenceType)
			return false;
		if (m_id < other.m_id)
			return true;
		if (m_id > other.m_id)
			return false;

		// string comparison last, because slowest
		return (m_name < other.m_name);
	}

};

} // namespace NANDRAD_MODEL

#endif // NM_ValueReferenceH
