/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

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

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#ifndef IBKMK_rational_numberH
#define IBKMK_rational_numberH

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iosfwd>

#include <IBK_Exception.h>

namespace IBKMK {

template <typename T>
class rational_number;
// declaration of class rational_number is necessary here because of function declaration below

/*! Input stream operator for rational number objects. */
template<typename T>
std::istream& operator>>(std::istream& in, rational_number<T>&);
// declaration necessary here because of friend declaration in class.

/*! A class for simplified handling and particularly input of rational numbers.

	This class is mainly used to enhance the ability of input files so that
	rational numbers in form of 1/3 or even 5.3/7 can be used in input files.
	You can also read normal floating point values such as 3.5 into a rational number,
	which is then stored as 3.5/1. Using the operator() the floating point value
	in computer precision can be extracted.
	The template class rational_number can be used as in the following example:
	<pre>
		// assume 'strm' is an input stream containing double and/or rational numbers
		vector<double> dbl_vec;
		rational_number<double> n;
		while (strm >> rn)
			dbl_vec.push_back(rn()); // note the additional ()
	</pre>
*/
template <typename T>
class rational_number {
  public:
	/*! Default constructor, creates the rational number 0/1. */
	rational_number() : m_nominator(0), m_denominator(1) {}
	/*! Initialization constructor, creates the rational number n/d.
		This function checks for a zero denominator and throws a
		std::IBK_exception exception if the denominator is equal zero.
	*/
	rational_number(T n, T d);

	/*! This function is used to set new values for nominator and
		denominator.
		This function checks for a zero denominator and throws a
		std::IBK_exception exception if the denominator is equal zero.
	*/
	void set(T n, T d);

	/*! This operator returns the calculated floating point value for
		this rational number. This function does not check for a
		zero denominator, since there is no way the denominator may
		become zero in this implementation.
	*/
	T operator()() const { return m_nominator/m_denominator; }

	/*! Returns the nominator. */
	T nominator() const { return m_nominator; }
	/*! Returns the denominator. */
	T denominator() const { return m_denominator; }

  private:
	T m_nominator;
	T m_denominator;

	// <> nach Operator eingefuegt um der Klasse zu sagen, dass das hier eine Template Funktion ist
	friend std::istream& operator>> <>(std::istream& in, rational_number<T>&)
};


template <typename T>
rational_number<T>::rational_number(T n, T d)
	: m_nominator(n), m_denominator(d)
{
	if (d == 0)
	  throw IBK::Exception( " Denominator == 0!", "[rational_number<T>::rational_number]" );
}

template <typename T>
void rational_number<T>::set(T n, T d) {
	if (d == 0)
	  throw IBK::Exception( "Denominator == 0!", "[rational_number<T>::set]" );
	m_nominator = n;
	m_denominator = d;
}

template<typename T>
std::istream& operator>>(std::istream& in, rational_number<T>& rn) {
	std::string tmp;
	if (!(in >> tmp)) {
		in.setstate(std::ios_base::badbit);
		return in;
	}
	std::size_t pos = tmp.find('/');
	// if not found, assume a pure floating point number
	if (pos == std::string::npos) {
		std::stringstream strm(tmp);
		if (!(strm >> rn.m_nominator)) {
			in.setstate(std::ios_base::badbit);
			return in;
		}
		rn.m_denominator = 1;
	}
	else {
		// otherwise read in nominator and denominator seperately
		std::stringstream nom(tmp.substr(0, pos));
		std::stringstream denom(tmp.substr(pos+1, std::string::npos));
		if (!(nom >> rn.m_nominator) || !(denom >> rn.m_denominator)
		   || rn.m_denominator == 0)
		{
			in.setstate(ios_base::badbit);
			return in;
		}
	}
	return in;
}


} // namespace IBKMK

/*! @file IBKMK_rational_number.h
	@brief Contains the declaration of the class template rational_number, a data type that
		   allows simple reading of rational numbers from input streams.

	@example rationalnumber_example.cpp
	This small example demonstrates the use of the class template IBK::rational_number.
*/

#endif // IBKMK_rational_numberH
