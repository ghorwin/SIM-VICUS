/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

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

using namespace std;

namespace NANDRAD_MODEL {

inline bool nearlyEqual(double a, double b, double eps = 1e-5) {
	return std::fabs(a-b) < eps;
}

#if 0
// Helper function for dewpoint calculation
class DewPointRootFunction : public IBK::ScalarFunction {
public:
	DewPointRootFunction(double pv) : m_pv(pv) {}
	virtual double operator() (double x) const {
		return m_pv - IBK::f_psat(x);
	}

	double m_pv;
};
#endif

// *** Loads ***

Loads::Loads() : DefaultModel(0,std::string()),
				m_year(0),
				m_startTime(NULL),
				m_t(-1),
				m_location(NULL),
				m_indexOfLastAcceptedTimePoint(-1)
{
}

void Loads::setup(const NANDRAD::Location & location, const NANDRAD::SimulationParameter &simPara,
	const std::map<std::string, IBK::Path> & pathPlaceHolders) {

	const char * const FUNC_ID = "[Loads::setup]";

	try {
		// fill m_dataset vector
		if (location.m_climateFileName.str().empty())
			throw IBK::Exception("Climate data location required (Location.ClimateReference).", FUNC_ID);

		IBK::Path climateFile = IBK::Path(location.m_climateFileName).withReplacedPlaceholders(pathPlaceHolders);

		try {
			m_solarRadiationModel.m_climateDataLoader.readClimateData(climateFile);
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading climate data from file '%1")
				.arg(climateFile.str()), FUNC_ID);
		}

		// set location pointer
		m_location = &location;

		m_year = simPara.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;
		// set start year for climate data container
		m_solarRadiationModel.m_climateDataLoader.m_startYear = m_year;

		// retreive start time offset
		IBK_ASSERT(!simPara.m_interval.m_para[NANDRAD::Interval::IP_START].name.empty());
		m_startTime = &simPara.m_interval.m_para[NANDRAD::Interval::IP_START].value;

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
		for (unsigned int i = 0; i < location.m_sensors.size(); ++i) {
			const NANDRAD::Sensor &sensor = location.m_sensors[i];
			const std::string quantity = sensor.m_quantity;
			// error: wrong quantity requested
			if (quantity != KeywordList::Keyword("Loads::VectorValuedResults", VVR_SWRadOnPlane)) {
				throw IBK::Exception(IBK::FormatString("Quantity '%1' of sensor with id #%2 is not supported!")
					.arg(quantity).arg(sensor.m_id), FUNC_ID);
			}
			// retrieve orientation and incliniation
			if (sensor.m_orientation.name.empty())
				throw IBK::Exception(IBK::FormatString("Missing Parameter 'Orientation' of sensor with id #%1!")
					.arg(sensor.m_id), FUNC_ID);
			double orientation = sensor.m_orientation.value;
			// find inclination
			if (sensor.m_inclination.name.empty())
				throw IBK::Exception(IBK::FormatString("Missing Parameter 'Inclination' of sensor with id #%1!")
					.arg(sensor.m_id), FUNC_ID);
			double inclination = sensor.m_inclination.value;

			unsigned int surfaceID = m_solarRadiationModel.addSurface(orientation, inclination);
			// store mapping of obejct 2 surface id
			m_sensorID2surfaceID[sensor.m_id] = surfaceID;
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing Loads!"), FUNC_ID);
	}
}


void Loads::initResults(const std::vector<AbstractModel*> & models) {
	const char * const FUNC_ID = "[Loads::initResults]";
	// resize m_results vector from keyword list
	DefaultModel::initResults(models);
	// check if model was setup already
	IBK_ASSERT(m_location != NULL);
	// check validity of the parameter data
	const char * const MISSING_PARAMETER_MESSAGE = "Error initializing climate data: "
			"Parameter '%1' is not defined.";
	// latitude
	const IBK::Parameter &latitude = m_location->m_para[NANDRAD::Location::LP_LATITUDE];
	if (latitude.name.empty())
		throw IBK::Exception(IBK::FormatString(MISSING_PARAMETER_MESSAGE).arg("Latitude"), FUNC_ID);
	if (latitude.get_value("Deg") < -90 || latitude.get_value("Deg") > 90) {
		throw IBK::Exception(IBK::FormatString("Error initializing climate data: "
			"Location parameter 'Latitude' is expected between -90 and 90 degrees."), FUNC_ID);
	}
	m_solarRadiationModel.m_sunPositionModel.m_latitude = latitude.value;

	// longitude
	const IBK::Parameter &longitude = m_location->m_para[NANDRAD::Location::LP_LONGITUDE];
	if (longitude.name.empty())
		throw IBK::Exception(IBK::FormatString(MISSING_PARAMETER_MESSAGE).arg("Longitude"), FUNC_ID);
	if (longitude.get_value("Deg") < -180 || longitude.get_value("Deg") > 180) {
		throw IBK::Exception(IBK::FormatString("Error initializing climate data: "
			"Location parameter 'Longitude' is expected between -180 and 180 degrees."), FUNC_ID);
	}
	m_solarRadiationModel.m_sunPositionModel.m_longitude = longitude.value;

	// albeda
	const IBK::Parameter &albedo = m_location->m_para[NANDRAD::Location::LP_ALBEDO];
	if (albedo.name.empty())
		throw IBK::Exception(IBK::FormatString(MISSING_PARAMETER_MESSAGE).arg("Albedo"), FUNC_ID);
	if (albedo.value < 0 || albedo.value > 1) {
		throw IBK::Exception(IBK::FormatString("Error initializing climate data: "
			"Location parameter 'Albedo' is expected between 0 and 1."), FUNC_ID);
	}
	m_solarRadiationModel.m_albedo = albedo.value;

	// height is optional for now -> defaults to zero
	const IBK::Parameter &height = m_location->m_para[NANDRAD::Location::LP_ALTITUDE];
	if (!height.name.empty()) {
		if (height.value < 0) {
			throw IBK::Exception(IBK::FormatString("Error initializing climate data: "
				"Location parameter 'Altitude' is expected greater than 0."), FUNC_ID);
		}
	}

	// transfer constant parameters
	m_results[R_Albedo]		= albedo;
	m_results[R_Latitude]	= latitude;
	m_results[R_CO2Concentration].value = 0.000450;
}


int Loads::setTime(double t) {
	const char * const FUNC_ID = "[Loads::setTime]";

	if (m_t == t)
		// signal success
		return 0;


	// set time
	m_t = t ;

	try {
		IBK_ASSERT(m_startTime != NULL);
		double t_climate = *m_startTime + m_t;
		while (t_climate >= 365*24*3600)
			t_climate -= 365*24*3600;
		m_solarRadiationModel.setTime(m_year, t_climate);
	}
	catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error setting time at time point %1 inside loads model!")
				.arg(t), FUNC_ID);
	}

	// now copy calculated angles to varaibles vector
	m_results[R_DeclinationAngle].value			= m_solarRadiationModel.m_sunPositionModel.m_declination;
	m_results[R_ElevationAngle].value			= m_solarRadiationModel.m_sunPositionModel.m_elevation;
	m_results[R_AzimuthAngle].value				= m_solarRadiationModel.m_sunPositionModel.m_azimuth;
	// copy states
	m_results[R_Temperature].value				= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::Temperature] + 273.15;
	m_results[R_RelativeHumidity].value			= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::RelativeHumidity] / 100.0;
	m_results[R_SWRadDirectNormal].value		= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::DirectRadiationNormal];
	m_results[R_SWRadDiffuseHorizontal].value	= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::DiffuseRadiationHorizontal];
	m_results[R_LWSkyRadiation].value			= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::LongWaveCounterRadiation];
	m_results[R_WindDirection].value			= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::WindDirection] * IBK::DEG2RAD;
	m_results[R_WindVelocity].value				= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::WindVelocity];
	m_results[R_AirPressure].value				= m_solarRadiationModel.m_climateDataLoader.m_currentData[CCM::ClimateDataLoader::AirPressure];

	// calculate moisture relatied quantities
	const double pSaturation = IBK::f_psat(m_results[R_Temperature].value);
	const double airTemperature = m_results[R_Temperature].value;
	const double vaporPressure = m_results[R_RelativeHumidity].value * pSaturation;
	const double moistureDensity = vaporPressure / (IBK::R_VAPOR * m_results[R_Temperature].value);
	const double CO2Density = m_results[R_CO2Concentration].value * MolarMassCO2 *
		IBK::GASPRESS_REF /(IBK::R_IDEAL_GAS * airTemperature);

	m_results[R_VaporPressure].value = vaporPressure;
	m_results[R_MoistureDensity].value = moistureDensity;
	m_results[R_CO2Density].value = CO2Density;

#ifdef TODO
	// calculate shading factors for current time points
	if (!m_shadingFactorFile.m_filename.str().empty()) {

		IBK_ASSERT(m_startTime != NULL);
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
#endif // TODO


	// signal success
	if (m_sensorID2surfaceID.empty())
		return 0;

	IBK_ASSERT(m_sensorID2surfaceID.size() == m_vectorValuedResults[VVR_SWRadOnPlane].size());
	// update all sensor values
	std::vector<double>::iterator sensorValIt = m_vectorValuedResults[VVR_SWRadOnPlane].begin();

	// loop over all sensors
	for (std::map<unsigned int, unsigned int>::const_iterator
		sensorIt = m_sensorID2surfaceID.begin();
		sensorIt != m_sensorID2surfaceID.end();
		++sensorIt, ++sensorValIt) {
		// update value
		// a sensor: n o shading is considered
		unsigned int surfaceId = sensorIt->second;
		// we only need surface id for radiant loads calculation
		// (surface is equal for all surface with same incidenceAngle)
		double qRadDir, qRadDiff, incidenceAngle;
		m_solarRadiationModel.radiationLoad(surfaceId, qRadDir, qRadDiff, incidenceAngle);
		// store value
		*sensorValIt = qRadDir + qRadDiff;
	}

	// signal success
	return 0;
}


void Loads::stepCompleted(double t) {
#ifdef TODO
	const char * const FUNC_ID = "[Loads::stepCompleted]";
	if (m_shadingFactorFile.m_filename.str().empty())
		return;

	// correct cyclic time
	IBK_ASSERT(m_startTime != NULL);
	double time = *m_startTime + t;

	// set starting point for search
	unsigned int i = 0;
	if (m_indexOfLastAcceptedTimePoint >= 0)
		i = m_indexOfLastAcceptedTimePoint;

	// cyclic behaviour: set to start if wqe exceed
	// period of 1 year
	while (time >= 365. * 24. * 3600.) {
		time -= 365. * 24. * 3600.;
	}
	// reset counter
	if (m_shadingFactorFile.m_timepoints[i] > time) {
		i = 0;
	}
	// set a new starter index for next time step
	for (;  i < m_shadingFactorFile.m_timepoints.size(); ++i) {
		if (m_shadingFactorFile.m_timepoints[i] > time)
			break;
	}
	// we exceed index
	if (i == m_shadingFactorFile.m_timepoints.size()) {
		throw IBK::Exception(IBK::FormatString("Missing shading factors for time point #%1! "
			"We expect shading factor values for one year!")
			.arg(t), FUNC_ID);
	}
	IBK_ASSERT(i > 0);
	m_indexOfLastAcceptedTimePoint = i - 1;
#endif
}

// use default implementation
void Loads::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	DefaultModel::resultDescriptions(resDesc);

	if (m_sensorID2surfaceID.empty())
		return;
	// select all sensor ids:
	// at the moment we only measure short wave radiation
	std::set<unsigned int> sensorIds;
	for (std::map<unsigned int, unsigned int>::const_iterator
		it = m_sensorID2surfaceID.begin();
		it != m_sensorID2surfaceID.end(); ++it)
		sensorIds.insert(it->first);

	std::string category = "Loads::VectorValuedResults";
	// resizte sensor quantities
	for (unsigned int i = 0; i < resDesc.size(); ++i) {

		// vector valued quantity descriptions store the description
		// of the quantity itself as well as key strings and descriptions
		// for all vector elements
		if(resDesc[i].m_name == KeywordList::Keyword(category.c_str(), VVR_SWRadOnPlane))
			resDesc[i].resize(sensorIds, VectorValuedQuantityIndex::IK_ModelID);
	}
}

const double * Loads::resultValueRef(const QuantityName & quantityName) const {
	/// \todo  check for special namings "SWRadDirect_xxx" and "SWRadDiffuse_xxx"


	// otherwise fall back to original implementation
	return DefaultModel::resultValueRef(quantityName);
}


#if 0
unsigned int Loads::FMU2Interfaces() const {
	return FMU2QuantityDescription::ID_Climate;
}

void Loads::FMU2ExportReference(const QuantityName &targetName,
	unsigned int &sourceID, int &modelType,
	QuantityName &quantity, bool &constant) const {

	std::string categoryTarget = "FMU2ExportModel::InputReferences";
	std::string categorySource = "Loads::Results";
	// extract quantity
	IBK_ASSERT(KeywordList::KeywordExists(categoryTarget.c_str(), targetName.name()));

	FMU2ExportModel::InputReferences targetQuantity =
		(FMU2ExportModel::InputReferences)
		KeywordList::Enumeration(categoryTarget.c_str(), targetName.name());

	quantity.clear();
	// decide which quantity to use
	switch (targetQuantity) {
	case FMU2ExportModel::InputRef_AmbientAirTemperature:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_Temperature);
	break;
	case FMU2ExportModel::InputRef_AmbientAirRelativeHumidity:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_RelativeHumidity);
	break;
	case FMU2ExportModel::InputRef_DirectRadiationNormal:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_SWRadDirectNormal);
	break;
	case FMU2ExportModel::InputRef_DiffuseRadiationHorizontal:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_SWRadDiffuseHorizontal);
	break;
	case FMU2ExportModel::InputRef_WindVelocity:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_WindVelocity);
	break;
	case FMU2ExportModel::InputRef_WindDirection:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_WindDirection);
	break;
	case FMU2ExportModel::InputRef_LongWaveSkyRadiation:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_LWSkyRadiation);
	break;
	case FMU2ExportModel::InputRef_AzimuthAngle:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_AzimuthAngle);
	break;
	case FMU2ExportModel::InputRef_ElevationAngle:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_ElevationAngle);
	break;
	case FMU2ExportModel::InputRef_AmbientAirPressure:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_AirPressure);
	break;
	case FMU2ExportModel::InputRef_AmbientAirCO2Concentration:
		quantity = KeywordList::Keyword(categorySource.c_str(), R_CO2Concentration);
	break;
	default: break;
	}

	// no quantity: skip
	if (quantity.empty())
		return;

	sourceID = id();
	modelType = (int) referenceType();
	constant = false;
}
#endif

void Loads::addSurface(unsigned int objectID, double orientation, double inclination) {
	const char * const FUNC_ID = "[Loads::addSurface]";

	unsigned int surfaceID = m_solarRadiationModel.addSurface(orientation, inclination);
	// store mapping of obejct 2 surface id
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
		bool verticalWall = nearlyEqual(inclination, 0.5 * IBK::PI);
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
	const char * const FUNC_ID = "[Loads::qSWRad]";
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
		IBK_ASSERT(valueIt->second != NULL);
		const double shadingFactor = *valueIt->second;
		// reduce radiation

		return shadingFactor * qRadDir + qRadDiff;
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error calulation solar radiation on object with id %1 at time %2!")
			.arg(objectID).arg(*m_startTime + m_t), FUNC_ID);
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
