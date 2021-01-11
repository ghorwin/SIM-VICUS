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

#include <algorithm>
#include <cmath>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_Time.h>
#include <IBK_math.h>

#include "CCM_SolarRadiationModel.h"
#include "CCM_Constants.h"

// include this file last
#include "CCM_Defines.h"

namespace CCM {

inline bool nearly_equal(double a, double b, double EPS = 1e-5) {
	return (a + EPS > b  &&  a < b + EPS);
}


SolarRadiationModel::SolarRadiationModel() :
	m_albedo(0.2),
	m_climateConversionModel(ASHRAE_ClearSky),
	m_diffuseRadiationPerezEnabled(false)
{
}


void SolarRadiationModel::setTime(int year, double secondsOfYear) {

	// correct time
	// correct local standart time bylongitude shift
	m_localMeanTime = localMeanTimeFromLocalStandardTime(secondsOfYear);
	// ... and orbital eccentricity of earth ...
	m_apparentSolarTime = apparentSolarTimeFromLocalMeanTime(m_localMeanTime);

	// update sun position
	m_sunPositionModel.setTime(m_apparentSolarTime);
	// update climate data loader
	m_climateDataLoader.setTime(year, secondsOfYear);

	// calculate local weather station time (correct longitude shift to time zone median)
	//double localStationTime = longitudeCorrectedTime(secondsOfYear,
	//	m_climateDataLoader.m_longitudeInDegree * DEG2RAD,
	//	m_climateDataLoader.m_timeZone);

	//double correctedTime = secondsOfYear + m_localObserverTime - m_localStationTime;
	//// perform a cyclic correction
	//if(correctedTime < 0.0)
	//	correctedTime += (double) SECONDS_PER_YEAR;
	//else if(correctedTime > (double) SECONDS_PER_YEAR)
	//	correctedTime -= (double) SECONDS_PER_YEAR;

	//// update climate data loader
	//m_climateDataLoader.setTime(year, correctedTime);

	// compute radiation load and incidence angle for all surfaces

	// retrieve sun elevation angle and azimuth angle in [rad]
	double elevationAngle = m_sunPositionModel.m_elevation;
	double azimuthAngle   = m_sunPositionModel.m_azimuth;

	bool sunBeyondHorizont	= elevationAngle <= 0;

	// for negative elevation angle set all radiation vector to 0
	// and incident angle to Pi
	if (sunBeyondHorizont) {
		// no direct radiation, incidence angle set to 90 degree
		std::fill(m_qRadDir.begin(), m_qRadDir.end(),0);
		std::fill(m_qRadDif.begin(), m_qRadDif.end(),0);
		std::fill(m_incidenceAngle.begin(), m_incidenceAngle.end(), PI_HALF);

		// Perez model calculation for situation after sun setting - no radiation!
		if (m_diffuseRadiationPerezEnabled) {
			return;
		}

		// Mind: there may still be diffuse radiation despite the sun being already down, so we calculate
		// the diffuse fraction onto surfaces

		// retrieve horizontal diffuse radiation [W/m2]
		double diffuseRadHorizontal = m_climateDataLoader.m_currentData[ClimateDataLoader::DiffuseRadiationHorizontal];
		if (diffuseRadHorizontal == 0.0)
			return; // no diffuse radiation, no calculation necessay
		for (unsigned int i = 0; i < m_surface.size(); ++i) {
			double inclinationAngle = m_surface[i].second;

			bool flatRoof			= nearly_equal(inclinationAngle, 0);
			// for flat roofs we can directly return the (measured) horizontal values
			if (flatRoof) {
				m_qRadDif[i] = diffuseRadHorizontal;
				continue;
			}

			// calculate diffuse radiation on upper and lower hemisphere
			double cosInclination2 = std::cos(0.5 * inclinationAngle);
			double viewFactorToSky = cosInclination2 * cosInclination2;
			double viewFactorToGround = 1-viewFactorToSky;

			double diffuseRadOfUpperHemisphere = viewFactorToSky * diffuseRadHorizontal;
			double diffuseRadOfLowerHemisphere = m_albedo * viewFactorToGround * diffuseRadHorizontal;

			// store values
			m_qRadDif[i] = diffuseRadOfUpperHemisphere + diffuseRadOfLowerHemisphere;
		}
		return;
	}

	// elevation clipping factor:
	// elevationAngle < 0      : smoothElevationClipping = 0
	// elevationAngle > 1e-4   : smoothElevationClipping = 1
	// in between : smooth transition - applied to direct radiation to avoid abrupt change from
	// some value to 0 when elevation angle becomes <= 0.
	double smoothElevationClipping = IBK::scale2(elevationAngle, 1e-4);
	// get direct solar radiation (normal direction) and apply smooth clipping rule
	double directRadNormal	  = smoothElevationClipping * m_climateDataLoader.m_currentData[ClimateDataLoader::DirectRadiationNormal];

	// calculate radiation fraction
	double radiationFractionHorizontal = std::sin(elevationAngle);
	double radiationFractionVertical   = std::cos(elevationAngle);
	// calculate horizontal and vertical solar radiation in [W/m2]
	double directRadHorizontal  = radiationFractionHorizontal * directRadNormal;
	double directRadVertical	= radiationFractionVertical * directRadNormal;
	// retrieve horizontal diffuse radiation [W/m2]
	double diffuseRadHorizontal = m_climateDataLoader.m_currentData[ClimateDataLoader::DiffuseRadiationHorizontal];

	for (unsigned int i = 0; i < m_surface.size(); ++i) {
		// retrieve orientation and inclination angle [rad]
		double orientationAngle = m_surface[i].first;
		double inclinationAngle = m_surface[i].second;

		bool flatRoof			= nearly_equal(inclinationAngle, 0);

		// for flat roofs we can directly return the horizontal values - also valid for Perez
		if (flatRoof) {
			m_qRadDir[i] = directRadHorizontal;
			m_qRadDif[i] = diffuseRadHorizontal;
			// incident angle = 90° - elevation angle
			m_incidenceAngle[i] = PI_HALF - elevationAngle;
			continue;
		}

		// *** compute incidence angle

		// caluclate fractions of horizontal and vertical solar radiation at all surfaces
		double cosInclination = std::cos(inclinationAngle);
		double sinInclination = std::sin(inclinationAngle);

		// calculate projection of surface normal to horizontal vertical radiation vector
		// use the identity normal angle on plane = 90° + (azimuth - orientation)
		// -> sin(normalAngle) = sin (90° + (azimuth - orientation)) = cos(azimuth - orientation)
		// normal angle in height = 90° - inclination
		// -> sin(normalAngle) = sin( 90° - inclination ) =   sin (inclination + 90°) = cos(inclination)
		// -> cos(normalAngle) = cos( 90° - inclination ) = - cos (inclination + 90°) = sin(inclination)
		double surfaceNormalInHorizontalRadDirection = cosInclination;
		double surfaceNormalInVerticalRadDirection   = sinInclination * std::cos(azimuthAngle - orientationAngle);

		// calulate incidence angle from scalar product: <radVector, surfaceNormal>
		// with <radVector, surfaceNormal> denoting the scalar product.
		// We use the formula <a, b> = || a || * || b || * cos(<)(a,b))
		// with the enclosed angle <)(a,b)
		double cosIncidence = surfaceNormalInHorizontalRadDirection * radiationFractionHorizontal +
							 surfaceNormalInVerticalRadDirection * radiationFractionVertical;

		// ensure, that rounding errors may not give cosIncidence > 1
		if (cosIncidence > 1)
			cosIncidence = 1;

		// incidence angle < 0 --> no direct radiation
		// we assume the worst - and set direct radiation and incidence angle to PI_HALF = 90°
		double directRadOnSurface = 0;
		double incidenceAngle = PI_HALF;
		if (cosIncidence >= 0) {

			// in the special case azimuthAngle == orientationAngle this formula gives
			// directRadOnSurface = directRadNormal * (cosInclination * sinElevation + sinInclination * cosElevation)
			//                    = directRadNormal * (sin(normalAngle) * sinElevation + cos(normalAngle) * cosElevation)
			//					  = directRadNormal * cos(normalAngle - elevationAngle)
			directRadOnSurface = surfaceNormalInHorizontalRadDirection * directRadHorizontal +
									surfaceNormalInVerticalRadDirection * directRadVertical;
			// incidenceAngle will be always between 0° and 180° (symmetry of cosinus function)
			incidenceAngle  = std::acos(cosIncidence); // in Rad
		}

		// calculate diffuse radiation on upper and lower hemisphere
		double cosInclination2 = std::cos(0.5 * inclinationAngle);
		double viewFactorToSky = cosInclination2 * cosInclination2;
		double viewFactorToGround = 1-viewFactorToSky; // = sinInclination2*sinInclination2 with sinInclination2 = std::sin(0.5 * inclinationAngle);

		// diffuse rad on upper hemisphere either with Perez or standard
		double diffuseRadOfUpperHemisphere;
		if (m_diffuseRadiationPerezEnabled) {
			diffuseRadOfUpperHemisphere = diffuseRadiationPerez(inclinationAngle, incidenceAngle, diffuseRadHorizontal, directRadNormal);
		}
		else {
			diffuseRadOfUpperHemisphere = viewFactorToSky * diffuseRadHorizontal;
		}
		double diffuseRadOfLowerHemisphere = m_albedo * viewFactorToGround
													* (diffuseRadHorizontal + directRadHorizontal);

		// sum up both parts
		double diffuseRadOnSurface = diffuseRadOfUpperHemisphere + diffuseRadOfLowerHemisphere;

		// store values
		m_qRadDir[i] = directRadOnSurface;
		m_qRadDif[i] = diffuseRadOnSurface;
		m_incidenceAngle[i] = incidenceAngle;
	}
}


unsigned int SolarRadiationModel::addSurface(double orientation, double inclination) {
	// search for existing surfaces with same orientation and inclination
	//	\todo Compute scalar product between all normal vectors of already registered surfaces and the normal vector
	//			of the new surfaces, and if angle between normals is less than a threshold (say 0.1 deg), treat
	//			surfaces as the same.
	for (unsigned int i=0; i<m_surface.size(); ++i) {
		if (nearly_equal(m_surface[i].first, orientation, 1e-5) &&
			nearly_equal(m_surface[i].second, inclination, 1e-5) )
		{
			// Same surface definition already known,
			// return the current index in the m_surface vector.
			return i;
		}
	}
	// add new surface definition
	m_surface.push_back( std::make_pair(orientation, inclination));
	// add entry for incindent angle
	m_incidenceAngle.push_back(0.0);
	// add entry for direct and diffuse solar radiation
	m_qRadDir.push_back(0.0);
	m_qRadDif.push_back(0.0);
	// construct a new surface id from container index
	unsigned int surfaceID = (unsigned int)m_surface.size()-1;

	return surfaceID;
}


double SolarRadiationModel::localMeanTimeFromLocalStandardTime( double secondsOfYear) {
	return localMeanTimeFromLocalStandardTime(secondsOfYear, m_climateDataLoader.m_timeZone, m_sunPositionModel.m_longitude/DEG2RAD);
}

double SolarRadiationModel::localMeanTimeFromLocalStandardTime( double secondsOfYear, double timeZone, double longitudeInDeg) {
	// Correction of standart clock time due to deviations from the time zone longitude
	// Kologirou - Solar Engineering, ch. 2.1.2.

	// start with meridian of the current time zone
	double standardLongitudeInDeg = (double) timeZone * 15.;
	double localLongitudeInDeg	  = longitudeInDeg;
	double longitudeDeviationInDeg = standardLongitudeInDeg - localLongitudeInDeg;

	// special treatment of time zone +/- 12
	if (timeZone == 12 || timeZone == -12) {
		if (longitudeDeviationInDeg >= 345.)
			longitudeDeviationInDeg -= 360.;
		else if (longitudeDeviationInDeg <= -345.)
			longitudeDeviationInDeg += 360.;
	}
	// one degree = 4 minutes
	double longitudeCorrection = - 4. * 60. * longitudeDeviationInDeg;

	double correctedTime = secondsOfYear + longitudeCorrection;
	// perform a cyclic correction
	if (correctedTime < 0.0)
		correctedTime += SECONDS_PER_YEAR;
	else if (correctedTime > SECONDS_PER_YEAR)
		correctedTime -= SECONDS_PER_YEAR;

	return correctedTime;
}


double SolarRadiationModel::apparentSolarTimeFromLocalMeanTime( double secondsOfYear ) {

	// Correction of day length due to eccentricity and earth's orbit around sun.
	// Kologirou - Solar Engineering, ch. 2.1.1.

	// time in days in [d]
	double td = secondsOfYear/(24.0*3600);
	// The result is time shift in seconds (equation 2.1. Kologirou in minutes)
	double B = 2 * PI * (td - 81.0) / 365.0;
	double eccentricityCorrection = (9.87 * std::sin(2*B) - 7.53 * std::cos(B) - 1.5  * std::sin(B)) * 60.0;
	double correctedTime = secondsOfYear + eccentricityCorrection;

	// perform a cyclic correction
	if(correctedTime < 0.0)
		correctedTime += SECONDS_PER_YEAR;
	else if(correctedTime > SECONDS_PER_YEAR)
		correctedTime -= SECONDS_PER_YEAR;

	return correctedTime;
}


void SolarRadiationModel::radiationLoad(unsigned int surfaceID, double & qRadDir, double & qRadDif, double & incidenceAngleInRad) const {
	FUNCID(SolarRadiationModel::radiationLoad);

	if (surfaceID >= m_surface.size())
		throw IBK::Exception( IBK::FormatString("Invalid surface ID %1.").arg(surfaceID), FUNC_ID);

	qRadDir = m_qRadDir[surfaceID];
	qRadDif = m_qRadDif[surfaceID];
	incidenceAngleInRad = m_incidenceAngle[surfaceID];
}


void SolarRadiationModel::convertHorizontalToNormalRadiation(
	double secondsOfYear,
	double qRadDirHorizontal,
	double &qRadDirNormal)
{

	FUNCID(SolarRadiationModel::convertHorizontalToNormalRadiation);
	// if not done already compose all splines
	if(m_climateConversionModel == ASHRAE_ClearSky
		&& m_opticalDepthPerMonthASHRAE.empty())
	{
		std::string errmsg;
		std::vector<double> x(14,0), y(14,0);
		// starting values: January 1th
		x[0] = 0.0;
		y[0] = 0.142;
		// values for 21th of each month
		IBK::Time time;
		// January
		time.set(2000, 0, 21, 0.0);
		x[1] = time.secondsOfYear();
		y[1] = 0.142;
		// February
		time.set(2000, 1, 21,0.0);
		x[2] = time.secondsOfYear();
		y[2] = 0.144;
		// March
		time.set(2000, 2, 21, 0.0);
		x[3] = time.secondsOfYear();
		y[3] = 0.156;
		// April
		time.set(2000, 3, 21, 0.0);
		x[4] = time.secondsOfYear();
		y[4] = 0.180;
		// May
		time.set(2000, 4, 21, 0.0);
		x[5] = time.secondsOfYear();
		y[5] = 0.196;
		// June
		time.set(2000, 5, 21, 0.0);
		x[6] = time.secondsOfYear();
		y[6] = 0.205;
		// July
		time.set(2000, 6, 21, 0.0);
		x[7] = time.secondsOfYear();
		y[7] = 0.207;
		// August
		time.set(2000, 7, 21, 0.0);
		x[8] = time.secondsOfYear();
		y[8] = 0.201;
		// September
		time.set(2000, 8, 21, 0.0);
		x[9] = time.secondsOfYear();
		y[9] = 0.177;
		// October
		time.set(2000, 9, 21, 0.0);
		x[10] = time.secondsOfYear();
		y[10] = 0.160;
		// November
		time.set(2000, 10, 21, 0.0);
		x[11] = time.secondsOfYear();
		y[11] = 0.149;
		// December
		time.set(2000, 11, 21, 0.0);
		x[12] = time.secondsOfYear();
		y[12] = 0.142;
		// end period
		x[13] = 365. * 24. * 3600.;
		y[13] = 0.142;

		m_opticalDepthPerMonthASHRAE.setValues(x,y);
		m_opticalDepthPerMonthASHRAE.makeSpline(errmsg);
		// error treatment
		if(!errmsg.empty() )
			throw IBK::Exception(IBK::FormatString("Error constructimng linear spline "
			"for ASHRAE optical depth values: %1").arg(errmsg), FUNC_ID);
	}

	// correct local standart time bylongitude shift
	double localMeanTime = localMeanTimeFromLocalStandardTime(secondsOfYear);
	// ... and orbital eccentricity of earth ...
	double apparentSolarTime = apparentSolarTimeFromLocalMeanTime(localMeanTime);

	// update sun position model
	m_sunPositionModel.setTime(apparentSolarTime);

	// retrieve sun elevation angle
	double elevationAngle = m_sunPositionModel.m_elevation;

	// night
	bool sunBeyondHorizont	= elevationAngle <= 0;

	// for negative elevation angle set all radiation vector to 0
	// and incident angle to Pi
	if (sunBeyondHorizont || qRadDirHorizontal == 0.0) {
		qRadDirNormal = 0.0;
		return;
	}

	// extraterrestrian solar radiation limits normal radiation
	// (we use appreant solar time for all athmospheric calculations)
	double tdApparentSolar = apparentSolarTime/(24.0*3600);
	// use solar constant [W/m2] = solar energy/ time unit at the mean distance
	// of earth from the sun
	double solarConstant = 1366.1;

	// compute normal radiation, mind that for low elevation angles and bad measurements normal radiation fluxes can
	// be physically invalid (too large)
	qRadDirNormal = qRadDirHorizontal / std::sin(elevationAngle);

	// measured extraterrestrian solar radiation [W/m2] on a normal plane to sun due to Kaliogirou
	// ch. 2.3.5 (maximum 1400 W/m2 in winter, minimum 1330 W/m2 in the middle of the year)
	double qRadExtraterrNormal = solarConstant * (1.0 + 0.033 * std::cos(2 * PI * tdApparentSolar / 365));

	// clip nomrmal radiation use ASHRAE clear sky model (ASHRAE HOF 2005 ch. 31) for an upper limit
	if (m_climateConversionModel == ASHRAE_ClearSky) {
		double athmosphericDepth = m_opticalDepthPerMonthASHRAE.value(apparentSolarTime);
		// for small angles cut calculation:
		double qRadNormalASHREAClearSky =  0.0;
		// 1e-3 is selected such, that qRadNormalASHREAClearSky is 1e-62 (approximately zero)
		// main reason for clipping is that evaluating exp() with extremely large negative values fails.
		if (elevationAngle > 1e-03)
			qRadNormalASHREAClearSky = qRadExtraterrNormal * std::exp(- athmosphericDepth / std::sin(elevationAngle));
		// correct ASHREA value by clearness number (Threkwelt and Jordan 1958)
		//double clearnessNumber = 1.0;
		//double qRadNormalASHREACloudySky = clearnessNumber * qRadNormalASHREAClearSky;


		// calculate normal radiation with limitation of the horizontal radiation to
		// the artifical ASHRAE climate
		qRadDirNormal  = std::min( qRadDirNormal, qRadNormalASHREAClearSky);
	}
	else {
		// ConversionMode = None: normal radiatopn always will be limited by extraterrestrian radiation
		qRadDirNormal = std::min(qRadDirHorizontal / std::sin(elevationAngle), qRadExtraterrNormal);
	}
}

void SolarRadiationModel::convertNormalToHorizontalRadiation(double secondsOfYear, double qRadDirNormal, double &qRadDirHorizontal) {

	// correct time
	// correct local standart time bylongitude shift
	m_localMeanTime = localMeanTimeFromLocalStandardTime(secondsOfYear);
	// ... and orbital eccentricity of earth ...
	m_apparentSolarTime = apparentSolarTimeFromLocalMeanTime(m_localMeanTime);

	// update sun position
	m_sunPositionModel.setTime(m_apparentSolarTime);

	// retrieve sun elevation angle
	double elevationAngle = m_sunPositionModel.m_elevation;

	// night
	bool sunBeyondHorizont	= elevationAngle <= 0;

	// for negative elevation angle set all radiation vector to 0
	// and incident angle to Pi
	if (sunBeyondHorizont || qRadDirNormal <= 0) {
		qRadDirHorizontal = 0.0;
		return;
	}

	// elevation clipping factor:
	// elevationAngle < 0      : smoothElevationClipping = 0
	// elevationAngle > 1e-4   : smoothElevationClipping = 1
	// in between : smooth transition - applied to direct radiation to avoid abrupt change from
	// some value to 0 when elevation angle becomes <= 0.
	double smoothElevationClipping = IBK::scale2(elevationAngle, 1e-4);
	// get direct solar radiation (normal direction) and apply smooth clipping rule
	double directRadNormal	  = smoothElevationClipping * qRadDirNormal;

	// calculate radiation fraction
	double radiationFractionHorizontal = std::sin(elevationAngle);
	// calculate horizontal and vertical solar radiation in [W/m2]
	qRadDirHorizontal  = radiationFractionHorizontal * directRadNormal;
}



double calcExtraTerrRad(const unsigned int dayOfYear) {
	double b = PI * 2 * dayOfYear / 365.0;
	return 1367 * 1.00011 + 0.034221 * std::cos(b) + 0.00128 * std::sin(b) + 0.000719 * std::cos(2*b) + 0.000077 * std::sin(2*b);
}


double SolarRadiationModel::diffuseRadiationPerez(double inclinationAngleRad, double incidenceAngleRad, double diffuseRadHorizontal, double directRadNormal) const {


	// we need air pressure
	double pa = m_climateDataLoader.m_currentData[ClimateDataLoader::AirPressure];
	if (pa == DATA_NOT_VALID)
		pa = 101325; // Pa

	double diffRad = 0;

	// get elevation angle
	double elevation = m_sunPositionModel.m_elevation;
	double solarZenithAngleRad = PI_HALF-elevation;
	IBK_ASSERT(solarZenithAngleRad >= 0 && solarZenithAngleRad <= PI_HALF);

	double a = std::cos(incidenceAngleRad);
	// Mind: rounding errors may cause a to become slightly smaller than 0
	a = std::max<double>(a,0);

	double b = std::max<double>(cos(85 * PI / 180), cos(solarZenithAngleRad));
	double rB = a/b;
	double kappa = 1.041;

	//relative optical air mass (simple model)
	double optAirMass = 1 / (cos(solarZenithAngleRad) + 0.50572 * std::pow(96.07995 - solarZenithAngleRad*180.0/PI,-1.6364));
	//optical air mass weighted by air pressure
	optAirMass *= pa / 101325;

	double td = m_apparentSolarTime/(24.0*3600); // day of year in [d] as floating point number
	unsigned int dayOfYear = static_cast<unsigned int>(std::floor(td)); // always 0 <= dayOfYear < 365

	double delta = diffuseRadHorizontal * optAirMass / calcExtraTerrRad(1 + dayOfYear); // formula requires first day to be 1

	double eps = ((diffuseRadHorizontal + directRadNormal)/diffuseRadHorizontal + kappa * std::pow(solarZenithAngleRad,3)) /
					(1+ kappa * std::pow(solarZenithAngleRad,3));

	//brightness coefficients F1 and F2
	double f1 = 0;
	double f2 = 0;

	//coeff.
	double f11 = 0;
	double f12 = 0;
	double f13 = 0;
	double f21 = 0;
	double f22 = 0;
	double f23 = 0;

	if(1<= eps && eps < 1.065)
	{
		f11 = -0.0083;		f12 = 0.5877;		f13 = -0.0621;
		f21 = -0.0596;		f22 = 0.0721;		f23 = -0.0220;
	}
	else if(1.065<= eps && eps < 1.230)
	{
		f11 = 0.1299;		f12 = 0.6826;		f13 = -0.1514;
		f21 = -0.0189;		f22 = 0.0660;		f23 = -0.0289;
	}
	else if(1.230<= eps && eps < 1.500)
	{
		f11 = 0.3297;		f12 = 0.4869;		f13 = -0.2211;
		f21 = 0.0554;		f22 = -0.0640;		f23 = -0.0261;
	}
	else if(1.500<= eps && eps < 1.950)
	{
		f11 = 0.5682;		f12 = 0.1875;		f13 = -0.2951;
		f21 = 0.1089;		f22 = -0.1519;		f23 = -0.0140;
	}
	else if(1.950<= eps && eps < 2.800)
	{
		f11 = 0.8730;		f12 = -0.3920;		f13 = -0.3616;
		f21 = 0.2256;		f22 = -0.4620;		f23 = 0.0012;
	}
	else if(2.800<= eps && eps < 4.500)
	{
		f11 = 1.1329;		f12 = -1.2367;		f13 = -0.4118;
		f21 = 0.2878;		f22 = -0.8230;		f23 = 0.0559;
	}
	else if(4.500<= eps && eps < 6.200)
	{
		f11 = 1.0602;		f12 = -1.5999;		f13 = -0.3589;
		f21 = 0.2648;		f22 = -1.1272;		f23 = 0.1311;
	}
	else if(6.200<= eps)
	{
		f11 = 0.6777;		f12 = -0.3279;		f13 = -0.2504;
		f21 = 0.1561;		f22 = -1.3765;		f23 = 0.2506;
	}

	f1 = std::max<double>(0, f11 + f12 * delta + solarZenithAngleRad * f13);
	f2 = f21 + f22 * delta + solarZenithAngleRad * f23;

	//diffuse radiation on surface with perez model
	diffRad = diffuseRadHorizontal * (     (1-f1) * (1+cos(inclinationAngleRad))*0.5
										  + f1 * rB
										  + f2 * sin(inclinationAngleRad)  );

	return diffRad;
}


} // namespace CCM


