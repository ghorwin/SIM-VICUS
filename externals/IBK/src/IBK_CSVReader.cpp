/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

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


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_CSVReader.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iterator>

#include "IBK_configuration.h"
#include "IBK_messages.h"
#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"

using namespace std;

namespace IBK {

bool CSVReader::haveTabSeparationChar(const IBK::Path & filename) {
	const char * const FUNC_ID = "[CSVReader::haveTabSeparationChar]";
	// first detect file format
	bool tabFormat = true;
#if defined(_WIN32)
	#if defined(_MSC_VER)
			std::ifstream in(filename.wstr().c_str());
	#else
			std::string filenameAnsi = IBK::WstringToANSI(filename.wstr(), false);
			std::ifstream in(filenameAnsi.c_str());
	#endif
#else // _WIN32
		std::ifstream in(filename.c_str());
#endif
	if (!in)
		throw IBK::Exception( IBK::FormatString("File doesn't exist or cannot open/access file."), FUNC_ID);

	// now try to read at least two lines of data
	std::string line1, line2;
	if (!std::getline(in, line1) || !std::getline(in,line2))
		throw IBK::Exception( IBK::FormatString("Error reading first two lines of data file."), FUNC_ID);
	// try to split both files at tab characters
	std::vector<std::string> tokens1, tokens2;
	IBK::explode(line1, tokens1, "\t", IBK::EF_NoFlags);
	IBK::explode(line2, tokens2, "\t", IBK::EF_NoFlags);
	if (tokens1.size() < 2 || tokens1.size() != tokens2.size()) {
		// try tab format
		IBK::explode(line1, tokens1, ",", IBK::EF_UseQuotes);
		IBK::explode(line2, tokens2, ",", IBK::EF_UseQuotes);
		// still no luck?
		if (tokens1.size() < 2 || tokens1.size() != tokens2.size()) {
			throw IBK::Exception( IBK::FormatString("Unable to determine csv flavour - inconsistent delimiters or missing/wrong quotation charactors (or only one column?)."), FUNC_ID);
		}
		tabFormat = false;
	}

	return tabFormat;
}


void CSVReader::read(const IBK::Path & filename, bool headerOnly, bool extractUnits) {
	const char * const FUNC_ID = "[CSVReader::read]";
	try {
#if defined(_WIN32)
	#if defined(_MSC_VER)
			std::ifstream in(filename.wstr().c_str());
	#else
			std::string filenameAnsi = IBK::WstringToANSI(filename.wstr(), false);
			std::ifstream in(filenameAnsi.c_str());
	#endif
#else // _WIN32
			std::ifstream in(filename.c_str());
#endif
		if (!in)
			throw IBK::Exception( IBK::FormatString("File doesn't exist or cannot open/access file."), FUNC_ID);

		std::string line;
		std::getline(in, line);
		std::string sepChars;
		sepChars.push_back(m_separationCharacter);
		if (m_separationCharacter == ',')
			IBK::explode(line, m_captions, sepChars, IBK::EF_UseQuotes);
		else
			IBK::explode(line, m_captions, sepChars, IBK::EF_NoFlags);
		m_nColumns = (unsigned int)m_captions.size();
		m_nRows = 0;
		m_units.clear();
		if (extractUnits) {
			for (unsigned int i=0; i<m_captions.size(); ++i) {
				const std::string & c = m_captions[i];
				std::size_t pos = c.find("[");
				std::size_t pos2 = c.find("]");
				if (pos != std::string::npos && pos != std::string::npos && pos < pos2) {
					m_units.push_back(c.substr(pos+1, pos2-pos-1));
					m_captions[i] = c.substr(0, pos);
					IBK::trim(m_captions[i], " \t\r\"");
				}
				else
					m_units.push_back(""); // no unit
			}
		}
		if (headerOnly)
			return;
		while (std::getline(in, line)) {
			++m_nRows; // also count empty rows, to get correct line numbers in error messages
			// skip empty rows
			if (line.empty() || line.find_first_not_of("\n\r\t ") == std::string::npos)
				continue;
			std::vector<std::string> tokens;
			if (m_separationCharacter == ',') {
				IBK::explode(line, tokens, sepChars, IBK::EF_UseQuotes);
				for (unsigned int i=0; i<tokens.size(); ++i) {
					IBK::trim(tokens[i], " \t\r\"");
				}
			}
			else
				IBK::explode(line, tokens, sepChars, IBK::EF_NoFlags);
			// error: wrong column size
			if(tokens.size() != m_nColumns) {
				throw IBK::Exception(IBK::FormatString("Wrong number of columns in line #%1!")
										.arg(m_nRows+1), FUNC_ID);
			}
			std::vector<double> values(m_nColumns);
			for (unsigned int i=0; i<m_nColumns; ++i) {
				try {
					values[i] = IBK::string2val<double>(tokens[i]);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString("Error reading value in column %1 in line #%2.")
										  .arg(i).arg(m_nRows+1), FUNC_ID);
				}
			}
			m_values.push_back(values);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading file '%1'.").arg(filename), FUNC_ID);
	}
	// store final number of rows
	m_nRows = m_values.size();
}
// ----------------------------------------------------------------------------


std::vector<double> CSVReader::colData(unsigned int colIndex) const {
	const char * const FUNC_ID = "[CSVReader::colData]";
	if (m_values.empty())
		return std::vector<double>();
	if (colIndex >= m_values[0].size())
		throw IBK::Exception(IBK::FormatString("Column index %1 out of range. Only have %2 columns in file.").arg(colIndex).arg(m_values[0].size()), FUNC_ID);
	std::vector<double> res;
	for (unsigned int j=0; j<m_values.size(); ++j) {
		if (colIndex >= m_values[j].size())
			throw IBK::Exception(IBK::FormatString("Column index %1 out of range in row %2 (only %3 columns in this row).").arg(colIndex).arg(j).arg(m_values[j].size()), FUNC_ID);
		res.push_back(m_values[j][colIndex]);
	}
	return res;
}
// ----------------------------------------------------------------------------


} // namespace IBK
