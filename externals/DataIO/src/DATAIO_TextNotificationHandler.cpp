/*	DataIO library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, St. Vogelsang
	All rights reserved.

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
*/

#include "DATAIO_TextNotificationHandler.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <IBK_messages.h>

namespace DATAIO {

/*! Characters shown as part of the animation for notification calls without percentage argument. */
char CHARS[4] = { '/', '-', '\\', '|' };

void TextNotificationHandler::notify() {
	if (m_quiet) return;
	m_counter = (m_counter + 1) % 4;
	std::stringstream strm;
	strm << '\r' << m_process << CHARS[m_counter];
	IBK::IBK_Message(strm.str(), IBK::MSG_CONTINUED, "[TextNotificationHandler::notify]", IBK::VL_STANDARD);
}


void TextNotificationHandler::notify(double percentage) {
	if (m_quiet) return;
	if (m_watch.difference() < 50)
		return;
	m_watch.start();
	std::stringstream strm;
	strm << '\r' << m_process;
	int percent = (int)(100*percentage);
	int barLength = (int)(70 - m_process.size()); // might be negative
	int completed = (int)((70 - m_process.size())*percentage); // might be negative
	strm << std::setw(3) << std::right << std::fixed << std::setprecision(0) << percent << "% ";
	if (barLength > 0) {
		std::string msg = "[";
		for (int i=0; i<barLength; ++i) {
			if (i < completed)
				msg += '#';
			else
				msg += ' ';
		}
		msg += ']';
		strm << msg;
	}
	IBK::IBK_Message(strm.str(), IBK::MSG_CONTINUED, "[TextNotificationHandler::notify]", IBK::VL_STANDARD);
}


} // namespace DATAIO
