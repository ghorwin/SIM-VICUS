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

#ifndef IBK_VersionH
#define IBK_VersionH

#include <iosfwd>
#include <string>

namespace IBK {

/*! Provides version encoding/decoding used in ASCII and Binary files.

	The following constants need to be defined for each file to uniquely identify type and version of the file:
	- Major file version number, e.g. 3
	- Minor file version number, e.g. 5
	so that the total file version number is '3.5'.

	For unique file identification:
	- The first number of the magic header
	- The second number of the magic header

	Examples:
	\code
	// use case: read header from stream
	std::ifstream in("...");
	IBK::Version::readHeader(in, magicNumberFirstBinary, magicNumberSecondBinary, magicNumberFirstASCII, magicNumberSecondASCII,
							 isBinary, majorVersion, minorVersion);
	// throws Exception if reading fails, otherwise 'in' is positioned after 4-integer header
	// information is stored in arguments isBinary, majorVersion and minorVersion

	// use case: write header to file
	std::ofstream out("...");
	IBK::Version::writeHeader(out, magicNumberFirstBinary, magicNumberSecondBinary, magicNumberFirstASCII, magicNumberSecondASCII,
							  isBinary, majorVersion, minorVersion);
	\endcode

*/
class Version {
public:
	/*! Reads the first 4 integers from input stream and decodes version information.
		The information of the header is stored in the arguments that are passed by reference.
	*/
	static void read(std::istream & in, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
					 unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
					 bool & isBinary, unsigned int & majorVersion, unsigned int & minorVersion);

	/*! Encodes and writes a file header the first 4 integers from input stream and decodes version information.
	*/
	static void write(std::ostream & out, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
					  unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
					  bool isBinary, unsigned int majorVersion, unsigned int minorVersion);

	/*! Extracts the major and minor version number from a string with version (2 numbers) or long version (3 numbers) format.
		\code
		unsigned int major, minor;
		Version::extractMajorMinorVersionNumber("2.4", major, minor); // returns true, and 2 and 4 as numbers in major and minor
		Version::extractMajorMinorVersionNumber("2.4.3", major, minor); // returns true, and 2 and 4 as numbers in major and minor
		Version::extractMajorMinorVersionNumber("2.", major, minor); // returns false, values of major and minor are undefined
		\endcode
		\return Returns true, if both numbers could be extracted, otherwise returns false.
	*/
	static bool extractMajorMinorVersionNumber(const std::string & versionString, unsigned int & major, unsigned int & minor);

	/*! Extracts the major and minor and patch version number from a string with long version (3 numbers) format.
		\code
		unsigned int major, minor, patch;
		Version::extractMajorMinorPatchVersionNumber("2.4.3", major, minor, patch); // returns true, and 2, 4 and 3 as numbers in major, minor and patch
		Version::extractMajorMinorPatchVersionNumber("2.4.3.3131", major, minor, patch); // returns true, and 2, 4 and 3 as numbers in major, minor and patch
		Version::extractMajorMinorPatchVersionNumber("2.4", major, minor, patch); // returns false, values of major and minor are undefined
		\endcode
		\return Returns true, if all three numbers could be extracted, otherwise returns false.
	*/
	static bool extractMajorMinorPatchVersionNumber(const std::string & versionString, unsigned int & major, unsigned int & minor, unsigned int & patch);

	/*! Compares two version number strings, so that "4.3.2" is smaller than "4.5.1" and
		lesserVersionNumber("4.3.2", "4.5.1") returns true.
	*/
	static bool lesserVersionNumber(const std::string & lhs, const std::string & rhs);

	/*! Prints the version number of the currently used compiler as IBK_Message. */
	static void printCompilerVersion();

private:
	/*! Decodes a combined single unsigned integer number into two version numbers.
		\param version	Takes version for encoding in ASCII encoded file.
		\param first	Returns first version number part from an ASCII encoded file.
		\param second	Returns second version number part from an ASCII encoded file.
	*/
	static void toASCIIEncoding( unsigned int version, unsigned int &first, unsigned int &second );

	/*! Encodes two version numbers into a single unsigned integer version number.
		\param first	Takes first version number part from an ASCII encoded file.
		\param second	Takes second version number part from an ASCII encoded file.
		\return			Version in ASCII encoding format.
	*/
	static unsigned int fromASCIIEncoding( unsigned int first, unsigned int second );

}; // Version

} // namespace IBK

/*! \brief Contains the class Version with version encoding/decoding functionality.
	\file IBK_Version.h
*/

#endif // IBK_VersionH
