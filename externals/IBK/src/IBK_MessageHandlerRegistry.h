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

#ifndef IBK_MessageHandlerRegistryH
#define IBK_MessageHandlerRegistryH

#include <string>
#include "IBK_Constants.h"
#include "IBK_MessageHandler.h"

namespace IBK {


/*! This class relays messages (error, debug or progress messages)
	from the IBK library towards the current message handling class.
	You can set a message handler via setMessageHandlerRegistry(). The object
	does not get owned by the MessageHandlerRegistry singleton.
*/
class MessageHandlerRegistry {
public:

	/*! Destructor, reset the default message receiver before exiting. */
	~MessageHandlerRegistry();

	/*! Returns a singleton instance to the current IBK message handler. */
	static MessageHandlerRegistry& instance();

	/*! Relays a message with given type to the message handler.
		\sa IBK_Message.
		*/
	void msg(	const std::string& msg,
				msg_type_t t = MSG_PROGRESS,
				const char * func_id = NULL,
				int verbose_level = VL_ALL)
	{
		m_msgHandler->msg(msg, t, func_id, verbose_level);
	}

	/*! Resets the default message handler. */
	void setDefaultMsgHandler();

	/*! Sets a message handler instance. */
	void setMessageHandler(MessageHandler * handle);

	/*! Returns the message handler instance. */
	MessageHandler * messageHandler() { return m_msgHandler; }

private:
	/*! Singleton - Constructor hidden from public. */
	MessageHandlerRegistry();

	/*! Singleton - Copy constructor hidden and not implemented. */
	MessageHandlerRegistry(const MessageHandlerRegistry&);

	/*! Singleton - Copy assignment operator hidden and not implemented. */
	MessageHandlerRegistry& operator=(const MessageHandlerRegistry&);

	MessageHandler				m_defaultMsgHandler;	///< Default message handler object.
	MessageHandler *			m_msgHandler;			///< Pointer to current message handler object.
};

} // namespace IBK

/*! \file IBK_MessageHandlerRegistry.h
	\brief Contains declaration of class IBK_MessageHandlerRegistry
*/

#endif // IBK_MessageHandlerRegistryH
