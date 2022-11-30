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
#include <IBK_Time.h>
#include <IBK_NotificationHandler.h>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Polygon3D.h>

#include "SH_Constants.h"

namespace SH {

class Notification : public IBK::NotificationHandler {
public:
	bool	m_aborted = false;
};


/*! data structure to hold structural shading data,
	both geometrical configurations of standard shapes and/or external shadings provided
	via file name.
*/
class StructuralShading  {
public:

	// TODO Stephan: add some documentation
	struct SunPosition {

		SunPosition(unsigned int secOfYear, double azi, double alti):
			m_secOfYear(secOfYear),
			m_azimuth(azi),
			m_altitude(alti)
		{}

		IBKMK::Vector3D calcNormal() const {
			IBKMK::Vector3D sunNormal ( std::cos(m_altitude)*std::sin(m_azimuth),
										std::cos(m_altitude)*std::cos(m_azimuth),
										std::sin(m_altitude) );
			sunNormal.normalize();
			return sunNormal;
		}

		unsigned int		m_secOfYear;						///< seconds of year
		double				m_azimuth;							///< in rad
		double				m_altitude;							///< in rad
	};


	struct ShadingObject {

		ShadingObject() {}

		ShadingObject(unsigned int id, const std::string &name, unsigned int parentId, const IBKMK::Polygon3D &poly3D,
					  const std::vector<IBKMK::Polygon2D> &holes, bool isObstacle = false) :
			m_id(latestId++),
			m_name(name),
			m_idVicus(id),
			m_idParent(parentId),
			m_holes(holes),
			m_polygon(poly3D),
			m_isObstacle(isObstacle)
		{}

		ShadingObject(unsigned int id, const std::string &name, unsigned int parentId, const IBKMK::Polygon3D &poly3D,
					  bool isObstacle = false) :
			m_id(latestId++),
			m_name(name),
			m_idVicus(id),
			m_idParent(parentId),
			m_polygon(poly3D),
			m_isObstacle(isObstacle)
		{}

		static unsigned int													latestId;

		unsigned int														m_id;
		std::string															m_name;
		std::set<unsigned int>												m_visibleSurfaces;				///< Set with ids of visible objects for this object
		unsigned int														m_idVicus;						///< Unique id of shading object
		unsigned int														m_idParent = INVALID_ID;		///< Unique id of parent surface, if INVALID no parent exists
		std::vector<IBKMK::Polygon2D>										m_holes;						///< Vector with all holes of surface
		IBKMK::Polygon3D													m_polygon;						///< polygon of shading object
		mutable std::map<unsigned int, std::vector<IBKMK::Vector2D>>		m_projectedPolys;				///< id of map is sun cone index in sun cone normals. projected points of polygon in sun pane
		mutable std::map<unsigned int,
						std::vector<std::vector<IBKMK::Vector2D>>>			m_projectedHoles;				///< id of map is sun cone index in sun cone normals. projected points of polygon in sun pane
		bool																m_isObstacle;					///< indicates whether it is a pure obstacle
	};


	StructuralShading() : m_startTime(2007,0) {}

	/*! Specifies location and pre-calculates sun positions and map for "similar sun positions".
		\todo param
	*/
	void initializeShadingCalculation(int timeZone, double longitudeInDeg, double latitudeInDeg,
					 const IBK::Time & startTime, unsigned int duration, unsigned int samplingPeriod,
					 double sunConeDeg = 3);

	/*! Initializes all variables for shading calculation such as obstacles and sun positions
		\param obstacles vector with all obstacle interfaces
	*/
	void setGeometry(const std::vector<ShadingObject> &surfaces, const std::vector<ShadingObject> &obstacles);

	/*! Calculates the shading factors for the given period
		\param duration Duration of period in seconds

		\todo param + units!
		duration + sampling period in seconds

		start time and duration = whole number multiple of samplingPeriod; if not -> warning issued
	*/
	void calculateShadingFactors(Notification * notify, double gridWidth = 0.1, bool useClippingMethod = false);

	// *** functions to retrieve calculation results

	const std::vector<SunPosition> & sunPositions() const { return m_sunPositions; }

	/*! Exports Shading Factors to a TSV-File */
	void writeShadingFactorsToTSV(const IBK::Path &path, const std::vector<unsigned int> & surfaceIDs, const std::vector<std::string> & surfaceDisplayNames);

	/*! Exports Shading Factors to a DataIO-File */
	void writeShadingFactorsToDataIO(const IBK::Path &path, const std::vector<unsigned int> & surfaceIDs, const std::vector<std::string> & surfaceDisplayNames, bool isBinary = true);


private:

	/*! Updates vector m_sunPositions based on current input data (location, time, ...). */
	void createSunNormals();

	/*! Tries to find similar normal vector in m_sunConeNormals that is within the same sun cone with inside angle m_sunConeDeg.
		\param sunNormal			normal vector of sun beam ( pointing from window to sun )
		\return Returns -1 if no sun cone was found and a new entry needs to be recorded
				Returns -2 if sun does not shine on the surface ( vector between normals is bigger than 90 Deg )
				Otherwise returns index of existing sunConeNormal.
	*/
	int findSimilarNormals(const IBKMK::Vector3D &sunNormal) const;

	/*! Finds all visible surfaces and obstacles that are shading a surface.
		This is only the case if at least one point lies in front of our surface.
		Fills in all ids of visible surfaces in ShadingObject membervariable m_visibleObjects
	*/
	void findVisibleSurfaces(bool useClipping = false);

	/*! Creates all Projected polygons in all sun panes. */
	void createProjectedPolygonsInSunPane();

	// ** input variables **

	int													m_timeZone = 13;
	double												m_longitudeInDeg = 13;
	double												m_latitudeInDeg = 51;
	/*! Minimum sun cone (half inner angle) used for shading calculation in [Deg].
		All normals within the same code are treated as one and calculation is one done once for all those sun positions.
	*/
	double												m_sunConeDeg = 3;

	IBK::Time											m_startTime;
	unsigned int										m_duration = 365*24*3600;			/// Duration in [s]
	unsigned int										m_samplingPeriod = 3600;			/// Sampling peroid/step size in [s]

	std::vector<ShadingObject>							m_obstacles;						///< Shading obstacles

	std::vector<ShadingObject>							m_surfaces;							///< Shading surface

	double												m_gridWidth;						///< Grid width in m used for shading calculation


	std::vector<SunPosition>							m_sunPositions;						///< Vector with all sun positions (size = number of sampling intervals)
	/*! Vector with cached normal vectors for each sun cone (size = number of cones, i.e. sufficiently different normal vectors */
	std::vector<IBKMK::Vector3D>						m_sunConeNormals;

	/*! Shading factors for each sunCone and surface.
		\code
		m_shadingFactors[surfaceIndex][sunConeIndex] = ... ; // shading factor of surface and sun cone

		// writing shading factors to file
		//
		for (sunConeIndex : m_sunConeNormals.size())
		  for (surfIndex : m_surfaces.size())
			// get shading factor
			sf = m_shadingFactors[sunConeIndex][surfIndex]
			// write for each original sun position index
			for (sunPosIndex : m_indexesOfSimilarNormals[sunConeIndex])
			  data[sunPosIndex][surfIndex] = sf
		\endcode
	*/
	std::vector< std::vector<double> >					m_shadingFactors;

	/*! Vector stores indexes of sun positions with similar normals to m_sunConeNormals. Size and indexes match those of m_sunConeNormals. */
	std::vector<std::vector<unsigned int> >				m_indexesOfSimilarNormals;

//	SunShadingAlgorithm									m_shading;							///< Object for shading calculation
};

} // namespace TH

#endif // TH_StructuralShadingH
