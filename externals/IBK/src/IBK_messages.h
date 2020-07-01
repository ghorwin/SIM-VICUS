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

#ifndef IBK_messagesH
#define IBK_messagesH

#include <string>
#include "IBK_FormatString.h"
#include "IBK_MessageHandlerRegistry.h"

namespace IBK {

#define IBK_MessageFilter(x) 	if (IBK::MessageHandlerRegistry::instance().messageHandler()->consoleVerbosityLevel() >= (x))

/*! Avoid costly construction of messages that will be disregarded anyway because of too low console verbosity level.
	Use as drop-in replacement for IBK::IBK_Message, for example:
	\code
		// Mind the parantheses...
		IBK_FastMessage(IBK::VL_DEVELOPER)("message text", ...);
	\endcode
*/
#define IBK_FastMessage(x) 	if (IBK::MessageHandlerRegistry::instance().messageHandler()->consoleVerbosityLevel() >= (x))IBK::IBK_Message

/*! Prototype for the message function.
	The function will be called from certain IBK classes.
*/
inline void IBK_Message(const std::string& msg, msg_type_t t = MSG_PROGRESS, const char * func_id = NULL, int verbose_level = VL_ALL) {
#ifdef _OPENMP
	#pragma omp master
#endif
	MessageHandlerRegistry::instance().msg(msg, t, func_id, verbose_level);
}


/*! Prototype for the message function.
	The function will be called from certain IBK classes.
*/
inline void IBK_Message(const IBK::FormatString& msg, msg_type_t t = MSG_PROGRESS, const char * func_id = NULL, int verbose_level = VL_ALL) {
#ifdef _OPENMP
	#pragma omp master
#endif
	MessageHandlerRegistry::instance().msg(msg.str(), t, func_id, verbose_level);
}


/*! Indentor can be used to increase the message indentation level
	within local function scopes.
*/
class MessageIndentor {
public:
	/*! Constructor increases indentation. */
	MessageIndentor() {
		++(MessageHandlerRegistry::instance().messageHandler()->m_indentation);
	}
	/*! Destructor decreases indentation. */
	~MessageIndentor() {
		--(MessageHandlerRegistry::instance().messageHandler()->m_indentation);
	}
};

/*! Foreground and background colors. */
enum ConsoleColor {
	CF_BLACK 			= 0x00,
	CF_BLUE				= 0x01,
	CF_GREEN			= 0x02,
	CF_CYAN 			= 0x03,
	CF_RED				= 0x04,
	CF_MAGENTA			= 0x05,
	CF_YELLOW			= 0x06,
	CF_GREY				= 0x07,
	CF_DARK_GREY		= 0x08,
	CF_BRIGHT_BLUE		= 0x09,
	CF_BRIGHT_GREEN		= 0x0A,
	CF_BRIGHT_CYAN		= 0x0B,
	CF_BRIGHT_RED 		= 0x0C,
	CF_BRIGHT_MAGENTA	= 0x0D,
	CF_BRIGHT_YELLOW	= 0x0E,
	CF_WHITE			= 0x0F,
	CB_BLACK 			= 0x00,
	CB_BLUE				= 0x10,
	CB_GREEN			= 0x20,
	CB_RED 				= 0x30,
	CB_MAGENTA			= 0x40,
	CB_CYAN				= 0x50,
	CB_YELLOW			= 0x60,
	CB_GREY				= 0x70,
	CB_DARK_GREY		= 0x80,
	CB_BRIGHT_BLUE		= 0x90,
	CB_BRIGHT_GREEN		= 0xA0,
	CB_BRIGHT_RED 		= 0xB0,
	CB_BRIGHT_MAGENTA	= 0xC0,
	CB_BRIGHT_CYAN		= 0xD0,
	CB_BRIGHT_YELLOW	= 0xE0,
	CB_WHITE			= 0xF0
};

/*! Sets the color of the console text used for the next output.
	The color 'c' is a logical or of the foreground and background colors
	defined in the ConsoleColor enum. To set a bright yellow text color
	with a blue background use:
	\code
	set_console_text_color(CF_BRIGHT_YELLOW | CB_BLUE);
	\endcode
*/
void set_console_text_color(ConsoleColor c);

/*! \file IBK_messages.h
	\brief Contains declarations for the IBK_Message() functions and the class MessageIndentor, central include file
			for IBK-Message system.
*/

extern const char * const TERMINAL_CODES[16];

} // namespace IBK

#endif // IBK_messagesH
