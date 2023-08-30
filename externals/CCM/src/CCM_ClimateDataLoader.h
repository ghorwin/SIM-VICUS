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

#ifndef CCM_ClimateDataLoaderH
#define CCM_ClimateDataLoaderH

#include <string>
#include <vector>
#include <array>

#include <IBK_LinearSpline.h>
#include <IBK_Path.h>

/*! Namespace of the Climate Calculation Module.

	\example CCM_Test.cpp
*/
namespace CCM {

class SolarRadiationModel;

/*! Reads climatic data and provides climate data for a given time point.
	Implements state-machine interface, i.e. class maintains a time state that
	can be changed via setTime() function and provides computed/interpolated climatic
	data at this time point.

	Data is stored in m_data for annual climate data:
	- annual data (365 days)
	- hourly data, 8760 values
	- for all climatic components

	If m_dataTimePoints is given (not empty), we have non-cyclic data with non-equidistant
	time steps (interpolation needed). Size of m_dataTimePoints must match size of all
	vectors in m_data. If m_overrideData is given, each spline data series
	replaces existing data series in m_data.
*/
class ClimateDataLoader {
public:
	/*! List of available/supported climate components. */
	enum ClimateComponents {
		/*! Ambient temperature [C] */
		Temperature,
		/*! Ambient relative humidity [0 to 100] */
		RelativeHumidity,
		/*! Direct short wave radiation in sun's normal direction [W/m2] */
		DirectRadiationNormal,
		/*! Diffuse short wave radiation on horizontal surface [W/m2] */
		DiffuseRadiationHorizontal,
		/*! Wind direction [Deg], 0 - North, 90 - East, < 360. */
		WindDirection,
		/*! Wind velocity [m/s] */
		WindVelocity,
		/*! Long wave counter radiation [W/m2] */
		LongWaveCounterRadiation,
		/*! Atmospheric pressure [Pa] */
		AirPressure,
		/*! Rain flux density on horizontal plane [l/m2h] */
		Rain,
		/*! Number of known/supported climate components. */
		NumClimateComponents
	};

	enum CheckBits {
		/*! All data for the components is given (no -9999 values) and appear to be within correct ranges.
			Range definitions are:
			- Temperature: -80C ... 100C
			- RH: 0 .. 100%
			- ...
		*/
		ALL_VALID = 0x00,
		/*! All data for this component is missing (i.e. all values are -9999). */
		ALL_DATA_MISSING = 0x01,
		/*! Some data for this component is missing (i.e. some values are -9999). */
		SOME_DATA_MISSING = 0x02,
		/*! Data is given (no -9999 values), but some or all data is outside allowed ranges. */
		SOME_DATA_INVALID = 0x04
	};

	/*! Default constructor. */
	ClimateDataLoader();

	/*! Initializes all data vectors with non valid data.
		The number of data dependes on length of m_dataTimePoints.
		If no user defined time values are given data vectors will contain 8760 values (hourly data of a year).
		Otherwise data vectors contain the same amount of data as the m_dataTimePoints vector.
	*/
	void initDataWithDefault();

	/*! Reads either EPW, WAC or c6b file, depending on file name extension.
		\param fname Full path to climate data file.
		\param headerOnly If true, reader stops after reading header (object is in incomplete state afterwards)
		\warning Call this function on an empty object, only. Otherwise the resulting state is
				 undefined.
	*/
	void readClimateData(const IBK::Path & fname, bool headerOnly=false);

	/*! Reads EPW file.
		\param fname Full path to epw file.
		\param headerOnly If true, reader stops after reading header (object is in incomplete state afterwards)
		\warning Call this function on an empty object, only. Otherwise the resulting state is
				 undefined.
	*/
	void readClimateDataEPW(const IBK::Path & fname, bool headerOnly=false);

	/*! Reads WAC file.
		\param fname Full path to file.
		\param headerOnly If true, reader stops after reading header (object is in incomplete state afterwards)
		\warning Call this function on an empty object, only. Otherwise the resulting state is
				 undefined.
	*/
	void readClimateDataWAC(const IBK::Path & fname, bool headerOnly=false);

	/*! Read climate data set from binary IBK (c6b) format.
		\param fname Full path to c6b file.
		\param headerOnly If true, reader stops after reading header (object is in incomplete state afterwards)
		\warning Call this function on an empty object, only. Otherwise the resulting state is
				 undefined.
	*/
	void readClimateDataIBK(const IBK::Path & fname, bool headerOnly=false);

	/*! Read climate data set from BBSR (reference years from 2017, with Lambert transformation)
		\param fname Full path to dat file.
		\param headerOnly If true, reader stops after reading header (object is in incomplete state afterwards)
		\warning Call this function on an empty object, only. Otherwise the resulting state is
				 undefined.
	*/
	void readClimateDataBBSRDat(const IBK::Path & fname, bool headerOnly=false);

	/*! Reads CCD files from directory using default file names.
		\param directory The path to the directory containing the files.
		\param radModel Reference to already instantiated solar radiation model (needed for direct radiation load conversion).
		\param skippedClimateDataFiles Holds a list of all climate data files that were not found.
		\note This function assumes direct sun radiation data to be stored with respect
		to a horizontal surface and needs a sun radiation model to compute sun's normal
		radiation.
	*/
	void readClimateDataCCDDirectory(const IBK::Path & directory,
									 SolarRadiationModel & radModel,
									 std::string & skippedClimateDataFiles);

	/*! Try to find the WMO data of a given weather station from a given MLID data base.
		If we are successful this function returns 'true'.
	*/
	bool fillWMOCodeFromMLID(const IBK::Path & MLIDDatabaseFile);

	/*! Write climate data set (hourly, annual) to binary IBK format.
		\note Function expects complete data set.
	*/
	void writeClimateDataIBK(const IBK::Path & fname);

	/*! Write climate data (hourly, annual) set to tab separated format.
		\note Function expects complete data set.
	*/
	void writeClimateDataTSV(const IBK::Path & fname);

	/*! Write climate data (hourly, annual) set to file with epw format.
		\note Function expects complete data set.
	*/
	void writeClimateDataEPW(const IBK::Path & fname);

	/*! Updates state of loaded to a given time point t in [s] (simulation time offset
		to January 1st midnight of the start year m_startYear).
		\param year Year as integer number (only needed for non-cyclic climate data).
		\param secondsOfYear Local Standard Time in [s] since start of year of selected time zone of
				climate station (note, may be different than building location, ie. time zone in
				solar radiation model), winter time, no leap years.

		When invalid data is given, the function does not correct it but simply returns invalid data.
		When data is missing (-9999 values), the following corrections are automatically applied:

		- Temperature: 293.15 K is set
		- RH: 80 % is set
		- Solar loads: 0 W/m2 is set
		- angles: 0 deg is set
		- windspeed: 0 m/s
		- rain: 0 kg/m2s
		- airpressure: 101325 Pa

		\warning Function does minimal error checking (only debug assertions), ensure data
				 consistency before calling this function.
	*/
	void setTime(int year, double secondsOfYear);

	/*! Read CCD description file 'description.xml'.
		This function is kept for compatibility with older formats.
		\para fname data file name
	*/
	void readDescriptionCCD(const IBK::Path & fname);

	/*! Check if the data set for the given component is valid.
		Non valid data will be marked with DATA_NOT_VALID. This can happen if this climate component was not available.
		\warning Mind to call updateCheckBits() whenever data has changed! This is done automatically in the various read functions.
	*/
	bool hasValidData(ClimateComponents component) const { return m_checkBits[component] == ALL_VALID; }

	/*! If time point vector is used, this function checks that all values are consistent
		(strictly monotonic increasing).
		If m_dataTimePoints is empty (i.e. not used), the function always returns true.
	*/
	bool hasValidTimePoints(std::string* errmsg = nullptr) const;

	/*! Processes currently stored data and updates the check bits. */
	void updateCheckBits();

	// Meta data

	/*! Longitude of climate station location [deg].
		\note To avoid rounding errors in input/output, conversion to [rad] shall be done in
			  only the solar radiation model.
	*/
	double		m_longitudeInDegree;
	/*! Latitude of climate station location [deg].
		\note To avoid rounding errors in input/output, conversion to [rad] shall be done in
			  only the solar radiation model.
	*/
	double		m_latitudeInDegree;

	/*! Elevation in [m] above NN. */
	double		m_elevation;
	/*! Observers time zone: -12 to 12
		Time zone is needed for true local time correction. Each time zone has a corresponding
		longitude. If longitude of observer (m_longitude) differs from this time-zone longitude,
		a time correction is applied.
	*/
	int			m_timeZone;

	/*! City of climate station (UTF8 encoded). */
	std::string m_city;
	/*! Country of climate station (UTF8 encoded). */
	std::string m_country;
	/*! WMO code. */
	std::string m_wmoCode;
	/*! Source identifier (UTF8 encoded) */
	std::string m_source;
	/*! Comment  (UTF8 encoded, can have line breaks). */
	std::string m_comment;


	// Actual data

	/*! Data vector.
		For cyclic annual data each climate component has exactly 8760 values. For non-cyclic
		data, size of vectors matches size of m_dataTimePoints.
		Values are interpreted as given at the end of each hour, so that the first value in the vector
		corresponds to the end of the first hour, i.e. exactly to 1:00 on January 1st.
		Within each hour, values are expected to be linearly interpolated.
	*/
	std::vector<double>		m_data[NumClimateComponents];

	/*! Optional, time points for values in m_data. If empty, m_data holds 8760 values
		for each hour of the year. Time points are given in [s] (simulation time offset
		to January 1st midnight).

		The actual date of the first time stamp is computed, for example via:
		\code
		IBK::Time date = IBK::Time(m_startYear, m_dataTimePoints[0]);
		\endcode

		\note When using non-cyclic data the value m_startYear is used and must match the correct start year.
	*/
	std::vector<double>		m_dataTimePoints;

	/*! For non-cyclic data (m_dataTimePoints not empty), this is the year that climate data starts in. */
	int						m_startYear;

	/*! Optional override data.
		Each spline that is not empty overrides the corresponding data set in m_data.
	*/
	IBK::LinearSpline		m_overrideData[NumClimateComponents];


	// Calculated/interpolated values

	/*! Cached interpolated values at current time point, updated in each call to setTime().
		Values are returned in units as defined in component type defintions.
		\sa ClimateComponents, currentData()
		\note The units are not converted to SI units to avoid rounding error issues (example:
				orientation angles in deg are converted to rad and back (for display purposes) to deg
				and will now show ugly rounding errors).
		\warning Values are updated in setTime(), when input data changes make sure to call
				 setTime() before accessing values.
	*/
	double m_currentData[NumClimateComponents];


	/*! Return values of read functions. */
	enum ReadFunctionReturnValues {
		RF_OK,
		/*! File not found or not accessible. */
		RF_InvalidFilePath,
		/*! Missing or invalid header. */
		RF_BadHeader,
		/*! Invalid time stamp, not strictly monotonic increasing time point or duplicate time point. */
		RF_InvalidTimeStamp,
		/*! Line is malformed, either time stamp format is wrong or value is missing or other. */
		RF_BadLineFormat
	};

	/*! Attempts to read CCD a file into given vector.
		This function does only syntactical checks and consistency checks regarding monotony
		of time stamps.
		After values have been read successfully, you have to check for consistency yourself:
		- maybe no data in file at all
		- maybe duplicate annual values (data not suitable for cyclic use)
		- maybe invalid unit
		- maybe unknown/invalid quantity

		\para fname data file name
		\para timeInSeconds all data time points
		\para dataVec all values (same size as timeInSeconds)
		\para comment String with comment from file header.
		\para quantity String identifying the quantity in header.
		\para dataUnit Data unit.
		\return Returns a return value of type ReadFunctionReturnValues,
			RF_OK is returned if data was read successfully, in all other cases errorLine contains
			the line within the file where the error occurred.
	*/
	static ReadFunctionReturnValues readCCDFile(const IBK::Path & fname,
												std::vector<double> & timeInSeconds,
												std::vector<double> & dataVec,
												std::string &comment,
												std::string &quantity,
												IBK::Unit &dataUnit,
												std::string &errorLine);

	/*! Attempts to read a CSV files into given vector.
		First line in CSV file is expected in format:
		\code
			"Time [<time unit>]\t<quantity> [<value unit>]"

			// for example
			"Time [h]\tTemperature [C]"
		\endcode

		If the filename has a suffix of form "?<column index>", the csv-reader will
		extract date from the given column, where index 1 is the first value column (index 0 would be
		the time column).

		Example: "measurements.tsv?3"

		This function does only syntactical checks and consistency checks regarding monotony
		of time stamps.
		After values have been read successfully, you have to check for consistency yourself:
		- maybe no data in file at all
		- maybe duplicate annual values (data not suitable for cyclic use)
		- maybe invalid unit
		- maybe unknown/invalid quantity

		\para fname data file name
		\para timeInSeconds all data time points
		\para dataVec all values (same size as timeInSeconds)
		\para comment String with comment from file header.
		\para quantity String identifying the quantity in header.
		\para dataUnit Data unit.
		\return Returns a return value of type ReadFunctionReturnValues,
			RF_OK is returned if data was read successfully, in all other cases errorLine contains
			the line within the file where the error occurred.
	*/
	static ReadFunctionReturnValues readCSVFile(const IBK::Path & fname,
												std::vector<double> & timeInSeconds,
												std::vector<double> & dataVec,
												std::string &quantity,
												IBK::Unit &dataUnit,
												std::string &errorLine);


	/*! Utility function that can be used to convert return value from readXXXFile() functions to
		meaningful english text suitable as error message for command line solvers.
	*/
	static std::string composeErrorText(ReadFunctionReturnValues res,
										const IBK::Path & fullPath,
										const std::string & errorLine);

	/*! Performs the "annual data check".
		Time points must not exceed 365d and time points 0d and 365d must
		not be present at the same time.
		Throws an exception with error message if any of the tests fails.
		\param timeVec Vector with time points in seconds.
	*/
	static void checkForValidCyclicData(const std::vector<double> & timeVec);

	/*! Performs the "annual data check". Overload of above function.
		Time points must not exceed 365d and time points 0d and 365d must
		not be present at the same time.
		Does not throw an exception, but returns true/false depending on check.
		\param timeVec Vector with time points in seconds.
		\param errmsg Contains error message in case of returning false
		\return Returns true if data checks out, false in case of failure. When failed, errmsg contains an
			english error text.
	*/
	static bool checkForValidCyclicData(const std::vector<double> & timeVec, std::string& errmsg);

	/*! Inserts hourly values when these are missing by using the previous value.
		This function checks for valid input data. Whenever an hourly value is
		missing, the preceeding and following value must be the same, otherwise
		this is considered an error and the function throws an exception.
		Also, if time points are in timeVec that do not correspond to
		an hour of the year, the function throws an exception.
	*/
	static void expandToAnnualHourlyData(std::vector<double> & timeVec,
										 std::vector<double> & dataVec);

	/*! Convenience interpolation function for annual cyclic data with arbitrary spacing between
		intervals.
		This function extends the interpolation functionality of spl for the case that timeOfYearInSeconds
		is less than the first time point in spl or larger than the last time point in spl. Then,
		the interpolation is done using linear interpolation between first and last (cyclic) value.
		\param spl Linear spline (valid) with time points and corresponding values.
		\param timeOfYearInSeconds The time of the year in [s], time point must be in the range (0,365*24*3600).
		\warning Using this function in an inner solver loop may not be good, because we need to look at the
			first and last time point in the spline (mostly very far away in memory). It might be better to
			use this function to precalculate the values at 0 and 365 d and insert them into the
			spline if not yet present.
	*/
	static double interpolateCyclicData(const IBK::LinearSpline & spl, double timeOfYearInSeconds);

	/*! Try to evaluate the climate component typ from a given quantity string.
		It uses comparisons with strings of different IBK climate libs.*/
	static ClimateDataLoader::ClimateComponents componentTypeFromQuantity(const std::string& quantity);

	/*! Default units for all climate data components. */
	static const char * const DEFAULT_UNITS[NumClimateComponents];

	/*! Cached check bits with error bits for individual components. */
	std::array<unsigned int, NumClimateComponents>	m_checkBits;
	/*! Array with row indices for first missing and first out-of-range value, for individual componts.
		first = row_of_missing, second = row_of_invalid
		\code
		if (m_checkBits[Temperature] == SOME_DATA_INVALID) {
			std::cout << "First invalid temperature in row " << m_checkBitsBadValueRowIndexes[Temperature].second;
		}
		\endcode
		Array is updated in m_checkBits.
		When all data is missing, m_checkBitsBadValueRowIndexes[XXX].first is always 0.
	*/
	std::array<std::pair<unsigned int,unsigned int>, NumClimateComponents>	m_checkBitsBadValueRowIndexes;
};


} // namespace CCM

/*! \file CCM_ClimateDataLoader.h
	\brief Contains declaration of class CCM::ClimateDataLoader.
*/

#endif // CCM_ClimateDataLoaderH
