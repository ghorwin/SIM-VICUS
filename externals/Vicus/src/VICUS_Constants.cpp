/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#include "VICUS_Constants.h"

namespace VICUS {

const char * const VERSION = "1.1";
const char * const LONG_VERSION = "1.1.0";

unsigned int INVALID_ID = 0xFFFFFFFF;

const double MIN_AREA_FOR_EXPORTED_SURFACES = 0.1;

const char * XML_READ_ERROR				= "Error in XML file, line %1: %2";
const char * XML_READ_UNKNOWN_ATTRIBUTE = "Unknown/unsupported attribute '%1' in line %2.";
const char * XML_READ_UNKNOWN_ELEMENT	= "Unknown/unsupported tag '%1' in line %2.";
const char * XML_READ_UNKNOWN_NAME		= "Name '%1' for tag '%2' in line %3 is invalid/unknown.";

const char * DATABASE_PLACEHOLDER_NAME			= "Database";
const char * USER_DATABASE_PLACEHOLDER_NAME		= "User Database";

const unsigned int SEGMENT_COUNT_ARC		= 15;
const unsigned int SEGMENT_COUNT_CIRCLE		= 15;
const unsigned int SEGMENT_COUNT_ELLIPSE	= 15;

// Multiplyer for different layers and their heights
const double Z_MULTIPLYER					= 0.000001;
// default line width
const double DEFAULT_LINE_WEIGHT			= 0.05;
// multiplier to apply to width of entities
const double DEFAULT_LINE_WEIGHT_SCALING	= 0.01;
// Default scaling of fonts in DXF
const double DEFAULT_FONT_SCALING			= 0.001;
const double DEFAULT_FONT_SIZE				= 0.2;
} // namespace VICUS

