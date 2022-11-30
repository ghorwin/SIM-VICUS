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
#include <IBK_StopWatch.h>
#include <IBK_math.h>

#include <IBKMK_Vector3D.h>
#include <IBKMK_3DCalculations.h>

#include <CCM_SunPositionModel.h>
#include <CCM_SolarRadiationModel.h>

#include <DATAIO_DataIO.h>

#include <cmath>

#include <QMatrix4x4>

#include <fstream>

#include "SH_ShadedSurfaceObject.h"

#if defined(_OPENMP)
#include <omp.h> // needed for omp_get_num_threads()
#endif

namespace SH {

QVector4D convertIBKMKVector3D2QVector4D(const IBKMK::Vector3D &v3d) {
	return QVector4D((float)v3d.m_x,
					 (float)v3d.m_y,
					 (float)v3d.m_z,
					 1.0);
}



unsigned int StructuralShading::ShadingObject::latestId = 0;

/*! Returns Angle between vectors in DEG */
static double angleVectors(const IBKMK::Vector3D &v1, const IBKMK::Vector3D &v2) {

	double dot = v1.scalarProduct(v2);    // between [x1, y1, z1] and [x2, y2, z2]
	//	double angle2 = std::acos( dot/sqrt(v1.magnitude() * v2.magnitude() ) ) / IBK::DEG2RAD;
	IBKMK::Vector3D cross = v1.crossProduct(v2); // Normal on both vectors
	double det =  v1.m_x*v2.m_y*cross.m_z + v1.m_z*v2.m_x*cross.m_y + v1.m_y*v2.m_z*cross.m_x
			- v1.m_z*v2.m_y*cross.m_x - v1.m_y*v2.m_x*cross.m_z - v1.m_x*v2.m_z*cross.m_y;
	double angle = std::atan2(det,dot) / IBK::DEG2RAD;

	//	IBK::IBK_Message(IBK::FormatString("Old: %1\tNew: %1").arg(angle).arg(angle2), IBK::MSG_PROGRESS);

	return angle;
}


void StructuralShading::initializeShadingCalculation(int timeZone, double longitudeInDeg, double latitudeInDeg,
													 const IBK::Time & startTime, unsigned int duration, unsigned int samplingPeriod,
													 double sunConeDeg)
{
	// TODO Stephan, validity checks, throw IBK::Exception if out of range
	m_timeZone =  timeZone;
	m_longitudeInDeg = longitudeInDeg;
	m_latitudeInDeg = latitudeInDeg;

	m_startTime = startTime;
	m_duration = duration;
	m_samplingPeriod = samplingPeriod;

	m_sunConeDeg = sunConeDeg;

	IBK_ASSERT(m_samplingPeriod > 0);
	// TODO : issue warning, if duration/samplingPeriod has reminder

	// create a vector with sun positions for sampling periods
	createSunNormals();
}


void StructuralShading::setGeometry(const std::vector<ShadingObject> & surfaces, const std::vector<ShadingObject> & obstacles) {
	FUNCID(StructuralShading::setGeometry);
	try {
		m_surfaces = surfaces;
		m_obstacles = obstacles;

		for (ShadingObject &so : m_surfaces) {
			if (!so.m_polygon.isValid())
				throw IBK::Exception(IBK::FormatString("Polygon is not valid."), FUNC_ID);
		}
		for (ShadingObject &so : m_obstacles) {
			if (!so.m_polygon.isValid())
				throw IBK::Exception(IBK::FormatString("Polygon is not valid."), FUNC_ID);
		}

	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Could not set geometry for calculation!"), FUNC_ID);
	}
}

void StructuralShading::calculateShadingFactors(Notification * notify, double gridWidth, bool useClippingMethod) {
	FUNCID(StructuralShading::calculateShadingFactors);

	// TODO Stephan, input data check
	m_gridWidth = gridWidth;
	if (gridWidth <= 0)
		throw IBK::Exception("Invalid grid width, must be > 0 m.", FUNC_ID);

	// if we didn't do a sun normal calculation, yet, do it now!
	if (m_sunConeNormals.empty())
		createSunNormals();

	// if we use the clipping algorithm to calculate shading factors, we precalculated projected polys
	if(useClippingMethod)
		createProjectedPolygonsInSunPane();

	// prepare target memory
	m_shadingFactors.resize(m_surfaces.size());
	for (std::vector<double> & sf : m_shadingFactors) {
		sf.resize(m_sunConeNormals.size(), 0.0); // initialize with 0, i.e. fully shaded; this is the default since we do not handle night-time
		// Note: resize on an existing vector doesn't do anything, and also doesn't reset existing values in the vector
		//       Hence, we need to manually set all values to zero again
		IBK::set_zero(sf);
	}

	// Find visible shading surfaces for each surface
	findVisibleSurfaces(useClippingMethod);

	IBK::IBK_Message(IBK::FormatString("Initialize shading calculation"));

#if defined(_OPENMP)
	int threadCount = 1;

#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) {
			threadCount = omp_get_num_threads();
			// we should leave one CPU free for the GUI update
			if (threadCount > 4) {
				--threadCount;
				omp_set_num_threads(threadCount);
			}
			IBK::IBK_Message(IBK::FormatString("Running shading calculation in parallel with %1 threads.\n").arg(threadCount));
		}
	}
#endif
	IBK::StopWatch totalTimer;
	totalTimer.start();
	// the stop watch object and progress counter are used only in a critical section
	IBK::StopWatch w, v;
	w.start();
	v.start();
	notify->notify(0);
	int surfacesCompleted = 0;

#ifdef WRITE_OUTPUT
	// Create Shading debugging path
	IBK::Path path ("C:/shading/");
	if(path.exists())
		IBK::Path::remove(path);
	IBK::Path::makePath(path);
#endif

#if defined(_OPENMP)
#pragma omp parallel for schedule(dynamic, 1)
#endif
	for (int surfCounter = 0; surfCounter < (int)m_surfaces.size(); ++surfCounter) {
		try {
			if (notify->m_aborted)
				continue; // skip ahead to quickly stop loop

			// our thread "owns" the shading factor vector, hence we can directly write to it without
			// openmp blocking sections
			// 'shadingFactors' is just a readability improvement to the existing vector
			std::vector<double> & shadingFactors = m_shadingFactors[(unsigned int)surfCounter];

			// readability improvement
			const ShadingObject & so = m_surfaces[(unsigned int)surfCounter];


			// 1. split polygon 'surf' into sub-polygons based on grid information and compute center point of these sub-polygons

			ShadedSurfaceObject surfaceObject;
			surfaceObject.setPolygon(so.m_idVicus, so.m_name, so.m_polygon, so.m_holes, so.m_idParent, m_gridWidth, useClippingMethod);

			std::vector<ShadingObject> shadingObstacles;

			// must only use read-only access to shared-memory variables
			for (const ShadingObject &shading : m_obstacles)
				if (so.m_visibleSurfaces.find(shading.m_id) != so.m_visibleSurfaces.end())
					shadingObstacles.push_back(shading);


			// 2. for each center point perform intersection tests again _all_ obstacle polygons
			for (unsigned int i=0; i<m_sunConeNormals.size(); ++i) {

#ifdef WRITE_OUTPUT
				std::ofstream out(QString("C:/shading/shading_info_%1_%2.txt")
								  .arg(QString::fromStdString(so.m_name))
								  .arg(i).toStdString());
				out << "Shading calculation for surface " << so.m_name << std::endl;
				out << "Sun normal: X:" << m_sunConeNormals[i].m_x << " Y: " << m_sunConeNormals[i].m_y << "Z: " << m_sunConeNormals[i].m_z;
				out << std::endl;
//				out << "Visible Shading objects:" << std::endl;
//				for (const ShadingObject &shading : shadingObstacles)
//					out << shading.m_idVicus << ": " <<shading.m_name << std::endl;
//				out << std::endl;
#endif

				if (notify->m_aborted)
					continue; // skip ahead to quickly stop loop

				double angle = angleVectors(m_sunConeNormals[i], so.m_polygon.normal());
				//				// if sun does not shine uppon surface, no shading factor needed
				if (angle >= 90)
					continue;

				double sf;
				if (!useClippingMethod)
					sf = surfaceObject.calcShadingFactorWithRayTracing(m_sunConeNormals[i], shadingObstacles);
				else {
					surfaceObject.setProjectedPolygonAndHoles(so.m_projectedPolys[i], so.m_projectedHoles[i]);
#ifdef WRITE_OUTPUT
					surfaceObject.setOutputFile(&out);
#endif
					sf = surfaceObject.calcShadingFactorWithClipping(i, m_sunConeNormals[i], shadingObstacles);
				}

				//				// 3. store shaded/not shaded information for sub-polygon and its surface area
				//				// 4. compute area-weighted sum of shading factors and devide by orginal polygon surface
				//				// 5. store in result vector
				shadingFactors[i] = sf;

#ifdef WRITE_OUTPUT
				out << "Calculated shading factor for sun cone index " << i  << ": " << sf << std::endl;
				out.flush();
#endif

				// master thread 0 updates the progress dialog; this should be good enough for longer runs
#if defined(_OPENMP)
				if ( omp_get_thread_num() == 0) {
#endif
					// only notify every second or so
					if (!notify->m_aborted && w.difference() > 100) {
						notify->notify(double(surfacesCompleted*m_sunConeNormals.size() + i) / (m_surfaces.size()*m_sunConeNormals.size()) );
						w.start();
					}
#if defined(_OPENMP)
				}
#endif
			}


			// increase number of completed surfaces (done by all threads, hence in critical section)
#if defined(_OPENMP)
#pragma omp critical
#endif
			++surfacesCompleted;

		}
		catch (...) {
			// notify->m_aborted = true;
			// IBK::IBK_Message(IBK::FormatString("Shading calculation encountered errors"), IBK::MSG_ERROR, FUNC_ID);
		}

	} // omp for loop
	notify->notify(1.0);


	IBK::IBK_Message(IBK::FormatString("Finished after %1.\n").arg(totalTimer.diff_str()));
}


void StructuralShading::writeShadingFactorsToTSV(const IBK::Path & path, const std::vector<unsigned int> & surfaceIDs,
												 const std::vector<std::string> & surfaceDisplayNames)
{
	FUNCID(StructuralShading::shadingFactorsTSV);

	std::vector< double >			   timePoints;
	std::vector< std::vector<double> > data;

	if(m_shadingFactors.empty()) {
		IBK::IBK_Message("No surfaces have been selected for shading calculation.");
		return;
	}

	std::vector<std::vector<double>> shadingFactors(m_shadingFactors[0].size());
	for (std::vector<double> &shadFactor : shadingFactors)
		shadFactor.resize(m_shadingFactors.size());

	for(unsigned int i=0; i<m_shadingFactors.size(); ++i) {
		for(unsigned int j=0; j<m_shadingFactors[i].size(); ++j) {
			shadingFactors[j][i] = m_shadingFactors[i][j];
		}
	}

	m_shadingFactors = shadingFactors;

	if (!path.isValid())
		throw IBK::Exception("Invalid filename or no filename set.", FUNC_ID);

	if (surfaceIDs.size() != m_surfaces.size())
		throw IBK::Exception("Size of surface ID vector does not match number of surfaces passed to calculation function.", FUNC_ID);

	// check for correct file extension
	std::string ext = path.extension();
	// only remove extension if d6b or d6o is used
	IBK::Path fname_wo_ext = path;
	if (ext == "tsv")
		fname_wo_ext = path.withoutExtension();
	fname_wo_ext.addExtension(".tsv");

	if (fname_wo_ext != path) {
		throw IBK::Exception(IBK::FormatString("Invalid filename (extension) of output file '%1', should have been '%2'!\n")
							 .arg(path).arg(fname_wo_ext), FUNC_ID);
	}

	std::ofstream tsvFile ( path.str() );

	if ( !tsvFile.is_open() )
		throw IBK::Exception(IBK::FormatString("Could not open output file '%1'\n").arg(path.str() ), FUNC_ID );


	// we have a map of different sunConeNormals to sunPositions i.e. time points in vector m_indexesOfSimilarNormals
	// we now generate the reverse mapping by creating a vector of sunConeNormal indexes corresponding to each sampling
	// interval

	unsigned int stepCount = (unsigned int)std::ceil(m_duration/m_samplingPeriod);
	IBK_ASSERT(stepCount == m_sunPositions.size());
	std::vector<unsigned int> samplingIndexToConeNormalMap(stepCount, (unsigned int)-1); // initialize with -1, which means: none available

	// now process all sunCodeNormal indexes and put them into revers mapping vector
	for (unsigned int i=0; i<m_indexesOfSimilarNormals.size(); ++i) {
		for (unsigned int intervalIdx : m_indexesOfSimilarNormals[i])
			samplingIndexToConeNormalMap[intervalIdx] = i;
	}

	// write header line
	tsvFile << "Time [h]\tAzimuth [Deg]\tAltitude [Deg]\t"; // write header
	//tsvFile << "Time [h]\t"; // write header
	for ( unsigned int i=0; i<m_surfaces.size(); ++i ) {
		// generate a parsable id / displayname string; problem: user may use # or ' as part of display name,
		// hence we need to have the ID first in the string, even though this is sub-optimal for PostProc usage
		tsvFile << surfaceIDs[i];
		if (!surfaceDisplayNames[i].empty())
			tsvFile <<  " '" + surfaceDisplayNames[i] + "'";
		tsvFile << " [---]";
		if (i+1 < m_surfaces.size())
			tsvFile << '\t';
	}
	tsvFile << "\n";

	// now write the data
	unsigned int startStep = (unsigned int)std::floor(m_startTime.secondsOfYear()/m_samplingPeriod);
	for (unsigned int i=0; i<stepCount; ++i) {
		unsigned int timeInSec = (startStep + i)*m_samplingPeriod;
		//		tsvFile << IBK::val2string<double>((double)timeInSec/3600, 4 ) << "\t";
		double time = (double)timeInSec/3600;
		tsvFile << std::to_string(time) << "\t";

		// get index of corresponding shading factor data
		unsigned int sfDataIndex = samplingIndexToConeNormalMap[i];

		tsvFile << m_sunPositions[i].m_azimuth / IBK::DEG2RAD << "\t";
		tsvFile << m_sunPositions[i].m_altitude / IBK::DEG2RAD << "\t";

		// no data available? (night-time?)
		if (sfDataIndex == (unsigned int)-1) {
			for (unsigned int j=0; j<m_surfaces.size(); ++j)  {
				tsvFile	<< "0";
				if (j+1 < m_surfaces.size())
					tsvFile << '\t';
			}
		}
		else {
			// write the cached shading factors
			const std::vector<double> & sfData = m_shadingFactors[sfDataIndex];
			for (unsigned int j=0; j<m_surfaces.size(); ++j) {
				tsvFile	<< sfData[j];
				if (j+1 < m_surfaces.size())
					tsvFile << '\t';
			}
		}
		tsvFile << "\n";
	}
	tsvFile.close();
}


void StructuralShading::writeShadingFactorsToDataIO(const IBK::Path & path, const std::vector<unsigned int> & surfaceIDs,
													const std::vector<std::string> & surfaceDisplayNames, bool isBinary)
{

	// We create a dataIO container
	DATAIO::DataIO dataContainer;
	dataContainer.m_filename = path;
	dataContainer.m_isBinary = isBinary;
	dataContainer.m_type = DATAIO::DataIO::T_REFERENCE;
	dataContainer.m_spaceType = DATAIO::DataIO::ST_SINGLE;
	dataContainer.m_timeType = DATAIO::DataIO::TT_NONE;
	dataContainer.m_timeUnit = "h";

	unsigned int stepCount = (unsigned int)std::ceil(m_duration/m_samplingPeriod);
	IBK_ASSERT(stepCount == m_sunPositions.size());
	std::vector<unsigned int> samplingIndexToConeNormalMap(stepCount, (unsigned int)-1); // initialize with -1, which means: none available

	// now process all sunCodeNormal indexes and put them into reverse mapping vector
	for (unsigned int i=0; i<m_indexesOfSimilarNormals.size(); ++i) {
		for (unsigned int intervalIdx : m_indexesOfSimilarNormals[i])
			samplingIndexToConeNormalMap[intervalIdx] = i;
	}

	// INDICES = <surface IDs>
	std::vector<unsigned int> nums = surfaceIDs;

	// QUANTITY = <name1> | <name2> | ... | <namen>
	// Note: we do not write IDs into the quantity, because the IDs are already stored in the nums vector
	std::string quantity;
	for (unsigned int i=0; i<surfaceIDs.size(); ++i) {
		if (!surfaceDisplayNames[i].empty())
			quantity += surfaceDisplayNames[i];
		else
			quantity += "#"+IBK::val2string(surfaceIDs[i]); // backup quantity name, in case displayname is empty
		if (i+1 < surfaceIDs.size())
			quantity +=" | ";
	}


	// reserve memory for time points and values
	std::vector< double > timePoints ( stepCount ) ;
	std::vector< std::vector<double> > values ( stepCount );

	unsigned int startStep = (unsigned int)std::floor(m_startTime.secondsOfYear()/m_samplingPeriod);
	for (unsigned int i=0; i<stepCount; ++i) {
		// store time point in seconds
		timePoints[i] = (startStep + i)*m_samplingPeriod;

		// get index of corresponding shading factor data
		unsigned int sfDataIndex = samplingIndexToConeNormalMap[i];

		// no data available? (night-time?)
		if (sfDataIndex == (unsigned int)-1) {
			values[i] = std::vector<double>(m_surfaces.size(), 0);
		}
		else {
			// write the cached shading factors
			const std::vector<double> & sfData = m_shadingFactors[sfDataIndex];
			values[i] = sfData;
		}
	}

	// we set our data
	dataContainer.m_nums = nums;
	dataContainer.m_quantity = quantity;
	dataContainer.m_valueUnit = "---";
	dataContainer.setData(timePoints, values);
	dataContainer.write();
}


void StructuralShading::createSunNormals() {

	// *** first calculation sun positions for each sampling interval

	m_sunPositions.clear();

	CCM::SunPositionModel sunModel;

	sunModel.m_latitude = m_latitudeInDeg * IBK::DEG2RAD;
	sunModel.m_longitude = m_longitudeInDeg * IBK::DEG2RAD;

	unsigned int startStep = (unsigned int)std::floor(m_startTime.secondsOfYear()/m_samplingPeriod);
	unsigned int stepCount = (unsigned int)std::ceil(m_duration/m_samplingPeriod);

	for (unsigned int i=0; i<stepCount; ++i) {
		unsigned int timeInSec = (startStep + i)*m_samplingPeriod;
		double localMeanTime = CCM::SolarRadiationModel::localMeanTimeFromLocalStandardTime(timeInSec, m_timeZone, m_longitudeInDeg);
		double apparentTime = CCM::SolarRadiationModel::apparentSolarTimeFromLocalMeanTime(localMeanTime);
		sunModel.setTime(apparentTime);		//sun position at middle of the hour; hourly interval

		m_sunPositions.emplace_back( SunPosition( timeInSec, sunModel.m_azimuth, sunModel.m_elevation ) );
	}

	// *** compute sun normals (clustered)

	m_sunConeNormals.clear();
	m_indexesOfSimilarNormals.clear();

	// We initialize all our coresponding sun normals.
	//
	// For each sampling interval we first compute the corresponding sun' normal vector.
	// Then, we search through our vector of previously computed sun normals and check if we are close enough.
	// This is done in function findSimilarNormal(). This function returns the index of the existing normal vector
	// if we are close enough, or -1 if we are too far away or -2 if sun is beyond horizon
	//
	// NOTE: in angles around 90 Deg between our surface normal and the sun beam
	// we can get faulty shading factors. But since radiation loads are also
	// low, we accept this. When the sun cone angle gets lower (eg 1 Deg) we
	// minimize this problem

	for (unsigned int i=0; i<m_sunPositions.size(); ++i) {
		IBKMK::Vector3D n = m_sunPositions[i].calcNormal();
		int id = findSimilarNormals(n);

		// record new different sun normal
		if (id == -1) {
			m_sunConeNormals.push_back(n);
			m_indexesOfSimilarNormals.push_back( std::vector<unsigned int>(1, i) );
		}
		else if (id >= 0) {
			// similar normal exists, append current sun positions index to vector
			m_indexesOfSimilarNormals[(unsigned int)id].push_back((unsigned int)i);
		}
	}

}


int StructuralShading::findSimilarNormals(const IBKMK::Vector3D &sunNormal) const {

	// if sun is not above horizon
	if (sunNormal.m_z < 0)
		return -2;

	// search through all previous computed normals and check if angle between normals is below threshold
	for (unsigned int i=0; i<m_sunConeNormals.size(); ++i) {
		double diffAngle = angleVectors(m_sunConeNormals[i], sunNormal);
		if (std::fabs(diffAngle) <= m_sunConeDeg)
			return (int)i;
	}

	return -1;
}

void StructuralShading::findVisibleSurfaces(bool useClipping) {
	// Finding shading partners for all surfaces
	for(ShadingObject &surf : m_surfaces) {
		// iterate through all vertex points of surface
		for(const ShadingObject &obst : m_obstacles) {

			for (size_t i=0; i<surf.m_polygon.vertexes().size(); ++i) {
			// iterate through all possible shading objects
				bool obstAdded = false;

				// skip parent objects (wall surface) of sub-surfaces such as windows
				if(!useClipping && (surf.m_idParent == obst.m_idVicus))
					continue;

				// skip parent objects (wall surface) of sub-surfaces such as windows
				if((obst.m_idParent != INVALID_ID) && (surf.m_idVicus != obst.m_idParent))
					continue;

				// iterate through all vertexes of shading objects
				for(const IBKMK::Vector3D v : obst.m_polygon.vertexes()) {
					double linefactor = 0;
					IBKMK::Vector3D p;

					// determining if we have at least one point in front of our surface
					IBKMK::lineToPointDistance(surf.m_polygon.vertexes()[i], surf.m_polygon.normal(), v, linefactor, p);

					// Tolerance to skip all possible windows that are inside our window
					if (linefactor>1e-4) {
						// store the surface that lies in front
						surf.m_visibleSurfaces.insert(obst.m_id);
						obstAdded = true; // we store that we already added the
						break;
					}
				}
				if(obstAdded)
					break; // Do not handle all the other surface vertexes
			}
		}
	}
}

void StructuralShading::createProjectedPolygonsInSunPane() {

	for (unsigned int i = 0; i<m_sunConeNormals.size(); ++i) {
		const IBKMK::Vector3D &vSun = m_sunConeNormals[i];
		QVector3D sun = 1000*QVector3D((float)vSun.m_x, (float)vSun.m_y, (float)vSun.m_z);
		QVector3D zero(0,0,0);
		QVector3D up(0,0,1);

		// qDebug() << "Sun: x: " << sun.x() << " y: " << sun.y() << " z: " << sun.z();

		QMatrix4x4 mat;
		mat.ortho(-100, 100, -100, 100, 0, 2000);
		mat.lookAt(sun, zero, up);

		for (ShadingObject &shading : m_obstacles) {
			std::vector<IBKMK::Vector2D> projectedVerts;
			for(const IBKMK::Vector3D v3D : shading.m_polygon.vertexes()) {
				QVector4D projectedP = mat * convertIBKMKVector3D2QVector4D(v3D);
				projectedP = projectedP/projectedP.w();
				projectedVerts.push_back(IBKMK::Vector2D(projectedP.x(), projectedP.y()));

				// qDebug() << "Surface: x: " << projectedP.x() << " y: " << projectedP.y();
			}
			shading.m_projectedPolys[i] = projectedVerts;

			for(const IBKMK::Polygon2D &hole : shading.m_holes) {
				std::vector<IBKMK::Vector2D> projectedHoleVerts;
				for(const IBKMK::Vector2D v2D : hole.vertexes()) {

					// Convert to polygon 3D and then project it
					const IBKMK::Polygon3D &p = shading.m_polygon;
					IBKMK::Vector3D v3D = p.offset() + p.localX()*v2D.m_x + p.localY()*v2D.m_y;

					QVector4D projectedP = mat * convertIBKMKVector3D2QVector4D(v3D);
					projectedP = projectedP/projectedP.w();
					projectedHoleVerts.push_back(IBKMK::Vector2D(projectedP.x(), projectedP.y()));
				}
				shading.m_projectedHoles[i].push_back(projectedHoleVerts);
			}
		}
		for (ShadingObject &s : m_surfaces) {
			std::vector<IBKMK::Vector2D> projectedVerts;
			for(const IBKMK::Vector3D v3D : s.m_polygon.vertexes()) {
				QVector4D projectedP = mat * convertIBKMKVector3D2QVector4D(v3D);
				projectedP = projectedP/projectedP.w();
				projectedVerts.push_back(IBKMK::Vector2D(projectedP.x(), projectedP.y()));

				// qDebug() << "Surface: x: " << projectedP.x() << " y: " << projectedP.y();
			}
			s.m_projectedPolys[i] = projectedVerts;

			for(const IBKMK::Polygon2D &hole : s.m_holes) {
				std::vector<IBKMK::Vector2D> projectedHoleVerts;
				for(const IBKMK::Vector2D &v2D : hole.vertexes()) {

					// Convert to polygon 3D and then project it
					const IBKMK::Polygon3D &p = s.m_polygon;
					IBKMK::Vector3D v3D = p.offset() + p.localX()*v2D.m_x + p.localY()*v2D.m_y;

					QVector4D projectedP = mat * convertIBKMKVector3D2QVector4D(v3D);
					projectedP = projectedP/projectedP.w();
					projectedHoleVerts.push_back(IBKMK::Vector2D(projectedP.x(), projectedP.y()));
				}
				s.m_projectedHoles[i].push_back(projectedHoleVerts);
			}
		}
	}
}

} // namespace TH
