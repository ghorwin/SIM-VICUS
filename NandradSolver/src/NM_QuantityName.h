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

#ifndef NM_QuantityNameH
#define NM_QuantityNameH

#include <limits>
#include <string>
#include <vector>

#include <IBK_Unit.h>

#include <NANDRAD_CodeGenMacros.h>
#include "NM_VectorValuedQuantityIndex.h"

namespace NANDRAD_MODEL {

/*! Datatype including name and index convention.
	It is used for description of InputReferences and ImplicitModelFeedback
	quantities (target and source).
*/
class QuantityName {
public:
	QuantityName() : m_index(-1) { }
	QuantityName(const std::string &name, int index) :
		m_name(name), m_index(index) { }
	QuantityName(const std::string &name) :
		m_name(name), m_index(-1) { }

	/*! Extract name and possibly index from a given quantity.
		Possible variants are "Temperature", "Temperature[index=5]" and "Temperature[id=4]"
		Throws an IBK::Exception in case of invalid formats.
	*/
	void fromEncodedString(const std::string &quantity);
	/*! Clears name and sets index to -1 (uninitialized/empty). */
	void clear();
	/*! Flag indicating an uninialized object (name is empty). */
	bool empty();

	// declare comparison operators
	NANDRAD_COMP(QuantityName)

	/*! Quantity identification name without [xx] */
	std::string		m_name;
	/*! Index, either -1 for scalar quantity or containing index extracted from qualified name. */
	int				m_index;
};

} // namespace NANDRAD_MODEL

#endif // NM_QuantityNameH
