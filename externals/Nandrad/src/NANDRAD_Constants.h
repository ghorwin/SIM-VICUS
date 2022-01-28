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

#ifndef NANDRAD_ConstantsH
#define NANDRAD_ConstantsH

namespace NANDRAD {

/*! Version number of the data model and project file. */
extern const char * const VERSION;
/*! Long version number of the data model and project file. */
extern const char * const LONG_VERSION;
/*! Release Date. */
extern const char * const RELEASE_DATE;

/*! defines an invalid id */
extern unsigned int INVALID_ID;

extern const char * XML_READ_ERROR;
extern const char * XML_READ_UNKNOWN_ATTRIBUTE;
extern const char * XML_READ_UNKNOWN_ELEMENT;
extern const char * XML_READ_UNKNOWN_NAME;

} // namespace NANDRAD

/*!
	\file NANDRAD_Constants.h
	\brief Contains global constants for the Nandrad data model.
*/

#endif // NANDRAD_ConstantsH
