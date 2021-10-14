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

#ifndef IBK_WaitOnExitH
#define IBK_WaitOnExitH

#include <iostream>
#include <cstdio>

#include "IBK_configuration.h"

namespace IBK {

/*! This is a simple class that calls the system command "pause" on windows
	systems to prevent automatic closing of console windows when run from
	Windows Explorer and similar.
	\code
	int main(int argc, char * argv[]) {
		IBK::WaitOnExit waitObj;

		// ... code ...
	} // at closing scope waitObj gets destroyed and calls system("pause") on Windows systems
	\endcode
*/
class WaitOnExit {
public:
	/*! Constructor.
		\param wait Optional argument that can be used to control the behavior of the object
					upon destruction.
	*/
	WaitOnExit(bool wait = true) : m_wait(wait) {}
	/*! Destructor, stops execution unless 'wait' is true. */
	~WaitOnExit() {
		// WARNING: on Unix-type systems there is not such thing as get-a-key-from-keyboard,
		//          since this all depends on the actual terminal emulator being used (or any other
		//          mechanism that feeds character input into applications). Also, solvers are most frequently
		//          started in the terminal window anyway, so we do not need a "wait for closing of terminal"
		//          feature.
		//          The best way to get such a functionality would be to call a wrapper shell script with
		//          a subsequent wait for keypress command once the application is finished.
#ifdef _WIN32
		if (m_wait) system("pause");
#endif // _WIN32
	}


	/*! If true, the "pause" command is issued on destruction of this object, if false, nothing happens. */
	bool m_wait;
};

}  // namespace IBK

/*! \file IBK_WaitOnExit.h
	\brief Contains the declaration of the class WaitOnExit.
*/

#endif // IBK_WaitOnExitH
