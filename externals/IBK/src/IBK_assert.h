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

#ifndef IBK_assertH
#define IBK_assertH

#include "IBK_configuration.h"

#include <cstring>

#include "IBK_Exception.h"
#include "IBK_StringUtils.h"

/*! \def IBK_GLIBC_PREREQ(major, minor)
	This macro allows to test for the version of the GNU C library (or
	a compatible C library that masquerades as glibc). It evaluates to
	0 if libc is not GNU libc or compatible.
	\code
	#if IBK_GLIBC_PREREQ(2, 3)
	...
	#endif
	\endcode

	\def IBK_GNUC_PREREQ(major, minor, patchlevel)
	This macro allows to test for the version of the GNU C++ compiler.
	Note that this also applies to compilers that masquerade as GCC,
	for example clang and the Intel C++ compiler for Linux.
	\code
	#if IBK_GNUC_PREREQ(4, 3, 1)
	...
	#endif
	\endcode
*/


#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
# define IBK_GLIBC_PREREQ(major, minor)                                    \
	((__GLIBC__ * 100 + __GLIBC_MINOR__) >= ((major) * 100 + (minor)))
#else
# define IBK_GLIBC_PREREQ(major, minor) 0
#endif


#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define IBK_GNUC_PREREQ(major, minor, patchlevel)                         \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >=   \
	 ((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define IBK_GNUC_PREREQ(major, minor, patchlevel)       \
	((__GNUC__ * 10000 + __GNUC_MINOR__) >=             \
	 ((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define IBK_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

#if defined(__clang__)
#define IBK_HAS_ATTRIBUTE_UNUSED (__has_attribute(unused))
#elif defined(__GNUC__)
/// Helper macro for checking for certain attributes
#define IBK_HAS_ATTRIBUTE_UNUSED (IBK_GNUC_PREREQ(2, 95, 0))
#endif

#if IBK_HAS_ATTRIBUTE_UNUSED
# define IBK_UNUSED __attribute__((unused))
#else
/// A macro to mark variables or types as unused, avoiding compiler warnings.
# define IBK_UNUSED
#endif

// IBK_DEBUG is defined in configuration.h
#ifdef IBK_DEBUG
  #include <stdexcept>

/// Function to extract file name from file path
inline const char * myFilename(const char * fullFilePath){

	// search last / or backslash
	const char * posLastBackslash = std::strrchr( fullFilePath, '\\');
	const char * posLastSlash = std::strrchr( fullFilePath, '/');

	if (posLastSlash != NULL) {
		if (posLastBackslash != NULL)
			return (posLastBackslash > posLastSlash) ? posLastBackslash+1 : posLastSlash+1;
		else
			return posLastSlash+1;
	}
	else if (posLastBackslash != NULL)
		return posLastBackslash+1;

	return fullFilePath;
}

/*! IBK-style runtime assertion macro, throws an IBK::Exception if condition fails. */
#define IBK_ASSERT(p)	if (!(p)) \
	{ std::stringstream strm; \
	strm << "Assertion failure\nCHECK: " << #p << "\nFILE:  " << myFilename(__FILE__) << "\nLINE:  " << __LINE__ << '\n'; \
	throw IBK::Exception(strm.str(), __FILE__); }(void)0
#define IBK_ASSERT_X(p,m)	if (!(p)) \
	{ std::stringstream strm; \
	strm << "Assertion failure\nCHECK: " << #p << ", " << #m <<  "\nFILE: " << myFilename(__FILE__) << "\nLINE:  " << __LINE__ << '\n'; \
	throw IBK::Exception(strm.str(), __FILE__); }(void)0
#define IBK_ASSERT_XX(p,m)	if (!(p)) \
	{ std::stringstream strm; \
	strm << "Assertion failure\nCHECK: " << #p << ", " << m <<  "\nFILE: " << myFilename(__FILE__) << "\nLINE:  " << __LINE__ << '\n'; \
	throw IBK::Exception(strm.str(), __FILE__); }(void)0

#else

/*! IBK-style runtime assertion macro, throws an IBK::Exception if condition fails. */
#define IBK_ASSERT(p)		(void)0;
/*! IBK-style runtime assertion macro with additional message, throws an IBK::Exception if condition fails.
	\code
	double b = 15;
	// message string is printed "as is"
	IBK_ASSERT_X(b == 12, "Value was: " << b);
	// gives: CHECK: 12*6 == b, "Value was: " << b
	\endcode
*/
#define IBK_ASSERT_X(p,m)	(void)0;
/*! IBK-style runtime assertion macro with additional message, throws an IBK::Exception if condition fails.
	\code
	double b = 15;
	// message string is evaluated and piped into stream, hence you can use stream operators to compose
	// complex message strings
	IBK_ASSERT_XX(12*6 == b, "Value was: " << b);
	// gives: CHECK: 12*6 == b, Value was: 15
	\endcode
*/
#define IBK_ASSERT_XX(p,m)	(void)0;

#endif

/// Helper macro for IBK_STATIC_ASSERT
#define DO_JOIN( X, Y ) X##Y

/*! Helper class for IBK_STATIC_ASSERT macro. */
template <bool x>
struct IBK_STATIC_ASSERTION_FAILURE;

/*! Helper class for IBK_STATIC_ASSERT macro. */
template <>
struct IBK_STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };

/*! Helper class for IBK_STATIC_ASSERT macro. */
template<int x>
struct IBK_STATIC_ASSERT_TEST{};

/*! Macro for static assertions (compile time checks).
	\code
	// the following line gives a error at compile time when compiled with a 32bit compiler
	IBK_STATIC_ASSERT(sizeof(long int) == 8);
	\endcode
*/
#define IBK_STATIC_ASSERT( B ) \
   typedef IBK_STATIC_ASSERT_TEST<\
	  sizeof(IBK_STATIC_ASSERTION_FAILURE< (bool)( B ) >)>\
		 DO_JOIN(static_assert_typedef_, __LINE__) IBK_UNUSED


/*! \file IBK_assert.h
	\brief Contains assertion definitions.
*/

#endif // IBK_assertH
