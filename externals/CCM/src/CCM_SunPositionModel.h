/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the CCM Library.

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

#ifndef CCM_SunPositionModelH
#define CCM_SunPositionModelH

namespace CCM {

/*! A calculation class to compute the sun position relative to the observer.
*/
class SunPositionModel {
public:
	/*! Constructor. */
	SunPositionModel();

	/*! Calculates the solar azimuth angle in [rad] (0 means north, clockwise defined) and
		the solar elevation angle in [rad] (0 means sun is at horizon, negative values mean sun is below horizon).
		\param apparentSolarTimeInSeconds Apparent solar time in [s] since start of year, winter time, no leap years.
	*/
	void setTime(double apparentSolarTimeInSeconds);

	/*!	Observers latitude in [rad], may differ from station latitude. */
	double	m_latitude;
	/*! Observers longitude in [rad], may differ from station longitude. */
	double	m_longitude;

	/*! Solar azimuth angle in [rad], 0 means north, clockwise defined. */
	double	m_azimuth;
	/*! Solar elevation angle in [rad], 0 means sun is at horizon, negative values mean sun is below horizon. */
	double	m_elevation;
	/*! Sun declination in [rad] */
	double	m_declination;

};


} // namespace CCM

/*! \file CCM_SunPositionModel.h
	\brief Contains declaration of class CCM::SunPositionModel.
*/

#endif // CCM_SunPositionModelH
