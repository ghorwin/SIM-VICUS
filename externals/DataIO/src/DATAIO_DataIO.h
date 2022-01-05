/*	DataIO library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, St. Vogelsang
	All rights reserved.

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

#ifndef DATAIO_DataIOH
#define DATAIO_DataIOH

#include <string>
#include <iomanip>
#include <iosfwd>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

#include <IBK_Unit.h>
#include <IBK_NotificationHandler.h>
#include <IBK_Path.h>


/*! The namespace DATAIO holds all classes and functions related to reading/writing
	and converting DELPHIN simulation output data.

	The main page (\ref index) provides some introductory information and pointers on how
	to read the API documentation.
*/
namespace DATAIO {

class GeoFile;

/*! \brief This class stores all output data from a Delphin output file.

	DataIO containers hold results from transient simulations. Hence, data is stored for different time points.
	For each time index, which corresponds to a certain time point, a number of values is stored
	(i.e. a time slice of the data). For field
	and flux outputs (see DATAIO::DataIO::T_FIELD an DATAIO::DataIO::T_FLUX) each value is associated to
	a spatial location. The mapping is done through the data in the corresponding geometry file, see DATAIO::GeoFile.

	Reading and writing of complete DataIO containers is done through member functions read() and write(). These functions
	read and write data in binary mode and are very fast. When the original file is present in ASCII encoding, the data is
	read as set of lines containing the actual data. When writing large data sets incrementally
	(when the simulation progresses), the output file stream is first opened
	using writeHeader() and afterwards data is appended with appendData().

	The main documentation page of the DataIO library contains examples on the principal usage of the DataIO class:
	see \ref intro_sec
*/
class DataIO {
public:

	/*! The principle type of the output file (field/state variables per element or flux across sides).
		For T_FIELD and T_FLUX type outputs, the variables are expected to have some form of spatial
		distribution. T_REFERENCE data is typically a set of time-value-series.
	*/
	enum type_t {
		/*! Element-based output, requires ELEMENTS tag. */
		T_FIELD,
		/*! Side-based output, requires SIDES tag. */
		T_FLUX,
		/*! Single value output (anything that is not related to elements or sides), requires REFERENCE tag. */
		T_REFERENCE,
		/*! Invalid value for type_t */
		NUM_T
	};

	/*! Spatial averaging/integration mode.
		This basically explains how the values were obtained and what they represent with respect to
		spatial averaging/integration.
	*/
	enum spaceType_t {
		/*! Values are written for each element. */
		ST_SINGLE,
		/*! Values are averaged using volume-weighted averaging. */
		ST_MEAN,
		/*! Values are integrated in space. */
		ST_INTEGRAL,
		/*! Invalid value for spaceType_t */
		NUM_ST
	};

	/*! Time averaging/integration mode.
		This basically explains how the values were obtained and what they represent with respect to time averages/time
		integration.
	*/
	enum timeType_t {
		/*! Values are written as computed at output time points. */
		TT_NONE,
		/*! Values are time-averaged using interval-length-weighted averaging. */
		TT_MEAN,
		/*! Values are integrated in time, using the trapozoidal rule (first-order integration). */
		TT_INTEGRAL,
		/*! Invalid value for timeType_t */
		NUM_TT
	};

	/*! Return value of determineDataFormat() and dataFormat(), identifies the type of data stored in the file. */
	enum dataFormat_t {
		/*! (time,value), can be created from any simulation grid. */
		DF_2D,
		/*! (time,x,value) or (time,y,value) or (time,z,value), can be created from any simulation grid. */
		DF_3D,
		/*! (time,x,y,value) or (time,y,z,value) or (time,x,z,value), created from 2D and 3D simulation grids.  */
		DF_4D,
		/*! (time,x,y,z,value), created from 3D simulation grids. */
		DF_5D,
		/*! Invalid value for dataFormat_t, also identifies that data format has not yet determined for a certain
			DataIO container object. */
		NUM_DF
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor.*/
	explicit DataIO();

	/*! Destructor, closes output file (of opened) and releases memory. */
	~DataIO();

	/*! Closes a potentially open output stream and resets all variables such that the class
		corresponds to a newly created object.
	*/
	void clear();

	/*! Function for reading output file header and data information.
		Only reads the data and is supposed to be very fast.
		Throws an IBK::Exception if an error occurs.
		\param fname		Path to the file to be read (utf8 encoded).
		\param headerOnly	If true, only the header will be read but no data.
		\param notify		An optional pointer to a notification object, to be called repeatedly while read() is executed.
		\sa TextNotificationHandler
	*/
	void read(const IBK::Path & fname, bool headerOnly = false,
		IBK::NotificationHandler * notify = nullptr);

	/*! Returns data for time index time_idx.

		If data had been requested before, simply the pointer to the corresponding internal data storage memory is returned.
		Otherwise:
		- for binary files, if data is requested the first time, the file is opened, and the requested chunk of data is read
		into internal storage memory and a pointer to this memory is returned.
		- for ASCII files, the previously read string with the numbers for this time index is parsed into internal storage memory,
		and a pointer to this memory is returned.

		Throws an IBK::Exception when data cannot be retrieved, for example when time_idx is invalid or file cannot be accessed or
		conversion of string to values fails.

		\param time_idx		The index of the time point, must be less than m_timepoints.size()
		\return				Returns a pointer to a memory array holding nValues() numbers. The pointer remains valid as
							long of the object isn't copied or the read(), clear() or setData() functions are called. The returned
							memory array is unaffected by subsequent calls to data().
		\note Even though the lazy evaluation cases internal cache values to change, all pointers retrieved so far remain
			  valid and the overal state of the object can be considered to be unchanged, thus this is a const function.
	*/
	const double * data(unsigned int time_idx) const;

	/*! Deletes all timepoints after the given one.
	   \param timePointInSeconds Last time point left in data set
	   Does nothing if the given timpoint is the last one or behind.
	   It clears the data set if the time point is before start.
	   It fills the internal data vector by calling dataVector() function before truncation of internal vectors.
	   \return true if the data were truncated otherwise false
	*/
	bool truncateData(double timePointInSeconds);

	/*! Deletes data and times for a given time index range.
		\param idxFrom		The index of the time point that the data should be deleted from. Must be less than m_timepoints.size().
		\param idxTo		The index of the time point that the data should be deleted to (including this index). Must be less than m_timepoints.size().
		Throws an IBK::Exception in case of error, for example invalid indexes.
		\note If you want to delete a single time index and its data, pass the same index as idxFrom and idxTo.
	*/
	void deleteData(unsigned int idxFrom, unsigned int idxTo);

	/*! Sets new data in the data container.
		This function will replace the internally stored timePoints and values vectors and
		also clear the m_valueStrings vector with cached string data (ASCII files only).
		The variable m_nValues (accessible with nValues()) will be reset based on the number of values in the values[0] vector.
		The function will throw an IBK::Exception if size of timePoints and values vectors mismatch, or if the
		vectors inside the values vector have different length.
		Section \ref writingDataFilesAtOnce discusses use of this function.

		\param timePoints A vector with time points in seconds.
		\param values Vector with vector of values for each time point. Values should be in value unit.
	*/
	void setData(const std::vector<double> & timePoints,
		const std::vector<std::vector<double> > & values);

	/*! Sets new data in the data container.

		A more efficient variant of the setData() function that swaps the contents of the vectors but otherwise
		works just like setData().
		Section \ref writingDataFilesAtOnce discusses use of this function.

		\warning Use this function only if you do not need the data in the original vector anylonger.
		\param timePoints A vector with time points in seconds, content will be swapped with old timepoints vector in object.
		\param values Vector with vector of values for each time point, content will be swapped with old values vector in object. Values should be in value unit.
	*/
	void swapData(std::vector<double> & timePoints,
		std::vector<std::vector<double> > & values);

	/*! Returns a pointer to a continuous memory block with m_timepoints.size() elements that contains
		the first value in the data vectors for each time point.
		This is essentially a convencience function for reading 2D output data, where each time point
		has only a single output value.

		This function uses lazy evaluation and only generates the vector when first called. Afterwards it
		only returns a pointer to the previously generated data vector.

		\return	Returns a pointer to a continuous memory array with m_timepoints.size() doubles. The pointer returned
				remains valid as long as the object isn't copied, or the read(), clear() or setData() functions are called.

		\note The generation of the vector requires reading of the whole data file, which can be slow on large files.
			However, the function is only meaningful for small data containers with one value per timepoint which will
			be fast in any case.
	*/
	const double * dataVector() const;

	/*! Changes/adds extension based on binary flag m_isBinary, i.e. sets d6o for ASCII files or d6b for
		binary files, respectively.
		This function expects a filename to be set already, otherwise it throws an Exception.
		If the existing filename has an extension other than d6o, d6b or out, the correct extension is appended.
	*/
	void adjustFileName();

	/*! Function for writing output file header and data.
		Output data is expected to be stored fully in the data structure. This function is actually a convenience
		wrapper function for writeHeader() and subsequent calls to appendData().
		The function requires a full set of member variables to be specified.
		Also, the vectors m_values and m_timepoints need to be filled in.
		Throws an IBK::Exception if writing fails.
		\param notify	An optional pointer to a notification object, to be called repeatedly while write() is executed.
		\sa writeHeader()
		\sa appendData()
		\sa TextNotificationHandler
	*/
	void write(IBK::NotificationHandler * notify = nullptr) const;

	/*! Function for creating an output file and writing the header, but no data.
		Throws an IBK::Exception if writing fails.

		This function is used for progressively writing output data using the function
		appendData().
		The function creates an output file stream using the current value of m_filename as output file name
		and keeps it open for subsequent calls. The file stream
		is closed and removed upon the next call to writeHeader(), or closeFile() or destruction of the DataIO object.

		\note The function also initializes the counter for number of values per time index m_nValues based on m_nums
		and m_spaceType (for integral or averaged values in space, only a single value is stored per time index,
		otherwise the number of values is determined by the indices in the m_nums vector.
	*/
	void writeHeader() const;

	/*! Adds new data to the output file.
		The file must have been previously opened by writeHeader() or reopenForWriting().
		The function only requires the m_nValues and m_timeUnit members to be set.
		Throws an IBK::Exception if data cannot be appended.
		\param t   Time point in [s] since midnight January first of the start year.
		\param values Pointer to linear list of values. Memory block must have size N (nValues())
					  as written in the header as number of ELEMENTS or SIDES.
	*/
	void appendData(double t, const double * values) const;

	/*! Number of values stored for each time index in the data file.
		This value is important for all memory blocks passed to and returned from member functions operating
		directly with memory arrays.
		\sa appendData
		\sa data
		\return Returns the number of values stored for each time index in the data file, i.e. length of memory arrays
				returned by data() and expected by appendData().
	*/
	unsigned int nValues() const { return m_nValues; }

	/*! Time stamp (encoded as if returned from ctime() ) of the file creation date/time.
		This value can be accessed after a successful call to read(), otherwise it is undefined.
		\see read
		\return Returns a formatted string containing the time stamp of the file creation date/time.
	*/
	std::string	 createdTime() const {
		if (m_createdTimeString.empty()) {
			std::time_t t = m_createdTime;
			return std::ctime(&t);
		}
		else
			return m_createdTimeString;
	}

	/*! Read-only pointer to the embedded geometry information (Delphin 5 outputs only).
		\return Returns a read-only pointer to the embedded geometry information (Delphin 5 outputs only). If
			DataIO container is of version 6 or greated, a NULL pointer will be returned.
	*/
	const GeoFile * geometryData() const { return m_geometryData; }

	/*! Attempts to re-open an output file to append data.
		Works essentially like writeHeader(), but instead of creating a new file and writing the header data,
		the function simply tries to open an existing output file, hereby automatically detecting
		the file type and whether it is written in binary or ASCII mode. The function DOES NOT check
		for consistency of the output file.
		After reading the header the file stream is positioned at the end of the file, so that appendData()
		can be called directly afterwards.

		\note Just like writeHeader(), the function also initializes the counter for number of values per
		time index m_nValues based on m_nums
		and m_spaceType (for integral or averaged values in space, only a single value is stored per time index,
		otherwise the number of values is determined by the indices in the m_nums vector.
		Throws an IBK::Exception if file does not exist or cannot be re-opened for writing.

		\param fname File path to DataIO container file.
	*/
	void reopenForWriting(const IBK::Path & fname);

	/*! Closes the file if opened for writing. */
	void close() const;

	/*! Major version number as 32bit integer.
		This value is set in the read() function.
		\return Returns the major version number of the file.
	*/
	unsigned int majorVersionNumber() const { return m_majorFileVersion; }

	/*! Minor version number as 32bit integer.
		This value is set in the read() function.
		\return Returns the minor version number of the file.
	*/
	unsigned int minorVersionNumber() const { return m_minorFileVersion; }

	/*! Determines the data format of the data stored in the data file based on the
		current geometry data.

		Thus function should be called directly after reading the DataIO container and
		its associated geometry file before processing the data.

		This function updates the min_i, max_i, ... max_k coordinates and then
		determines the data format of the data stored in a DataIO
		container and uses hereby the geometry data and the element/side selection
		in the DataIO structure. For example, if a 2D geometry is used, but the
		selection of elements only contains a single line of elements, the output
		format will be DATAIO::DataIO::DF_3D (time,x,value). If a matrix is selected, the
		output format will be DATAIO::DataIO::DF_4D (time,x,y,value).
		The function also determines the min and max indices for all coordinate
		directions and stores them in the m_min... and m_max... variables.

		\param geoFile Geometry data object, for DataUI containers with embedded geometry data
					   pass a reference to this data.

		\code
		// D5-type DataIO with embedded GeoFile data
		dataFormat_t df = dataIO.determineDataFormat(*dataIO.geometryData());

		// D6-type DataIO, geoFile is a geometry file object
		dataFormat_t df = dataIO.determineDataFormat(geoFile);
		\endcode

		\return Returns the newly computed data format.
		\sa dataFormat_t
		\warning This function throws an IBK::Exception if GeoFile data and/or DataIO data mismatches.
	*/
	dataFormat_t determineDataFormat( const GeoFile & geoFile );

	/*! Returns the data format for this DataIO container.
		Call determineDataFormat() first before accessing this function.
		You can use this function to check whether the min/max values were calculated already (always the
		case whenever dataFormat() != NUM_DF and m_dataFormat > DF_2D.
		\return Returns a dataformat of type dataFormat_t
		\sa dataFormat_t
	*/
	dataFormat_t dataFormat() const { return m_dataFormat; }

	/*! Returns a read-only vector with values.
		The vector has the same size as m_timepoints.

		\warning The elements of the vector (which are vectors of values themselves) may
			be empty. Use data() to access values still need to be parsed. Accessing all values with
			this function may be done savely, when data() had been called for all time indexes already.
	*/
	const std::vector< std::vector<double> > & values() const { return m_values; }

	/*! Returns a vector with values for a given column index, values are in m_valueUnit.
		Size of vector matches size of time points vector. Returns an empty vector if
		no data is stored in dataIO.
		Throws an IBK::Exception when index is out of range.
	*/
	std::vector<double> columnValues(size_t colIdx) const;

	// *** PUBLIC MEMBER VARIABLES ***


	/*! Filename of the output file, set in read() and reopenForWriting() functions.
		Filename is mutable so that it can be changed even if DataIO container is constant
		(use with caution).
		The filename is utf8 encoded.
	*/
	mutable IBK::Path					m_filename;

	/*! Flag indicating whether the file is in binary format.
		This value is set in the read() function. When writing data files, set this
		flag before a call to writeHeader(), or write().
	*/
	bool								m_isBinary;

	/*! Format flags for ASCII file output, used for time points and values.
		Default is empty. Typically you may want to set std::fixed or std::scientific.
		\code
		// enable scientific format
		m_asciiFmtFlags |= std::ios_base::scientific;

		// to unset format flags
		m_asciiFmtFlags &= ~std::ios_base::scientific;
		\endcode
	*/
	std::ios_base::fmtflags				m_asciiFmtFlags;

	/*! Precision used for ASCII file output, -1 means default. */
	int									m_asciiPrecision;

	/*! Vector with index numbers of elements (field outputs) or sides (flux outputs).
		\see nValues()
		\warning The size of this vector should be the same as nValues() and the size of the vectors
				 in m_values (unless spaceType_t is not ST_SINGLE).
	*/
	std::vector<unsigned int>			m_nums;

	// this is the data description
	type_t								m_type;  			///< Type of output file.
	std::string 						m_projectFileName;	///< Project file string (should help identify the project that created this file).
	std::string 						m_quantity;			///< Quantity description string (primary use as axis/series label in diagrams).
	std::string 						m_quantityKeyword;	///< The keyword of the quantity (Delphin 6 only, used for grouping/adding similar types of quantities, for example mass densities).
	spaceType_t							m_spaceType;		///< Identifies whether values were computed by spatial averaging/integration or written as single values.
	timeType_t							m_timeType;			///< Identifies whether values were computed by time averaging/integration or written as instantaneous values.
	std::string 						m_valueUnit;		///< Physical unit of values (no unit conversion is done for values).
	/*! Input/output unit for time points for ASCII format reading/writing.
		This unit is used for writing ASCII format output files. In ASCII DataIO files, the first value in each line is
		the time point in the given time unit. For example, if the first time point is at 3600 seconds, and the
		time unit is "h" (hour), the value "1" will be written in the file. The unit conversion is done in the appendData()
		or write() functions. It must be possible to convert the time unit into "s" with the IBK::UnitList.
		\warning The actual values inside the m_timepoints vector must be _always_ in seconds.
	*/
	std::string 						m_timeUnit;
	IBK::Path							m_geoFileName;		///< Relative path to geometry file (if one exists, for 2D or reference type files this can be empty; Delphin 6 only).
	unsigned int						m_geoFileHash;		///< Hash value of the content of the geometry file (Delphin 6 only).
	/*! Start year of the simulation.
		The output time points are defined from midnight January 1st of the start year.
		\warning Delphin 5 output files did not have a valid start year. In this case, the
		interpretation of the output time must be done manually when analyzing the data.
	*/
	int 								m_startYear;

	/*! Vector with time points in [s].
		The size of the vector also defines the range of time indices to be passed to data().
		The vector is filled in a call to read().
		\note The values in the vector m_timepoints are _always_ in seconds. If they are displayed
			  in a chart or similar, they should be converted to the suggested input/output unit
			  stored in m_timeUnit.
		\sa data
		\todo By allowing read/write access to this variables, users might create inconsistent DataIO containers,
		where length of timepoints vector mismatches with length of values vector. However, for most relevant
		use cases this is not an issue. However, it should be checked whether creating a simple const access function
		to the time points vector may not be sufficient.
	*/
	std::vector<double> 				m_timepoints;

	/*! Minimum row index of selected elements in DataIO, updated in last call to determineDataFormat(). */
	int									m_mini;
	/*! Minimum column index of selected elements in DataIO, updated in last call to determineDataFormat(). */
	int									m_minj;
	/*! Minimum stack index of selected elements in DataIO, updated in last call to determineDataFormat(). */
	int									m_mink;
	/*! Maximum row index, updated in last call to determineDataFormat(). */
	int									m_maxi;
	/*! Maximum column index, updated in last call to determineDataFormat(). */
	int									m_maxj;
	/*! Maximum stack index, updated in last call to determineDataFormat(). */
	int									m_maxk;


	static std::string dataFormat2String(dataFormat_t df);
	/*! Converts the given string into a type_t enum if possible.
		\param str The string (either FIELD, or FLUX or REFERENCE).
		\return Returns an enumeration value of type type_t.
	*/
	static type_t string2type(const std::string& str);
	/*! Converts a type_t enumeration value to its corresponding string representation.
		\param t The enumeration type for file data type.
		\return Returns a string suitable for identifying the file type inside the file header.
	*/
	static std::string type2string(type_t t);

	/*! Converts the given string into a spaceType_t object if possible.
		\param str The string that describes the space type.
		\return Returns an enumeration value of type spaceType_t.
	*/
	static spaceType_t string2spaceType(const std::string& str);
	/*! Converts a spaceType_t enumeration value to its corresponding string representation.
		\param t The enumeration type for the space type.
		\return Returns a string suitable for identifying the space type inside the file header.
	*/
	static std::string spaceType2string(spaceType_t t);

	/*! Converts the given string into a timeType_t object if possible.
		\param str The string that describes the time type.
		\return Returns an enumeration value of type timeType_t.
	*/
	static timeType_t string2timeType(const std::string& str);
	/*! Converts a timeType_t enumeration value to its corresponding string representation.
		\param t The enumeration type for the time type.
		\return Returns a string suitable for identifying the time type inside the file header.
	*/
	static std::string timeType2string(timeType_t t);


private:

	/*! Copy constructor is disabled (each DataIO object holds a file stream resource which may not be duplicated). */
	DataIO(const DataIO &);
	/*! Assignment operator is disabled (each DataIO object holds a file stream resource which may not be duplicated). */
	const DataIO& operator=(const DataIO &);

	/*! Attempts to open DataIO file with given file path and reads just the header.
		Content of DataIO object is cleared through call of clear() before attempting to read file.
		If file header was read successfully, m_filename is set to fname argument.
		Function throws an IBK::Exception in case of error.
		\param fname File path to DataIO file.
		\param in Input file stream, opened in function and left open (if not in error) for caller to continue reading.
		\param notify Pointer to notification handler (pass NULL to disable notification calls).
	*/
	void openAndReadHeader(const IBK::Path & fname, std::ifstream &in, IBK::NotificationHandler * notify);

	/*! Reads the data file from binary input stream, version 6 and newer.

		The input stream is expected to be binary file stream, and positioned directly after
		the version number. The function reads the header, but not the data itself.
		The actual data in the file is read-on-demand using data().
		\param in Input file stream (as binary file stream).
	*/
	void readBinaryHeader(std::istream& in);

	/*! Reads the data file from binary input stream for pre version 6 formats.

		The input stream is expected to be binary file stream and positioned directly after the
		first line (begin of the header).
		The function reads the header, and geometry data, but not the data itself.
		The actual data in the file is read-on-demand using data().
		\param in Input file stream (as binary file stream).
	*/
	void readBinaryHeaderPreV6(std::istream& in);

	/*! Parses the binary data section and updates the time points vector.

		Starts parsing the data section from m_dataSectionOffset (which must have been
		computed in readBinaryHeader()). This function can be used to refresh the data
		when the file was extended.

		\param in Input file stream (as binary file stream).
		\param notify Pointer to notification handler (pass NULL to disable notification calls).
	*/
	void readBinaryData(std::istream& in, IBK::NotificationHandler * notify);

	/*! Reads the header from the data file, which must be in ASCII format.

		The function reads the header data. The file is read from the begin, but the function reads
		only as much data as needed to display meta information about the project.
		The actual reading of the data section is done in readASCIIData().

		\param fname The file path to the input file.
		\param notify Pointer to notification handler (pass NULL to disable notification calls).
	*/
	void readASCIIHeader(const IBK::Path &fname, IBK::NotificationHandler * notify);

	/*! Reads the complete file, which must be in ASCII format.

		The file is read from the begin, but the header data is ignored and only data from
		the data section is kept.
		\param fname The file path to the input file.
		\param notify Pointer to notification handler (pass NULL to disable notification calls).
	*/
	void readASCIIData(const IBK::Path &fname, IBK::NotificationHandler * notify);

	/*! Utility function for read algorithm.

		The function extracts the time point from the line and appends it to m_timepoints. The remaining string is appended
		to m_valueStrings. An empty vector is appended to the m_values vector to keep the size of m_values and m_valueStrings
		consistent. The parsing of the string into values is delayed until accessing of values with data().
		\param line The line to parse in format '<time point> <value1> <value2> ...'.
	*/
	void addValueLine(const std::string & line);

	/*! Checks if m_timepoints vector and m_values vector have the same length, also checks that m_nValues and size
		of vectors in m_values matches.
		Throws an Exception if any of the conditions fails.
	*/
	void checkDataConsistency() const;


	// *** PRIVATE MEMBER VARIABLES ***

	/*! Major file version, for old DELPHIN 5 format this is 5, for new format 6 and higher. */
	unsigned int						m_majorFileVersion;
	/*! Minor file version, for old DELPHIN 5 format this is always 0. */
	unsigned int						m_minorFileVersion;

	/*! Time (encoded as returned from time() ) of output file writing.
		Value is set in read() and can be queried with createdTime().
	*/
	int64_t								m_createdTime;

	/*! Time (as returned from ctime() ) of output file writing.
		Value is set in read() and can be queried with createdTime().
	*/
	std::string							m_createdTimeString;

	/*! Number of values in the data file.
		For output data with m_spaceType = ST_SINGLE the number of elements/sides m_nValues is equal to m_nums.size().
		For other space types (mean and integral values) m_nValues is equal to 1.
		The functions read(), writeHeader() and setData() set this value.
		It can be queried with the access function nValues().
		The member is mutable, so that is can be modified in const member functions such as writeHeader().

		\note This member variables is only redudant when container holds data in m_values. In this case it matches
				the number of values in each vector, i.e. m_nValues == m_values[0].size(). However, when data is
				written incrementally, the appendData() function needs to known the size of memory beforehead, since
				it only receives a pointer to the data memory. Therefore, this member variable is important mostly
				for writing.
	*/
	mutable unsigned int				m_nValues;

	/*! Vector with values.
		The vector has the same size as m_timepoints. The elements of the vector (which are vectors of values themselves) may
		be empty and are read only, when the values have been parsed already.
		\note This vector is mutable so that we can use lazy-evaluation in a const DataIO object.
	*/
	mutable std::vector< std::vector<double> > 	m_values;

	/*! Vector for first values of all time points (usually for 2D data).
		This vector is by default not used and only populated if dataVector() is called.
		It is essentially a convenience storage member that allows accessing output files with
		averaged or integral values, or single value outputs (senser locations etc.) in a simpler
		way.
		\see dataVector()
		\note This vector is mutable because it is populated during the first access (lazy evaluation),
			  but this does not constitute a change of the DataIO state.
	*/
	mutable std::vector<double>			m_valueVector;

	/*! Vector with strings read from ASCII output files.
		Initially, ASCII files are read as binary files as well, and stored inside the m_valueStrings vector.
		Upon a call to data(), a string is parsed and converted into values, stored in the m_values vector and the string
		is cleared. This way the time consuming string parsing is delayed and only executed on demand.
		\note This vector is mutable so that we can use lazy-evaluation in a const DataIO object.
	*/
	mutable std::vector< std::string > 	m_valueStrings;

	/*! Used during reading to report the progress. */
	unsigned int						m_fileSize;

	/*! Holds the data format represented by the DataIO container.
		Initially this is set to NUM_DF, but updated in determineDataFormat().
	*/
	dataFormat_t						m_dataFormat;

	/*! The file stream used for writing.
		Holds a NULL pointer if file is not opened for writing. After a call to writeHeader() it holds
		a pointer to the file stream.
	*/
	mutable std::ofstream				*m_ofstream;


	/*! Cached time unit to avoid repeatitive lookup of time unit from m_timeUnit string in read functions.
		\warning DO NOT ACCESS this variable, since it will only be synchronized with m_timeUnit during read() calls.
	*/
	IBK::Unit							m_cachedTimeUnit;

	// *** COMPATIBILITY MEMBERS FOR DELPHIN 5 FORMAT ***

	/*! Owned by object, only used when reading Delphin 5 format. Can be retrieved via geometryData(). */
	GeoFile								*m_geometryData;

	/*! Offset of beginning data section (binary files only). */
	unsigned int						m_dataSectionOffset;

	// *** friend classes ***

	friend class FileReaderDataProcessorAddData; ///< Necessary for using addValueLine() in data processor.

};

} // namespace DATAIO

/*! \file DATAIO_DataIO.h
	\brief Contains the declaration of the class DataIO.
*/

#endif // DATAIO_DataIOH
