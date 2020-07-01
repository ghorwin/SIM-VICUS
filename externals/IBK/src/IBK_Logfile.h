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

#ifndef IBK_LogfileH
#define IBK_LogfileH

#include <string>
#include <iosfwd>
#include <fstream>

namespace IBK {

/*! The class Logfile allows the redirection of standard streams into file
	streams.
	This class is a convinient way of using standard streams such as clog or
	cerr into log- or error files. As long as the redirection object exists
	all output to the redirected streams will be written into the files
	instead. With the member function redirect() the file can be changed or
	logging can be turned off (useful if logging can be turned on/off during
	runtime).
*/
class Logfile {
  public:
	/*! Constructor, creates an object for stream redirection
		Creates, overwrites or appends the file 'filename' depending on the
		'filename'. All further output to the standard stream 'original'
		will be written into the logfile instead. This redirection works until
		the destruction of the logfile object or a call to the member function
		redirect().
		\param original  may be either of cout, cerr or clog
		\param filename  filename of the log file (ansi encoded)
		\param ioflags   (optional) open flags, see ios_base::openmode.
						 [default: overwrite]
	*/
	Logfile(std::ostream& original, const std::string& filename,
			std::ios_base::openmode ioflags=std::ios_base::out | std::ios_base::trunc);

	/*! Destructor.
		When the logfile object is destructed the redirection will be removed.
	*/
	~Logfile();

	/*! Removes the stream redirection.
		In combination with the other redirect() member function this can be
		used for turning logging on/off turing runtime.
	*/
	void redirect();

	/*! Redirects the stream into another log file.
		This function is useful for changing the log file of a certain
		redirected stream. Use this function if you want to change the logfile
		name (e.g. if you change the file name daily) but want to keep the
		logfile object. If you turned logging off using the other redirect()
		member function previously this function can be used to turn the logging
		back on.
		\param filename  filename of the logfile
		\param ioflags   (optional) open flags, see ios_base::openmode.
						 [default: overwrite]
	*/
	void redirect(const std::string& filename, std::ios_base::openmode ioflags=std::ios_base::out | std::ios_base::trunc);

private:
	std::basic_ostream<char>*   m_original;      // original buffer
	std::basic_streambuf<char>* m_buffer;        // file stream buffer
	std::basic_ofstream<char>   m_filestream;    // file stream
};

/*! \file IBK_Logfile.h
	\brief Contains the class Logfile for redirecting standard stream output to log files.

	\example Logfile.cpp
	This is an example of how to use the class IBK::Logfile.
*/

} // namespace IBK

#endif // IBK_LogfileH
