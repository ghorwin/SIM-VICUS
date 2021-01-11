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

#ifndef CCM_SolarRadiationModelH
#define CCM_SolarRadiationModelH

#include <utility>

#include "CCM_ClimateDataLoader.h"
#include "CCM_SunPositionModel.h"

namespace CCM {

/*! A calculation class to compute the solar radiation on a given surface near the observer (building).

	The class utilizes the functionality of the ClimateDataLoader and SunPosition. These members
	have to be initialized correctly before using the SolarRadiationModel.

	The class computes short wave radiation loads (direct and diffuse) normal to the surface. Also,
	the incidence angle is computed as angle between the surface's normal and the sun's normal.

	Primary access function is setTime(), which takes the local time (in the time zone) of the
	observer and updates all computed quantities to this time point. Indirectly, the states of the
	members m_sunPositionModel and m_climateDataLoader are also updated. The time correction
	(local time to apparent solar time) for the sun position model is done automatically within
	the setTime() function.
*/
class SolarRadiationModel {
public:

	/*! Identifier for synthetic climate model we use as an upper
		limit for normal radiation. This option is needed for the conversion
		of horizontal to normal solar radiation*/
	enum ClimateConversionModel {
		ASHRAE_ClearSky,
		None
	};

	/*! Constructor. */
	SolarRadiationModel();

	/*! Calculates the solar azimuth angle in [rad] (0 means north, clockwise defined) and
		the solar elevation angle in [rad] (0 means sun is at horizon, negative values mean sun is below horizon).
		\param year Year as integer number (only needed for non-cyclic climate data).
		\param secondsOfYear Local standart time in [s] (of selected time zone, ie the time on a clock in the building)
							 since start of year, winter time, no leap years.
	*/
	void setTime(int year, double secondsOfYear);

	/*! Adds a new surface to compute radiation loads on.
		\param orientation Orientation of surface (direction of normal vector of surface) in [rad], defined clockwise, 0 points north.
		\param inclination Inclination of surface in [rad], flat roof has 0 rad (normal vector points upwards),
						  wall has PI/2  (normal vector parallel to ground), surface facing downwards has inclination angle of PI.
		\return Function returns an index of the surface that was just added, or the index of
				an existing surface with same orientation and inclination.
	*/
	unsigned int addSurface(double orientation, double inclination);

	/*! Returns the computed radiation load for the given surface.
		\param surfaceID The index/ID of a previously added surface.
		\param qRadDir The direct solar radiation in [W/m2] normal to the surface is stored in this variable.
		\param qRadDif The diffuse solar radiation in [W/m2] normal to the surface is stored in this variable.
		\param incidenceAngle The angle of incidence of the sun onto the surface in [rad] is stored in this variable.
		\warning Function throws an IBK::Exception if surfaceID does not exist (if surfaceID >= size of m_surfaces).
		\code
		// the call to radiationLoad()
		radModel->radiationLoad(15, qRadDir, qRadDif, incidenceAngle);
		// just simplifies the code, also member vectors are private and may not be altered except in setTime()
		qRadDir = radModel->m_qRadDir[15]; // not possible
		qRadDif = radModel->m_qRadDif[15]; // not possible
		incidenceAngle = radModel->m_incidenceAngle[15]; // not possible
		\endcode
	*/
	void radiationLoad(unsigned int surfaceID, double & qRadDir, double & qRadDif, double & incidenceAngle) const;

	/*! Returns cached local mean time and apparent solar time. */
	void convertedTimes(double & localMeanTime, double & apparentSolarTime) const { localMeanTime = m_localMeanTime; apparentSolarTime = m_apparentSolarTime; }

	/*! Conversion functionality for horizontal direct radiation into normal direct radiation. It performs
		a limitation of direct radiation values for a sun elevation angle that is close to zero.
		The function is called from ClimateDataLoader in the case of CCD-data for each measurement time point.
		\param secondsOfYear local standard time at the weather station in [s] since start of year.
		\param qRadDirHorizontal The direct solar radiation in [W/m2] on a horizontal surface.
		\param qRadDirNormal The direct solar radiation in [W/m2] normal to the surface is stored in this variable.
	*/
	void convertHorizontalToNormalRadiation(double secondsOfYear, double qRadDirHorizontal, double &qRadDirNormal);

	/*! Conversion functionality for normal direct radiation into horizontal direct radiation.
		\param secondsOfYear local standard time at the weather station in [s] since start of year.
		\param qRadDirNormal The direct solar radiation in [W/m2] normal to the surface.
		\param qRadDirHorizontal The direct solar radiation in [W/m2] on a horizontal surface is stored in this variable.
	*/
	void convertNormalToHorizontalRadiation(double secondsOfYear, double qRadDirNormal, double &qRadDirHorizontal);

	/*! Computes diffuse radiation from upper hemisphere onto a given surface for the current time.
		Documentation:
			https://pvpmc.sandia.gov/modeling-steps/1-weather-design-inputs/plane-of-array-poa-irradiance/calculating-poa-irradiance/poa-sky-diffuse/perez-sky-diffuse-model/
			https://plantpredict.com/algorithm/irradiance-radiation/	(Attention on Perez total diffuse equation a bracket error)
			https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=5&ved=2ahUKEwjJzZCd6v3lAhUCb1AKHTyHAzsQFjAEegQIARAE&url=https%3A%2F%2Fwww.osti.gov%2Fservlets%2Fpurl%2F7024029&usg=AOvVaw0ror83xSuG7niJGJv8XrLD
			google search -> "A new simplified version of the Perez diffuse irradiance model for tilted surfaces. Solar Energy 39"
		\param inclinationAngle Inclination angle in [rad].
		\param incidenceAngleRad Incidence angle (angle between surface normal and sun normal direction, [0...PI_HALF]) in [rad].
		\param diffuseRadHorizontal Diffuse radiation onto horizontal surface [W/m2].
		\param directRadNormal Direct radiation in sun's normal direction [W/m2].

		\returns Returns diffuse radiation portion from upper hemisphere in surface's normal direction in [W/m2].
	*/
	double diffuseRadiationPerez(double inclinationAngle, double incidenceAngleRad, double diffuseRadHorizontal, double directRadNormal) const;

	/*! Calculates local time from local location standard time
		This calculation includes the correction due to longitude correction to the time zone median
		\param secondsOfYear seconds of the current year in local standard time
		\param timeZone Time zone of location in h
		\param longitudeInDeg Longitude of location in DEG
	*/
	static double localMeanTimeFromLocalStandardTime( double secondsOfYear, double timeZone, double longitudeInDeg);

	/*! Calculate observer (apparent solar) time from local time at time zone median.
		This calculation includes the correction due to orbital eccentricity of earth
		\para secondsOfYear Seconds of the current year in local time (always 0 <= t < 365*24*3600)
	*/
	static double apparentSolarTimeFromLocalMeanTime( double secondsOfYear );

	/*! The sun position calculation model. */
	SunPositionModel							m_sunPositionModel;

	/*! The climate data loader model. */
	ClimateDataLoader							m_climateDataLoader;

	/*! Albedo value of the observers surrounding [0..1]. */
	double										m_albedo;

	/*! Synthetic climate model we use as an upper limit for the conversion
		of horizontal into normal solar radiation.
		Default: ASHRAE clear sky model.*/
	ClimateConversionModel						m_climateConversionModel;
	/*! Stored parametric values for ASHRAE clear sky model (one for each month). */
	IBK::LinearSpline							m_opticalDepthPerMonthASHRAE;

	/*! If true, diffuse radiation is computed with Perez model.
		Needs direct normal radiation, diffuse radiation on horizontal surface, air pressure
		(if missing, that means value = DATA_NOT_VALID, the default pressure of 101325 Pa is used).
		Also, inclination/orientation from surface, elevation/azimuth angle from sun position model.
	*/
	bool										m_diffuseRadiationPerezEnabled;

private:
	/*! Calculates local time from local location standard time
		This calculation includes the correction due to longitude correction to the time zone median
		\para secondsOfYear seconds of the current year in local standard time
	*/
	double localMeanTimeFromLocalStandardTime( double secondsOfYear);


	/*! Vector of surfaces to compute radiation loads on, first value in pair is the orientation,
		second value in pair is the inclination (both in [rad]).
	*/
	std::vector< std::pair<double, double> >	m_surface;

	/*! Cached values for direct solar radiation in [W/m2] normal to each surface,
		retrieve via member function radiationLoad()
	*/
	std::vector<double>							m_qRadDir;
	/*! Cached values for diffuse solar radiation in [W/m2] normal to each surface,
		retrieve via member function radiationLoad()
	*/
	std::vector<double>							m_qRadDif;
	/*! Cached values for incidence angle in [rad],
		retrieve via member function radiationLoad()
	*/
	std::vector<double>							m_incidenceAngle;

	/*! Local mean time [s], updated in setTime() */
	double										m_localMeanTime;

	/*! Apparent solar time [s], updated in setTime() */
	double										m_apparentSolarTime;


};


} // namespace CCM

/*! \file CCM_SolarRadiationModel.h
	\brief Contains declaration of class CCM::SolarRadiationModel.
*/

#endif // CCM_SolarRadiationModelH
