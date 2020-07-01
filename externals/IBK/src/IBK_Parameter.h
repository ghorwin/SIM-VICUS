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

#ifndef IBK_ParameterH
#define IBK_ParameterH

#include <string>
#include "IBK_Unit.h"
#include "IBK_math.h"
#include "IBK_assert.h"

namespace IBK {

/*! The class Parameter represents a physical parameter consisting of a descriptive
	keyword, a value and unit used for input/output. Parameters can be used in
	calculation programs for any kind of input parameters. The advantage of the
	use of a parameter is, that the input data can be given in a unit that differs
	from the SI base unit used for calculation.
	When created or set, a parameter always converts the value given in the IO unit
	into the internal SI base unit which can be retrieved via the 'value' member
	variable. The only exception is, when the parameter has been created with an
	'undefined' unit (unit id = 0).
	The member function get_value() can be used to retrieve the parameters value in
	the required unit.
*/
class Parameter {
public:
	/*! Default constructor, creates an empty parameter with undefined unit. */
	Parameter() : name(""), value(0), IO_unit(0) {}

	/*! Constructor to initialise the parameter with name, value and IO unit.
		When you create a parameter like this the value 'val' will be converted into
		the base unit of 'unit', if available. For example when passing val=20 and
		mm as unit the value of the parameter will be 0.02 afterwards.
	*/
	Parameter(const std::string& str, double val, const Unit unit=Unit(0));

	/*! Function is just for convenience and works essentially like the function above.
		If the unit is invalid, the function throws an exception.
	*/
	Parameter(const std::string& str, double val, const std::string& unit_str);

	/*! Sets value and unit with conversion.
		The value 'val' will be converted from 'unit' in the base unit of 'unit'.
	*/
	bool set(double val, Unit unit, std::string * errmsg = NULL);

	/*! Sets name, value and unit with conversion.
		The value 'val' will be converted from 'unit' in the base unit of 'unit'.
	*/
	bool set(const std::string& str, double val, Unit unit, std::string * errmsg = NULL);

	/*! Sets name and value without conversion.
		The value 'val' will not be converted and the unit will be set to 'undefined'.
	*/
	void set(const std::string& str, double val);

	/*! Sets name, value and unit with conversion.
		The value 'val' will be converted from unit 'ustr' in the base unit of 'ustr'.
	*/
	bool set(const std::string& name_, double val, const std::string& ustr, std::string * errmsg = NULL) { return set(name_, val, Unit(ustr), errmsg); }

	/*! Sets the parameter by name and value, whereas valstr is composed of the actual value and the IO_unit.
		The value will be converted in the corresponding base unit.
	*/
	bool set(const std::string& str, const std::string& valstr, std::string * errmsg = NULL);

	/*! Returns the value of the parameter converted into its IO unit. */
	double get_value() const { return get_value(IO_unit); }

	/*! Returns the value of the parameter converted into 'unit'.
		The function throws an IBK::Exception if the units cannot be related.
	*/
	double get_value(Unit unit) const;

	/*! Returns the value of the parameter converted into unit 'ustr'.
		The function throws a IBK::Exception exception if the units cannot be related.
	*/
	double get_value(const std::string& ustr) const { return get_value(Unit(ustr)); }

	/*! Returns the value of the parameter converted to 'unit' or the default value if the parameter's name is empty
		(parameter not set).
	*/
	double get_value_or_default(Unit unit, double defaultValue) const;

	/*! Convenience function overload, takes unit as string. */
	double get_value_or_default(const std::string &ustr, double defaultValue) const {
		return get_value_or_default(IBK::Unit(ustr), defaultValue);
	}

	/*! Returns the parameter fully formatted as std::string.
		The parameter is converted into the IO_unit.
		\param without_name If false, the parameter string is of format "<name> = <value> <unit>", otherwise of
			format "<value> <unit>".
	*/
	std::string toString(bool without_name = true) const;

	/*! Returns the parameter fully formatted as std::string.
		\param u The output unit.
		\param without_name If false, the parameter string is of format "<name> = <value> <unit>", otherwise of
			format "<value> <unit>".
	*/
	std::string toString(const IBK::Unit & u, bool without_name = true) const;

	/*! Reads a parameter from the stream 'in'.
		The format for reading is:
		\code
		<name> <value> <unit>
		\endcode
		When a parameter should be read without a name/keyword,
		pass 'true' as second parameter.
		The value that has been read will be converted from the IO-unit to
		the base unit.
		\param in       The input stream.
		\param no_name (optional) Flag that tells the read function to read only the value and unit.
	*/
	void read(std::istream& in, bool no_name=false);

	/*! Writes a parameter into the output stream.
		The output format is: name value unit <br>
		If the parameter hasn't got a name, only the value and the unit is written.
		The value will be first converted into the IO unit and then written.
		The optional parameters can be used to specify the layout.
		\param out          The output stream.
		\param indent       (optional) Number of spaces, that should be put in front.
		\param paramWidth   (optional) Width of the parameter name (exclusive indentation),
									   a " = " is always appended
		\param writeName    (optional) Should the name (keyword) be written or not.
	*/
	void write(std::ostream& out, unsigned int indent=0, unsigned int paramWidth=0, bool writeName=true) const;

	/*! Reads a parameter from the stream 'in'.
		It uses the binary representation.
		\param in       The input stream.
	*/
	void readBinary(std::istream& in);

	/*! Writes a parameter into the output stream.
		It uses the binary representation.
		\param out          The output stream.
	*/
	void writeBinary(std::ostream& out) const;

	/*! Returns the current base unit of the parameter. */
	Unit unit() const { return Unit(IO_unit.base_id()); }

	/*! Clears the parameter (name and IO_unit are set to empty strings). */
	void clear();

	/*! Returns true if the parameter is empty (a parameter is empty when it does not
		have a name).
	*/
	bool empty() const { return name.empty(); }

	/*! Compares this instance and another by content and returns true if they differ. */
	bool operator!=(const Parameter& other) const {
		if (IO_unit != other.IO_unit)
				return true;

		if (value != other.value)
				return true;

		if (name != other.name)
				return true;

		return false;
	}

	/*! Compares this instance and another by content and returns true if are the same. */
	bool operator==(const Parameter& other) const {
			return !operator!=(other);
	}

	/*! Comparison function, compares two parameters with 5 digits accuracy.
		If you need a different level of accuracy, compare the values of the parameters
		directly.
		Requires the IO_units of this and the other parameter to have the same
		base unit.
	*/
	bool equalTo(const Parameter& other) {
		IBK_ASSERT(IO_unit.base_id() == other.IO_unit.base_id());
		// compare with 5 digits accuracy
		return IBK::nearly_equal<5>(value, other.value);
	}

	/*! This method is used to test if a physical parameter is lower than a given limit.
		It throws an exception if the value is not lower or not lower equal.
		Other errors are missmatching parameter names or missmatching base units.
		\param limit Unit, parameter name and value to be tested against.
		\param isLessEqual If set to true comparison operator is <= is used, if false < is applied.
		\return True is returned if parameter name and unit can be matched, as well as the value test is valid as well.
		\todo refactor to name checkIfValueIsBelowLimit
	*/
	void checkIfValueIsLowerBound (	const IBK::Parameter& limit,
							bool isLessEqual ) const;

	/*! This method is used to test if a physical parameter is higher than a given limit.
		It throws an exception if the value is not higher or not higher equal.
		Other errors are missmatching parameter names or missmatching base units.
		\param limit Unit, parameter name and value to be tested against.
		\param isGreaterEqual If set to true comparison operator is >= is used, if false > is applied.
		\todo refactor to name checkIfValueIsAboveLimit
	*/
	void checkIfValueIsUpperBound (	const IBK::Parameter& limit,
							bool isGreaterEqual ) const;


	// ****** member variables *************************************************

	/*! The descriptive keyword/name of the parameter. */
	std::string 	name;
	/*! The value of the parameter for calculations (always in SI base unit). */
	double 			value;
	/*! Input and output unit for the parameter. */
	Unit 			IO_unit;

private:

	/*! Operators used by private test routine. */
	enum oper_t {
		OP_LT,	///< less then
		OP_LE,	///< less equal then
		OP_GE,	///< greater equal then
		OP_GT,	///< greater then
	};


	/*! Test routine for code reuse by \sa checkIfValueIsUpperBound and \sa checkIfValueIsLowerBound. */
	void test( const IBK::Parameter& val, oper_t op ) const;

};

/*! "Less then" operator, returns true if value of left parameter is less than value of right parameter.
	Requires the IO_units of this and the other parameter to have the same
	base unit.
*/
inline bool operator<(const Parameter& lhs, const Parameter& rhs) {
	IBK_ASSERT(lhs.IO_unit.base_id() == rhs.IO_unit.base_id());
	return lhs.value < rhs.value;
}

}  // namespace IBK

/*! \file IBK_Parameter.h
	\brief Contains the declaration of the class Parameter, a class for a physical parameter
		   with name, value and assiciated Input/Output unit.
*/

#endif // IBK_ParameterH
