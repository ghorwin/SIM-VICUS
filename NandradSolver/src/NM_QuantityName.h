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
	QuantityName(const char *name, int index) :
		m_name(name), m_index(index) { }
	QuantityName(const char *name, unsigned int index) :
		m_name(name), m_index((int)index) { }
	QuantityName(const std::string &name, unsigned int index) :
		m_name(name), m_index((int)index) { }
	QuantityName(const std::string &name) : 
		m_name(name), m_index(-1) { }
	QuantityName(const char *name): 
		m_name(name), m_index(-1) { }
	// copy constructor
	QuantityName(const QuantityName &name) { *this = name; }

	// access to name
	const std::string &name() const { return m_name; }
	// access to index
	int index() const { return m_index; }

	// extract from a given quantity
	void fromEncodedString(const std::string &quantity);
	// clear name and index
	void clear();
	// flag indicating an uninialized object
	bool empty();

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const QuantityName & other) const;
	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const QuantityName & other) const { return !operator!=(other); }

private:
	std::string		m_name;
	int				m_index;
};

} // namespace NANDRAD_MODEL

#endif // NM_QuantityNameH
