/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NM_Loads.h"

#include "NM_Physics.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>

#include <IBK_assert.h>
#include <IBK_FileUtils.h>
#include <IBK_physics.h>
#include <IBK_ScalarFunction.h>
#include <IBK_messages.h>
#include <IBK_Constants.h>
#include <IBK_FormatString.h>

//#include <IBKMK_numerics.h>

#include <NANDRAD_Location.h>
#include <NANDRAD_SimulationParameter.h>

#include "NM_KeywordList.h"

#include <CCM_Defines.h>  // include this last, since here we have defines that would otherwise conflict with included files

namespace NANDRAD_MODEL {

inline bool nearlyEqual(double a, double b, double eps = 1e-5) {
	return std::fabs(a-b) < eps;
}

// *** Loads ***

void Loads::setup(const NANDRAD::Location & location, const NANDRAD::SimulationParameter &simPara,
	const std::map<std::string, IBK::Path> & pathPlaceHolders)
{
	FUNCID(Loads::setup);

	try {
		// for now we require a climate data file
		// if dummy values are needed, it is possible to create a simple dummy climate data file
		// with constant values throughout the year
		if (location.m_climateFileName.str().empty())
			throw IBK::Exception("Climate data location required (Location.ClimateReference).", FUNC_ID);

		IBK::Path climateFile = IBK::Path(location.m_climateFileName).withReplacedPlaceholders(pathPlaceHolders);

		try {
			IBK::IBK_Message(IBK::FormatString("Reading climate data file '%1'\n").arg(climateFile), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			m_solarRadiationModel.m_climateDataLoader.readClimateData(climateFile);
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading climate data from file '%1")
				.arg(climateFile.str()), FUNC_ID);
		}

		// cache required location properties and check for mandatory parameters


		// *** location and albedo

		// Note: we should now have the parameters from the climate data loader:

		// - m_solarRadiationModel.m_climateDataLoader.m_latitudeInDegree
		// - m_solarRadiationModel.m_climateDataLoader.m_longitudeInDegree

		// However, the user may have specified also latitude/longitude in the project file. If these values are provided,
		// we should overwrite the location settings in the climate data file, even though this may lead in some
		// cases to completely wrong results (at least, when latitude is changed).


		IBK::IBK_Message(IBK::FormatString("Climate data set location: Latitude: %1 deg, Longitude: %2 deg\n")
						 .arg(m_solarRadiationModel.m_climateDataLoader.m_latitudeInDegree)
						 .arg(m_solarRadiationModel.m_climateDataLoader.m_longitudeInDegree), IBK::MSG_PROGRESS,
						 FUNC_ID, IBK::VL_INFO);

		// latitude
		const IBK::Parameter &latitude = location.m_para[NANDRAD::Location::P_Latitude];
		if (!latitude.name.empty()) {
			double latInDeg = latitude.get_value("Deg");
			if (latInDeg < -90 || latInDeg > 90) {
				throw IBK::Exception(IBK::FormatString("Error initializing climate data: "
					"Location parameter 'Latitude' is expected to be between -90 and 90 degrees."), FUNC_ID);
			}
			IBK::IBK_Message(IBK::FormatString("Setting latitude to %1 deg\n").arg(latInDeg),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			m_solarRadiationModel.m_climateDataLoader.m_latitudeInDegree = latInDeg;
		}

		// longitude
		const IBK::Parameter &longitude = location.m_para[NANDRAD::Location::P_Longitude];
		if (!longitude.name.empty()) {
			double longInDeg = longitude.get_value("Deg");
			if (longInDeg < -180 || longInDeg > 180) {
				throw IBK::Exception(IBK::FormatString("Error initializing climate data: "
					"Location parameter 'Longitude' is expected to be between -180 and 180 degrees."), FUNC_ID);
			}
			IBK::IBK_Message(IBK::FormatString("Setting latitude to %1 deg\n").arg(longInDeg),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			m_solarRadiationModel.m_climateDataLoader.m_longitudeInDegree = longInDeg;
		}
		// ensure that either both latitude and longitude are given, or none
		if (!((latitude.name.empty() && longitude.name.empty()) || (!latitude.name.empty() && !longitude.name.empty())))
			throw IBK::Exception("If specifying 'Latitude' or 'Longitude', you need to specify always both.", FUNC_ID);

		// albedo

		// TODO : replace code below with .checkValue()

		const IBK::Parameter &albedo = location.m_para[NANDRAD::Location::P_Albedo];
		if (albedo.name.empty())
			throw IBK::Exception(IBK::FormatString("Error initializing climate data: Missing parameter 'Albedo'."), FUNC_ID);
		if (albedo.value < 0 || albedo.value > 1) {
			throw IBK::Exception(IBK::FormatString("Error initializing climate data: "
				"Location parameter 'Albedo' is expected between 0 and 1."), FUNC_ID);
		}
		m_solarRadiationModel.m_albedo = albedo.value;

		// finally update the latitude and longitude in the sunPositionModel
		m_solarRadiationModel.m_sunPositionModel.m_latitude = m_solarRadiationModel.m_climateDataLoader.m_latitudeInDegree * DEG2RAD;
		m_solarRadiationModel.m_sunPositionModel.m_longitude = m_solarRadiationModel.m_climateDataLoader.m_longitudeInDegree * DEG2RAD;

		// enable Perez-Model if requested
		if (location.m_perezDiffuseRadiationModel.isEnabled())
			m_solarRadiationModel.m_diffuseRadiationPerezEnabled = true;


		// store start time offset as year and start time
		m_year = simPara.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
		m_startTime = simPara.m_interval.m_para[NANDRAD::Interval::P_Start].value;


		// *** shading factors ***

		// if we have a shading factor file given, read it and cache it in memory (not file access during simulation)

		// create a data IO file for shading factor
		if (!location.m_shadingFactorFileName.str().empty()) {
#ifdef TODO
			try {
				// remove all place holde attributes
				IBK::Path filename = IBK::Path(location.m_shadingFactorFileName)
					.withReplacedPlaceholders(pathPlaceHolders);
				m_shadingFactorFile.read(filename);
			}
			catch (IBK::Exception &ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error reading shading factors data from file '%1")
					.arg(location.m_shadingFactorFileName), FUNC_ID);
			}
			// empty files are not allowed
			if(m_shadingFactorFile.m_timepoints.empty() || m_shadingFactorFile.nValues() == 0) {
				throw IBK::Exception(IBK::FormatString("Error reading shading factors data from file '%1: "
					"No shading data!")
					.arg(location.m_shadingFactorFileName), FUNC_ID);
			}

			// resize shading factor vector
			m_shadingFactors.resize(m_shadingFactorFile.nValues());
#endif // TODO
		}

		// setup all sensors
		// 1. for all sensors check that the requested quantity is known to the model
		// 2. register surface with CCM so that it will calculate radiation on those surfaces
		//    (mind, different sensors may share the same orientation/inclination)
		// 3. store the association between sensor ID and CCM-surface ID
		for (unsigned int i = 0; i < location.m_sensors.size(); ++i) {
			const NANDRAD::Sensor &sensor = location.m_sensors[i];

			// check sensor parameters
			sensor.checkParameters();

			// retrieve orientation and incliniation
			double orientation = sensor.m_orientation.value; // in rad
			double inclination = sensor.m_inclination.value; // in rad

			// register sensor surface with solar radiation model
			unsigned int surfaceID = m_solarRadiationModel.addSurface(orientation, inclination);
			// and remember the surface ID from the solar radiation model for the sensor id
			m_sensorID2surfaceID[sensor.m_id] = surfaceID;
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing Loads!"), FUNC_ID);
	}
}


void Loads::initResults(const std::vector<AbstractModel*> & models) {
//	FUNCID(Loads::initResults);
	// resize m_results vector from keyword list
	DefaultModel::initResults(models);

	// transfer constant parameters - other models may access these values, but they are constant values and
	// never updated during the simulation.
	m_results[R_Albedo] = m_solarRadiationModel.m_albedo;
	m_results[R_Latitude] = IBK::Parameter("Latitude", m_solarRadiationModel.m_climateDataLoader.m_latitudeInDegree, "Deg").value;
	m_results[R_Longitude] = IBK::Parameter("Longitude", m_solarRadiationModel.m_climateDataLoader.m_longitudeInDegree, "Deg").value;
	m_results[R_CO2Concentration] = 0.000450;
}


int Loads::setTime(double t) {
	FUNCID(Loads::setTime);

	// cache time
	m_t = t;

	try {
		double t_climate = m_startTime + m_t;
		// Mind: if the climate data is not a cyclic (annual) data set, this is handled inside the solar radiation model
		//       hence, we do not need to apply cyclic clipping here
		m_solarRadiationModel.setTime(m_year, t_climate);
	}
	catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error setting time at time point %1 inside loads model!")
				.arg(t), FUNC_ID);
	}

	// now copy calculated angles to variables vector, but mind the base SI unit that we store our values in
	m_results[R_DeclinationAngle]				= m_solarRadiationModel.m_sunPositionModel.m_declination;
	m_results[R_ElevationAngle]					= m_solarRadiationModel.m_sunPositionModel.m_elevation;
	m_results[R_AzimuthAngle]					= m_solarRadiationModel.m_sunPositionModel.m_azimuth;
	// copy states
	m_results[R_Temperature]					= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::Temperature] + 273.15;
	m_results[R_RelativeHumidity]				= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::RelativeHumidity] / 100.0;
	m_results[R_SWRadDirectNormal]				= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::DirectRadiationNormal];
	m_results[R_SWRadDiffuseHorizontal]			= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::DiffuseRadiationHorizontal];
	m_results[R_LWSkyRadiation]					= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::LongWaveCounterRadiation];
	m_results[R_WindDirection]					= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::WindDirection] * DEG2RAD;
	m_results[R_WindVelocity]					= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::WindVelocity];
	m_results[R_AirPressure]					= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::AirPressure];

	// calculate moisture related quantities
	const double pSaturation = IBK::f_psat(m_results[R_Temperature]);
	const double airTemperature = m_results[R_Temperature];
	const double vaporPressure = m_results[R_RelativeHumidity] * pSaturation;
	const double moistureDensity = vaporPressure / (IBK::R_VAPOR * m_results[R_Temperature]);
	const double CO2Density = m_results[R_CO2Concentration] * MolarMassCO2 *
		IBK::GASPRESS_REF /(IBK::R_IDEAL_GAS * airTemperature);

	m_results[R_VaporPressure] = vaporPressure;
	m_results[R_MoistureDensity] = moistureDensity;
	m_results[R_CO2Density] = CO2Density;

#ifdef TODO
	// calculate shading factors for current time points
	if (!m_shadingFactorFile.m_filename.str().empty()) {

		IBK_ASSERT(m_startTime != nullptr);
		// correct cyclic time
		double time = *m_startTime + m_t;

		while (time >= 365. * 24. * 3600.) {
			time -= 365. * 24. * 3600.;
		}

		unsigned int upperIndex = 0;
		if (m_indexOfLastAcceptedTimePoint >= 0) {
			upperIndex = m_indexOfLastAcceptedTimePoint;
			// reset counter
			if (m_shadingFactorFile.m_timepoints[upperIndex] > time) {
				upperIndex = 0;
			}
		}
		for (; upperIndex < m_shadingFactorFile.m_timepoints.size(); ++upperIndex) {
			if (m_shadingFactorFile.m_timepoints[upperIndex] > time)
				break;
		}
		IBK_ASSERT(upperIndex > 0);

		unsigned int lowerIndex = upperIndex - 1;
		// we exceed index
		if (upperIndex == m_shadingFactorFile.m_timepoints.size()) {
			throw IBK::Exception(IBK::FormatString("Missing shading factors for time point #%1! "
				"We expect shading factor values for one year!")
				.arg(t), FUNC_ID);
		}
		// interpolate shading factors
		double alpha = 1, beta = 0;
		if (upperIndex > lowerIndex) {
			double dt = m_shadingFactorFile.m_timepoints[upperIndex] -
				m_shadingFactorFile.m_timepoints[lowerIndex];
			// error in data file
			if (dt < 0) {
				throw IBK::Exception(IBK::FormatString("Error loading shading factors for time point %#1: "
					"Time points are expected in increasing order!")
					.arg(t), FUNC_ID);
			}
			alpha = (time - m_shadingFactorFile.m_timepoints[lowerIndex]) / dt;
			beta = 1. - alpha;
			IBK_ASSERT(alpha <= 1 && beta <= 1 && alpha >= 0 && beta >= 0);
		}
		// update data
		const double *lastData = m_shadingFactorFile.data(lowerIndex);
		const double *nextData = m_shadingFactorFile.data(upperIndex);

		for (unsigned int i = 0; i < m_shadingFactors.size(); ++i) {
			m_shadingFactors[i] = alpha * nextData[i] +	beta * lastData[i];
		}
	}
#endif

	if (!m_sensorID2surfaceID.empty()) {
		IBK_ASSERT(m_sensorID2surfaceID.size() == m_vectorValuedResults[VVR_DirectSWRadOnPlane].size());
		// update all sensor values

		std::vector<double>::iterator dirRadIt = m_vectorValuedResults[VVR_DirectSWRadOnPlane].begin();
		std::vector<double>::iterator difRadIt = m_vectorValuedResults[VVR_DiffuseSWRadOnPlane].begin();
		std::vector<double>::iterator globRadIt = m_vectorValuedResults[VVR_GlobalSWRadOnPlane].begin();
		std::vector<double>::iterator incRadIt = m_vectorValuedResults[VVR_IncidenceAngleOnPlane].begin();

		// loop over all sensors
		for (std::map<unsigned int, unsigned int>::const_iterator sensorIt = m_sensorID2surfaceID.begin();
			sensorIt != m_sensorID2surfaceID.end(); ++sensorIt, ++dirRadIt, ++difRadIt, ++globRadIt, ++incRadIt)
		{
			// map: key (first) = ID of the sensor
			//      value (second) = ID of the CCM-surface

			// a sensor: no shading is considered
			unsigned int surfaceId = sensorIt->second;
			// we only need surface id for radiant loads calculation
			// (surface is equal for all surface with same incidenceAngle)
			double qRadDir, qRadDiff, incidenceAngle;
			// Note: the following call will only return the values cached for the surface
			//       but not trigger any (expensive) re-calculation.
			m_solarRadiationModel.radiationLoad(surfaceId, qRadDir, qRadDiff, incidenceAngle);
			// store values
			*dirRadIt = qRadDir;
			*difRadIt = qRadDiff;
			*globRadIt = qRadDir + qRadDiff;
			*incRadIt = incidenceAngle; // in rad
		}
	}

	// signal success
	return 0;
}


// use default implementation
void Loads::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	// generate result quantities from enums Results and VectorValuedResults
	// Note: vector valued results have zero elements so far
	DefaultModel::resultDescriptions(resDesc);

	if (m_sensorID2surfaceID.empty())
		return;

	// select all sensor ids and store them in a vector
	std::vector<unsigned int> sensorIds;
	for (std::map<unsigned int, unsigned int>::const_iterator
		it = m_sensorID2surfaceID.begin();
		it != m_sensorID2surfaceID.end(); ++it)
	{
		sensorIds.push_back(it->first);
	}

	// resize vector-valued sensor quantities
	for (int j=0; j<NUM_VVR; ++j) {
		for (unsigned int i = 0; i < resDesc.size(); ++i) {

			// vector valued quantity descriptions store the description
			// of the quantity itself as well as key strings and descriptions
			// for all vector elements

			// If we this is the result of a radiation sensor, we add ids to the vector value result,
			// so that lookup is possible via "DirectSWRadOnPlane[id=14]"
			if (resDesc[i].m_name == KeywordList::Keyword("Loads::VectorValuedResults", j))
				resDesc[i].resize(sensorIds, VectorValuedQuantityIndex::IK_ModelID);
		}
	}
}


const double * Loads::resultValueRef(const QuantityName & quantityName) const {
	// otherwise fall back to original implementation
	return DefaultModel::resultValueRef(quantityName);
}


void Loads::addSurface(unsigned int objectID, double orientationInDeg, double inclinationInDeg) {
//	FUNCID(Loads::addSurface);

	double inclination = inclinationInDeg*DEG2RAD; // in rad
	unsigned int surfaceID = m_solarRadiationModel.addSurface(orientationInDeg*DEG2RAD, inclination);
	// store mapping of object 2 CCM-surface id
	// Note: it is likely that several objects (i.e. outside surfaces of constructions) have the same orientation and
	//       inclination, like on a multi-storey west facade. The CCM will only compute the radiation loads
	//       for this surface once. And thanks to the mapping, we can retrieve pass the pointer to these results to
	//       all dependent objects.
	m_objectID2surfaceID[objectID] = surfaceID;

	// store mapping of inclination
	unsigned int inclinationIdx = 0;
	for (; inclinationIdx< m_inclinations.size(); ++inclinationIdx) {
		if (nearlyEqual(m_inclinations[inclinationIdx], inclination, 1e-5))
		{
			// Same surface definition already known,
			break;
		}
	}
	// new inclined surface
	if (inclinationIdx == m_inclinations.size()) {
		m_inclinations.push_back(inclination);

		double skyVisibility = 0.0;
		bool flatRoof = nearlyEqual(inclination, 0);
		bool verticalWall = nearlyEqual(inclination, 0.5 * PI);
		// for flat roofs we can directly return the (measured) horizontal values
		if (flatRoof) {
			skyVisibility = 1.0;
		}
		else if (verticalWall) {
			skyVisibility = 0.5;
		}
		else {
			// calculate diffuse radiation on upper and lower hemisphere
			double cosInclination = std::cos(0.5 * inclination);
			skyVisibility = cosInclination * cosInclination;
		}
		m_skyVisbility.push_back(skyVisibility);

	}
	// store mapping
	m_objectID2inclinations[objectID] = inclinationIdx;

#ifdef TODO
	// retrieve all values for external shading
	if (!m_shadingFactorFile.m_filename.str().empty() ) {
		// find object id inside shading factor data container
		unsigned int index = 0;
		for ( ; index < m_shadingFactorFile.m_nums.size(); ++index) {
			// nums vector must! contain all outside object ids
			if (m_shadingFactorFile.m_nums[index] == objectID) {
				break;
			}
		}
		// error: object is not found
		if (index == m_shadingFactorFile.m_nums.size()) {
			throw IBK::Exception(IBK::FormatString("Missing shading factor inside file '#%1'!"
				"Error registering ourtside surface object with id %2!")
				.arg(m_shadingFactorFile.m_filename).arg(objectID),
				FUNC_ID);
		}
		// error: shading factor file is not resized correctly
		IBK_ASSERT(index < m_shadingFactors.size());
		// store pointer to shading factor
		m_shadingFactorsForObjectID[objectID] = &m_shadingFactors[index];
	}
#endif // TODO

}


double Loads::qSWRad(unsigned int objectID, double & qRadDir, double & qRadDiff, double & incidenceAngle) const {
	FUNCID(Loads::qSWRad);
	try {
		// find unique surface id
		std::map<unsigned int, unsigned int>::const_iterator it =
			m_objectID2surfaceID.find(objectID);
		// a real surface
		IBK_ASSERT(it != m_objectID2surfaceID.end());
		unsigned int surfaceId = it->second;
		// we only need surface id for rdaiant loads calculation
		// (surface is equal for all surface with same incidenceAngle)
		m_solarRadiationModel.radiationLoad(surfaceId, qRadDir, qRadDiff, incidenceAngle);

		if (m_shadingFactors.empty()) {
			return qRadDir + qRadDiff;
		}
		// reduce radiation by external shading
		std::map<unsigned int, const double*>::const_iterator valueIt
			= m_shadingFactorsForObjectID.find(objectID);
		// we alread checked validity
		IBK_ASSERT(valueIt != m_shadingFactorsForObjectID.end());
		IBK_ASSERT(valueIt->second != nullptr);
		const double shadingFactor = *valueIt->second;
		// reduce radiation

		return shadingFactor * qRadDir + qRadDiff;
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error calulation solar radiation on object with id %1 at time %2!")
			.arg(objectID).arg(m_startTime + m_t), FUNC_ID);
	}
}


double Loads::skyVisibility(unsigned int objectID) const {
	// find unique surface id
	std::map<unsigned int, unsigned int>::const_iterator it =
		m_objectID2inclinations.find(objectID);
	IBK_ASSERT(it != m_objectID2inclinations.end());
	unsigned int inclinationIdx = it->second;
	// we only need index for rdaiant loads calculation
	// (surface is equal for all surface with same inclination)
	IBK_ASSERT(inclinationIdx < m_skyVisbility.size());
	return m_skyVisbility[inclinationIdx];
}

} // namespace NANDRAD_MODEL
