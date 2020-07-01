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

#ifndef IBK_FormatStringH
#define IBK_FormatStringH

#include <string>
#include <sstream>
#include <iosfwd>
#include <vector>

#include "IBK_configuration.h"
#include "IBK_MultiLanguageString.h"


namespace IBK {

class Unit;
class Path;

/*! Class FormatString helps assembling error/progress messages without
	the use of the old fashioned and error prone C function sprintf.
	Define a string to be formatted like "This is section %1 of file '%2' with %3 entries".
	Use the function arg() to replace placeholders with data.
	Example:
	\code
	string filename = "inputfile.txt";
	int line = 14;
	throw IBK::Exception( FormatString("Error in line %1 of file '%2'!").arg(line).arg(filename) );
	\endcode
*/
class FormatString {
public:
	/*! Default constructor, creates an empty format string. */
	FormatString();
	/*! String parameterized constructor taking a string as argument.
		\param str Message string.
	*/
	explicit FormatString(const std::string& str);
	/*! Constructor taking a MultiLanguage string as argument.
		\param str MultiLanguage string (the english name is used for the
		error message).
	*/
	explicit FormatString(const IBK::MultiLanguageString& str);
	/*! Clears the string. */
	void clear();

	/*! Parametrizes a format string of style "This is section % with % entries".
		The previously inserted data is deleted.
		\param str message string
	*/
	void set(const std::string& str);

	/*! Returns the complete format string as std::string. */
	std::string str() const;

	/*! Inserts 's' at the next location of '%' in the current format string.
		Spezialization for strings.
		\param		s		   string for substitution of '%'.
		\param		fieldWidth string field width, optional.
		\return		FormatString including the substitution argument.
	*/
	FormatString & arg(const std::string & s, int fieldWidth=0);
	/*! Inserts MultiLanguageString at the next location of '%'.
		\param		s		   MultiLanguageString for substitution of '%', uses the english string.
		\param		fieldWidth string field width, optional.
		\return		FormatString including the substitution argument.
	*/
	FormatString & arg(const IBK::MultiLanguageString & s, int fieldWidth=0);
	/*! Inserts IBK::Path at the next location of '%'.
		\param		path		   IBK::Path for substitution of '%'.
		\return		FormatString including the substitution argument.
	*/
	FormatString & arg(const IBK::Path & path);

	/*! Inserts IBK::Unit at the next location of '%'.
		\param		unit		   IBK::Unit for substitution of '%'.
		\return		FormatString including the substitution argument.
	*/
	FormatString & arg(const IBK::Unit & unit);

	/*! Inserts an integer at the next location of '%'.
		\param		i			 integer value for substitution of '%'.
		\param		fieldWidth   string field width, optional.
		\return		FormatString including the substitution argument.
	*/
	FormatString & arg(int i, int fieldWidth=0);
	/*! Inserts an unsigned integer at the next location of '%'.
		\param		i			 unsigned integer value for substitution of '%'.
		\param		fieldWidth   string field width, optional.
		\return		FormatString including the substitution argument.
	*/
	FormatString & arg(unsigned int i, int fieldWidth=0);
#if !defined(_WIN32) || defined(_WIN64)
	/*! Inserts 'i' at the next location of '%' in the current format string.
		Spezialization for size_t.
	*/
	FormatString & arg(size_t i, int fieldWidth=0);
#endif
	/*! Inserts a double at the next location of '%'.
		\param		d			 double value for substitution of '%'.
		\param		fieldWidth   string field width, optional.
		\param		format		 character encoding number format ('g'- default floating
								 point notation, 'f' fixed-point notation, 'e'/'E' scientific notation.
		\param		precision    double precision for string conversion, optional.
		\param		fillChar     fill character for empty field positions, optional.
		\return		FormatString including the substitution argument.
		\sa QString::arg(double, ...) for description of arguments
	*/
	FormatString & arg(double d, int fieldWidth = 0, char format = 'g', int precision = -1, const char & fillChar = ' ' );

	/*! Inserts another FomatString at the next location of '%'.
		\return		FormatString including the substitution argument.
	*/
	FormatString & arg(const IBK::FormatString & s) {
		arg(s.str());
		return *this;
	}

	/*! Operator + for concatenation of a std::string (convenience function).
		Returns a new format string.
	*/
	std::string operator+(const std::string& s) const;

	/*! Operator += for string concatenation.
		This operator does all the parsing work.
		\return Returns a const reference to the format string.
	*/
	const FormatString & operator+=(const std::string& s);

private:
	/*! Used to describe a token in the original format string.
		A token can either be a placeholder for an argument, or a
		string token.
	*/
	struct Token {
		Token() : index(0) {}
		Token(std::string t, unsigned int id) : token(t), index(id) {}
		std::string		token;
		unsigned int	index;
	};

	/*! The list of tokens that the string is composed of. */
	std::vector<Token>			m_tokens;
	/*! The list of arguments (size equals number of calls to arg(). */
	std::vector<std::string>	m_arguments;
};

/*! Stream output operator. */
std::ostream& operator<<(std::ostream& strm, const FormatString& fstr);

/*! String concatenation operator. */
std::string operator+(const std::string& lhs, const FormatString& rhs);

} // namespace IBK

/*! \file IBK_FormatString.h
	\brief Contains declaration of class FormatString, a class that replaces printf().
*/

#endif // IBK_FormatStringH
