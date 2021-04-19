/*	The Thermal Room Model
	Copyright (C) 2010  Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SH_StructuralShading.h"


#include <IBK_messages.h>
#include <IBK_FormatString.h>
#include <IBK_CSVReader.h>
#include <IBK_UnitVector.h>
#include <IBK_physics.h>

#include <IBKMK_Vector3D.h>

#include <CCM_SunPositionModel.h>
#include <CCM_SolarRadiationModel.h>

#include <cmath>

#if defined(_OPENMP)
#include <omp.h> // needed for omp_get_num_threads()
#endif

namespace SH {

void StructuralShading::setLocation(int timeZone, double longitudeInDeg, double latitudeInDeg) {
	m_location = Location (timeZone, longitudeInDeg, latitudeInDeg);
}

void StructuralShading::setShadingParameters(double gridWith, double sunCone) {
	m_gridWidth = gridWith;
	m_sunCone = sunCone;
}

void StructuralShading::initializeShadingCalculation(const std::vector<std::vector<IBKMK::Vector3D> > &obstacles) {
	FUNCID(StructuralShading::initializeShadingCalculation);

	m_obstacles.clear();

	// first we set our obstacles
	try {
		for (const std::vector<IBKMK::Vector3D> &polyline : obstacles) {
			m_obstacles.push_back( Polygon(polyline) );
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Could not set obstacles for calculation!"), FUNC_ID);
	}


	// we initialize the sun positions
	if ( m_location.m_timeZone == 99 )
		throw IBK::Exception(IBK::FormatString("Location was not set."), FUNC_ID);
	// create a vector with sun positions for all 8760 hours of a year
	createSunNormals(m_sunPositions);

	///ToDo Stephan we have a slight problem here
	///		with our sun cones



}

/*! Returns Angle between vectors in DEG */
static double angleVectors(const IBKMK::Vector3D &v1, const IBKMK::Vector3D &v2) {
	double dot = v1.scalarProduct(v2);    // between [x1, y1, z1] and [x2, y2, z2]
	double angle = std::acos( dot/sqrt(v1.magnitude() * v2.magnitude() ) ) / IBK::DEG2RAD;

	return angle;
}

/*! Checks weather a sun normal lies within the specified sun cone
	Returns the timepoint of the sun normal if a sun normal can be found
	Returns -1 if no sun cone was found and a new entry in the map was added
	Returns -2 if sun does not shine on the surface ( vector between normals is bigger than 90 Deg )

	\param timepointToNormal	map of all sun normals
	\param timepoint			time point
	\param sunNormal			normal vector of sun beam ( pointing from window to sun )
	\param windowNormal			normal vector of window
*/

static int createSimilarNormals(std::map<unsigned int, IBKMK::Vector3D> & timepointToNormal, unsigned int timepoint,
								const IBKMK::Vector3D &sunNormal, const IBKMK::Vector3D &windowNormal, const double maxDiffAngleDeg=3) {


	double angleWindowSun = angleVectors(sunNormal, windowNormal);

	// if sun is not above horizon
	if( sunNormal.m_z<0 )
		return -2;

	if ( angleWindowSun < 90 && angleWindowSun > ( 90 - maxDiffAngleDeg ) ) {
		timepointToNormal[timepoint] = sunNormal;
		return -1;
	} else if ( angleWindowSun > 90 )
		return -2;

	// now if sun shines on window we search for sun cones in which the new sun beam lies
	for(std::map<unsigned int, IBKMK::Vector3D>::iterator it=timepointToNormal.begin();
		it!=timepointToNormal.end();
		++it){
		double diffAngle = angleVectors(it->second, sunNormal);
		if(std::abs(diffAngle) <=  maxDiffAngleDeg)
			return it->first;
	}

	timepointToNormal[timepoint] = sunNormal;
	return -1;
}

void StructuralShading::calculateShadingFactors(Notification * notify) {
	FUNCID(StructuralShading::calculateShadingFactors);

#if defined(_OPENMP)
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0)
			IBK::IBK_Message(IBK::FormatString("Num threads = %1").arg(omp_get_num_threads()));
	}
#endif

#pragma omp parallel for
	for (int surfCounter = 0; surfCounter < m_surfaces.size(); ++surfCounter) {
		if (notify->m_aborted)
			continue;
		Polygon &surf = m_surfaces[surfCounter];

		// we analyse the sun for same sun positions
		std::map<unsigned int, std::vector<unsigned int>>	timepointToAllEqualTimepoints;
		std::map<unsigned int, IBKMK::Vector3D>				timepointToNormal;
		IBKMK::Vector3D										surfaceNormal;

		Polygon shadingPoly (surf);

		m_shading.m_obstacles = m_obstacles;
		m_shading.m_shadingObjects.clear();
		SunShadingAlgorithm::ShadingObj shadingObj;
		shadingObj.m_polygon = surf;
		m_shading.m_shadingObjects.push_back(shadingObj);

		// now we initialize our shading calculation object
		try {
			m_shading.m_gridLength = m_gridWidth;
			m_shading.initializeGrid();
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Could not initialize grid for calculation!"), FUNC_ID);
		}

		for (size_t i=0; i<m_sunPositions.size(); ++i) {

			int id = createSimilarNormals(timepointToNormal, i, m_sunPositions[i].calcNormal(), shadingPoly.calcNormal(true), m_sunCone);
			if(id==-1)
				timepointToAllEqualTimepoints[i];
			else if(id==-2)
				timepointToAllEqualTimepoints[std::numeric_limits<unsigned int>::max()].push_back(i);
			else
				timepointToAllEqualTimepoints[id].push_back(i);
		}

		// result vector
		std::vector<double> sunShadings(m_sunPositions.size(),0);

		IBKMK::Vector3D vSun, vSunTemp;
		IBKMK::Vector3D newSunNormal, sunNormal;

		unsigned int mapSize = timepointToAllEqualTimepoints.size();
		unsigned int counter = 0;

		for (std::map<unsigned int, std::vector<unsigned int>>::iterator itNormal = timepointToAllEqualTimepoints.begin();
			 itNormal != timepointToAllEqualTimepoints.end();
			 ++itNormal) {

			++counter;

			unsigned int i = itNormal->first;

			if ( counter % 300 == 0 ) {

#if defined(_OPENMP)
				if (omp_get_thread_num() == 0)
#endif
					notify->notify(double(surfCounter*mapSize + counter) / (m_surfaces.size()*mapSize));

				if (notify->m_aborted)
					continue;
			}

			if(i==std::numeric_limits<unsigned int>::max())
				continue;

			try {
				double alti = m_sunPositions[i].m_altitude;
				double azi = m_sunPositions[i].m_azimuth;


				double altiInDeg = m_sunPositions[i].m_altitude / IBK::DEG2RAD;
				double aziInDeg = m_sunPositions[i].m_azimuth / IBK::DEG2RAD;


				//sun is not in zenith
				if(alti < IBK::PI * 0.5 ) {
					sunNormal.m_x = std::cos(alti) * std::sin(azi);
					sunNormal.m_y = std::cos(alti) * std::cos(azi);
					sunNormal.m_z = std::sin(alti);
				}
				//sun is in zenith
				else {
					sunNormal.m_x = std::cos(IBK::PI) * std::sin(azi);
					sunNormal.m_y = std::cos(IBK::PI) * std::cos(azi);
					sunNormal.m_z = std::sin(IBK::PI);
				}

				// rotate sun to fit to south wall
				IBKMK::Vector3D sunVector ( sunNormal.m_x, sunNormal.m_y, sunNormal.m_z );

				// calculate shading factors
				m_shading.calcShading(sunVector);

				for (size_t j=0; j<itNormal->second.size(); ++j)
					sunShadings[itNormal->second[j]] = m_shading.m_shadingObjects.back().m_shadingValue;

				// give back shading factor
				sunShadings[i] = m_shading.m_shadingObjects.back().m_shadingValue;



			}
			catch (IBK::Exception &ex) {
				throw IBK::Exception(ex, IBK::FormatString("Could not finish shading calculation!"), FUNC_ID);
			}
		}

		IBK::UnitVector time(8760, 0, IBK::Unit(IBK_UNIT_ID_SECONDS));
		IBK::UnitVector shadingFact(8760, 1, IBK::Unit("---"));
		for(size_t i=0; i<8760; ++i) {
			time.m_data[i] = i * 3600;
			shadingFact.m_data[i] = sunShadings[i];
		}
		// now generate the spline
		try {
			surf.m_shadingFactors.setValues(time.m_data, shadingFact.m_data);
			std::string errorMsg;
			if (!surf.m_shadingFactors.makeSpline(errorMsg))
				throw IBK::Exception(errorMsg, FUNC_ID);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Unable to create shading vector."), FUNC_ID);
		}
	}
}


void StructuralShading::createSunNormals(std::vector<SunPosition>& sunPositions){

	CCM::SunPositionModel sunModel;

	sunModel.m_latitude = m_location.m_latitudeInDeg * IBK::DEG2RAD;
	sunModel.m_longitude = m_location.m_longitudeInDeg * IBK::DEG2RAD;

	for( unsigned int i=0; i<8760*2; ++i) {
		double timeInSec = i * 3600.0/2;
		double localMeanTime = CCM::SolarRadiationModel::localMeanTimeFromLocalStandardTime(timeInSec,m_location.m_timeZone,
																							m_location.m_longitudeInDeg);
		double apparentTime = CCM::SolarRadiationModel::apparentSolarTimeFromLocalMeanTime(localMeanTime);
		sunModel.setTime(apparentTime);		//sun position at middle of the hour; hourly interval

		sunPositions.emplace_back( SunPosition(sunModel.m_azimuth, sunModel.m_elevation) );
	}
}

std::vector<StructuralShading::SunPosition> StructuralShading::sunPositions() const
{
	return m_sunPositions;
}


} // namespace TH
