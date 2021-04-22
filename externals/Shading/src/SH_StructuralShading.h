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

#ifndef SH_StructuralShadingH
#define SH_StructuralShadingH

#include <vector>

#include <IBK_LinearSpline.h>

#include <IBKMK_Vector3D.h>

#include "SH_SunShadingAlgorithm.h"
#include "SH_Polygon.h"

namespace SH {

class Notification : public IBK::NotificationHandler {
public:
	bool	m_aborted = false;
};


/*! Window-specific data structure to hold structural shading data,
	both geometrical configurations of standard shapes and/or external shadings provided
	via file name.
*/
class StructuralShading  {
public:

	struct Location {

		Location() {}

		Location (int timeZone, double longitudeInDeg, double latitudeInDeg) :
			m_timeZone(timeZone),
			m_longitudeInDeg(longitudeInDeg),
			m_latitudeInDeg(latitudeInDeg)
		{}

		int					m_timeZone = 99;
		double				m_longitudeInDeg;
		double				m_latitudeInDeg;
	};


	struct SunPosition {

		SunPosition(double azi, double alti):
			m_azimuth(azi),
			m_altitude(alti)
		{}

		IBKMK::Vector3D calcNormal(){
			IBKMK::Vector3D sunNormal ( std::cos(m_altitude)*std::sin(m_azimuth),
										std::cos(m_altitude)*std::cos(m_azimuth),
										std::sin(m_altitude) );
			sunNormal.normalize();
			return sunNormal;
		}

		double				m_azimuth;							///< in rad
		double				m_altitude;							///< in rad
	};

	StructuralShading() :
		m_gridWidth(0.1),
		m_sunConeDeg(3.0)
	{}

	StructuralShading(int timeZone, double longitudeInDeg, double latitudeInDeg, double gridWidth, double sunConeDeg) :
		m_location( Location(timeZone, longitudeInDeg, latitudeInDeg ) ),
		m_gridWidth(gridWidth),
		m_sunConeDeg(sunConeDeg)
	{ }

	/*! Initializes all variables for shading calculation such as obstacles and sun positions
		\param obstacles vector with all obstacle interfaces
	*/
	void initializeShadingCalculation(const std::vector<std::vector<IBKMK::Vector3D> > &obstacles);

	/*! Calculates the shading factors for the given period */
	void calculateShadingFactors(Notification * notify);

	/*! Exports Shading Factors to a TSV-File */
	void writeShadingFactorsToTSV(const IBK::Path &path);

	/*! Exports Shading Factors to a DataIO-File */
	void writeShadingFactorsToDataIO(const IBK::Path &path, bool isBinary = true);

	std::vector<SunPosition> sunPositions() const;

	std::vector<Polygon>								m_obstacles;						///< Shading obstacles

	std::vector<Polygon>								m_surfaces;							///< Shading surface

private:

	void createSunNormals(std::vector<SunPosition>& sunPositions);

	Location											m_location;							///< Location object

	std::vector<SunPosition>							m_sunPositions;						///< vector with all sun positions

	std::map<unsigned int, std::vector<unsigned int>>	m_timepointToAllEqualTimepoints; 	///< we store all time points that already have a time point with a similar sun

	double												m_gridWidth;						///< Grid width in m used for shading calculation

	double												m_sunConeDeg;						///< minimum sun cone angle used for shading calculation in Degree

	SunShadingAlgorithm									m_shading;							///< Object for shading calculation

};

} // namespace TH

#endif // TH_StructuralShadingH