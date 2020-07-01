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

#include "IBK_configuration.h"

#include <iostream>
#include <fstream>
#include <streambuf>

#include "IBK_Logfile.h"
#include "IBK_messages.h"
#include "IBK_FormatString.h"

namespace IBK {

// Constructor
Logfile::Logfile(std::ostream& original, const std::string& filename, std::ios_base::openmode ioflags)
	: m_original(NULL), m_buffer(NULL)
{
	m_filestream.open(filename.c_str(), ioflags);
	if (!m_filestream) {
		IBK::IBK_Message( FormatString("Couldn't create file '%1'.") .arg(filename), MSG_ERROR, "[LogFile::LogFile]", 3);
		return;
	}
	m_original=&original;
	m_buffer=original.rdbuf();
	original.rdbuf(m_filestream.rdbuf());
};

// Destructor
Logfile::~Logfile() {
	if ((m_original!=NULL) && (m_buffer!=NULL))
		m_original->rdbuf(m_buffer);
}

// Turns logging off
void Logfile::redirect() {
	m_original->rdbuf(m_buffer);
}

// Turns logging on
void Logfile::redirect(const std::string& filename, std::ios_base::openmode ioflags) {
	// if there is already a file opened, close it first
	if (m_filestream)
		m_filestream.close();
	// open new logfile
	m_filestream.open(filename.c_str(), ioflags);
	if (!m_filestream) {
		IBK::IBK_Message(FormatString("Couldn't create file '%1'.") .arg(filename), MSG_ERROR, "[Logfile::redirect]", 3);
		return;
	}
	if ((m_original!=NULL) && (m_buffer!=NULL))
		m_original->rdbuf(m_filestream.rdbuf());
}

} // namespace IBK

