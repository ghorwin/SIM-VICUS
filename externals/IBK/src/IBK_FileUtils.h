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

#ifndef IBK_FileUtilsH
#define IBK_FileUtilsH

#include <vector>
#include <string>
#include <cstring>

namespace IBK {

class Path;

/*! Extracts the ID number from a filename.
	\param fname A filename with format "fname_<xxx>.<ext>".
	\return Function returns 0 for invalid filenames or filename with different format.

	Example:
	\code
	string fname = "myfile_17.txt";
	// extract id number
	unsigned int id = extract_ID_from_filename(fname);
	// id is now 17.
	\endcode
*/
unsigned int extract_ID_from_filename(const IBK::Path & fname);


/*! Replaces the ID number in a filename.
	\param fname	A filename with format "fname_<xxx>.<ext>".
	\param newId	New ID for the given filename.
	\return			Returns the new filename with format "fname_<newId>.<ext>".

	Example:
	\code
	std::string fname = "myfile_17.txt";
	unsigned int newId = 100;
	// extract id with
	std::string newFName = replace_ID_in_filename(fname, newId);
	// fname is now "myfile_100.txt".
	\endcode
*/
IBK::Path replace_ID_in_filename( const IBK::Path & fname, const unsigned int newId );

/*! Tries to read N bytes in binary mode from file \a filename.
	This is the checked version.
	\param filename Name of the file.
	\param size Maximum number of bytes for reading.
	\param errmsg Error message.
	\return the vector which could be read.
*/
std::vector<unsigned char> read_some_bytes(const IBK::Path& filename, unsigned int size, std::string& errmsg);

/*! Reads ASCII file into string, not very performant, but simple.
	\param fname Name of the file.
	\return File content as string.
*/
std::string file2String(const IBK::Path & fname);

/*! Reads first line of an ASCII file into string.
	\param fname Name of the file.
	\return First line as string.

	Throws an exception if reading fails.
*/
std::string read_one_line(const IBK::Path & fname);

/*! Takes a filename in format 'file.ext?12' and splits this into 'file.exe' and a number converted to integer.
	\param fname The original filename.
	\param adjustedFileName If the file extension contains a '?', adjustedFileName will contain everything in front of the '?',
		otherwise it will contain the entire unmodified fname.
	\param number The number following the '?' (only modified if the parsing was successful).
	\return Returns true, if a number was successfully parsed.
*/
bool extract_number_suffix(const IBK::Path & fname, IBK::Path & adjustedFileName, int & number);


/*! Converts a sentence of bytes given by \a bytes to the given value.
	No check for vector size take place.
	\param bytes Vector for reading.
	\param value Value to be read from bytes
	\param begin Start position in bytes for reading
	\return position one after the readed value in bytes.
*/
template<typename T>
unsigned int bytes2value(const std::vector<unsigned char>& bytes, T& value, unsigned int begin = 0) {
	std::memcpy((unsigned char*)&value, (&bytes[0]) + begin, sizeof(T));
	return begin + sizeof(T);
}

/*! Returns the directory for storing user files.
	Each user must have write access to this directory.
	\return Directory as UTF8 encoded string according POSIX standard.
*/
IBK::Path userDirectory();

}  // namespace IBK


/*! \file IBK_FileUtils.h
	\brief Contains helper functions for file access on windows/posix systems.

*/
#endif // IBK_FileUtilsH
