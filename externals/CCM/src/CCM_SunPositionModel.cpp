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

#include <cmath>

#include "CCM_SunPositionModel.h"
#include "CCM_Constants.h"

#include "CCM_Defines.h"	// include this file last

namespace CCM {


SunPositionModel::SunPositionModel() :
	m_latitude(51.1 * DEG2RAD),
	m_longitude(13.7 * DEG2RAD)
{
	setTime(0);
}


void SunPositionModel::setTime(double apparentSolarTimeInSeconds) {


	// time in days in [d]
	double td = apparentSolarTimeInSeconds/(24.0*3600);

	// *** Declination ***

//#define DECLINATION_SPENCER
#ifdef DECLINATION_SPENCER
	//Spencer equation - Spencer 1971, referenced in Kalogirou "Solar Energy Engineering" 2009  equ. no. 2.6 and 2.7
	//result of this equation is angle in rad
	double dayAngle = 2.0 * PI * td / 365.0;
	double dec1 = 0.006918 - 0.399912 * cos(dayAngle) + 0.070257 * sin(dayAngle);
	double dec2 = 0.006758 * cos(2 * dayAngle) + 0.000907  * sin (2 * dayAngle);
	double dec3 = 0.002697 * cos(3 * dayAngle) + 0.00148 * sin(3 * dayAngle);
	m_declination = dec1 - dec2 - dec3;

#else // DECLINATION_SPENCER
	//ASHRAE equation 2007, Ashrae handbook
	m_declination = DEG2RAD * 23.45 * std::sin( 2 * PI * (td + 284.0 )/ 365.0);
	//referenced by Haeupl, source unknown
	//double dec = IBK::DEG2RAD() * -1 * 23.5 * sin( 2 * PI / 365.0 * (td + 10 + (365.0 / 4.0) )) ;
	//           = IBK::DEG2RAD() * -1 * 23.5 * sin( 2 * PI * (td + 101.25 )/ 365.0);
	//           = IBK::DEG2RAD() * 23.5 * sin( 2 * PI * (td + 281.25 )/ 365.0);
#endif // DECLINATION_SPENCER

	// *** Elevation ***

	// time of day [d] (0...1)
	double t_d = td - std::floor(td);

	//modified solar altitude equation, source: Kalogirou  - Solar Engineering, 2007, elsevier pub. p.58
	// original: double hourAngle = IBK::DEG2RAD()*(((t_d * 24.0) - 12.0)/24)*360;
	// simplified equation
	double hourAngle = 2*PI*(t_d - 0.5);
	double sin_hs = std::sin(m_latitude)*std::sin(m_declination) + std::cos(m_latitude)*std::cos(m_declination)*std::cos(hourAngle);

	m_elevation = std::asin(sin_hs);



	// *** Azimuth ***


#define ANDREAS_AZIMUTH
#ifdef NAVY_AZIMUTH
	// original azimuth equation: http://aa.usno.navy.mil/faq/docs/Alt_Az.php
	// tan A = -sin(LHA)/[tan(dec)cos(xhi)-sin(xhi)cos(LHA)]
	double hourAngle = 2*PI*(t_d - 0.5);
	m_azimuth = atan2(-sin(hourAngle),
						tan(dec)*cos(m_latitude)-sin(m_latitude)*cos(hourAngle));
	if (m_azimuth < 0)
		m_azimuth += 2*PI;

#elif defined(ANDREAS_AZIMUTH)
	// same as equations above, but with different definition of hour angle, and tan(dec) split up
	// and rearranged, equation results in same azimuth angles as above equation
	m_azimuth = std::atan2(std::sin(2*PI*t_d)*std::cos(m_declination),
						  std::sin(m_latitude)*std::cos(2*PI*t_d)*std::cos(m_declination) + std::sin(m_declination)*std::cos(m_latitude));
	if (m_azimuth < 0)
		m_azimuth += 2*PI;
#else
	//Ashrae, 1975 referenced by Kalogirou - Solar Engineering, 2007, elsevier pub. p.58
	//equation for angles within east- west line range, condition requested
	//double altitude = IBK::DEG2RAD() * elevation(t);
	double altitude = m_elevation;
	double hourAngle = IBK::DEG2RAD()*(((t_d * 24.0) - 12.0) * 15.0);
	double zetha = std::asin( std::cos(m_declination) * std::sin(hourAngle) / std::cos(altitude));
	if ( cos(hourAngle) <= std::tan(m_declination)/std::tan(m_latitude) ){
		//morning hours, angle values negative
		if (t_d <= 0.5){
			zetha =  std::fabs(zetha) - PI ;
		}
		//afternoon hours, angle values positive
		else {
			zetha =  PI - zetha;
		}
	}
	//transfer angle range to original one (north = 0 deg, positive eastwards)
	zetha += PI;
	m_azimuth = zetha;
#endif

}

} // namespace CCM


