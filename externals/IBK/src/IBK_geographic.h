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

	Based on:
	IOGP Publication 373-7-2 – Geomatics Guidance Note number 7, part 2 – September 2019
	To facilitate improvement, this document is subject to revision.
	The current version is available at www.epsg.org
	https://www.bbsr.bund.de/BBSR/DE/FP/ZB/Auftragsforschung/5EnergieKlimaBauen/2013/testreferenzjahre/01-start.html?nn=436654&notFirst=true&docId=1595620


*/

#ifndef IBK_geographicH
#define IBK_geographicH

namespace IBK {

/*! Transform WSG84 values (longitude and latitude) in lambert projection values (north and east).
	\param longitude Longitude in Deg
	\param latitude Latitude in deg
	\param lambertEast East coordinate for Lambert projection in m
	\param lambertNorth East coordinate for Lambert projection in m
*/
void transformWSG84ToLambertProjection(double longitude, double latitude, double& lambertEast, double& lambertNorth);

/*! Transform lambert projection values (north and east) in WSG84 values (longitude and latitude).
	\param lambertEast East coordinate for Lambert projection in m
	\param lambertNorth East coordinate for Lambert projection in m
	\param longitude Longitude in Deg
	\param latitude Latitude in deg
*/
void transformLambertProjectionToWSG84(double lambertEast, double lambertNorth, double& longitude, double& latitude);

} // end namespace

#endif // IBK_geographicH
