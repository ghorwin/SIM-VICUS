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

#include "DATAIO_DataIO.h"

#include <cstdlib>
#include <numeric>
#include <cmath>
#include <memory>
#include <cstring>
#include <iterator>
#include <sstream>
#include <list>
#include <algorithm>

#include <IBK_FileUtils.h>
#include <IBK_InputOutput.h>
#include <IBK_FormatString.h>
#include <IBK_Exception.h>
#include <IBK_StopWatch.h>
#include <IBK_FileReader.h>
#include <IBK_Time.h>
#include <IBK_Version.h>
#include <IBK_UnitList.h>
#include <IBK_assert.h>
#include <IBK_UnitVector.h>

#include "DATAIO_Constants.h"
#include "DATAIO_GeoFile.h"

#if (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__))
  #include <windows.h>
#endif

//#define COPY_OUTPUT_FILE_BEFORE_READING

//#define SPEED_RATING_MESSAGES

namespace DATAIO {

/*! Data processor used in FileReader for handling lines from data files.
	It calls DataIO::addValueLine() in order to extract data and time points.
	Additionally the line will be added to m_valueStrings vector for caching (inside addValueLine()).
*/
class FileReaderDataProcessorAddData : public IBK::AbstractFileReaderDataProcessor {
public:
	/*! Standard constructor. Takes a reference of an external line vector.
		\param dataio Pointer to DataIO object to be filled.
	*/
	FileReaderDataProcessorAddData(DataIO* dataio);

	/*! Adds a line to the line vector. Error is only possible in case of unsufficent memory.
		\param line Line for processing.
	*/
	virtual void processLine( const std::string& line );

	/*! Returns the number of time indexes (number of data vectors).
		This function can be used for function in FileReader with maxlines parameter.
		\return Returns number of lines/time indexes read so far.
	*/
	virtual int lineCount() const;

private:
	DataIO* m_dataio;	///< DataIO object.
};

FileReaderDataProcessorAddData::FileReaderDataProcessorAddData(DataIO* dataio) :
	m_dataio(dataio)
{
}


void FileReaderDataProcessorAddData::processLine(const std::string& line) {
	FUNCID(FileReaderDataProcessorAddData::processLine);
	try {
		m_dataio->addValueLine(line);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error extracting data from line '%1'.").arg(line), FUNC_ID);
	}
}


int FileReaderDataProcessorAddData::lineCount() const {
	return (int)m_dataio->m_timepoints.size();
}
// ----------------------------------------------------------------------------


DataIO::DataIO() :
	m_ofstream(nullptr),
	m_geometryData(nullptr)
{
	clear();
	// confirm correct seconds unit
	IBK_ASSERT(IBK::Unit("s").id() == IBK_UNIT_ID_SECONDS);
}
// ----------------------------------------------------------------------------


DataIO::~DataIO() {
	delete m_ofstream;
	m_ofstream = nullptr;
	delete m_geometryData;
	m_geometryData = nullptr;
}
// ----------------------------------------------------------------------------


void DataIO::clear() {
	m_filename.clear();

	m_type = NUM_T;
	m_projectFileName.clear();
	m_geoFileName.clear();
	m_geoFileHash = 0;
	m_quantity.clear();
	m_quantityKeyword.clear();
	m_spaceType = NUM_ST;
	m_timeType = NUM_TT;
	m_valueUnit.clear();
	m_timeUnit.clear();
	m_nums.clear();
	m_nValues = 0;
	m_isBinary = true;
	m_asciiFmtFlags = std::ios_base::fmtflags();
	m_asciiPrecision = -1;

	m_startYear = INVALID_START_YEAR;

	m_mini = -1;
	m_minj = -1;
	m_mink = -1;
	m_maxi = -1;
	m_maxj = -1;
	m_maxk = -1;

	m_fileSize = 0;
	m_dataFormat = NUM_DF;

	m_timepoints.clear();
	m_values.clear();
	m_valueStrings.clear();
	m_valueVector.clear();

	delete m_ofstream;
	m_ofstream = nullptr;
	delete m_geometryData;
	m_geometryData = nullptr;
}
// ----------------------------------------------------------------------------


void DataIO::read(const IBK::Path &fname, bool headerOnly,
	IBK::NotificationHandler * notify)
{
	FUNCID(DataIO::read);

	try {
		std::ifstream in;
		openAndReadHeader(fname, in, notify);

		if (headerOnly)
			return; // done after header

		bool haveD6Format = m_majorFileVersion >= 6;

		if (haveD6Format) {

			// check if this is a binary data file (DELPHIN 6 and later)
			if (m_isBinary) {

				// m_dataSectionOffset now holds the file position where
				// the actual data starts

				// now read the content of the file
				try {
					readBinaryData(in, notify);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading binary data from file.", FUNC_ID);
				}
			}

			else {

				// read the content of the file, without actually parsing the
				// ASCII numbers to the respective double vectors
				try {
					readASCIIData(fname, notify); // readASCIIData reopens the file and reads it completely
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading ASCII content from file.", FUNC_ID);
				}

			}

		}
		else {
			// OLD DELPHIN 5 file format

			if (m_isBinary) {

				// m_dataSectionOffset now holds the file position where
				// the actual data starts

				// now read the content of the file
				try {
					readBinaryData( in, notify);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading binary content from file.", FUNC_ID);
				}

			}

			// the file is in ASCII format
			else {

				// read the content of the file, without actually parsing the
				// ASCII numbers to the respective double vectors
				try {
					readASCIIData(fname, notify); // readASCIIData reopens the file and reads it completely
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading ASCII content from file.", FUNC_ID);
				}

			} // else D5 ASCII format

			if (m_geometryData != nullptr) {
				// in Delphin 5, the matnr of the geometry file has to be interpreted as ID matching one of
				// the IDs in the material definitions table

				for (unsigned int i=0; i<m_geometryData->m_elementsVec.size(); ++i) {
					GeoFile::Element & e = m_geometryData->m_elementsVec[i];
					unsigned int matId = e.matnr;
					// search for matIdx in materials table
					unsigned int matIdx=0;
					for (; matIdx<m_geometryData->m_matDefs.size(); ++matIdx)
						if (m_geometryData->m_matDefs[matIdx].ID == matId)
							break;
					if (matIdx == m_geometryData->m_matDefs.size()) {
						IBK::IBK_Message( IBK::FormatString("Error in geometry data, element #%1 references material with ID #%2, "
										 "which does not exist in materials table.\n").arg(i).arg(matId), IBK::MSG_WARNING, FUNC_ID);
						matIdx = 0;
					}
					e.matnr = matIdx;
				}

			}

		} // D6 m_isBinary

		if (notify != nullptr)	notify->notify(1); // we are done
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading DataIO file '%1'").arg(fname), FUNC_ID);
	}

}  // read()
// ----------------------------------------------------------------------------


const double * DataIO::data(unsigned int time_idx) const {
	FUNCID(DataIO::data);
	// check that index is within range
	IBK_ASSERT(time_idx < m_timepoints.size());
	IBK_ASSERT(m_timepoints.size() == m_values.size());
	if (m_values[time_idx].empty()) {
		if (!m_isBinary) {
			// first access attempt, need to convert
			IBK_ASSERT(time_idx < m_valueStrings.size());
			m_values[time_idx].reserve(m_nValues);
			IBK::string2valueVector(m_valueStrings[time_idx], m_values[time_idx]);
			if (m_values[time_idx].size() != m_nValues) {
				throw IBK::Exception(IBK::FormatString("Not enough values for time index %1 in output file, "
					"only got %2 values, but needed %3.")
					.arg(time_idx)
					.arg((unsigned int)m_values[time_idx].size())
					.arg(m_nValues), FUNC_ID);
			}
			std::string().swap(m_valueStrings[time_idx]); // replace with empty string so that memory is freed.
		}
		else {
#ifndef READ_FULL_FILE
			// open output file
			std::ifstream in;
			if (!IBK::open_ifstream(in, m_filename, std::ios_base::binary))
				throw IBK::Exception("Cannot open output file for reading.", FUNC_ID);

			in.seekg(m_dataSectionOffset, std::ios_base::beg);
			in.seekg((time_idx*(1 + m_nValues) + 1)*sizeof(double), std::ios_base::cur);
			m_values[time_idx].resize(m_nValues);
			in.read(reinterpret_cast<char *>(&(m_values[time_idx][0])), sizeof(double)*m_nValues );
			if (!in)
				throw IBK::Exception("Error reading data for this time index.", FUNC_ID);
#endif // READ_FULL_FILE
		}
	}
	return &m_values[time_idx][0];
}
// ----------------------------------------------------------------------------


bool DataIO::truncateData(double timePointInSeconds) {

	// TODO : think about correct return values - in the first two cases, the DataIO does indeed
	//        contain no data beyond the timePointInSeconds limit... so the function had returned successfully?

	if (m_timepoints.empty())
		return false; // nothing to truncate, already empty

	std::vector<double>::iterator it = std::lower_bound(m_timepoints.begin(), m_timepoints.end(), timePointInSeconds);
	if (it == m_timepoints.end())
		return false; // nothing to truncate, already empty

	if (it == m_timepoints.begin()) {
		// Note: can't call clear() here, because that erases all meta data as well. Instead, we just need to wipe
		//       out the data vector and time points vectors entirely.
		m_timepoints.clear();
		m_valueStrings.clear();
		m_values.clear();
		return true;
	}

	unsigned int idx = it - m_timepoints.begin();
	IBK_ASSERT(idx < m_values.size());

	// we could call deleteData() here, but directly truncating the data is easier

	// truncate time points vector
	m_timepoints.erase(it, m_timepoints.end());
	// truncate values vector
	m_values.erase(m_values.begin() + idx, m_values.end());

	// for ASCII format we need to erase m_valueStrings as well
	if (!m_valueStrings.empty()) {
		// ASCII format sanity checks
		IBK_ASSERT(idx < m_valueStrings.size());
		m_valueStrings.erase(m_valueStrings.begin() + idx, m_valueStrings.end());
	}

	return true;
}


void DataIO::deleteData(unsigned int idxFrom, unsigned int idxTo) {
	FUNCID(DataIO::deleteData);

	if (idxFrom >= m_timepoints.size())
		throw IBK::Exception("First time index 'idxFrom' out of range.", FUNC_ID);

	if (idxTo >= m_timepoints.size())
		throw IBK::Exception("Second time index 'idxTo' out of range.", FUNC_ID);

	IBK_ASSERT(idxTo < m_values.size());
	IBK_ASSERT(idxFrom < m_values.size());

	if (idxFrom == idxTo) {

		std::vector< std::vector<double> >::iterator values_it1 = m_values.begin() + idxFrom;
		/// \todo Performance hack: removing a vector from within a vector will be done by copying
		///		  later vectors over earlier vectors. Speedup will be significant if instead of copying
		///		  later vectors will be swapped (pointer-change) bit-by-bit with earlier vectors.
		m_values.erase(values_it1);

		// for ASCII format we need to erase m_valueStrings as well
		if (!m_valueStrings.empty()) {
			// ASCII format sanity checks
			IBK_ASSERT(idxTo < m_valueStrings.size());
			IBK_ASSERT(idxFrom < m_valueStrings.size());

			std::vector< std::string >::iterator values_it1 = m_valueStrings.begin() + idxFrom;
			m_valueStrings.erase(values_it1);
		}

		std::vector<double>::iterator timepoints_it1 = m_timepoints.begin() + idxFrom;
		m_timepoints.erase(timepoints_it1);
	}
	else {

		/// \todo Performance improvement, use temperary data structures IO, copy only the values that remain
		/// (use proper reserve() calls, and afterwards swap the vectors.
		std::vector< std::vector<double> >::iterator values_it1 = m_values.begin() + idxFrom;
		std::vector< std::vector<double> >::iterator values_it2 = m_values.begin() + idxTo+1;
		m_values.erase(values_it1,values_it2);

		if (!m_valueStrings.empty()) {
			// ASCII format sanity checks
			IBK_ASSERT(idxTo < m_valueStrings.size());
			IBK_ASSERT(idxFrom < m_valueStrings.size());

			std::vector< std::string >::iterator values_it1 = m_valueStrings.begin() + idxFrom;
			std::vector< std::string >::iterator values_it2 = m_valueStrings.begin() + idxTo+1;
			m_valueStrings.erase(values_it1,values_it2);
		}

		std::vector<double>::iterator timepoints_it1 = m_timepoints.begin() + idxFrom;
		std::vector<double>::iterator timepoints_it2 = m_timepoints.begin() + idxTo+1;
		m_timepoints.erase(timepoints_it1,timepoints_it2);
	}

}
// ----------------------------------------------------------------------------


void DataIO::setData(const std::vector<double> & timePoints,
	const std::vector<std::vector<double> > & values)
{
	FUNCID(DataIO::setData);
	if (values.size() != timePoints.size())
		throw IBK::Exception("Size mismatch between values and timePoints vectors.", FUNC_ID);

	if (!values.empty()) {
		m_nValues = (unsigned int)values[0].size();
		for (unsigned int i=1; i<values.size(); ++i)
			if (values[i].size() != m_nValues)
				throw IBK::Exception("Size mismatch between individual vectors in values vector.", FUNC_ID);
	}
	else {
		m_nValues = 0;
	}
	m_valueStrings.clear();
	m_valueVector.clear();
	m_timepoints = timePoints;
	m_values = values;
}
// ----------------------------------------------------------------------------


void DataIO::swapData(std::vector<double> & timePoints,
	std::vector<std::vector<double> > & values)
{
	FUNCID(DataIO::swapData);
	checkDataConsistency();
	if (values.size() != timePoints.size())
		throw IBK::Exception("Size mismatch between values and timePoints vectors.", FUNC_ID);

	if (values.empty()) {
		m_nValues = (unsigned int)values[0].size();
		for (unsigned int i=1; i<values.size(); ++i)
			if (values[i].size() != m_nValues)
				throw IBK::Exception("Size mismatch between individual vectors in values vector.", FUNC_ID);
	}
	else {
		m_nValues = 0;
	}
	m_valueStrings.clear();
	m_valueVector.clear();
	m_timepoints.swap(timePoints);
	m_values.swap(values);
}
// ----------------------------------------------------------------------------


const double * DataIO::dataVector() const {
	FUNCID(DataIO::dataVector);

	if (m_timepoints.empty())
		throw IBK::Exception("No data in output file.", FUNC_ID);

	if (m_valueVector.empty()) {
		m_valueVector.reserve(m_timepoints.size());
		double lastTime = m_timepoints.front();
		for (unsigned int i=0; i<m_timepoints.size(); ++i) {
			try {
				const double * val = data(i);
				m_valueVector.push_back(*val);
				if (lastTime > m_timepoints[i]) {
					IBK::IBK_Message( IBK::FormatString("Time value %1 follows %2, not monotonically increasing time points found.")
						.arg( IBK::Time::format_time_difference(m_timepoints[i], m_timeUnit))
						.arg( IBK::Time::format_time_difference(lastTime, m_timeUnit)), IBK::MSG_DEBUG, FUNC_ID);
				}
				lastTime = m_timepoints[i];
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error retrieving values for time index %1.").arg(i), FUNC_ID);
			}
		}
	}
	return &m_valueVector[0];
}
// ----------------------------------------------------------------------------


void DataIO::adjustFileName() {
	FUNCID(DataIO::adjustFileName);

	if (!m_filename.isValid())
		throw IBK::Exception("Missing or invalid filename.", FUNC_ID);

	// substitute extension if not specified
	std::string ext = m_filename.extension();

	// only remove extension if existing extension is either out, d6o or d6b
	IBK::Path fname_wo_ext = m_filename;
	if (ext == "d6o" || ext == "d6b" || ext == "out")
		fname_wo_ext = m_filename.withoutExtension();
	if (m_isBinary)		fname_wo_ext.addExtension(".d6b");
	else				fname_wo_ext.addExtension(".d6o");

	m_filename = fname_wo_ext;
}
// ----------------------------------------------------------------------------


void DataIO::write(IBK::NotificationHandler * notify) const {
	FUNCID(DataIO::write);

	IBK::StopWatch timer;
	timer.setIntervalLength(1);

	checkDataConsistency();

	// Data is valid, now write the header (here, the remaining checks are done)
	// write header (and create output file in the process)
	writeHeader();

	// write data
	for (unsigned int i=0; i<m_timepoints.size(); ++i) {
		try {
			appendData(m_timepoints[i], &m_values[i][0]);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error writing data at time index %1.").arg(i), FUNC_ID);
		}
		if (notify && timer.intervalCompleted())
			notify->notify(double(i+1)/m_timepoints.size());
	}
	if (notify != 0)
		notify->notify(1);
	const_cast<DataIO*>(this)->close();
}
// ----------------------------------------------------------------------------


void DataIO::writeHeader() const {
	FUNCID(DataIO::writeHeader);

	delete m_ofstream; // delete old file stream (does nothing, if no file was open)
	m_ofstream = nullptr;

	if (!m_filename.isValid())
		throw IBK::Exception("Invalid filename or no filename set.", FUNC_ID);

	// check for correct file extension
	std::string ext = m_filename.extension();
	// only remove extension if d6b or d6o is used
	IBK::Path fname_wo_ext = m_filename;
	if (ext == "d6o" || ext == "d6b" || ext == "out")
		fname_wo_ext = m_filename.withoutExtension();
	if (m_isBinary)		fname_wo_ext.addExtension(".d6b");
	else				fname_wo_ext.addExtension(".d6o");

	if (fname_wo_ext != m_filename) {
		throw IBK::Exception(IBK::FormatString("Invalid filename (extension) of output file '%1', should have been '%2'!\n")
			.arg(m_filename).arg(fname_wo_ext), FUNC_ID);
	}

	// create output file stream

	m_ofstream = IBK::create_ofstream(m_filename, std::ios_base::binary | std::ios_base::trunc);

	// check if successful
	if (!m_ofstream->good()) {
		delete m_ofstream;
		m_ofstream = nullptr;
		throw IBK::Exception( IBK::FormatString("Couldn't open output file '%1'.").arg(m_filename), FUNC_ID);
	}
	std::ostream & out = *m_ofstream; // readability improvement

	// write magic header and version number
	IBK::Version::write(out, MAGIC_NUMBER_BINARY_DATAIO, SECOND_MAGIC_NUMBER, MAGIC_NUMBER_ASCII_DATAIO, SECOND_MAGIC_NUMBER,
						m_isBinary, MAJOR_FILE_VERSION, MINOR_FILE_VERSION);

	// set number of values per output time based on space type setting and nums vector
	if (m_spaceType == ST_SINGLE)
		m_nValues = (unsigned int)m_nums.size();
	else
		m_nValues = 1;

	// different handling based on binary or ASCII settings
	if (m_isBinary) {
		// write dummy integer with the offset of the data section
		IBK::write_uint32_binary( out, 0 );
		// write number of values in data blocks
		IBK::write_uint32_binary( out, m_nValues );

		// type id number
		out.write(reinterpret_cast<const char *>(&m_type), sizeof(unsigned int));

		// project file
		IBK::write_string_binary(out, m_projectFileName);
		// geometry file
		IBK::write_string_binary(out, m_geoFileName.str());
		// geometry hash
		out.write(reinterpret_cast<const char *>(&m_geoFileHash), sizeof(unsigned int));

		// created time
		int64_t t = static_cast<int>(time(nullptr));
		out.write(reinterpret_cast<char *>(&t), sizeof(int64_t));

		// quantity description
		IBK::write_string_binary(out, m_quantity);
		// quantity keyword
		IBK::write_string_binary(out, m_quantityKeyword);

		// spatial averaging/integration type
		out.write(reinterpret_cast<const char *>(&m_spaceType), sizeof(unsigned int));
		// time averaging/integration type
		out.write(reinterpret_cast<const char *>(&m_timeType), sizeof(unsigned int));
		// value unit as string
		IBK::write_string_binary(out, m_valueUnit);
		// time unit as string
		IBK::write_string_binary(out, m_timeUnit);
		// write start year
		out.write(reinterpret_cast<const char *>(&m_startYear), sizeof(int));
		// write nums vector
		IBK::write_vector_binary<unsigned int>(out, m_nums);

		// end writing binary header

		// determine file offset
		unsigned int offset = (unsigned int)out.tellp();
		// now go back to file offset with data section offset and write the offset
		out.seekp(16, std::ios_base::beg);
		IBK::write_uint32_binary( out, offset );
		// and seek back to the data section start
		out.seekp(offset, std::ios_base::beg);

	}
	else { // if (m_isBinary) {

		// otherwise write standard ASCII output file
		out << "\n";
		out << "TYPE          = " << type2string(m_type) << '\n';
		out << "PROJECT_FILE  = " << m_projectFileName << '\n';
		std::time_t t = time(nullptr);
		out << "CREATED       = " << std::ctime(&t);
		out << "GEO_FILE      = " << m_geoFileName << '\n';
		out << "GEO_FILE_HASH = " << m_geoFileHash << '\n';
		out << "QUANTITY      = " << m_quantity << '\n';
		out << "QUANTITY_KW   = " << m_quantityKeyword << '\n';
		out << "SPACE_TYPE    = " << spaceType2string(m_spaceType) << '\n';
		out << "TIME_TYPE     = " << timeType2string(m_timeType) << '\n';
		out << "VALUE_UNIT    = " << m_valueUnit << '\n';
		out << "TIME_UNIT     = " << m_timeUnit << '\n';
		out << "START_YEAR    = " << m_startYear << '\n';
		out << "INDICES       = ";
		std::copy( m_nums.begin(), m_nums.end(), std::ostream_iterator<unsigned int>(out, " "));

		out << std::endl << std::endl;
		out.flush();

		// done writing ASCII header

		// prepare ASCII output format
		if (m_asciiPrecision > -1)
			out.precision(m_asciiPrecision);
		if (m_asciiFmtFlags & std::ios_base::floatfield) {
			out.setf(m_asciiFmtFlags);
		}

	}
	if (!out.good())
		throw IBK::Exception( IBK::FormatString("Error writing file '%1'.").arg(m_filename), FUNC_ID);
}
// ----------------------------------------------------------------------------


void DataIO::appendData(double t, const double * values) const {
	FUNCID(DataIO::appendData);

	if (m_ofstream == nullptr)
		throw IBK::Exception("File not opened for writing.", FUNC_ID);

	std::ostream & out = *m_ofstream; // readability improvement
	// convert output time into correct output unit
	try {
		IBK::UnitList::instance().convert( IBK::Unit(IBK_UNIT_ID_SECONDS), IBK::Unit(m_timeUnit), t);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Invalid time unit in DataIO container.", FUNC_ID);
	}
	if (m_isBinary) {
		out.write(reinterpret_cast<char *>(&t), sizeof(double));
		out.write((const char*)values, sizeof(double)*m_nValues);
	}
	else {
		out << std::setw(12) << std::left << t << " \t";
		for (unsigned int j=0; j<m_nValues; ++j) {
			out << std::setw(15) << std::left << values[j] << '\t';
		}
		out << '\n';
	}
	// flush is required for post-proc live-update to work
	out.flush();
}
// ----------------------------------------------------------------------------


void DataIO::reopenForWriting(const IBK::Path &fname) {
	FUNCID(DataIO::reopenForWriting);

	try {
		std::ifstream in;
		openAndReadHeader(fname, in, nullptr);

		// we have read the header, we have no need for the file stream anylonger
		in.close();
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading DataIO file '%1'").arg(fname), FUNC_ID);
	}

	// reopen file and attempt to read header

	if (m_isBinary) {
		m_ofstream = IBK::create_ofstream(m_filename, std::ios_base::binary | std::ios_base::app);
	}
	else {
		m_ofstream = IBK::create_ofstream(m_filename, std::ios_base::app);
	}

	// check if successful
	if (!m_ofstream->good()) {
		delete m_ofstream;
		m_ofstream = nullptr;
		throw IBK::Exception( IBK::FormatString("Couldn't open output file '%1'.").arg(fname), FUNC_ID);
	}

	// set number of values per output time based on space type setting and nums vector
	if (m_spaceType == ST_SINGLE)
		m_nValues = (unsigned int)m_nums.size();
	else {
		if (m_type == DataIO::T_REFERENCE)
			m_nValues = (unsigned int)m_nums.size();
		else
			m_nValues = 1;
	}
}
// ----------------------------------------------------------------------------


void DataIO::close() const {
	if (m_ofstream != nullptr) {
		m_ofstream->close();
		delete m_ofstream;
		m_ofstream = nullptr;
	}
}

// ----------------------------------------------------------------------------


DataIO::dataFormat_t DataIO::determineDataFormat(const GeoFile & geoFile) {
	FUNCID(DataIO::determineDataFormat);

	if (nValues() == 1 || m_type == T_REFERENCE) {

		m_dataFormat = DF_2D;

	}
	else {

		// do we have geometry data at all?
		if (geoFile.m_majorFileVersion == 0) {
			IBK::IBK_Message( "Empty/uninitialized geometry data.", IBK::MSG_ERROR, FUNC_ID);
			m_dataFormat = NUM_DF;
			return m_dataFormat;
		}

		// set to the maximal size which is possible
		m_mini = (int)geoFile.m_grid.xwidths.size()+1;
		m_minj = (int)geoFile.m_grid.ywidths.size()+1;
		m_mink = (int)geoFile.m_grid.zwidths.size()+1;
		// set to the minimum size that is possible
		m_maxi = -1;
		m_maxj = -1;
		m_maxk = -1;

		// check if same x and y coordinates are used
		if (m_type == T_FLUX) {

			// missing data in GeoFile -> invalid dataFormat
			unsigned int sidesCount = (unsigned int)geoFile.m_sidesVec.size();
			if (sidesCount == 0) {
				IBK::IBK_Message( "Missing/empty sides vector in Geometry file.", IBK::MSG_ERROR, FUNC_ID);
				m_dataFormat = NUM_DF;
				return m_dataFormat;
			}

			// set min and max for columns, rows, and stacks
			for (unsigned int j=0; j<m_nums.size(); ++j) {
				int s = m_nums[j];
				// invalid nums vector
				if ((unsigned int)s >= sidesCount) {
					IBK::IBK_Message( IBK::FormatString("Invalid side index #%1, sides vector has only %2 entries.")
									  .arg(s).arg(sidesCount), IBK::MSG_ERROR, FUNC_ID);
					m_dataFormat = NUM_DF;
					return m_dataFormat;
				}
				m_maxi = std::max<int>(m_maxi, geoFile.m_sidesVec[s].i);
				m_maxj = std::max<int>(m_maxj, geoFile.m_sidesVec[s].j);
				m_maxk = std::max<int>(m_maxk, geoFile.m_sidesVec[s].k);

				m_mini = std::min<int>(m_mini, geoFile.m_sidesVec[s].i);
				m_minj = std::min<int>(m_minj, geoFile.m_sidesVec[s].j);
				m_mink = std::min<int>(m_mink, geoFile.m_sidesVec[s].k);
			}
		}
		else {

			// missing data in GeoFile -> invalid dataFormat
			unsigned int elementCount = (unsigned int)geoFile.m_elementsVec.size();
			if (elementCount == 0) {
				IBK::IBK_Message( "Missing/empty elements vector in Geometry file.", IBK::MSG_ERROR, FUNC_ID);
				m_dataFormat = NUM_DF;
				return m_dataFormat;
			}

			// set min and max for columns, rows, and stacks
			for (unsigned int j=0; j<m_nums.size(); ++j) {
				int e = m_nums[j];
				// invalid nums vector
				if ((unsigned int)e >= elementCount) {
					IBK::IBK_Message( IBK::FormatString("Invalid element index #%1, elements vector has only %2 entries.")
									  .arg(e).arg(elementCount), IBK::MSG_ERROR, FUNC_ID);
					m_dataFormat = NUM_DF;
					return m_dataFormat;
				}
				m_maxi = std::max<int>(m_maxi, geoFile.m_elementsVec[e].i);
				m_maxj = std::max<int>(m_maxj, geoFile.m_elementsVec[e].j);
				m_maxk = std::max<int>(m_maxk, geoFile.m_elementsVec[e].k);

				m_mini = std::min<int>(m_mini, geoFile.m_elementsVec[e].i);
				m_minj = std::min<int>(m_minj, geoFile.m_elementsVec[e].j);
				m_mink = std::min<int>(m_mink, geoFile.m_elementsVec[e].k);
			}
		}

		// 5D format, if all coordinates differs
		if (m_maxi != m_mini && m_maxj != m_minj && m_maxk != m_mink )
			m_dataFormat = DF_5D;

		// 4D format, if either same z or
		else if (m_maxi != m_mini && m_maxj != m_minj && m_maxk == m_mink )
			m_dataFormat = DF_4D;

		// 4D format, if either same y or
		else if (m_maxi != m_mini && m_maxj == m_minj && m_maxk != m_mink )
			m_dataFormat = DF_4D;

		// 4D format, same x coordinates
		else if (m_maxi == m_mini && m_maxj != m_minj && m_maxk != m_mink )
			m_dataFormat = DF_4D;

		// 3D format, if either same x,y or
		else if (m_maxi == m_mini && m_maxj == m_minj && m_maxk != m_mink )
			m_dataFormat = DF_3D;

		// 3D format, if either same x,z or
		else if (m_maxi == m_mini && m_maxj != m_minj && m_maxk == m_mink )
			m_dataFormat = DF_3D;

		// 3D format, same y,z coordinates
		else if (m_maxi != m_mini && m_maxj == m_minj && m_maxk == m_mink )
			m_dataFormat = DF_3D;

		else {
			IBK::IBK_Message("Cannot determine data format.", IBK::MSG_ERROR, FUNC_ID);
			m_dataFormat = NUM_DF;
		}

	}

	return m_dataFormat;
}
// ----------------------------------------------------------------------------



// *** IMPLEMENTATION PRIVATE MEMBER FUNCTIONS ***

std::string DataIO::dataFormat2String(dataFormat_t df) {
	switch ( df ) {
		case DataIO::DF_2D :
			return "2D [time, value]";
		case DataIO::DF_3D :
			return "3D [time, vector of values]";
		case DataIO::DF_4D :
			return "4D [time, matrix 2x2 of values]";
		case DataIO::DF_5D :
			return "5D [time, matrix 3x3 of values]";
		default :
			return "undefined";
	}
}


DataIO::type_t DataIO::string2type(const std::string& str) {
	if (str.find("FIELD")			!=	std::string::npos)	return T_FIELD;
	else if (str.find("FLUX")		!=	std::string::npos)	return T_FLUX;
	else if (str.find("REFERENCE")	!=	std::string::npos)	return T_REFERENCE;
	else 													return NUM_T;
}
// ----------------------------------------------------------------------------


std::string DataIO::type2string(type_t t) {
	switch (t) {
		case T_FIELD		 	: return "FIELD";
		case T_FLUX				: return "FLUX";
		case T_REFERENCE		: return "REFERENCE";
		default							:
			throw IBK::Exception(IBK::FormatString("Invalid/undefined type."), "[DataIO::type2string]");
	}
}
// ----------------------------------------------------------------------------


DataIO::spaceType_t DataIO::string2spaceType(const std::string& str) {
	if (str.find("SINGLE")			!=	std::string::npos)	return ST_SINGLE;
	else if (str.find("MEAN")		!=	std::string::npos)	return ST_MEAN;
	else if (str.find("INTEGRAL")	!=	std::string::npos)	return ST_INTEGRAL;
	else 													return NUM_ST;
}
// ----------------------------------------------------------------------------


std::string DataIO::spaceType2string(spaceType_t t) {
	switch (t) {
		case ST_SINGLE 		: return "SINGLE";
		case ST_MEAN		: return "MEAN";
		case ST_INTEGRAL	: return "INTEGRAL";
		default 			: return "UNKNOWN";
	}
}
// ----------------------------------------------------------------------------


DataIO::timeType_t DataIO::string2timeType(const std::string& str) {
	if (str.find("NONE")			!=	std::string::npos)	return TT_NONE;
	else if (str.find("MEAN")		!=	std::string::npos)	return TT_MEAN;
	else if (str.find("INTEGRAL")	!=	std::string::npos)	return TT_INTEGRAL;
	else 													return NUM_TT;
}
// ----------------------------------------------------------------------------


std::string DataIO::timeType2string(timeType_t t) {
	switch (t) {
		case TT_NONE 			: return "NONE";
		case TT_MEAN			: return "MEAN";
		case TT_INTEGRAL		: return "INTEGRAL";
		default 				: return "UNKNOWN";
	}
}
// ----------------------------------------------------------------------------


void DataIO::openAndReadHeader(const IBK::Path & fname, std::ifstream & in, IBK::NotificationHandler *notify) {
	FUNCID(DataIO::openAndReadHeader);

	// make sure the sizes are correct
	IBK_STATIC_ASSERT(sizeof(unsigned int) == 4);
	IBK_STATIC_ASSERT(sizeof(int) == 4);
	IBK_STATIC_ASSERT(sizeof(double) == 8);

	if (!fname.isValid())
		throw IBK::Exception("No filename or invalid file path given.", FUNC_ID);
	if (!fname.exists())
		throw IBK::Exception( IBK::FormatString("File '%1' doesn't exist.").arg(fname), FUNC_ID);

	// Note: In case of reopening a file for appending outputs, the function will be called with m_filename as
	//		 argument. If fname were a const reference to this member variable, its content would be cleared
	//		 during execution of clear(). Therefore create a temporary copy of the file path.
	IBK::Path fname_tmp = fname;

	// Empty data in object
	clear();

	// open file in binary mode and determine file size
	if (!IBK::open_ifstream(in, fname_tmp, std::ios_base::binary))
		throw IBK::Exception( IBK::FormatString("Cannot open file '%1'").arg(fname), FUNC_ID);

	try {

		in.seekg(0, std::ios_base::end);
		m_fileSize = (unsigned int)in.tellg();
		in.seekg(0, std::ios_base::beg);
		m_dataSectionOffset = 0;


		// read magic header and determine file format and version
		bool haveD6Format = false;
		try {
			IBK::Version::read(in, MAGIC_NUMBER_BINARY_DATAIO, SECOND_MAGIC_NUMBER, MAGIC_NUMBER_ASCII_DATAIO, SECOND_MAGIC_NUMBER,
							   m_isBinary, m_majorFileVersion, m_minorFileVersion);
			haveD6Format = true;
		}
		catch (...) {
		}


		if (haveD6Format) {

			// check if this is a binary data file (DELPHIN 6 and later)
			if (m_isBinary) {

				// now read the content of the file
				try {
					readBinaryHeader(in);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading binary header from file.", FUNC_ID);
				}

			}

			else {

				// now read the header of the file
				try {
					in.close();
					readASCIIHeader(fname_tmp, notify);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading ASCII header from file.", FUNC_ID);
				}

			}

		}
		else {
			// check for OLD DELPHIN 5 file format

			// try to read first line in old DELPHIN 5 format
			const int BUF_LEN = 40;
			char buf[BUF_LEN];
			in.seekg(0, std::ios_base::beg);
			in.read(buf, BUF_LEN);
			buf[BUF_LEN-1] = 0;
			// check that we have indeed a Delphin output file
			std::string firstline(buf);
			if (firstline.find("Delphin 5 Output file") == std::string::npos) {
				throw IBK::Exception("File format not recognized!", FUNC_ID);
			}
			// set version numbers
			m_majorFileVersion = 5;
			m_minorFileVersion = 0;
			// binary file?
			if (firstline.find("- binary") != std::string::npos) {
				m_isBinary = true;

				// now read the header of the file
				try {
					readBinaryHeaderPreV6( in);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading binary header from file.", FUNC_ID);
				}

			}

			// the file is in ASCII format
			else {
				m_isBinary = false;

				// now read the header of the file
				try {
					in.close();
					readASCIIHeader(fname_tmp, notify);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, "Error on reading ASCII header from file.", FUNC_ID);
				}

			} // else D5 ASCII format
		} // D6 m_isBinary

	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading DataIO file '%1'").arg(fname), FUNC_ID);
	}

	// file was read successfully, store filename
	m_filename = fname_tmp;
}
// ----------------------------------------------------------------------------


void DataIO::readBinaryHeader(std::istream& in) {
	FUNCID(DataIO::readBinaryHeader);
	// reads only Delphin 6 format and higher, in which case in is positioned right after the magic header

	try {

		unsigned int offset;
		unsigned int nVals;
		// read data section offset
		in.read(reinterpret_cast<char *>(&offset), sizeof(unsigned int));
		// read number of values in data section
		in.read(reinterpret_cast<char *>(&nVals), sizeof(unsigned int));

		// read type
		in.read(reinterpret_cast<char *>(&m_type), sizeof(unsigned int));
		// project file
		try { IBK::read_string_binary(in, m_projectFileName, 10000); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading PROJECT_FILE property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		// geometry file
		try {
			std::string fname;
			IBK::read_string_binary(in, fname, 10000);
			m_geoFileName = fname;
		}
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading GEO_FILE property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		// geometry file hash
		in.read(reinterpret_cast<char *>(&m_geoFileHash), sizeof(unsigned int));

		// read created time
		in.read(reinterpret_cast<char *>(&m_createdTime), sizeof(int64_t));

		// read quantity
		try { IBK::read_string_binary(in, m_quantity, 500000); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading QUANTITY property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		// Read quantity keyword
		try { IBK::read_string_binary(in, m_quantityKeyword, 500000); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading QUANTITY_KW property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		in.read(reinterpret_cast<char *>(&m_spaceType), sizeof(unsigned int));
		in.read(reinterpret_cast<char *>(&m_timeType), sizeof(unsigned int));
		try { IBK::read_string_binary(in, m_valueUnit, 100); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading VALUE_UNIT property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		try { IBK::read_string_binary(in, m_timeUnit, 100); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading TIME_UNIT property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		// Read start year
		in.read(reinterpret_cast<char *>(&m_startYear), sizeof(int));

		// read number vector
		try { IBK::read_vector_binary(in, m_nums, 1000000); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading element/sides numbers vector: "+ std::string(ex.what()), FUNC_ID);
		}

		// compute number of values stored per time point
		if (m_spaceType == ST_SINGLE)
			m_nValues = (unsigned int)m_nums.size();
		else {
			if (m_type == DataIO::T_REFERENCE)
				m_nValues = (unsigned int)m_nums.size();
			else
				m_nValues = 1;
		}

		// set data format
		if (m_nValues == 1 || m_type == T_REFERENCE)
			// for 2D and Reference type files
			m_dataFormat = DF_2D;

		// store current stream position, directly positioned after geometry/header section, and thus
		// at begin of data section
		m_dataSectionOffset = (unsigned int)in.tellg();

		// additional checks for Version 6 and newer files
		if (m_nValues != nVals) {
			throw IBK::Exception(IBK::FormatString(
									 "Number of values in data section %1  defined at header start does not match "
									 "number of values computed from INDICES and SPACE_TYPE = %2.").arg(nVals).arg(m_nValues).str(),
								 FUNC_ID);
		}
		if (m_dataSectionOffset != offset) {
			throw IBK::Exception("Data section offset defined at header start does not match file offset "
								 "after reading header section.", FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error reading binary header.", FUNC_ID);
	}
}
// ----------------------------------------------------------------------------


void DataIO::readBinaryHeaderPreV6(std::istream& in) {
	FUNCID(DataIO::readBinaryHeaderPreV6);
	// reads old DELPHIN 5 format where the input stream is positioned right after the first ID line

	try {

		// read type
		in.read(reinterpret_cast<char *>(&m_type), sizeof(unsigned int));
		// project file
		try { IBK::read_string_binary_with_termination_character(in, m_projectFileName, 10000); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading PROJECT_FILE property in header: "+ std::string(ex.what()), FUNC_ID);
		}

		// read time
		int t;
		in.read(reinterpret_cast<char *>(&t), sizeof(unsigned int));
		m_createdTime = (int64_t)t;

		// read other data
		try { IBK::read_string_binary_with_termination_character(in, m_quantity, 1000); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading QUANTITY property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		in.read(reinterpret_cast<char *>(&m_spaceType), sizeof(unsigned int));
		in.read(reinterpret_cast<char *>(&m_timeType), sizeof(unsigned int));
		try { IBK::read_string_binary_with_termination_character(in, m_valueUnit, 100); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading VALUE_UNIT property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		try { IBK::read_string_binary_with_termination_character(in, m_timeUnit, 100); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading TIME_UNIT property in header: "+ std::string(ex.what()), FUNC_ID);
		}
		// read geometry data
		// read length of geometry block
		unsigned int geo_len;
		in.read(reinterpret_cast<char *>(&geo_len), sizeof(unsigned int));
		unsigned int current_pos = (unsigned int)in.tellg();
		unsigned int pos_after_geo = current_pos + geo_len;
		m_geometryData = new GeoFile;
		m_geometryData->m_majorFileVersion = m_majorFileVersion;
		try {
			// set major file version number in geometry data
			m_geometryData->m_majorFileVersion = 5;
			m_geometryData->readBinaryGeometryData(in);
			m_geometryData->m_constructionLines.generate(*m_geometryData);
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, "Error reading geometry data:\n", FUNC_ID);
		}
		current_pos = (unsigned int)in.tellg();
		if (current_pos != pos_after_geo) {
			throw IBK::Exception("[DataIO::readBinaryHeader]  Size mismatch in geometry section!", FUNC_ID);
		}

		// read number vector
		try { IBK::read_vector_binary(in, m_nums, 1000000); }
		catch (std::exception & ex) {
			throw IBK::Exception("Error reading element/sides numbers vector: "+ std::string(ex.what()), FUNC_ID);
		}

		// read and check header end tag
		std::string end_tag;
		try { IBK::read_string_binary_with_termination_character(in, end_tag, 100); }
		catch (std::exception &) {
			throw IBK::Exception("Binary header data corrupt! Failed to read HDR_END tag.", FUNC_ID);
		}

		if (end_tag != std::string("HDR_END")) {
			throw IBK::Exception("Binary header data corrupt!", FUNC_ID);
		}

		// compute number of values stored per time point
		if (m_spaceType == ST_SINGLE)
			m_nValues = (unsigned int)m_nums.size();
		else {
			if (m_type == DataIO::T_REFERENCE)
				m_nValues = (unsigned int)m_nums.size();
			else
				m_nValues = 1;
		}

		// set data format
		if (m_nValues == 1 || m_type == T_REFERENCE)
			// for 2D and Reference type files
			m_dataFormat = DF_2D;
		else if (m_geometryData != nullptr) {
			// and for D5 outputs with embedded geometry data
			try {
				determineDataFormat(*m_geometryData);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, "Error in DataIO or embedded geometry data.", FUNC_ID);
			}
		}

		// store current stream position, directly positioned after geometry/header section, and thus
		// at begin of data section
		m_dataSectionOffset = (unsigned int)in.tellg();

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error reading binary header (pre V6 version).", FUNC_ID);
	}

}
// ----------------------------------------------------------------------------


void DataIO::readBinaryData(std::istream& in, IBK::NotificationHandler * notify) {
	FUNCID(DataIO::readBinaryData);
	// we expect m_dataSectionOffset to hold the position of the data section
	// in must be an opened binary file stream

	try {

		in.seekg(m_dataSectionOffset, std::ios_base::beg);

		// read data section
		// clear time points vector
		m_timepoints.clear();
		m_values.clear();
		/// \todo add performance tweak: keep already read values in m_values and avoid re-reading after update

		// cache time unit
		m_cachedTimeUnit.set(m_timeUnit); // this may throw an exception, if m_timeUnit is invalid

		IBK::UnitVector tpVec;
		// check stream flags in addition to isBinary flag
		while (in) {
			unsigned int pos = (unsigned int)in.tellg();
			if (notify)
				notify->notify(double(pos)/m_fileSize);
			// read timepoint
			double tmp;
			in.read(reinterpret_cast<char *>(&tmp), sizeof(double));
			/// \todo handle incomplete reads (when file is concurrently written)
			if (in.eof()) break; // nothing more to read, abort
			// convert output time into correct output unit
			tpVec.m_data.push_back(tmp);
			// add empty vector
			m_values.push_back(std::vector<double>());
			// seek to next record
			in.seekg(sizeof(double) * m_nValues, std::ios_base::cur);
		}
		// convert time points into seconds
		tpVec.m_unit = m_cachedTimeUnit;
		tpVec.convert( IBK::Unit(IBK_UNIT_ID_SECONDS) ); // may throw an exception if m_timeUnit is not a time unit
		// now swap values with timepoints vector
		m_timepoints.swap(tpVec.m_data);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error on read binary data.", FUNC_ID);
	}
}
// ----------------------------------------------------------------------------


void DataIO::readASCIIHeader(const IBK::Path & fname, IBK::NotificationHandler * notify) {
	FUNCID(DataIO::readASCIIHeader);

	// Note: do not call clear() in this function! See note in openAndReadHeader().

	try {

		IBK::StopWatch w;

		// read all lines of the header into a vector of strings
		std::vector<std::string> headerLines;

		// define header end tag based on file version
		std::vector<std::string> lastLineTokens;
		if (m_majorFileVersion < 6) {
			lastLineTokens.push_back("ELEMENTS");
			lastLineTokens.push_back("SIDES");
		}
		else {
			lastLineTokens.push_back("INDICES");
		}

		long long streamPosition = IBK::FileReader::readAll(fname, headerLines, lastLineTokens, 0, notify);
		if( streamPosition == -1) {
			throw IBK::Exception("Error reading geometry file.", FUNC_ID);
		}

#ifdef SPEED_RATING_MESSAGES
		std::cout << "Time for reading of header: " << w.diff_str() << std::endl;
#endif

		// store current stream position as starting position for data section
		m_dataSectionOffset = (unsigned int)streamPosition;

		// parse header
		unsigned int lineNumber = 0;
		// this iterator will store the first token of the geometry section for Delphin 5 files
		std::vector<std::string>::iterator geometrySectionStart = headerLines.end();

		// loop over all lines
		for (std::vector<std::string>::iterator lineIt=headerLines.begin();
			 lineIt != headerLines.end(); ++lineIt, ++lineNumber)
		{
			// strip # from line
			std::string line = IBK::trim_copy(*lineIt, "\t #\n\r");
			if (line.empty()) continue; // empty line
			// split line into keyword and value
			std::string::size_type pos = line.find("=");
			// find empty lines after the =  must be more generic otherwise error / pos might be out of bounds
			// since pos + 2 is not defined
			if (pos != std::string::npos) {

				// continue on keyword witch carry no information
				std::string test = line.substr(pos+1, std::string::npos);
				if ( test.empty() )
					continue;

				if (line.find("TYPE") == 0) {
					m_type = string2type(line.substr(pos+2, std::string::npos));
				}
				else if (line.find("PROJECT_FILE") == 0) {
					m_projectFileName = line.substr(pos+2, std::string::npos);
				}
				else if (line.find("GEO_FILE_HASH") == 0) {
					std::string valueString = IBK::trim_copy(line.substr(pos+2, std::string::npos));
					if (valueString.empty())
						throw IBK::Exception(IBK::FormatString("Missing value for GEO_FILE_HASH keyword."), FUNC_ID);
					try {
						m_geoFileHash = IBK::string2val<unsigned int>(valueString);
					}
					catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString("Invalid value for GEO_FILE_HASH keyword."), FUNC_ID);
					}
				}
				else if (line.find("GEO_FILE") == 0) {
					m_geoFileName = line.substr(pos+2, std::string::npos);
				}
				else if (line.find("CREATED") == 0) {
					m_createdTimeString = line.substr(pos+2, std::string::npos);
				}
				// specific case must be before QUANTITY otherwise it doesn't work!
				else if (line.find("QUANTITY_KW") == 0) {
					m_quantityKeyword = line.substr(pos+2, std::string::npos);
				}
				else if (line.find("QUANTITY") == 0) {
					m_quantity = line.substr(pos+2, std::string::npos);
				}
				else if (line.find("SPACE_TYPE") == 0) {
					m_spaceType = string2spaceType(line.substr(pos+2, std::string::npos));
				}
				else if (line.find("TIME_TYPE") == 0) {
					m_timeType = string2timeType(line.substr(pos+2, std::string::npos));
				}
				else if (line.find("VALUE_UNIT") == 0) {
					m_valueUnit = line.substr(pos+2, std::string::npos);
				}
				else if (line.find("TIME_UNIT") == 0) {
					m_timeUnit = line.substr(pos+2, std::string::npos);
				}
				else if (line.find("START_YEAR") == 0) {
					std::string valueString = IBK::trim_copy(line.substr(pos+2, std::string::npos));
					if (valueString.empty())
						throw IBK::Exception(IBK::FormatString("Missing value for START_YEAR keyword."), FUNC_ID);
					try {
						m_startYear = IBK::string2val<int>(valueString);
					}
					catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString("Invalid value for START_YEAR keyword."), FUNC_ID);
					}
				}
				else if (line.find("ELEMENTS") == 0 || line.find("SIDES") == 0 || line.find("INDICES") == 0) {
					std::string valueString = IBK::trim_copy(line.substr(pos+2, std::string::npos));
					if (valueString.empty())
						throw IBK::Exception(IBK::FormatString("Missing value for ELEMENTS|SIDES|INDICES keyword."), FUNC_ID);
					try {
						IBK::string2vector<unsigned int>(valueString, m_nums);
					}
					catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString("Invalid value for ELEMENTS|SIDES|INDICES keyword."), FUNC_ID);
					}
					break; // stop parsing after this line
				}
			}
			// not a keyword = value line
			else {
				if (line.find("TABLE") == 0) {
					// this can only happen for Delphin 5 files with embedded geometry section
					// remember iterator for this section
					geometrySectionStart = lineIt;
					break; // stop parsing here
				}
			}
		}
		// for DELPHIN 5 data, we need to read the last line to extract the nums vector
		if (m_majorFileVersion < 6) {
			std::string line = headerLines.back();
			std::string::size_type pos = line.find("=");
			if (pos == std::string::npos)
				throw IBK::Exception("Missing = character in last line of geometry header.", FUNC_ID);
			IBK::string2vector<unsigned int>(line.substr(pos+2, std::string::npos), m_nums);

			// strip header from geometry section data
			headerLines.erase(headerLines.begin(), geometrySectionStart);
			// remove last line with the ELEMENTS/SIDES/INDICES keyword (erase might raise a there is no backiterator assertion)
			if (headerLines.size() > 0)
				headerLines.pop_back();

			// read geometry section from strings
			w.start();
			m_geometryData = new GeoFile;
			m_geometryData->m_majorFileVersion = 5; // we are reading embedded geometry data, so it must be in Delphin 5 format.

			try {
				m_geometryData->parseGeometryData(headerLines);
				m_geometryData->m_constructionLines.generate(*m_geometryData);
			}
			catch (IBK::Exception &ex) {
				if (notify) notify->notify(1);
				throw IBK::Exception(ex, "Error reading geometry section.", FUNC_ID);
			}

	#ifdef SPEED_RATING_MESSAGES
			std::cout << "Time for parsing geometry file: " << w.diff_str() << std::endl;
	#endif
		}

		// done reading header, compute m_nValues
		if (m_spaceType == ST_SINGLE)
			m_nValues = (unsigned int)m_nums.size();
		else {
			if (m_type == DataIO::T_REFERENCE)
				m_nValues = (unsigned int)m_nums.size();
			else
				m_nValues = 1;
		}

		// set data format
		if (m_nValues == 1 || m_type == T_REFERENCE)
			// for 2D and Reference type files
			m_dataFormat = DF_2D;
		else if (m_geometryData != nullptr) {
			// and for D5 outputs with embedded geometry data
			try {
				determineDataFormat(*m_geometryData);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, "Error in DataIO or embedded geometry data.", FUNC_ID);
			}
		}

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error on read binary data.", FUNC_ID);
	}
}
// ----------------------------------------------------------------------------


void DataIO::readASCIIData(const IBK::Path & fname, IBK::NotificationHandler * notify) {
	FUNCID(DataIO::readASCIIData);

	try {
		// cache time unit
		m_cachedTimeUnit.set(m_timeUnit); // this may throw an exception, if m_timeUnit is invalid

#ifdef SPEED_RATING_MESSAGES
		IBK::StopWatch w;
#endif

		FileReaderDataProcessorAddData dataProcessor(this);
		long long pos = IBK::FileReader::readAll(fname, &dataProcessor, std::vector<std::string>(), m_dataSectionOffset, notify);

#ifdef SPEED_RATING_MESSAGES
		std::cout << "Time for reading and extracting data: " << w.diff_str() << std::endl;
#endif

		if (pos == -1) {
			throw IBK::Exception("Error reading file with FileReader.", FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error reading ASCII data.", FUNC_ID);
	}
}
// ----------------------------------------------------------------------------


void DataIO::addValueLine(const std::string & line) {
	FUNCID(DataIO::addValueLine);
	// skip empty lines

	if (line.find_first_not_of(" \t") == std::string::npos)
		return;
	// if we get here, we are reading the value section already
	// we need to extract the first number
	bool number = false;
	unsigned int j=0;
	for (; j<line.size(); ++j) {
		if (line[j]==' ' || line[j] == '\t') {
			if (number)
				break;
		}
		else {
			number = true;
		}
	}
	std::stringstream tp_strm(line.substr(0,j));
	double tp;
	if (!(tp_strm >> tp))
		throw IBK::Exception( IBK::FormatString("Error parsing time point from line '%1', using substr '%2'.").arg(line).arg(line.substr(0,j)), FUNC_ID);
	// convert output time into correct output unit
	IBK::UnitList::instance().convert( m_cachedTimeUnit, IBK::Unit(IBK_UNIT_ID_SECONDS), tp);

	m_timepoints.push_back(tp);

	m_valueStrings.push_back(std::string(line.c_str() + j));
	m_values.push_back(std::vector<double>());

	// Note: the parsing of the string is delayed until access
}
// ----------------------------------------------------------------------------


void DataIO::checkDataConsistency() const {
	FUNCID(DataIO::checkDataConsistency);
	// check data
	if (m_timepoints.size() != m_values.size())
		throw IBK::Exception( IBK::FormatString("Time points and value vector have different sizes: %1 <-> %2 elements.")
				.arg((int) m_timepoints.size()).arg((int) m_values.size()), FUNC_ID);

	for (unsigned int i=0; i<m_timepoints.size(); ++i) {
		if (m_values[i].size() != m_nValues) {
			throw IBK::Exception( IBK::FormatString("Vector of values for time index %1 does not hold the right amount of values: "
									   "has %2, should be %3.").arg(i).arg((int) m_values[i].size()).arg(m_nValues), FUNC_ID);
		}
	}
}
// ----------------------------------------------------------------------------

std::vector<double> DataIO::columnValues(size_t colIDx) const {
	FUNCID(DataIO::columnValues);

	// index check, first ensure that we have at least one value vector
	if (m_values.empty())
		return std::vector<double>(); // no values, no column values

	if (colIDx >= m_values[0].size())
		throw IBK::Exception(IBK::FormatString("Index %1 out of range (data set has %2 columns).").arg(colIDx).arg(m_values[0].size()), FUNC_ID);

	// create vector with correct size
	std::vector<double> values(m_timepoints.size());
	// copy data
	for (size_t i=0; i<m_timepoints.size(); ++i)
		values[i] = m_values[i][colIDx];

	return values;
}

// ----------------------------------------------------------------------------

} // namespace DATAIO

