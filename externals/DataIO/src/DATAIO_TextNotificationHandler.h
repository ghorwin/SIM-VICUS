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

#ifndef DATAIO_TextNotificationHandlerH
#define DATAIO_TextNotificationHandlerH

#include <string>

#include <IBK_NotificationHandler.h>
#include <IBK_StopWatch.h>

namespace DATAIO {

/*! Standard implementation of a notification handler to be used by
	DATAIO input/output routines.

	It prints a progress bar for operations that know the length of
	the operation (like reading a file), or an animated character.
	Setting the parameter m_quiet to true disables all output to console.

	You can use this notification handler also in own code. When calling
	the notify() function without argument, it prints the string in
	m_process followed by a character that changes in sequence with each
	subsequent call.

	Calling notify with a number between 0 and 1 prints the string in
	m_process and afterwards a progress bar, if there is enough space
	(line length is limited to 75 characters).
*/
class TextNotificationHandler : public IBK::NotificationHandler {
public:
	/*! Constructor, initializes counter to zero. */
	TextNotificationHandler() : m_quiet(false), m_counter(0) {
		m_watch.start();
	}
	/*! Prints general progress info. */
	virtual void notify();
	/*! Prints progress bar.
		\param percentage A value between 0 and 1 indicating the progress of the operation.
	*/
	virtual void notify(double percentage);

	/*! Output string to indicate the actual used process. */
	std::string m_process;

	/*! If true, notification messages are disabled (defaults to false). */
	bool m_quiet;

private:
	/*! Counter used to draw rotating dash. */
	unsigned int m_counter;
	/*! Used to skip messages that are sent to frequently. */
	IBK::StopWatch m_watch;
};

} // namespace DATAIO

/*! \file DATAIO_TextNotificationHandler.h
	\brief Contains declaration of class TextNotificationHandler, a console progress message implementation.
*/

#endif // DATAIO_TextNotificationHandlerH
