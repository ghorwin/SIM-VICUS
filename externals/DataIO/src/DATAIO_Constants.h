/*	DataIO library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, St. Vogelsang
	All rights reserved.

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
*/

#ifndef DATAIO_ConstantsH
#define DATAIO_ConstantsH

namespace DATAIO {

/*! Version number of the API. */
extern const char * const VERSION;

/*! Long version number of the API. */
extern const char * const LONG_VERSION;

/*! Major revision number (for file format) */
extern const unsigned int MAJOR_FILE_VERSION;

/*! Minor revision number (for file format)  */
extern const unsigned int MINOR_FILE_VERSION;

/*! Magic header for the binary DataIO ile. */
extern const unsigned int MAGIC_NUMBER_BINARY_DATAIO;

/*! Magic header for ASCII DataIO file. */
extern const unsigned int MAGIC_NUMBER_ASCII_DATAIO;

/*! Magic header for binary geometry file. */
extern const unsigned int MAGIC_NUMBER_BINARY_GEOFILE;

/*! Magic header for ASCII geometry file. */
extern const unsigned int MAGIC_NUMBER_ASCII_GEOFILE;

/*! Secondary magic header (same for DataIO and geometry files). */
extern const unsigned int SECOND_MAGIC_NUMBER;

/*! Constant used to identify an invalid start year (Delphin 5 output files). */
extern const int INVALID_START_YEAR;

} // namespace DATAIO

/*! \file DATAIO_Constants.h
	\brief Contains library and file version constants.
*/

#endif // DATAIO_ConstantsH
