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

#ifndef IBK_configurationH
#define IBK_configurationH

#include "IBK_BuildFlags.h"

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
#elif _GLIBCXX_HAVE_STDINT_H
	#include <stdint.h>
#endif

// is C++11 supported?

// We have to test explicitely for VC compiler since they still have the bug
// of reporting an invalid __cplusplus constant.
#if (_MSC_VER >= 1900) || (__cplusplus > 199711L)
	// do not mess with C++11 keywords
#else
// C++ 2003 or older

	// define override to nothing - keyword will be removed by preprocessor
	#define override
#endif

/// Enables/Disables function argument checking for all math functions.
#define IBK_ENABLE_SAFE_MATH

/// Enables/Disables colored text in the console window.
#define IBK_ENABLE_COLORED_CONSOLE

#ifdef IBK_DEBUG

/// Enables/Disables name string in IBK::Unit (for debugging)
#define IBK_ENABLE_UNIT_NAME

#endif // IBK_DEBUG

/*! \file IBK_configuration.h
	\brief Central configuration file for IBK library, include in all header/source files that
			make use of the IBK macros and defines.

	Configuration header files for IBK library
	------------------------------------------

	If you don't use the CMake build system, copy this file over to
	IBK_configuration.h and manually define all options you want to have
	enabled.

	This file should be included into every header and source file of the
	IBK lib where compilation options/macros are used.

	\warning This header file is generated from IBK_configuration.h.in, so any changes made in this file will
	be lost in the next cmake configuration.

*/

/*! \file IBK_configuration.h
	\brief Compilation configurations and macros for CMake build system.
*/

#endif // IBK_configurationH
