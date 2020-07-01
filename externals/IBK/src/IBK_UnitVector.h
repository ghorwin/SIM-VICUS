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

#ifndef IBK_UnitVectorH
#define IBK_UnitVectorH

#include "IBK_configuration.h"

#include <iosfwd>
#include <string>
#include <vector>
#include "IBK_Unit.h"

namespace IBK {

/*! A data vector with associated unit.
	The IBK::UnitVector encapsulates a std::vector with doubles whose elements
	are physical values with an unit. In addition to that the vector
	contains the input/output unit of the vector. The data inside the
	vector is always in the base SI unit.

	UnitVector is very fast for converting units in a container.
	\code
	// suppose you want to convert values in a std::vector
	std::vector<double> dataVec; // holds the data values

	// ... get some data into dataVec

	// convert from a given source unit to target unit
	IBK::UnitVector uvec;		// create empty unit vector
	uvec.m_unit = srcUnit;		// set source unit
	uvec.m_data.swap(dataVec);	// swap data in - no memcopy needed
	uvec.convert(targetUnit);	// perform in-place conversion
	uvec.m_data.swap(dataVec);	// swap data out - again no memcopy needed
	\endcode
*/
class UnitVector {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	UnitVector() {}

	/*! Constructor (creates a vector with 'size' elements). */
	explicit UnitVector(unsigned int n)
		: m_data(n) {}

	/*! Constructor (creates a vector with 'size' elements and the IO-unit 'ioUnit'). */
	UnitVector(unsigned int n, Unit IO_unit)
		: m_unit(IO_unit), m_data(n) {}

	/*! Constructor (creates a vector with 'size' elements, initial values 'value' and the IO-unit 'ioUnit'). */
	UnitVector(unsigned int n, double value, Unit IO_unit)
		: m_unit(IO_unit), m_data(n, value) {}

	/*! Constructor (creates a vector with 0 elements, a name 'name' and the IO-unit 'ioUnit'). */
	UnitVector(const std::string & name, Unit IO_unit)
		: m_name(name), m_unit(IO_unit) {}

	/*! Constructor (creates a vector from the given range and the optional IO-unit 'IO_unit'). */
	template<typename inIt>
	UnitVector(inIt first, inIt last, Unit IO_unit=Unit())
		: m_unit(IO_unit), m_data(first, last) {}

	/*! Reads a unit vector (data and unit) from a string.
		The format of the string needs to be "1.0 1.2 1.5 m", a series of whitespace-separated
		numbers followed by a unit (unit string must be convertible to an IBK::Unit).
		This function throws an IBK::Exception if the string is malformed.

		Example:
		\code
		string unit_vector_data = "100 150 180 cm";
		UnitVector vec;
		vec.read(unit_vector_data);
		// vec.m_data = [1, 1.5, 1.8]
		// vec.m_unit = IBK::Unit("cm");
		\endcode

		\param str The input string in format "<value 1> <value 2> <...> <value n> <unit>".
		\param convertToSI If true values will be converted into corresponding SI base unit
			after reading. If false, the unit string at the end of the vector is simply stored as
			m_unit (this can be used to read unit vectors with units not yet known
			to the IBK::Unit system).

		\warning The conversion to the base SI unit from the IO unit is a standard behavior of UnitVector.
				If the values are read without conversion, the caller is responsible for converting
				values manually and setting a valid IO unit in the m_unit variable.
	*/
	void read(const std::string& str, bool convertToSI = true);


	/*! Writes a unit vector into the stream using 'indent' spaces as indentation.
		The values stored in the vector are hereby converted into the UI unit given by m_unit.

		Example:
		\code
		UnitVector vec;
		// vec.m_data = [1, 1.5, 1.8]
		// vec.m_unit = IBK::Unit("cm");
		vec.write(cout, 4);
		// writes : "    100 150 180 cm"
		vec.write(cout, 4, false);
		// writes : "    100 150 180"
		\endcode
	*/
	void write(std::ostream& out, unsigned int indent, bool includeUnit=true) const;

	/*!
		write in Binary, no conversion done, written in SI base unit. \sa readBinary
	*/
	void writeBinary(std::ostream& out) const;

	/*! read binary in SI unit. \sa writeBinary */
	void readBinary( std::istream& in );

	/*! Returns the unit vector written in a string. */
	std::string toString(bool includeUnit=true) const;

	/*! Resizes the vector to 'n' elements, fill it with 'value' and sets the IO unit to 'ustr'. */
	void set(unsigned int n, double value, const std::string& ustr);

	/*! Resets the vector to its initial state. */
	void clear();

	/*! Converts the data in the vector from actual unit to unit 'target_unit' and sets the internal unit. */
	void convert(Unit target_unit);

	/*! Resizes content vector (provided for convenience). */
	void resize(unsigned int n) { m_data.resize(n); }

	/*! Returns the size of the vector (convenience function). */
	std::size_t size() const { return m_data.size(); }

	/*! Returns whether the vector is empty or not (convenience function). */
	bool empty() const { return m_data.empty(); }

#ifndef IBK_DEBUG
	/*! Returns a reference to the vector element at index i.
		In debug mode it throws an out_of_range exception if the
		indexes are invalid.
	*/
	double & operator[] (unsigned int i) { return m_data[i]; }

	/*! Returns a constant reference to the vector element at index i.
		In debug mode it throws an out_of_range exception if the
		indexes are invalid.
	*/
	const double & operator[] (unsigned int i) const { return m_data[i]; }
#else
	double & operator[] (unsigned int i);
	const double & operator[] (unsigned int i) const;
#endif // IBK_DEBUG

	/*! Compares this unit vector with another by value and returns true if they differ.
		Unit vectors are equal if m_data vector and m_unit are the same.
	*/
	bool operator!=(const UnitVector & uvec) const;

	/*! Compares this unit vector with another by value and returns true if they are the same.
		Unit vectors are equal if m_data vector and m_unit are the same.
	*/
	bool operator==(const UnitVector & uvec) const { return ! operator!=(uvec); }

	// *** PUBLIC MEMBER VARIABLES ***

	std::string				m_name;			///< optional description
	Unit					m_unit;			///< The input/output unit of the vector.
	std::vector<double>		m_data;			///< Contains the actual data of the vector.

private:
	/*! Converts the data in the vector from actual unit to unit 'target_unit'.
		It will be called from convert if necessary.
		\note m_unit is not modified.
	*/
	void convert_special(Unit target_unit, double & val);

};

} // namespace IBK

/*! \file IBK_UnitVector.h
	\brief Contains the class UnitVector.
*/

#endif // IBK_UnitVectorH
