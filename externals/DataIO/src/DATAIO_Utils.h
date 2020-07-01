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

#ifndef DATAIO_UtilsH
#define DATAIO_UtilsH

#include "DATAIO_DataIO.h"

namespace DATAIO {

class GeoFile;

/*! Dumps all header meta information from a DataIO container file to std::cout.
	\param dataIO DataIO container to extract header data from.
	\param geofile Associated geometry container.
*/
void listDataIOHeader( const DataIO & dataIO, GeoFile & geofile);

/*! Dumps all header meta information from a DataIO container file to std::cout.
	This is actually a convenience function that first reads DataIO container and GeoFile
	referenced from DataIO and then calls listDataIOHeader(dataIO, geofile).
	\param inputFilePath File path to DataIO container to extract header data from.
*/
void listDataIOHeader(const IBK::Path & inputFilePath );

} // namespace DATAIO

/*! \file DATAIO_Utils.h
	\brief Contains declarations for utility functions that show/list contents of DataIO containers.
*/

#endif // DATAIO_UtilsH
