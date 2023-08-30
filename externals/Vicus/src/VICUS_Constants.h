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

#ifndef VICUS_ConstantsH
#define VICUS_ConstantsH

namespace VICUS {

/*! Version number of the data model and project file. */
extern const char * const VERSION;
/*! Long version number of the data model and project file. */
extern const char * const LONG_VERSION;

/*! Defines an invalid id */
extern unsigned int INVALID_ID;

/*! Minimum area for export of surfaces. */
extern const double MIN_AREA_FOR_EXPORTED_SURFACES;

extern const char * XML_READ_ERROR;
extern const char * XML_READ_UNKNOWN_ATTRIBUTE;
extern const char * XML_READ_UNKNOWN_ELEMENT;
extern const char * XML_READ_UNKNOWN_NAME;

extern const char * DATABASE_PLACEHOLDER_NAME;
extern const char * USER_DATABASE_PLACEHOLDER_NAME;

const double MAX_SEGMENT_ARC_LENGHT			= 30;
const unsigned int SEGMENT_COUNT_CIRCLE		= 30;
const unsigned int SEGMENT_COUNT_ELLIPSE	= 30;

// Multiplyer for different layers and their heights
const double Z_MULTIPLYER					= 0.00005;
// default line width
const double DEFAULT_LINE_WEIGHT			= 0.05;
// multiplier to apply to width of entities
const double DEFAULT_LINE_WEIGHT_SCALING	= 0.005;

#define VICUS_PLANE_PROJECTION_TOLERANCE 1e-3

} // namespace VICUS

/*!
	\file VICUS_Constants.h
	\brief Contains global constants for the SIM-VICUS data model.
*/

#endif // VICUS_ConstantsH
