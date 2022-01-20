
#include <sstream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <cassert>
#include <cmath>
#include <iostream>

#include "fmi2common/fmi2Functions.h"
#include "fmi2common/fmi2FunctionTypes.h"

#include "CO2ComfortVentilation.h"
#include "Helpers.h"

// FMI interface variables

#define FMI_PARA_ResultRootDir 42
#define FMI_INPUT_AmbientTemperature 1
#define FMI_INPUT_RoomAirTemperature 1000
#define FMI_INPUT_MaximumCO2Concentration 2000
#define FMI_INPUT_MinimumAirTemperature 3000
#define FMI_INPUT_MaximumAirTemperature 4000
#define FMI_INPUT_MinimumAirChangeRate 5000
#define FMI_INPUT_MaximumAirChangeRateIncrease 6000
#define FMI_OUTPUT_AirChangeRate 7000
#define FMI_OUTPUT_CO2Concentration 8000


// *** Variables and functions to be implemented in user code. ***

// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{17d077d8-59b4-11ec-8093-3ce1a14c97e0}";

const double CO2ComfortVentilation::RIdealGas = 8.314472;
const double CO2ComfortVentilation::molarMassCO2 = 44.01e-03;
const double CO2ComfortVentilation::referencePressure = 101325;
const double CO2ComfortVentilation::EPS = 0.1;
const unsigned int CO2ComfortVentilation::MAX_ITER = 10;
const unsigned int CO2ComfortVentilation::NUM_INT_INTVALS = 4;


// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new CO2ComfortVentilation; // caller takes ownership
}


CO2ComfortVentilation::CO2ComfortVentilation() :
	InstanceData()
{
	// initialize input variables and/or parameters
	m_stringVar[FMI_PARA_ResultRootDir] = "";
	m_realVar[FMI_INPUT_AmbientTemperature] = 283.15;
}


CO2ComfortVentilation::~CO2ComfortVentilation() {
}


// create a model instance
void CO2ComfortVentilation::init() {
	logger(fmi2OK, "progress", "Starting initialization.");

	if (m_modelExchange) {
		// not supported, yet
	}
	else {
		try {

			// initialize from project file
			// Generate project filename
			Path resourcePath = Path::fromURI(m_resourceLocation);
			Path filePath = resourcePath / "Project.txt"; // append project file, always the same within FMU container
			read(filePath);

			// create path name including schedules
			Path tsvBasePathCo2Load = m_resourceLocation / Path("CO2LoadPerAreaSchedule");

			// select all schedule names
			std::set<std::string> scheduleCO2SourceNames;
			for (const std::pair<const int, std::string> &scheduleName : m_zoneScheduleNames) {
				// skip empty names
				if(scheduleName.second.empty())
					continue;

				scheduleCO2SourceNames.insert(scheduleName.second);
			}

			// now create splines for all schedules
			for (const std::string &scheduleName : scheduleCO2SourceNames) {

				// read CO2 load schedule
				Path tsvPath = resourcePath / scheduleName;
				tsvPath.addExtension("tsv");

				try {
					// read schedule splines
					LinearSpline spline;
					spline.readTsv(tsvPath);

					// store spline
					m_co2LoadPerZoneFloorAreasSplines[scheduleName] = spline;
				}
				catch (std::runtime_error &ex) {
					throw std::runtime_error(std::string("Error reading schedule from file '")
						+ tsvPath.str() + std::string("': ") + ex.what() + std::string("."));
				}

			}

			// read ambient CO2 concentration spline
			Path tsvBasePathAmbientCO2 = resourcePath / "AmbientCO2ConcentrationSchedule.tsv";
			try {
				// read schedule splines
				LinearSpline spline;
				spline.readTsv(tsvBasePathAmbientCO2);

				// store spline
				m_ambientCO2ConcentrationSpline = spline;
			}
			catch (std::runtime_error &ex) {
				throw std::runtime_error(std::string("Error reading schedule from file '")
					+ tsvBasePathAmbientCO2.str() + std::string("': ") + ex.what() + std::string("."));
			}

			// initialize result quantities, input output variables
			for (const std::pair<const int, double> &zoneVol : m_zoneVolumes) {
				// retrieve zone id
				int zoneId = zoneVol.first;
				// set start values
				double startAirChangeRate = 0.;
				// resize and set input varables
				m_realVar[FMI_INPUT_MinimumAirChangeRate + zoneId] = startAirChangeRate;
				// zone air temperature [K] from FMI inputs
				m_realVar[FMI_INPUT_RoomAirTemperature + zoneId] = m_startAirTemperature;
				// maximum CO2 concentration [mol/mol] from FMI inputs
				m_realVar[FMI_INPUT_MaximumCO2Concentration + zoneId] = m_startCO2Concentration;
				// minimum air change rate [1/s] from FMI inputs
				m_realVar[FMI_INPUT_MinimumAirChangeRate + zoneId] = 0.0;
				// maximum air change rate [1/s] from FMI inputs
				m_realVar[FMI_INPUT_MaximumAirChangeRateIncrease + zoneId] = 0.0;
				// minimum air temperature [K] from FMI inputs
				m_realVar[FMI_INPUT_MinimumAirTemperature + zoneId] = m_startAirTemperature;
				// maximum air temperature [K] from FMI inputs
				m_realVar[FMI_INPUT_MaximumAirTemperature + zoneId] = m_startAirTemperature;

				// set individual values

				// cluclate air change rate
				calculateAirChangeRate(startAirChangeRate, m_startAirTemperature, 273.15, m_startCO2Concentration,
									   m_startCO2Concentration, m_startAirTemperature, m_startAirTemperature,
									   0.0, 0.0);
				// calculte mass
				double startCO2Density = m_startCO2Concentration * molarMassCO2 * referencePressure/( RIdealGas * m_startAirTemperature);

				m_zoneAirChangeRates[zoneId] = startAirChangeRate;
				m_zoneCO2Masses[zoneId] = startCO2Density * zoneVol.second;
				m_realVar[FMI_OUTPUT_AirChangeRate + zoneId] = startAirChangeRate;
				m_realVar[FMI_OUTPUT_CO2Concentration + zoneId] = m_startCO2Concentration;
			}

			// initialize integrator for co-simulation
			m_currentTimePoint = 0;
			m_lastTimePoint = 0;
		}
		catch (std::runtime_error &ex) {
			logger(fmi2Error, "logStatusError", ex.what());
			throw std::runtime_error("Error initializing FMU.");
		}
	}

	logger(fmi2OK, "progress", "Initialization complete.");
}


// model exchange: implementation of derivative and output update
void CO2ComfortVentilation::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;

	// not supported, yet

	// reset externalInputVarsModified flag
	m_externalInputVarsModified = false;
}


// Co-simulation: time integration
void CO2ComfortVentilation::integrateTo(double tCommunicationIntervalEnd) {

	// state of FMU before integration:
	m_zoneCO2MassesLastTimePoint = m_zoneCO2Masses;
	m_lastTimePoint = m_currentTimePoint;

	// get input variables
	double ambientTemperature = m_realVar[FMI_INPUT_AmbientTemperature];

	// calculated quantities
	// ambient CO2 density [kg/m3]
	double ambientCO2Density = m_ambientCO2ConcentrationSpline.value(m_lastTimePoint)
							* molarMassCO2 * referencePressure / (RIdealGas * ambientTemperature);
	// time step size
	double deltaT = tCommunicationIntervalEnd - m_lastTimePoint;

	// we loop through all containers via iterator in order to avoid unneccesary search operations
	std::map<int, double>::const_iterator zoneIt = m_zoneVolumes.begin();

	// loop over all zones
	for (; zoneIt != m_zoneVolumes.end();	++zoneIt) {

		// retrieve zone id
		int zoneId = zoneIt->first;

		// zone air temperature [K] from FMI inputs
		double zoneAirTemperature = m_realVar[FMI_INPUT_RoomAirTemperature + zoneId];
		// maximum CO2 concentration [mol/mol] from FMI inputs
		double maximumCO2Concentration = m_realVar[FMI_INPUT_MaximumCO2Concentration + zoneId];
		// minimum air change rate [1/s] from FMI inputs
		double minimumAirChangeRate = m_realVar[FMI_INPUT_MinimumAirChangeRate + zoneId];
		// maximum air change rate [1/s] from FMI inputs
		double maximumAirChangeRate = m_realVar[FMI_INPUT_MaximumAirChangeRateIncrease + zoneId] + minimumAirChangeRate;
		// minimum air temperature [K] from FMI inputs
		double minimumAirTemperature = m_realVar[FMI_INPUT_MinimumAirTemperature + zoneId];
		// maximum air temperature [K] from FMI inputs
		double maximumAirTemperature = m_realVar[FMI_INPUT_MaximumAirTemperature + zoneId];

		// CO2 mass of last time step [kg/s] from solution vector
		double zoneCO2MassLast = m_zoneCO2MassesLastTimePoint[zoneId];
		// zone volume [m3]from parameter vector
		double zoneVolume = m_zoneVolumes[zoneId];
		// zone area [m2] from parameter vector
		double zoneArea = m_zoneFloorAreas[zoneId];

		// maximum CO2 mass of ambient air [kg]
		double ambientCO2Mass = ambientCO2Density * zoneVolume;
		// CO2 density of last time step [kg/s] from solution vector
		double CO2DensityLast = zoneCO2MassLast / zoneVolume;
		// calcualtes quantties: CO2 setpoint density [kg/m3]
		double zoneCO2ConcentrationLast = CO2DensityLast/ molarMassCO2 * RIdealGas * zoneAirTemperature /referencePressure;
		// calculate air exchange rate
		double &zoneAirChangeRate = m_zoneAirChangeRates[zoneId];

		calculateAirChangeRate(zoneAirChangeRate, zoneAirTemperature, ambientTemperature, zoneCO2ConcentrationLast,
							   maximumCO2Concentration, minimumAirTemperature, maximumAirTemperature,
							   minimumAirChangeRate, maximumAirChangeRate);

		// solve ODE
		double expN = std::exp(-zoneAirChangeRate * deltaT);

		// time integration steps
		double dt = deltaT / (double)NUM_INT_INTVALS;
		double tNext = m_lastTimePoint;
		double expNNext = expN;
		// start integral values

		std::string scheduleName = m_zoneScheduleNames[zoneId];

		// integral values
		double CO2SourceIntegral = 0.0;

		// integrate CO2 sources (only for existing schedules
		if(!scheduleName.empty()) {

			// retrieve linear spline
			std::map<std::string, LinearSpline>::iterator CO2SourcePerArea =
				m_co2LoadPerZoneFloorAreasSplines.find(scheduleName);
			assert(CO2SourcePerArea != m_co2LoadPerZoneFloorAreasSplines.end());

			double CO2Source = CO2SourcePerArea->second.value(tNext) * zoneArea;
			double CO2SourceExp = CO2Source * expNNext;
			double CO2SourceNext = 0.0;
			double CO2SourceExpNext = 0.0;

			// calculate multiplicator
			double expFacDt = std::exp(zoneAirChangeRate * dt);

			// numerical integration of source terms
			for (unsigned int i = 0; i < NUM_INT_INTVALS; ++i) {

				// get next integration point
				tNext += dt;
				// calculate source vale at next integration time point
				CO2SourceNext = CO2SourcePerArea->second.value(tNext) * zoneArea;

				// calculate next values
				// expNNext = exp(-airExhangeRate * (tEnd - tNext)) = exp(-airExhangeRate * (tEnd - tPrev - dt))
				//          = exp(-airExhangeRate * (tEnd - tPrev) * exp(airExhangeRate * dt)
				expNNext *= expFacDt;
				CO2SourceExpNext = CO2SourceNext * expNNext;

				// cummulate integral values
				CO2SourceIntegral += 0.5 * (CO2SourceExp + CO2SourceExpNext) * dt;

				// store old value
				CO2SourceExp = CO2SourceExpNext;
			}
		}

		// calculate new value
		double zoneCO2Mass = zoneCO2MassLast * expN + ambientCO2Mass * (1. - expN) + CO2SourceIntegral;
		// store new value
		m_zoneCO2Masses[zoneId] = zoneCO2Mass;
		double zoneCO2Density = zoneCO2Mass / zoneVolume;
		double zoneCO2Concentration = zoneCO2Density / molarMassCO2 * RIdealGas * zoneAirTemperature / referencePressure;

		// output variables
		m_realVar[FMI_OUTPUT_AirChangeRate + zoneId] = zoneAirChangeRate;
		m_realVar[FMI_OUTPUT_CO2Concentration + zoneId] = zoneCO2Concentration;
	}


	// update time points
	m_currentTimePoint = tCommunicationIntervalEnd;

	// state of FMU after integration:
	//   m_currentTimePoint = tCommunicationIntervalEnd;
}


void CO2ComfortVentilation::computeFMUStateSize() {
	// store time, states and outputs
	m_fmuStateSize = sizeof(double)*1;
	// we store all cached variables
	m_fmuStateSize += (sizeof(int) + sizeof(double))*m_zoneCO2Masses.size();

	// for all 4 maps, we store the size for sanity checks
	m_fmuStateSize += sizeof(int)*4;

	// serialization of the maps: first the valueRef, then the actual value

	m_fmuStateSize += (sizeof(int) + sizeof(double))*m_realVar.size();
	m_fmuStateSize += (sizeof(int) + sizeof(int))*m_integerVar.size();
	m_fmuStateSize += (sizeof(int) + sizeof(int))*m_boolVar.size(); // booleans are stored as int

	// strings are serialized in checkable format: first length, then zero-terminated string
	for (std::map<int, std::string>::const_iterator it = m_stringVar.begin();
		 it != m_stringVar.end(); ++it)
	{
		m_fmuStateSize += sizeof(int) + sizeof(int) + it->second.size() + 1; // add one char for \0
	}
}


// macro for storing a POD and increasing the pointer to the linear memory array
#define SERIALIZE(type, storageDataPtr, value)\
{\
  *reinterpret_cast<type *>(storageDataPtr) = (value);\
  (storageDataPtr) = reinterpret_cast<char *>(storageDataPtr) + sizeof(type);\
}

// macro for retrieving a POD and increasing the pointer to the linear memory array
#define DESERIALIZE(type, storageDataPtr, value)\
{\
  (value) = *reinterpret_cast<type *>(storageDataPtr);\
  (storageDataPtr) = reinterpret_cast<const char *>(storageDataPtr) + sizeof(type);\
}


template <typename T>
bool deserializeMap(CO2ComfortVentilation * obj, const char * & dataPtr, const char * typeID, std::map<int, T> & varMap) {
	// now de-serialize the maps: first the size (for checking), then each key-value pair
	int mapsize;
	DESERIALIZE(const int, dataPtr, mapsize)
	if (mapsize != static_cast<int>(varMap.size())) {
		std::stringstream strm;
		strm << "Bad binary data or invalid/uninitialized model data. "<< typeID << "-Map size mismatch.";
		obj->logger(fmi2Error, "deserialization", strm.str());
		return false;
	}
	for (int i=0; i<mapsize; ++i) {
		int valueRef;
		T val;
		DESERIALIZE(const int, dataPtr, valueRef)
		if (varMap.find(valueRef) == varMap.end()) {
			std::stringstream strm;
			strm << "Bad binary data or invalid/uninitialized model data. "<< typeID << "-Variable with value ref "<< valueRef
				 << " does not exist in "<< typeID << "-variable map.";
			obj->logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		DESERIALIZE(const T, dataPtr, val)
		varMap[valueRef] = val;
	}
	return true;
}



void CO2ComfortVentilation::serializeFMUstate(void * FMUstate) {
	char * dataPtr = reinterpret_cast<char*>(FMUstate);
	if (m_modelExchange) {
		SERIALIZE(double, dataPtr, m_tInput)

		// TODO ModelExchange-specific serialization
	}
	else {
		SERIALIZE(double, dataPtr, m_currentTimePoint)

		// store actual solution data
		for (std::map<int, double>::const_iterator it = m_zoneCO2Masses.begin(); it != m_zoneCO2Masses.end(); ++it) {
			SERIALIZE(int, dataPtr, it->first)
			SERIALIZE(double, dataPtr, it->second)
		}
	}

	// write map size for checking
	int mapSize = static_cast<int>(m_realVar.size());
	SERIALIZE(int, dataPtr, mapSize)
	// now serialize all members of the map
	for (std::map<int,double>::const_iterator it = m_realVar.begin(); it != m_realVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first)
		SERIALIZE(double, dataPtr, it->second)
	}
	mapSize = static_cast<int>(m_integerVar.size());
	SERIALIZE(int, dataPtr, mapSize)
	for (std::map<int,int>::const_iterator it = m_integerVar.begin(); it != m_integerVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first)
		SERIALIZE(int, dataPtr, it->second)
	}
	mapSize = static_cast<int>(m_boolVar.size());
	SERIALIZE(int, dataPtr, mapSize)
	for (std::map<int,int>::const_iterator it = m_boolVar.begin(); it != m_boolVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first)
		SERIALIZE(int, dataPtr, it->second)
	}
	mapSize = static_cast<int>(m_stringVar.size());
	SERIALIZE(int, dataPtr, mapSize)
	for (std::map<int, std::string>::const_iterator it = m_stringVar.begin();
		 it != m_stringVar.end(); ++it)
	{
		SERIALIZE(int, dataPtr, it->first)		// map key
		SERIALIZE(int, dataPtr, static_cast<int>(it->second.size()))		// string size
		std::memcpy(dataPtr, it->second.c_str(), it->second.size()+1); // also copy the trailing \0
		dataPtr += it->second.size()+1;
	}
}


bool CO2ComfortVentilation::deserializeFMUstate(void * FMUstate) {
	const char * dataPtr = reinterpret_cast<const char*>(FMUstate);
	DESERIALIZE(const double, dataPtr, m_currentTimePoint)

	// deserialize solution state
	if (!deserializeMap(this, dataPtr, "real", m_zoneCO2Masses))
		return false;

	if (!deserializeMap(this, dataPtr, "real", m_realVar))
		return false;
	if (!deserializeMap(this, dataPtr, "integer", m_integerVar))
		return false;
	if (!deserializeMap(this, dataPtr, "boolean", m_boolVar))
		return false;

	// special handling for deserialization of string map
	int mapsize;
	DESERIALIZE(const int, dataPtr, mapsize)
	if (mapsize != static_cast<int>(m_stringVar.size())) {
		logger(fmi2Error, "deserialization", "Bad binary data or invalid/uninitialized model data. string-variable map size mismatch.");
		return false;
	}
	for (int i=0; i<mapsize; ++i) {
		int valueRef;
		DESERIALIZE(const int, dataPtr, valueRef)
		if (m_stringVar.find(valueRef) == m_stringVar.end()) {
			std::stringstream strm;
			strm << "Bad binary data or invalid/uninitialized model data. string-variable with value ref "<< valueRef
				 << " does not exist in real variable map.";
			logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		// get length of string
		int strLen;
		DESERIALIZE(const int, dataPtr, strLen)
		// create a string of requested length
		std::string s(static_cast<size_t>(strLen), ' ');
		// copy contents of string
		std::memcpy(&s[0], dataPtr, static_cast<size_t>(strLen)); // do not copy the trailing \0
		dataPtr += strLen;
		// check that next character is a \0
		if (*dataPtr != '\0') {
			std::stringstream strm;
			strm << "Bad binary data. string-variable with value ref "<< valueRef
				 << " does not have a trailing \0.";
			logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		++dataPtr;
		// replace value in map
		m_stringVar[valueRef] = s;
	}

	return true;
}


void CO2ComfortVentilation::read(const Path &fpath) {

	// this is now the path without column indicator
	try {
#if defined(_WIN32)
#if defined(_MSC_VER)
		std::ifstream in(fpath.wstr().c_str());
#endif
#else // _WIN32
		std::ifstream in(fpath.c_str());
#endif
		if (!in)
			throw std::runtime_error(std::string("File '") + fpath.str() + std::string("' doesn't exist or cannot open/access file."));

		std::string line;
		// values and definitions are divided by assignment '='
		std::string sepChars;
		sepChars.push_back('=');

		// first read zone ids
		std::set<unsigned int> zoneIds;
		// we read file line by line
		while (std::getline(in, line)) {
			if (line.empty() || line.find_first_not_of("\n\r\t ") == std::string::npos)
				continue;
			std::vector<std::string> values;
			explode(line, values, sepChars);

			// we expect 2 tokens
			if (values.size() != 2)
				throw std::runtime_error((std::string("Malformed line '") + line + std::string(". Expected format '<attribute> = <value>'.")).c_str());

			// extract attribute
			std::string attribute = values[0];
			std::string value = values[1];
			trim(attribute, "\t\r ");
			// try to exclude index notation
			explode(attribute, values, "[");
			attribute = values[0];
			trim(attribute, " ");

			// get index
			std::string index;
			if (values.size() > 1) {
				index = values[1];
				trim(index, "] ");
			}
			// get value
			trim(value, "\t\r ");

			// choose attribute:
			if (attribute != "zoneIds")
				continue;
			// no indexes are allowed
			if (!index.empty())
				throw std::runtime_error("Invalid index notation in attribute ')" + attribute + "'.");

			// trim value attribute
			trim(value, "[]{}");
			// and read zone ids
			explode(value, values, ",");

			try {
				for (std::string idStr : values) {
					unsigned int id = string2val<unsigned int>(idStr);
					if (zoneIds.find(id) != zoneIds.end())
						throw std::runtime_error(std::string("Duplicate zone id ") + idStr + std::string("."));

					// temporarilly store zone ids
					zoneIds.insert(id);
				}
			}
			catch (std::runtime_error &ex) {
				throw std::runtime_error((std::string("Malformed value '") + value + std::string("in '")
					+ attribute + std::string("' attribute: ") + ex.what()).c_str());
			}
			// break loop
			break;
		}

		// no we have read all zone information
		// set counter to start of file
		in.clear();
		in.seekg(0);

		// we read file line by line
		while (std::getline(in, line)) {
			if (line.empty() || line.find_first_not_of("\n\r\t ") == std::string::npos)
				continue;
			std::vector<std::string> values;
			explode(line, values, sepChars);

			// we expect 2 tokens
			if (values.size() != 2)
				throw std::runtime_error((std::string("Malformed line '") + line + std::string(". Expected format '<attribute> = <value>'.")).c_str());

			// extract attribute
			std::string attribute = values[0];
			std::string value = values[1];
			trim(attribute, "\t\r ");

			// zone ids were read already
			if (attribute == "zoneIds")
				continue;

			// try to exclude index notation
			explode(attribute, values, "[");
			attribute = values[0];
			trim(attribute, " ");

			// get index
			std::string index;
			if (values.size() > 1) {
				index = values[1];
				trim(index, "] ");
			}
			// get value
			trim(value, "\t\r ");

			// schedule names fotr CO2 source
			if (attribute == "zoneScheduleNames") {
				// indexes are requested
				if (index.empty()) {
					throw std::runtime_error((std::string("Missing index notation in attribute ')") +
						attribute + std::string("'.")).c_str());
				}
				// zoneIds are requested
				if (zoneIds.empty())
					throw std::runtime_error("Missing zone id definition. Define attribute 'zoneIds' first!");

				unsigned int id = 0;
				// read id
				try {
					id = string2val<unsigned int>(index);
				}
				catch (std::runtime_error &ex) {
					throw std::runtime_error((std::string("Malformed value '") + value + std::string("in '")
						+ attribute + std::string("' attribute: ") + ex.what()).c_str());
				}
				// id must be contained inside zone ids container
				if (zoneIds.find(id) == zoneIds.end()) {
					throw std::runtime_error((std::string("Index in attribute '") +
						attribute + std::string("' must be listed inside zoneIds vector.")).c_str());
				}
				// trim value
				trim(value, "\"");
				// value is already set
				if (m_zoneScheduleNames.find(id) != m_zoneScheduleNames.end()) {
					throw std::runtime_error((std::string("Duplicate id ") + index + std::string(" in attribute '") +
						attribute + std::string(".")).c_str());
				}
				// store value
				m_zoneScheduleNames[id] = value;
			}
			// physical parameters
			else {
				// define target unit
				std::string targetUnit;
				if (attribute == "zoneFloorAreas")
					targetUnit = "m2";
				else if (attribute == "zoneVolumes")
					targetUnit = "m3";
				else if (attribute == "startCO2Concentration")
					targetUnit = "mol/mol";
				else if (attribute == "startAirTemperature")
					targetUnit = "K";
				else if (attribute == "CO2ToleranceBand")
					targetUnit = "mol/mol";
				else if (attribute == "temperatureToleranceBand")
					targetUnit = "K";
				else {
					throw std::runtime_error((std::string("Unknown attribute '") + attribute + std::string("'.")).c_str());
				}

				// filter value and unit
				explode(value, values, " ");
				if (values.size() != 2)
				{
					throw std::runtime_error((std::string("Malformed value '") + value + std::string("in '")
						+ attribute + std::string("' attribute. Expected '<value> <unit>'.")).c_str());
				}
				// check unit
				std::string unit = values[1];
				value = values[0];
				double val = 0.0;
				// read value
				try {
					val = string2val<double>(value);
				}
				catch (std::runtime_error &ex) {
					throw std::runtime_error((std::string("Malformed value '") + value + std::string("in '")
						+ attribute + std::string("' attribute: ") + ex.what()).c_str());
				}
				// convert to base unit
				std::string baseUnit;
				convertToBaseUnit(val, baseUnit, unit);
				// check unit
				if (baseUnit != targetUnit) {
					throw std::runtime_error((std::string("Wong unit value '") + unit + std::string("in '")
						+ attribute + std::string("' attribute. Expected 'm2'.")).c_str());
				}

				// now perform special treatment:
				// zone specific attributes
				if (attribute == "zoneFloorAreas" || attribute == "zoneVolumes") {
					// indexes are requested
					if (index.empty()) {
						throw std::runtime_error((std::string("Missing index notation in attribute ')") +
							attribute + std::string("'.")).c_str());
					}

					unsigned int id = 0;
					// read id
					try {
						id = string2val<unsigned int>(index);
					}
					catch (std::runtime_error &ex) {
						throw std::runtime_error((std::string("Malformed value '") + value + std::string("in '")
							+ attribute + std::string("' attribute: ") + ex.what()).c_str());
					}
					// id must be contained inside zone ids container
					if (zoneIds.find(id) == zoneIds.end()) {
						throw std::runtime_error((std::string("Index in attribute '") +
							attribute + std::string("' must be listed inside zoneIds vector.")).c_str());
					}

					// duplicate definition
					if (attribute == "zoneFloorAreas" &&m_zoneFloorAreas.find(id) != m_zoneFloorAreas.end()
						|| attribute == "zoneVolumes" &&m_zoneVolumes.find(id) != m_zoneVolumes.end()) {
						throw std::runtime_error((std::string("Duplicate id ") + index + std::string(" in attribute '") +
							attribute + std::string(".")).c_str());
					}
					// all right, store value
					if (attribute == "zoneFloorAreas")
						m_zoneFloorAreas[id] = val;
					else
						m_zoneVolumes[id] = val;
				}
				//scalar attributes
				else {
					// no indexes are allowed
					if (!index.empty()) {
						throw std::runtime_error((std::string("Invalid index notation in attribute ')") +
							attribute + std::string("'.")).c_str());
					}

					if (attribute == "startCO2Concentration")
						m_startCO2Concentration = val;
					else if (attribute == "startAirTemperature")
						m_startAirTemperature = val;
					else if (attribute == "CO2ToleranceBand")
						m_CO2ToleranceBand = val;
					else if (attribute == "temperatureToleranceBand")
						m_temperatureToleranceBand = val;
				}
			}
		}
		// fill all missing CO2 load schedules with empty name
		if (m_zoneScheduleNames.size() != zoneIds.size()) {
			// find missing quantities
			std::set<unsigned int> missingIds = zoneIds;
			for (const std::pair<unsigned int, std::string> &scheduleName : m_zoneScheduleNames) {
				missingIds.erase(scheduleName.first);
			}
			// add empty schedule name to list
			for (std::set<unsigned int>::const_iterator it = missingIds.begin(); it != missingIds.end(); ++it) {
				m_zoneScheduleNames[(int) *it] = std::string();
			}
		}
		// check that we filled all zoen specific quantities
		if (m_zoneFloorAreas.size() != zoneIds.size()) {
			// find missing quantities
			std::set<unsigned int> missingIds = zoneIds;
			for (const std::pair<unsigned int, double> &floorArea : m_zoneFloorAreas) {
				missingIds.erase(floorArea.first);
			}
			// write missing ids into string (minimum one is missing
			std::set<unsigned int>::const_iterator it = missingIds.begin();
			assert(it != missingIds.end());
			// add to string
			std::string missingIdsStr = val2string<unsigned int>(*it);
			++it;
			for (; it != missingIds.end(); ++it) {
				missingIdsStr += std::string(",") + val2string<unsigned int>(*it);
			}
			// create a meaningful error message
			throw std::runtime_error((std::string("Missing ids ") + missingIdsStr +
				std::string("in attribute 'zoneFloorAreas'")).c_str());
		}
		if (m_zoneVolumes.size() != zoneIds.size()) {
			// find missing quantities
			std::set<unsigned int> missingIds = zoneIds;
			for (const std::pair<unsigned int, double> &volume : m_zoneVolumes) {
				missingIds.erase(volume.first);
			}
			// write missing ids into string (minimum one is missing
			std::set<unsigned int>::const_iterator it = missingIds.begin();
			assert(it != missingIds.end());
			// add to string
			std::string missingIdsStr = val2string<unsigned int>(*it);
			++it;
			for (; it != missingIds.end(); ++it) {
				missingIdsStr += std::string(",") + val2string<unsigned int>(*it);
			}
			// create a meaningful error message
			throw std::runtime_error((std::string("Missing ids ") + missingIdsStr +
				std::string("in attribute 'zoneVolumes'")).c_str());
		}
		// check obligatory parameters
		if (m_startCO2Concentration == -999)
			throw std::runtime_error("Missing attribute 'startCO2Concentration'.");
		else if (m_startAirTemperature == -999)
			throw std::runtime_error("Missing attribute 'startAirTemperature'.");
	}
	catch (std::runtime_error & ex) {
		throw std::runtime_error((std::string("Error reading file ") + fpath.str() + std::string(": ") + ex.what()).c_str());
	}
}


void CO2ComfortVentilation::calculateAirChangeRate(double &airChangeRate, double airTemperature, double ambientTemperature,
												   double CO2Concentration,
												   double maxCO2Concentration, double minAirTemperature, double maxAirTemperature,
												   double minAirChangeRate, double maxAirChangeRate) {

	// return maximum rate either if CO2 concentraton is exceeded...
	if (CO2Concentration > maxCO2Concentration + 0.5 * m_CO2ToleranceBand) {
		airChangeRate = maxAirChangeRate;
		return;
	}
	// ...or air temperature is above limit
	if (airTemperature > maxAirTemperature + 0.5 * m_temperatureToleranceBand && airTemperature > ambientTemperature) {
		airChangeRate = maxAirChangeRate;
		return;
	}
	// ...or air temperature is below limit
	if (airTemperature < minAirTemperature - 0.5 * m_temperatureToleranceBand &&
		airTemperature < ambientTemperature) {
		airChangeRate = maxAirChangeRate;
		return;
	}
	// return minimum rate either if CO2 is below tolerance band...
	if (CO2Concentration <= maxCO2Concentration - 0.5 * m_CO2ToleranceBand) {
		// ... and temperature is below tolerance band (cooling case)
		if (airTemperature <= maxAirTemperature - 0.5 * m_temperatureToleranceBand && airTemperature > ambientTemperature) {
			airChangeRate = minAirChangeRate;
			return;
		}
		// ... and temperature is above tolerance band (heating case
		if (airTemperature >= minAirTemperature + 0.5 * m_temperatureToleranceBand && airTemperature < ambientTemperature) {
			airChangeRate = minAirChangeRate;
			return;
		}
	}
	// no changes otherwise
}





