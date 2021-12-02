/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
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
#include <IBK_CSVReader.h>
#include <IBK_UnitVector.h>

#include <DATAIO_DataIO.h>

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
		// basic parameter checking
		location.checkParameters();

		// for now we require a climate data file
		// if dummy values are needed, it is possible to create a simple dummy climate data file
		// with constant values throughout the year
		if (location.m_climateFilePath.str().empty())
			throw IBK::Exception("Climate data location required (Location.ClimateReference).", FUNC_ID);

		IBK::Path climateFile = IBK::Path(location.m_climateFilePath).withReplacedPlaceholders(pathPlaceHolders);

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
		m_solarRadiationModel.m_albedo = location.m_para[NANDRAD::Location::P_Albedo].value;

		// finally update the latitude and longitude in the sunPositionModel
		m_solarRadiationModel.m_sunPositionModel.m_latitude = m_solarRadiationModel.m_climateDataLoader.m_latitudeInDegree * DEG2RAD;
		m_solarRadiationModel.m_sunPositionModel.m_longitude = m_solarRadiationModel.m_climateDataLoader.m_longitudeInDegree * DEG2RAD;

		// enable Perez-Model if requested
		m_solarRadiationModel.m_diffuseRadiationPerezEnabled = location.m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].isEnabled();

		// store start time offset as year and start time
		m_year = simPara.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
		m_startTime = simPara.m_interval.m_para[NANDRAD::Interval::P_Start].value;



		// *** shading factors ***

		// shading factor files are expected to be cyclic data, except flag is set
		if (location.m_flags[NANDRAD::Location::F_ContinuousShadingFactorData].isEnabled())
			m_cyclicShadingFactors = false;

		// if we have a shading factor file given, read it and cache it in memory (not file access during simulation)

		// create a data IO file for shading factor
		if (location.m_shadingFactorFilePath.isValid()) {
			// check file extension: tsv are read with tsv-reader, d6o and d6b are read with DataIO.

			// replace path placeholders
			IBK::Path fullShadingFilePath = location.m_shadingFactorFilePath.withReplacedPlaceholders(pathPlaceHolders);

			// data is transfered into IBK::Matrix structure where it will be interpolated accordingly


			// *** TSV Files ***

			if (IBK::string_nocase_compare(fullShadingFilePath.extension(), "tsv")) {
				IBK::CSVReader reader;
				try {
					reader.read(fullShadingFilePath, false /*read full file*/, true /*extract units*/);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error reading shading factors data from file '%1'.")
						.arg(fullShadingFilePath), FUNC_ID);
				}
				// empty files are not allowed
				if (reader.m_nRows == 0 || reader.m_nColumns < 2) {
					throw IBK::Exception(IBK::FormatString("Shading factor file '%1' does not contain valid data (no time points or columns).")
						.arg(fullShadingFilePath), FUNC_ID);
				}
				// first column header must have time unit
				if (reader.m_units[0].empty())
					throw IBK::Exception(IBK::FormatString("Missing time unit in first column of file '%1'.")
						.arg(fullShadingFilePath), FUNC_ID);
				IBK::Unit timeUnit;
				try {
					timeUnit = IBK::Unit(reader.m_units[0]);
					if (timeUnit.base_id() != IBK_UNIT_ID_SECONDS)
						throw IBK::Exception(IBK::FormatString("'%1' is not a known time unit.").arg(timeUnit.name()), FUNC_ID);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Invalid time unit '%2' in first column of file '%1'.")
						.arg(fullShadingFilePath).arg(reader.m_units[0]), FUNC_ID);
				}
				// check units of all other columns units of all other columns are not checked
				try {
					for (unsigned int i=1; i<reader.m_nColumns; ++i) {
						m_externalShadingFactorIDs.push_back(IBK::string2val<unsigned int>(reader.m_captions[i]));
						if (reader.m_units[i] != "---")
							throw IBK::Exception(IBK::FormatString("Invalid unit for shading factors '%1', expected '---'.")
												 .arg(reader.m_units[i]), FUNC_ID);
					}
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Invalid value unit in file '%1'.")
						.arg(fullShadingFilePath), FUNC_ID);
				}

				// now copy data to vectors
				IBK::UnitVector tvec("tvec", timeUnit);
				tvec.resize(reader.m_nRows);
				m_externalShadingFactors.resize(reader.m_nRows);

				for (unsigned int i=0; i<reader.m_nRows; ++i) {
					tvec.m_data[i] = reader.colData(0)[i]; // first column - time point

					m_externalShadingFactors[i].resize(reader.m_nColumns-1);
					for (unsigned int j=1; j<reader.m_nColumns; ++j) {
						m_externalShadingFactors[i][j-1] = reader.colData(j)[i];
					}

				}
				tvec.convert(IBK::Unit(IBK_UNIT_ID_SECONDS));
				m_externalShadingFactorTimePoints.swap(tvec.m_data);
			}


			// *** DATAIO ***

			else if (IBK::string_nocase_compare(fullShadingFilePath.extension(), "d6o") ||
					 IBK::string_nocase_compare(fullShadingFilePath.extension(), "d6b"))
			{
				DATAIO::DataIO shadingFile;
				try {
					shadingFile.read(fullShadingFilePath);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error reading shading factors data from file '%1'.")
						.arg(fullShadingFilePath), FUNC_ID);
				}
				// empty files are not allowed
				if (shadingFile.m_timepoints.empty() || shadingFile.nValues() == 0) {
					throw IBK::Exception(IBK::FormatString("Shading factor file '%1' does not contain valid data (no time points or columns).")
						.arg(fullShadingFilePath), FUNC_ID);
				}
				// transfer data into our working data structure
				try {
					IBK::Unit timeUnit(shadingFile.m_timeUnit);
					if (timeUnit.base_id() != IBK_UNIT_ID_SECONDS)
						throw IBK::Exception(IBK::FormatString("'%1' is not a known time unit.").arg(timeUnit.name()), FUNC_ID);
					// time points in DataIO containers are already in seconds
					m_externalShadingFactorTimePoints = shadingFile.m_timepoints;
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Invalid time unit '%2' in file '%1'.")
						.arg(fullShadingFilePath).arg(shadingFile.m_timeUnit), FUNC_ID);
				}
				m_externalShadingFactorIDs = shadingFile.m_nums;

				// check value unit - we require unit --- so that we do not have to convert units while reading
				try {
					if (shadingFile.m_valueUnit != "---")
						throw IBK::Exception(IBK::FormatString("Invalid unit for shading factors '%1', expected '---'.").arg(shadingFile.m_valueUnit), FUNC_ID);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Invalid value unit in file '%1'.")
						.arg(fullShadingFilePath), FUNC_ID);
				}
				for (unsigned int i=0; i<shadingFile.m_timepoints.size(); ++i) {
					try {
						const double * data = shadingFile.data(i);
						std::vector<double> dataVec(data, data + shadingFile.nValues());
						m_externalShadingFactors.emplace_back(dataVec);
					}
					catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString("Error reading shading factors data from file '%1'.")
							.arg(fullShadingFilePath), FUNC_ID);
					}
				}
			}

			else {
				throw IBK::Exception(IBK::FormatString("Unrecognized file type for shading factors file '%1'.")
					.arg(fullShadingFilePath), FUNC_ID);
			}

			// in cyclic use, make sure, that time points do not exceed one year
			if (m_cyclicShadingFactors) {
				if (m_externalShadingFactorTimePoints.back() >= SECONDS_PER_YEAR)
					throw IBK::Exception(IBK::FormatString("Last time point in shading factors file '%1' exceeds one year but must be < one year for cyclic usage.")
						.arg(fullShadingFilePath), FUNC_ID);
			}

			// check for continuous time points
			double t_last = m_externalShadingFactorTimePoints.front();
			for (unsigned int i=1; i<m_externalShadingFactorTimePoints.size(); ++i) {
				if (t_last >= m_externalShadingFactorTimePoints[i])
					throw IBK::Exception(IBK::FormatString("Time point '%1' in shading factors file '%2' exceeds previous time point (strictly monotonic time series required).")
						.arg(m_externalShadingFactorTimePoints[i]).arg(fullShadingFilePath), FUNC_ID);
				t_last = m_externalShadingFactorTimePoints[i];
			}

			// resize shading factor cache for calculates values
			m_shadingFactors.resize(m_externalShadingFactorIDs.size());
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
	double t_climate = m_startTime + m_t;

	try {
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

	// calculate shading factors for current time points
	if (!m_externalShadingFactors.empty()) {

		// correct cyclic time
		if (m_cyclicShadingFactors) {
			while (t_climate >= SECONDS_PER_YEAR) {
				t_climate -= SECONDS_PER_YEAR;
			}
		}

		std::vector<double>::const_iterator tIt = std::lower_bound(
					m_externalShadingFactorTimePoints.begin(),
					m_externalShadingFactorTimePoints.end(), t_climate);
		unsigned int timeIndex = tIt - m_externalShadingFactorTimePoints.begin();

		unsigned int upperIndex = 0, lowerIndex = 0;
		double alpha = 0.0;
		bool needInterpolation = true;
		// special case handling
		if (t_climate <= m_externalShadingFactorTimePoints.front()) {
			if (m_cyclicShadingFactors) {
				// interpolate between last time point (of last year) and first time point of this year
				upperIndex = m_externalShadingFactorTimePoints.size()-1;
				lowerIndex = 0;
				double time_left_at_end_of_year = SECONDS_PER_YEAR - m_externalShadingFactorTimePoints.back();
				alpha = (time_left_at_end_of_year + t_climate)/(m_externalShadingFactorTimePoints.front() + time_left_at_end_of_year);
			}
			else {
				// not-cyclic data, just copy first data set
				std::copy(m_externalShadingFactors.front().begin(),
						  m_externalShadingFactors.front().end(),
						  m_shadingFactors.begin());
				needInterpolation = false;
			}
		}
		else if (t_climate > m_externalShadingFactorTimePoints.back()) {
			if (m_cyclicShadingFactors) {
				// interpolate between last time point (of last year) and first time point of this year
				upperIndex = m_externalShadingFactorTimePoints.size()-1;
				lowerIndex = 0;
				double time_left_at_end_of_year = SECONDS_PER_YEAR - m_externalShadingFactorTimePoints.back();
				alpha = (t_climate - m_externalShadingFactorTimePoints.back())/(m_externalShadingFactorTimePoints.front() + time_left_at_end_of_year);
			}
			else {
				// not-cyclic data, just copy first data set
				std::copy(m_externalShadingFactors.back().begin(),
						  m_externalShadingFactors.back().end(),
						  m_shadingFactors.begin());
				needInterpolation = false;
			}
		}
		else {
			// standard case, set interval that contains current time point
			upperIndex = timeIndex;
			lowerIndex = timeIndex-1;
			alpha = (t_climate - m_externalShadingFactorTimePoints[lowerIndex])/
					(m_externalShadingFactorTimePoints[upperIndex] - m_externalShadingFactorTimePoints[lowerIndex]);
		}


		if (needInterpolation) {
			const double *lastData = m_externalShadingFactors[lowerIndex].data();
			const double *nextData = m_externalShadingFactors[upperIndex].data();
			double beta = 1 - alpha;
			IBK_ASSERT(beta >= 0.0 && beta <= 1.0);

			for (unsigned int i = 0; i < m_shadingFactors.size(); ++i) {
				m_shadingFactors[i] = alpha * nextData[i] +	beta * lastData[i];
			}

		}
	}

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


void Loads::addSurface(unsigned int objectID, double orientationInDeg, double inclinationInDeg) {
	FUNCID(Loads::addSurface);

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


	// Issue warning if no shading factors are provided for a construction/embedded object surface

	if (!m_externalShadingFactors.empty()) {
		int idx = -1;
		for (unsigned int i=0; i<m_externalShadingFactorIDs.size(); ++i) {
			if (m_externalShadingFactorIDs[i] == objectID) {
				m_shadingFactorsForObjectID[objectID] = &m_shadingFactors[i];
				idx = (int)i;
				break;
			}
		}
		if (idx == -1)
			IBK::IBK_Message(IBK::FormatString("Exterior shading factor file does not contain data for surface/object with ID #%1.")
							 .arg(objectID), IBK::MSG_WARNING, FUNC_ID);
	}

}


double Loads::qSWRad(unsigned int objectID, double & qRadDir, double & qRadDiff, double & incidenceAngle) const {
	FUNCID(Loads::qSWRad);
	try {

		// test if objectID matches a construction instance or embedded object
		std::map<unsigned int, unsigned int>::const_iterator it = m_objectID2surfaceID.find(objectID);
		if (it != m_objectID2surfaceID.end()) {
			// we only need surface id for radiant loads calculation
			// (surface is equal for all surface with same incidenceAngle)
			unsigned int surfaceId = it->second; // surface id used in CCM RadiationModel
			// retrieve cached radiation loads onto surface from CCM library
			m_solarRadiationModel.radiationLoad(surfaceId, qRadDir, qRadDiff, incidenceAngle);

			// without external shading we can already return these results
			if (m_shadingFactors.empty())
				return qRadDir + qRadDiff;

			// reduce radiation by external shading
			std::map<unsigned int, const double*>::const_iterator valueIt = m_shadingFactorsForObjectID.find(objectID);

			// if no shading factor is available for this surface ID, we return 1 ("unshaded" by default)
			if (valueIt == m_shadingFactorsForObjectID.end())
				return qRadDir + qRadDiff;

			IBK_ASSERT(valueIt->second != nullptr);

			const double shadingFactor = *valueIt->second;
			// reduce direct radiation
			qRadDir *= shadingFactor;
			return qRadDir + qRadDiff;
		}
		// a sensor id
		else {
			// find unique sensor id
			std::map<unsigned int, unsigned int>::const_iterator it = m_sensorID2surfaceID.find(objectID);

			// Callers of this function may directly send an ID for a surface from user parametrization, and hence
			// this ID may be invalid and unchecked. Hence, we must handle the case with an exception.
			if (it == m_sensorID2surfaceID.end())
				throw IBK::Exception(IBK::FormatString("Unknown sensor/construction instance/embedded "
													   "object ID #%1.").arg(objectID), FUNC_ID);
			// find unique surface id
			unsigned int surfaceId = it->second;
			// we only need surface id for rdaiant loads calculation
			// (surface is equal for all surface with same incidenceAngle)
			m_solarRadiationModel.radiationLoad(surfaceId, qRadDir, qRadDiff, incidenceAngle);
			// sensors are never shaded (if you need a shaded window sensor, use the radiation on the embedded object
			// itself as sensor)
			return qRadDir + qRadDiff;
		}
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error calculating solar radiation on object with id %1 at time %2!")
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
