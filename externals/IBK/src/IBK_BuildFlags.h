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

#ifndef IBK_BuildFlagsH
#define IBK_BuildFlagsH

#if defined(_MSC_VER)

	#if defined(_DEBUG)
		#define IBK_DEBUG
	#else
		#undef IBK_DEBUG
	#endif // _DEBUG

#elif defined(__BORLANDC__)

	#ifdef _DEBUG
		#define IBK_DEBUG
	#else
		#undef IBK_DEBUG
	#endif // _DEBUG

#elif defined(__GNUC__)

	// On Linux/Mac, the NDEBUG define is set when gcc runs in release or releaseWithDebInfo mode.
	// For Qt-creator builds, we set this in IBK.pri, so standard projects should be fine.
	#if !defined(NDEBUG)
		#define IBK_DEBUG
	#else
		#undef IBK_DEBUG
	#endif // _DEBUG

#else
	#error Define this for your compiler
#endif

#ifdef IBK_DEBUG
	#undef IBK_DEPLOYMENT
#else
/*! If defined, all applications/libraries based on IBK library will build in Deployment mode.
	\warning All files accessing the IBK_DEPLOYMENT flag need to include this file!!!

	!!!! This flag must only be modified by Stefan and/or Andreas !!!!!

	This flag must not be committed in enabled state to the subversion repository.
*/

// DO NOT CHANGE THE FORMAT OF THE FOLLOWING LINE, IT IS RECOGNIZED BY THE RELEASE SCRIPTS
//#define IBK_DEPLOYMENT

#endif // IBK_DEBUG


/*! \file IBK_BuildFlags.h
	\brief Build-related defines and macros.
*/

#endif // IBK_BuildFlagsH
