/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_QuantityH
#define IBK_QuantityH

#include <string>
#include <vector>

#include "IBK_Unit.h"
#include "IBK_Color.h"

namespace IBK {

/*!	The class Quantity describes a quantity, uniquely identified by its ID name and type.
*/
class Quantity {
public:
	/*! Different types of quantities, recognized by this class. */
	enum type_t {
		STATE,               // Keyword: STATE       'State variable or related quantity'
		FLUX,                // Keyword: FLUX        'Flux between elements'
		BCFLUX,              // Keyword: BCFLUX      'Boundary flux'
		SOURCE,              // Keyword: SOURCE      'Element-based source/sink'
		INTERFACE,           // Keyword: INTERFACE   'Loads on a specific interface (boundary flux inputs)'
		LOAD,                // Keyword: LOAD        'Values of time-value data tables (e.g. climatic data)'
		NUM_TYPES
	};

	/*! Default constructor, creates an undefined quantity. */
	Quantity() : m_type(NUM_TYPES) {}
	/*! Initializing constructor. */
	Quantity(type_t t, const std::string & name);
	/*! Initializing constructor. */
	Quantity(Quantity::type_t t, const std::string & name, const std::string & desc, const IBK::Unit & u, const IBK::Color & color = IBK::Color());

	/*! Resets the member variables. */
	void clear();
	/*! Reads quantity from string (single-line format).
		\code
		// line to parse has format
		STATE Temperature [C]   'Temperature'  {#ff0000}
		// where [C] is the default IO Unit, and {#ff0000} is the hex color code
		\endcode
	*/
	void read(const std::string & line);
	/*! Writes quantity to string (single-line format) and ends line with line break. */
	void write(std::ostream & out, int indent);

	/*! The unique identification string. */
	std::string			m_name;
	/*! The kind of quantity, one from type_t. */
	type_t				m_type;

	/*! Description of the quantity. */
	std::string			m_description;
	/*! Default IO Unit. */
	IBK::Unit			m_unit;
	/*! Color for representation in UI. */
	IBK::Color			m_color;

	/*! Converts enumeration type to corresponding string. */
	static const char * type2string(type_t t);
	/*! Converts type identification string to corresponding enumeration type. */
	static type_t string2type(const std::string & typestr);
};

/*! Compares IBK::Quantities by type and name. */
inline bool operator==(const IBK::Quantity & left, const IBK::Quantity & right) {
	return left.m_type == right.m_type && left.m_name == right.m_name;
}

/*! Sort operator, sorts first by name and then by type. */
inline bool operator<(const IBK::Quantity & left, const IBK::Quantity & right) {
	if (left.m_name < right.m_name)
		return true;
	if (left.m_name > right.m_name)
		return false;
	return left.m_type < right.m_type;
}

/*! Compares IBK::Quantities name and another name. */
inline bool operator==(const IBK::Quantity & left, const std::string & name) {
	return left.m_name == name;
}


} // namespace IBK

/*! \file IBK_Quantity.h
	\brief Contains the declaration of class Quantity.
*/

#endif // IBK_QuantityH
