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

#include "IBK_Version.h"

#include <cstdlib>

#include "IBK_InputOutput.h"
#include "IBK_StringUtils.h"
#include "IBK_assert.h"
#include "IBK_messages.h"

namespace IBK {


/*! Mask for the major version number. */
unsigned majorMaskBinary = 0x000000ff;

/*! Mask for the minor version number. */
unsigned int minorMaskBinary = 0x0000ff00;

/*! Mask for the major version number. */
unsigned int majorMaskASCII = 0x0000ff00;

/*! Mask for the minor version number. */
unsigned int minorMaskASCII = 0x000000ff;


/*! Returns the complete version number as unsigned int formated for a binary output little endian. */
unsigned int versionNumberBinary(unsigned int majorNumber, unsigned int minorNumber ) {
	return ( majorNumber | (minorNumber << 8 ) );
}

/*! Extracts major number from version id
	\param version The version number read from an output file.
*/
unsigned int majorFromVersionBinary( unsigned int version ) {
	return ( version & majorMaskBinary );
}

/*! Extracts major number from version id
	\param version The version number read from an output file.
*/
unsigned int minorFromVersionBinary( unsigned int version ){
	return (( version & minorMaskBinary ) >> 8 );
}

/*! Returns the version number as ASCII encoded string */
unsigned int versionNumberASCII(unsigned int majorNumber, unsigned int minorNumber) {
	return ( (majorNumber << 8) | minorNumber );
}

/*! Extracts major number from version id
	\param version The version number read from an output file.
*/
unsigned int majorFromVersionASCII(unsigned int version) {
	return (( version & majorMaskASCII ) >> 8 );
}

/*! Extracts minor number from version id
	\param version The version number read from an output file.
*/
unsigned int minorFromVersionASCII( unsigned int version ) {
	return ( version & minorMaskASCII );
}


void Version::read(std::istream & in, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
				 unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
				 bool & isBinary, unsigned int & majorVersion, unsigned int & minorVersion)
{
	const char * const FUNC_ID = "[Version::read]";

	// read first unsigned int from stream
	unsigned int test;
	IBK::read_uint32_binary( in, test);

	// check if this is a binary data file
	if ( test == magicNumberFirstBinary) {

		// read second unsigned int from stream
		IBK::read_uint32_binary( in, test);

		// test magic header
		if (test != magicNumberSecondBinary)
			throw IBK::Exception("Wrong second magic number!", FUNC_ID);

		// we have binary format
		isBinary = true;

		// read file version and split it
		IBK::read_uint32_binary( in, test);
		majorVersion = majorFromVersionBinary( test );
		minorVersion = minorFromVersionBinary( test );
		unsigned int dummy;
		// read last unsigned integer
		IBK::read_uint32_binary( in, dummy);
		IBK_ASSERT(dummy == 0); // safety check

	}
	else if (test == magicNumberFirstASCII) {

		// read second unsigned int from stream
		IBK::read_uint32_binary( in, test);

		// test magic header
		if (test != magicNumberSecondASCII)
			throw IBK::Exception("Wrong second magic number!", FUNC_ID);

		// not a binary file
		isBinary = false;

		// read file version and split it
		IBK::read_uint32_binary( in, test); // 3rd uint
		unsigned int second;
		IBK::read_uint32_binary( in, second); // 4th uint
		unsigned int version = fromASCIIEncoding(test, second);

		majorVersion = majorFromVersionASCII( version );
		minorVersion = minorFromVersionASCII( version );

	}
	else
		throw IBK::Exception("Invalid magic header or unknown file format.", FUNC_ID);


}


void Version::write(std::ostream & out, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
				  unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
				  bool isBinary, unsigned int majorVersion, unsigned int minorVersion)
{
	// write magic header
	if (isBinary) {

		IBK::write_uint32_binary( out, magicNumberFirstBinary );
		IBK::write_uint32_binary( out, magicNumberSecondBinary );
		IBK::write_uint32_binary( out, versionNumberBinary(majorVersion, minorVersion) );
		IBK::write_uint32_binary( out, 0 );

	}
	else {

		IBK::write_uint32_binary( out, magicNumberFirstASCII );
		IBK::write_uint32_binary( out, magicNumberSecondASCII );
		unsigned int first, second;
		first = 0; second = 0; // this done to prevent windows environment from complaining
		toASCIIEncoding( versionNumberASCII(majorVersion, minorVersion), first, second );
		IBK::write_uint32_binary( out, first );
		IBK::write_uint32_binary( out, second );
	}
}


bool Version::extractMajorMinorVersionNumber(const std::string & versionString, unsigned int & major, unsigned int & minor) {
	std::vector<std::string> tokens;
	IBK::explode(versionString, tokens, ".", IBK::EF_TrimTokens);
	if (tokens.size() < 2)
		return false;
	try {
		major = IBK::string2val<unsigned int>(tokens[0]);
		minor = IBK::string2val<unsigned int>(tokens[1]);
		return true;
	}
	catch (...) {
		return false;
	}
}


bool Version::extractMajorMinorPatchVersionNumber(const std::string & versionString, unsigned int & major, unsigned int & minor, unsigned int & patch) {
	std::vector<std::string> tokens;
	IBK::explode(versionString, tokens, ".", IBK::EF_TrimTokens);
	if (tokens.size() < 3)
		return false;
	try {
		major = IBK::string2val<unsigned int>(tokens[0]);
		minor = IBK::string2val<unsigned int>(tokens[1]);
		patch = IBK::string2val<unsigned int>(tokens[2]);
		return true;
	}
	catch (...) {
		return false;
	}
}


bool Version::lesserVersionNumber(const std::string & lhs, const std::string & rhs) {
	// first extract major, minor and patch numbers
	unsigned int maj1, min1, pat1;
	unsigned int maj2, min2, pat2;
	if (!extractMajorMinorPatchVersionNumber(lhs, maj1, min1, pat1) ||
			!extractMajorMinorPatchVersionNumber(rhs, maj2, min2, pat2))
	{
		throw IBK::Exception("Bad version numbers.", "[Version::lesserVersionNumber]");
	}

	if (maj1 < maj2) return true;
	if (maj1 > maj2) return false;
	if (min1 < min2) return true;
	if (min1 > min2) return false;
	return pat1<pat2;
}


/*! Helper struct for functions toASCIIEncoding() and fromASCIIEncoding(). */
union exchangeType {
	unsigned int	dword;
	char			byte[4];
};


void Version::toASCIIEncoding( unsigned int version, unsigned int &first, unsigned int &second ) {


	// split numbers
	char * versionArray = new char[ 2*sizeof(unsigned int) ];

	// extract version numbers
	unsigned int major = majorFromVersionASCII( version );
	unsigned int minor = minorFromVersionASCII( version );

	// initialize
	first = 0;
	second = 0;

	// create hex array to base 10
	unsigned int counter = major;
	unsigned int position = 0;
	unsigned int spacer = 3;
	while ( counter ) {

		// a mod 10 =>
		// a div a
		std::div_t divresult = std::div (counter,10);
		counter = divresult.quot;
		unsigned int rest = divresult.rem;

		versionArray[ position ] = '0' + rest;
		++position;
		--spacer;

	}

	while( spacer ){
		versionArray[position] = '0';
		--spacer;
		++position;
	}


	spacer = 3;
	counter = minor;
	while ( counter ) {

		// a mod 10 =>
		// a div a
		std::div_t divresult = std::div (counter,10);
		counter = divresult.quot;
		unsigned int rest = divresult.rem;

		versionArray[ position ] = '0' + rest;
		++position;
		--spacer;

	}

	while( spacer ){
		versionArray[position] = '0';
		--spacer;
		++position;
	}

	exchangeType temp;
	temp.byte[0] = 32;
	temp.byte[1] = versionArray[2];
	temp.byte[2] = versionArray[1];
	temp.byte[3] = versionArray[0];

	first = temp.dword;

	temp.byte[0] = 46;
	temp.byte[1] = versionArray[5];
	temp.byte[2] = versionArray[4];
	temp.byte[3] = versionArray[3];

	second = temp.dword;

	// clean up
	delete[] versionArray;

}


unsigned int Version::fromASCIIEncoding( unsigned int first, unsigned int second ){

	exchangeType tmp;
	tmp.dword = first;
	unsigned int major = (tmp.byte[1] - '0') * 100 + (tmp.byte[2] - '0') * 10 + (tmp.byte[3] - '0');

	tmp.dword = second;
	unsigned int minor = (tmp.byte[1] - '0') * 100 + (tmp.byte[2] - '0') * 10 + (tmp.byte[3] - '0');

	return ((major << 8) | minor);
}

void Version::printCompilerVersion() {
	const char * const FUNC_ID = "[Version::printCompilerVersion]";
	// print compiler and version information
#if defined(__GNUC__)

#if defined(__MINGW32__)
	IBK::IBK_Message("Compiled with MinGW32-GCC                        " + std::string(__VERSION__) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
#else // defined(__MINGW32__)
	IBK::IBK_Message("Compiled with GCC                                " + std::string(__VERSION__) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
#endif // defined(__MINGW32__)

#elif defined(_MSC_VER)
	switch (_MSC_VER) {
		case 1400 : IBK::IBK_Message("Compiled with Visual Studio 2005\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 1500 : IBK::IBK_Message("Compiled with Visual Studio 2008\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 1600 : IBK::IBK_Message("Compiled with Visual Studio 2010\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 1700 : IBK::IBK_Message("Compiled with Visual Studio 2012\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 1800 : IBK::IBK_Message("Compiled with Visual Studio 2013\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 1900 : IBK::IBK_Message("Compiled with Visual Studio 2015\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 1910 : IBK::IBK_Message("Compiled with Visual Studio 2017\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		default:
			IBK::IBK_Message( IBK::FormatString("Compiled with Visual Studio, version ID          %1\n").arg(_MSC_VER), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
#elif defined(__BORLANDC__)
	switch (__BORLANDC__) {
		case 0x200 : IBK::IBK_Message("Compiled with Borland C++ 2.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x400 : IBK::IBK_Message("Compiled with Borland C++ 3.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x410 : IBK::IBK_Message("Compiled with Borland C++ 3.1\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x452 : IBK::IBK_Message("Compiled with Borland C++ 4.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x500 : IBK::IBK_Message("Compiled with Borland C++ 5.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x520 : IBK::IBK_Message("Compiled with Borland C++ Builder 1.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x530 : IBK::IBK_Message("Compiled with Borland C++ Builder 3.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x540 : IBK::IBK_Message("Compiled with Borland C++ Builder 4.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x550 : IBK::IBK_Message("Compiled with Borland C++ Builder 5.0\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x551 : IBK::IBK_Message("Compiled with Borland C++ 5.51\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x562 : IBK::IBK_Message("Compiled with Borland C++ 5.6.4\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x570 : IBK::IBK_Message("Compiled with Borland C++ Builder 2006\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x590 : IBK::IBK_Message("Compiled with Borland C++ Builder 2007\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x613 : IBK::IBK_Message("Compiled with Borland C++ Builder 2009\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x621 : IBK::IBK_Message("Compiled with Borland C++ Builder 2010\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x630 : IBK::IBK_Message("Compiled with Borland C++ Builder XE\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x640 : IBK::IBK_Message("Compiled with Borland C++ Builder XE2\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x650 : IBK::IBK_Message("Compiled with Borland C++ Builder XE3\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		case 0x660 : IBK::IBK_Message("Compiled with Borland C++ Builder XE4\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD); break;
		default:
			IBK::IBK_Message( IBK::FormatString("Compiled with Borland C++, version ID          %1\n").arg(__BORLANDC__), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
#else
	#error "Add your compiler info here!"
#endif
}


} // namespace IBK

