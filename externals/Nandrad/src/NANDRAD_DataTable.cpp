/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_DataTable.h"

#include <algorithm>

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

namespace NANDRAD {


void DataTable::setEncodedString(const std::string & str) {
	FUNCID(DataTable::setEncodedString);
	std::vector<std::string> tokens;
	IBK::explode(str, tokens, ';', IBK::EF_TrimTokens);
	m_values.clear();
	int count = -1;
	for (std::string & colStr : tokens) {
		// tokens might have leading/trailing \n, so we execute a trim again
		IBK::trim(colStr, " \t\r\n");
		std::vector<std::string> parts;
		IBK::explode_in2(colStr, parts, ':');
		if (parts.size() != 2)
			throw IBK::Exception(IBK::FormatString("Invalid data in table, expected ':' as separator in column data '%1'").arg(colStr), FUNC_ID);
		std::vector<double> val;
		std::string line_with_spaces = parts[1];
		line_with_spaces = IBK::replace_string(parts[1], ",", " ");
		try {
			IBK::string2valueVector(line_with_spaces, val);
		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Invalid data in table, in column data '%1'").arg(colStr), FUNC_ID);
		}
		// check for correct column count
		if (count == -1)
			count = val.size();
		else if (count != (int)val.size())
			throw IBK::Exception(IBK::FormatString("Mismatching number of values (=%1) in column '%2' compared to previous columns.").arg(val.size()).arg(parts[0]), FUNC_ID);
		// check that column name isn't duplicated
		if (m_values.find(parts[0]) != m_values.end())
			throw IBK::Exception(IBK::FormatString("Duplicate column ID '%1' in table.").arg(parts[0]), FUNC_ID);
		m_values[parts[0]] = val;
	}
}


std::string DataTable::encodedString() const {
	// encode data table
	std::stringstream strm;
	for (const std::pair<const std::string, std::vector<double> > & column : m_values) {
		strm << IBK::trim_copy(column.first) << ":";
		for (unsigned int i=0; i<column.second.size(); ++i) {
			strm << column.second[i];
			if (i < column.second.size()-1)
				strm << ",";
		}

		strm << ";";
	}
	return strm.str();
}


const std::vector<double> & DataTable::valueVector(const std::string & parameterName) const {
	FUNCID(DataTable::valueVector);
	std::map<std::string, std::vector<double> >::const_iterator it = m_values.find(parameterName);
	if (it == m_values.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined parameter name '%1'.").arg(parameterName), FUNC_ID);
	return it->second;
}


} // namespace NANDRAD
