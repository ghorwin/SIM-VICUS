/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include  "SVConstants.h"

const char * const ORG_NAME			= "IBK";

const char * const UPDATE_FILE_URL	= "...";
const char * const NEWS_FILE_URL	= "...";
const char * const BUG_REPORT_URL	= "https://github.com/ghorwin/SIM-VICUS/issues";
const char * const FORUM_URL		= "https://github.com/ghorwin/SIM-VICUS";
const char * const MANUAL_URL		= "https://ghorwin.github.io/SIM-VICUS";

const char * const THIRD_LANGUAGE	= "fr";

const char * const SUPPORT_EMAIL	= "sim-vicus@listserv.dfn.de";

const int AUTO_SAVE_INTERVALL		= 300000;

#if defined(Q_OS_MAC) // Q_OS_UNIX

const char * const FIXED_FONT_FAMILY = "Monaco";

#elif defined(Q_OS_UNIX)

const char * const FIXED_FONT_FAMILY = "Monospace";

#else

const char * const FIXED_FONT_FAMILY = "Courier New";

#endif

