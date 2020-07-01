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

#ifndef IBK_IntParaH
#define IBK_IntParaH

#include <string>
#include "IBK_Unit.h"
#include "IBK_math.h"
#include "IBK_assert.h"

namespace IBK {

/*! The class IntPara represents a named integer parameter. The name/keyword
	can be used for reading/writing the parameter to input data files. Also,
	it can be used to distinguish whether the parameter is defined or not (empty name).
*/
class IntPara {
public:
	/*! Default constructor, creates an empty unnamed IntPara. */
	IntPara() : name(""), value(0) {}

	/*! Constructor to initialise the IntPara with name and value. */
	IntPara(const std::string& str, int val);

	/*! Sets name and value. */
	void set(const std::string& str, int val);

	/*! Returns the IntPara fully formatted as std::string.
		\param without_name If false, the IntPara string is of format "<name> = <value>", otherwise just
				the value.
	*/
	std::string toString(bool without_name = true) const;

	/*! Reads a IntPara from the stream 'in'.
		The format for reading is:
		\code
		<name> <value>
		\endcode
		When a IntPara should be read without a name/keyword,
		pass 'true' as second IntPara.
		\param in       The input stream.
		\param no_name (optional) Flag that tells the read function to read only the value.
	*/
	void read(std::istream& in, bool no_name=false);

	/*! Writes a IntPara into the output stream.
		The output format is: name value<br>
		If the IntPara hasn't got a name, only the value is written.
		The optional IntParas can be used to specify the layout.
		\param out          The output stream.
		\param indent       (optional) Number of spaces, that should be put in front.
		\param paramWidth   (optional) Width of the IntPara name (exclusive indentation),
									   a " = " is always appended
		\param writeName    (optional) Should the name (keyword) be written or not.
	*/
	void write(std::ostream& out, unsigned int indent=0, unsigned int paramWidth=0, bool writeName=true) const;

	/*! Reads a IntPara from the stream 'in'.
		It uses the binary representation.
		\param in       The input stream.
	*/
	void readBinary(std::istream& in);

	/*! Writes a IntPara into the output stream.
		It uses the binary representation.
		\param out          The output stream.
	*/
	void writeBinary(std::ostream& out) const;

	/*! Clears the IntPara name is cleared and value set to 0. */
	void clear();

	/*! Returns true if the IntPara is empty (a IntPara is empty when it does not
		have a name).
	*/
	bool empty() const { return name.empty(); }

	/*! Compares this instance and another by content and returns true if they differ. */
	bool operator!=(const IntPara& other) const {
		if (value != other.value)
				return true;

		if (name != other.name)
				return true;

		return false;
	}

	/*! Compares this instance and another by content and returns true if are the same. */
	bool operator==(const IntPara& other) const {
			return !operator!=(other);
	}

	/*! This method is used to test if an IntPara is defined and lower than a given limit.
		It throws a formatted exception if the value is not lower or not lower equal.
		Other errors are missmatching IntPara names.
		\param limit IntPara to be tested against.
		\param isLessEqual If set to true comparison operator is <= is used, if false < is applied.
		\return True is returned if IntPara name can be matched, as well as the value test is valid as well.
	*/
	void checkIfValueIsBelowLimit (	const IBK::IntPara& limit, bool isLessEqual ) const;

	/*! This method is used to test if an IntPara is defined and higher than a given limit.
		It throws a formatted exception if the value is not higher or not higher equal.
		Other errors are missmatching IntPara names.
		\param limit IntPara to be tested against.
		\param isGreaterEqual If set to true comparison operator is >= is used, if false > is applied.
		\return True is returned if IntPara name can be matched, as well as the value test is valid as well.
	*/
	void checkIfValueIsAboveLimit (	const IBK::IntPara& limit, bool isGreaterEqual ) const;


	// ****** member variables *************************************************

	/*! The descriptive keyword/name of the IntPara. */
	std::string 	name;
	/*! The value of the IntPara. */
	int				value;

private:

	/*! Operators used by private test routine. */
	enum oper_t {
		OP_LT,	///< less then
		OP_LE,	///< less equal then
		OP_GE,	///< greater equal then
		OP_GT,	///< greater then
	};

	/*! Test routine for code reuse by \sa checkIfValueIsBelowLimit and \sa checkIfValueIsAboveLimit. */
	void test( const IBK::IntPara& val, oper_t op ) const;
};

}  // namespace IBK

/*! \file IBK_IntPara.h
	\brief Contains the declaration of the class IntPara, a class for a named integer.
*/

#endif // IBK_IntParaH
