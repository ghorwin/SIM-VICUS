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

#include "IBK_Constants.h"

namespace IBK {

const unsigned int INVALID_ELEMENT = 0xFFFFFFFF;

const char * const VERSION = "4.2.4";


const char * const  MONTH_NAMES[12]         = { "Jan", "Feb", "Mar", "Apr",
												"May", "Jun", "Jul", "Aug",
												"Sep", "Oct", "Nov", "Dec"};

unsigned int SECONDS_PER_MONTH[12] 			= { 31*86400, 28*86400, 31*86400, 30*86400,
												31*86400, 30*86400, 31*86400, 31*86400,
												30*86400, 31*86400, 30*86400, 31*86400 };

unsigned int SECONDS_UNTIL_MONTH[12]	= { 0, 31*86400, (31+28)*86400, (31+28+31)*86400,
												(31+28+31+30)*86400, (31+28+31+30+31)*86400,
												(31+28+31+30+31+30)*86400, (31+28+31+30+31+30+31)*86400,
												(31+28+31+30+31+30+31+31)*86400, (31+28+31+30+31+30+31+31+30)*86400,
												(31+28+31+30+31+30+31+31+30+31)*86400, (31+28+31+30+31+30+31+31+30+31+30)*86400 };


const unsigned int SECONDS_PER_YEAR = 31536000;

const unsigned int SECONDS_PER_DAY = 86400;

const unsigned int USER_ID_START = 1 << 11;

const unsigned int VOID_ID = (unsigned int)(-1);


const char * const PLACEHOLDER_PROJECT_DIR = "Project Directory";

const char * const PLACEHOLDER_INSTALL_DIR = "Install Directory";

const char * const PLACEHOLDER_MATERIALS_DIR = "Material Database";

const char * const PLACEHOLDER_CLIMATE_DIR = "Climate Database";

} // namespace IBK

