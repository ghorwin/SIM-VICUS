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

#include <fstream>
#include <sstream>
#include <iostream>

#include "IBK_messages.h"
#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"

#if defined (IBK_ENABLE_COLORED_CONSOLE) && (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__))
  #include <windows.h>
  #include <cstdio>
#endif

namespace IBK {

/// All terminal codes used for ANSI color writing.
extern const char * const TERMINAL_CODES[16];

void set_console_text_color(ConsoleColor c) {
#ifdef IBK_ENABLE_COLORED_CONSOLE

#if (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__))
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
	SetConsoleTextAttribute(hConsole,static_cast<WORD>(c));
#else
	// terminal codes > 16 are bold fonts and must be mapped accordingly
	int i = static_cast<int>(c);
	if (i>=16)
		i = i >> 1; // also set bold flag?
	std::cerr << TERMINAL_CODES[i];
	std::cout << TERMINAL_CODES[i];
#endif

#else // IBK_ENABLE_COLORED_CONSOLE
	(void)c;
#endif // IBK_ENABLE_COLORED_CONSOLE
}

extern const char * const TERMINAL_CODES[16] =
{
	"\033[22;30m", // black			CF_BLACK
	"\033[22;34m", // blue 			CF_BLUE
	"\033[22;32m", // green 		CF_GREEN
	"\033[22;36m", // cyan 			CF_CYAN
	"\033[22;31m", // red			CF_RED
	"\033[22;35m", // magenta		CF_MAGENTA
	"\033[22;33m", // brown			CF_YELLOW
	"\033[22;37m", // gray			CF_GREY
	"\033[01;30m", // dark gray 	CF_DARK_GREY
	"\033[01;34m", // light blue	CF_BRIGHT_BLUE
	"\033[01;32m", // light green	CF_BRIGHT_GREEN
	"\033[01;36m", // light cyan	CF_BRIGHT_CYAN
	"\033[01;31m", // light red		CF_BRIGHT_RED
	"\033[01;35m", // light magenta	CF_BRIGHT_MAGENTA
	"\033[01;33m", // yellow		CF_BRIGHT_YELLOW
	"\033[01;37m"  // white			CF_WHITE
};


}  // namespace IBK

