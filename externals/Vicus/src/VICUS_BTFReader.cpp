#include "VICUS_BTFReader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <utility>

#include <IBK_UnitVector.h>
#include <IBK_Exception.h>
#include <IBK_assert.h>
#include <IBK_messages.h>
#include <IBK_InputOutput.h>

#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QList>
#include <QCoreApplication>

#include <DATAIO_DataIO.h>
#include <DATAIO_GeoFile.h>


namespace VICUS {

static const int MAX_CHARS = 500000;


// *** Definition of FileReaderBTF class methods ***

BTFReader::BTFReader()
{
}


void BTFReader::parseHeaderData(const QString & fname, std::vector<std::string> & captions, std::vector<std::string> & valueUnits) {

	// assume tab-separated-file, read first line and explode tokens

	// ok, may be our file, start analysing filename in more detail
	// read first line in file
	IBK::Path filePath(fname.toStdString());
	// open file in binary mode and check if file exists
	std::ifstream in;
	IBK::open_ifstream(in, filePath); // TODO : Error handling

	// accepts file already reads everything until and including header line, so that
	// should be safe
	char magicHeaderBuf[9];
	in.read(magicHeaderBuf, 8);
	unsigned int startYear;
	IBK::read_uint32_binary(in, startYear);
	m_startYear.set((int)startYear, 0);
	std::string headerLine;
	IBK::read_string_binary(in, headerLine, MAX_CHARS); // 500000 chars for header should be enough

	// extract columns
	std::vector<std::string> columnHeaders;
	IBK::explode(headerLine, columnHeaders, '\t', true);

	// may throws exception
	IBK::Unit timeUnit;
	extractUnits(columnHeaders, timeUnit, captions, valueUnits);
}


void BTFReader::readData(const QString & fname, IBK::UnitVector &timePoints, std::vector<std::vector<double> > & values,
						 std::vector<std::string> & captions, std::vector<std::string> & valueUnits)
{
	FUNCID(BTFReader::readData);

	std::ifstream in;
	IBK::open_ifstream(in, IBK::Path(fname.toStdString())); // TODO : Error handling

	// accepts file already reads everything until and including header line, so that
	// should be safe
	char magicHeaderBuf[9];
	in.read(magicHeaderBuf, 8);
	unsigned int startYear;
	IBK::read_uint32_binary(in, startYear);
	m_startYear.set((int)startYear, 0);
	std::string headerLine;
	IBK::read_string_binary(in, headerLine, MAX_CHARS); // 50000 chars for header should be enough

	// extract columns
	std::vector<std::string> columnHeaders;
	IBK::explode(headerLine, columnHeaders, '\t', true);

	// extract time unit and mapping
	IBK::Unit timeUnit;
	try {
		extractUnits(columnHeaders, timeUnit, captions, valueUnits);
	}  catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Could not extract units from file '%1'.").arg(fname.toStdString()), FUNC_ID);
	}

	// init values vector
	unsigned int nCols = captions.size();
	values = std::vector<std::vector<double>>(nCols, std::vector<double>());

	// read all data from file
	timePoints.m_unit = timeUnit;
	std::vector<double> rowVals;
	while (in.peek()!=EOF) {
		try {
			IBK::read_vector_binary(in, rowVals, MAX_CHARS);
		} catch (...) {
			break; // so far we cannot distingish between incomplete lines, binary data mismatch, ... just stop reading here
		}

		if (rowVals.size() != nCols + 1)  {
			throw IBK::Exception( IBK::FormatString("Expected '%1' columns in data section of btf-file '%2'. "
												"However, one or more record contain a different number of values. Looks like an invalid or incomplete file.")
							  .arg(nCols+1).arg(fname.toStdString()), FUNC_ID);
		}

		// add to time column
		timePoints.m_data.push_back( rowVals[0] ); // Note: unit conversion is done at the end

		// read to column major vector
		for (unsigned int i=0; i<nCols; ++i)
			values[i].push_back( rowVals[i+1] );

	}// while

	// time unit conversion
	timePoints.convert(IBK::Unit("s"));

}



// *** private functions ***

void BTFReader::extractUnits(const std::vector<std::string> & columnHeaders,
								 IBK::Unit & timeUnit,
								std::vector<std::string> & captions,
								 std::vector<std::string> & valueUnits)
{
	const char * const FUNC_ID = "[FileReaderBTF::extractUnits]";

	// check that first column is a time column
	std::string columnHeader = columnHeaders[0];
	std::string::size_type openBracketPos = columnHeader.rfind("[");
	std::string::size_type closeBracketPos = columnHeader.rfind("]");
	if (openBracketPos != std::string::npos && closeBracketPos != std::string::npos && openBracketPos < closeBracketPos) {
		std::string ustr = columnHeader.substr(openBracketPos+1, closeBracketPos-openBracketPos-1);
		try {
			timeUnit = IBK::Unit(ustr);
			if (timeUnit.base_id() != IBK::Unit("s").base_id()) {
				throw IBK::Exception("First column must have a time unit in header.", FUNC_ID);
			}
		}
		catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, "First column has an invalid unit in header.", FUNC_ID);
		}
	}

	valueUnits.clear();
	captions.clear();

	// extract unit, but skip first column
	for (unsigned int i=1; i<columnHeaders.size(); ++i) {
		// extract unit
		std::string columnHeader = columnHeaders[i];
		std::string::size_type openBracketPos = columnHeader.rfind("[");
		std::string::size_type closeBracketPos = columnHeader.rfind("]");
		if (openBracketPos != std::string::npos && closeBracketPos != std::string::npos && openBracketPos < closeBracketPos) {
			std::string ustr = columnHeader.substr(openBracketPos+1, closeBracketPos-openBracketPos-1);
			std::string description = IBK::trim_copy(columnHeader.substr(0, openBracketPos));
			captions.push_back(description);
			valueUnits.push_back(ustr);
		}
		else {
			throw IBK::Exception(IBK::FormatString("Invalid format of header in column %1.").arg(i), FUNC_ID);
		}
	}
}



} // namespace VICUS
