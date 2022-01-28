/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_Constants.h"

namespace NANDRAD {

const char * const VERSION			= "2.0";
const char * const LONG_VERSION		= "2.0.0";
const char * const RELEASE_DATE		= "January 23rd, 2022";

unsigned int INVALID_ID = 0xFFFFFFFF;

const char * XML_READ_ERROR = "Error in XML file, line %1: %2";
const char * XML_READ_UNKNOWN_ATTRIBUTE = "Unknown/unsupported attribute '%1' in line %2.";
const char * XML_READ_UNKNOWN_ELEMENT = "Unknown/unsupported tag '%1' in line %2.";
const char * XML_READ_UNKNOWN_NAME = "Name '%1' for tag '%2' in line %3 is invalid/unknown.";

} // namespace NANDRAD

